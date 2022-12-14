#include "uri.h"
#include "ctb/ctb.h"
#include "sc/errcode.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct uri
{
    char *str;
    char const *proto;
    union
    {
        struct
        {
            char const *filepath;
        } pipe;
        struct
        {
            char const *ip4;
            char *port;
        } tcp;
    };
};

static bool check_ip4(char const *src);
static bool check_port(char const *port);
static bool is_protocol_known(char const *proto);

struct uri *sc_uri_new(char const *instr, int *errcode)
{
    *errcode = SC_EURIPARSE;
    struct uri *uri = malloc(sizeof(*uri));
    if (!uri)
    {
        *errcode = SC_ENOMEM;
        return 0;
    }

    uri->str = ctb_strdup(instr);
    if (!uri->str)
    {
        *errcode = SC_ENOMEM;
        free(uri);
        return 0;
    }

    size_t urimax = strlen(uri->str) + 1;
    char *path = 0;
    uri->proto = ctb_strtok_s(uri->str, &urimax, ":", &path);
    if (!path) goto cleanup;

    if (!is_protocol_known(uri->proto))
    {
        *errcode = SC_EINVPROTO;
        goto cleanup;
    }

    if (strlen(path) <= 2) goto cleanup;
    if (*(path++) != '/') goto cleanup;
    if (*(path++) != '/') goto cleanup;

    if (!strcmp(uri->proto, "pipe"))
    {
        uri->pipe.filepath = path;
    }
    else
    {
        size_t pathmax = strlen(path) + 1;
        uri->tcp.port = 0;
        uri->tcp.ip4 = ctb_strtok_s(path, &pathmax, ":", &uri->tcp.port);
        if (!check_ip4(uri->tcp.ip4)) goto cleanup;
        if (!uri->tcp.port || !check_port(uri->tcp.port)) goto cleanup;
    }

    return uri;

cleanup:
    free(uri->str);
    free(uri);
    return 0;
}

void sc_uri_del(struct uri const *uri)
{
    free(uri->str);
    free((void *)uri);
}

enum proto sc_uri_protocol(struct uri const *uri)
{
    assert(is_protocol_known(uri->proto));
    if (!strcmp(uri->proto, "pipe")) return PROTO_PIPE;
    if (!strcmp(uri->proto, "tcp")) return PROTO_TCP;
    return PROTO_NOSET;
}

char const *sc_uri_pipe_filepath(struct uri const *uri)
{
    return uri->pipe.filepath;
}

char const *sc_uri_tcp_ip4(struct uri const *uri) { return uri->tcp.ip4; }

unsigned sc_uri_tcp_port(struct uri const *uri)
{
    return (unsigned)strtol(uri->tcp.port, 0, 10);
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

static bool is_protocol_known(char const *proto)
{
    return !strcmp(proto, "pipe") || !strcmp(proto, "tcp");
}
