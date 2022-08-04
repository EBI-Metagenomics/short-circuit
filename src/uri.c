#include "uri.h"
#include "ctb/ctb.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct uri
{
    char *str;
    struct
    {
        char *lib;
        char *proto;
    } scheme;
    union
    {
        struct
        {
            char *filepath;
        } pipe;
        struct
        {
            char *ip4;
            char *port;
        } tcp;
    };
};

static bool check_ip4(char const *src);
static bool check_port(char const *port);

struct uri *uri_new(char const *instr)
{
    struct uri *uri = malloc(sizeof(*uri));
    if (!uri) return 0;

    uri->str = ctb_strdup(instr);
    if (!uri->str)
    {
        free(uri);
        return 0;
    }
    size_t urimax = strlen(uri->str) + 1;
    char *path = 0;
    char *scheme = ctb_strtok_s(uri->str, &urimax, ":", &path);
    if (!path) goto cleanup;

    size_t schememax = strlen(scheme) + 1;
    char *scheme_ntok = 0;
    uri->scheme.lib = ctb_strtok_s(scheme, &schememax, "+", &scheme_ntok);
    if (!uri->scheme.lib || strcmp(uri->scheme.lib, "sc")) goto cleanup;
    if (!scheme_ntok) goto cleanup;

    uri->scheme.proto = ctb_strtok_s(0, &schememax, "+", &scheme_ntok);
    if (!uri->scheme.proto ||
        (strcmp(uri->scheme.proto, "pipe") & strcmp(uri->scheme.proto, "tcp")))
        goto cleanup;

    if (!strcmp(uri->scheme.proto, "pipe"))
        uri->pipe.filepath = path;
    else if (!strcmp(uri->scheme.proto, "tcp"))
    {
        size_t pathmax = strlen(path) + 1;
        uri->tcp.port = 0;
        uri->tcp.ip4 = ctb_strtok_s(path, &pathmax, ":", &uri->tcp.port);
        if (!check_ip4(uri->tcp.ip4)) goto cleanup;
        if (!uri->tcp.port || !check_port(uri->tcp.port)) goto cleanup;
    }
    else
        goto cleanup;

    return uri;

cleanup:
    free(uri->str);
    free(uri);
    return 0;
}

void uri_del(struct uri const *uri)
{
    free(uri->str);
    free((void *)uri);
}

enum uri_scheme uri_scheme(struct uri const *uri)
{
    if (!strcmp(uri->scheme.proto, "pipe")) return SC_PIPE;
    if (!strcmp(uri->scheme.proto, "tcp")) return SC_TCP;
    assert(0);
}

char const *uri_pipe_filepath(struct uri const *uri)
{
    return uri->pipe.filepath;
}

char const *uri_tcp_ip4(struct uri const *uri) { return uri->tcp.ip4; }

unsigned uri_tcp_port(struct uri const *uri)
{
    return uri->tcp.port ? (unsigned)strtol(uri->tcp.port, 0, 10) : 0;
}

#ifndef NS_INADDRSZ
#define NS_INADDRSZ 4
#endif

/* Acknowledge: inet_pton4 implemented by Paul Vixie, 1996. */
static bool check_ip4(char const *src)
{
    static const char digits[] = "0123456789";
    int ch = 0;
    unsigned char tmp[NS_INADDRSZ] = {0};
    unsigned char *tp = 0;

    int saw_digit = 0;
    int octets = 0;
    *(tp = tmp) = 0;
    while ((ch = *src++) != '\0')
    {
        const char *pch;

        if ((pch = strchr(digits, ch)))
        {
            unsigned int new = *tp * 10 + (pch - digits);

            if (new > 255) return false;
            *tp = new;
            if (!saw_digit)
            {
                if (++octets > 4) return false;
                saw_digit = 1;
            }
        }
        else if (ch == '.' && saw_digit)
        {
            if (octets == 4) return false;
            *++tp = 0;
            saw_digit = 0;
        }
        else
            return false;
    }
    if (octets < 4) return false;
    return true;
}

static bool check_port(char const *port)
{
    char *end = 0;
    long num = strtol(port, &end, 10);
    if (port == end) return false;
    if (num < 1 || num > 65535) return false;
    return true;
}
