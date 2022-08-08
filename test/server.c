#include "fatal.h"
#include "sc/sc.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <uv.h>

struct server;

struct client
{
    struct server *server;
    struct sc_socket *socket;
    struct sc_watcher watcher;
    bool active;
};

enum terminate
{
    NOTSET = 0,
    CLIENT = 1,
    SERVER = 2,
};

struct server
{
    struct backend_uv_data uv;
    struct uv_async_s async;
    struct uv_signal_s sigterm;
    struct uv_signal_s sigint;
    struct uv_idle_s idle;

    struct sc_socket *socket;
    struct sc_watcher watcher;

    struct client client;
    enum terminate terminate;
};

static char const *uri = 0;
static FILE *output = 0;

static void outf(char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(output, fmt, args);
    fflush(output);
    va_end(args);
}

static void close_socket(struct sc_socket *socket)
{
    if (sc_socket_close(socket)) fatal("sc_socket_close");
}

static void print_record(struct sc_record const *record)
{
    char const *msg = (char const *)record->data;
    unsigned size = sc_record_size(record);
    outf("[%u](%.*s)\n", size, (int)size, msg);
}

static void server_on_connection_success(struct sc_watcher *w)
{
    struct server *server = w->data;
    if (server->terminate) return;

    if (sc_socket_accept(server->socket, server->client.socket))
        close_socket(server->client.socket);
}

static void server_on_recv_success(struct sc_watcher *w,
                                   struct sc_record *record)
{
    (void)w;
    print_record(record);
}

static void server_on_close(struct sc_watcher *w)
{
    struct server *server = w->data;
    uv_close((struct uv_handle_s *)&server->sigterm, 0);
    uv_close((struct uv_handle_s *)&server->sigint, 0);
}

static void client_on_accept_success(struct sc_watcher *w)
{
    struct client *client = w->data;
    struct server *server = client->server;
    client->active = true;
    if (server->terminate) close_socket(client->socket);
}

static void client_on_recv_success(struct sc_watcher *w,
                                   struct sc_record *record)
{
    print_record(record);
    struct client *client = w->data;
    struct server *server = client->server;
    if (server->terminate) close_socket(client->socket);
}

static void client_on_recv_eof(struct sc_watcher *w)
{
    struct client *client = w->data;
    close_socket(client->socket);
}

static void client_on_close(struct sc_watcher *w)
{
    struct client *client = w->data;
    client->active = false;
}

static struct sc_record *alloc_record(uint32_t size)
{
    struct sc_record *record = malloc(sizeof(struct sc_record));
    record->data = malloc(size);
    return record;
}

static void free_record(struct sc_record *record)
{
    free(record->data);
    free(record);
}

static void parse_args(int argc, char **argv)
{
    if (argc != 3) fatal("wrong number of arguments");
    uri = argv[1];
    if (!(output = fopen(argv[2], "w"))) fatal("failed to create output");
}

static void async_cb(struct uv_async_s *handle)
{
    struct server *server = handle->data;
    server->terminate = CLIENT;
    uv_close((struct uv_handle_s *)&server->async, 0);
}

static void signal_cb(struct uv_signal_s *handle, int signum)
{
    (void)signum;
    assert(signum == SIGTERM || signum == SIGINT);
    struct server *server = handle->data;
    if (server->terminate) return;
    if (uv_async_send(&server->async)) fatal("uv_async_send");
}

static void idle_cb(struct uv_idle_s *handle)
{
    struct server *server = handle->data;

    if (!server->client.active && server->terminate == CLIENT)
        server->terminate = SERVER;
    else if (server->terminate == SERVER)
    {
        uv_close((struct uv_handle_s *)&server->idle, 0);
        close_socket(server->socket);
    }
}

static void async_init(struct server *server)
{
    if (uv_async_init(server->uv.loop, &server->async, async_cb))
        fatal("uv_async_init");

    server->async.data = server;
}

static void signal_init(struct server *server, struct uv_signal_s *signal,
                        uv_signal_cb signal_cb, int signum)
{
    if (uv_signal_init(server->uv.loop, signal)) fatal("uv_signal_init");

    if (uv_signal_start(signal, signal_cb, signum)) fatal("uv_signal_start");

    signal->data = server;
}

static void idle_init(struct server *server)
{
    if (uv_idle_init(server->uv.loop, &server->idle)) fatal("uv_idle_init");

    if (uv_idle_start(&server->idle, idle_cb)) fatal("uv_idle_start");

    server->idle.data = server;
}

static void server_init(struct server *server)
{
    async_init(server);
    signal_init(server, &server->sigterm, signal_cb, SIGTERM);
    signal_init(server, &server->sigint, signal_cb, SIGINT);
    idle_init(server);

    sc_watcher_init(&server->watcher);
    server->watcher.data = server;
    server->watcher.on_connection_success = &server_on_connection_success;
    server->watcher.on_recv_success = &server_on_recv_success;
    server->watcher.on_close = &server_on_close;

    if (!(server->socket = sc_socket_new(&server->watcher)))
        fatal("sc_socket_new");

    server->terminate = NOTSET;
}

static void client_init(struct client *client, struct server *server)
{
    client->server = server;
    sc_watcher_init(&client->watcher);
    client->watcher.data = client;
    client->watcher.on_accept_success = &client_on_accept_success;
    client->watcher.on_recv_success = &client_on_recv_success;
    client->watcher.on_recv_eof = &client_on_recv_eof;
    client->watcher.on_close = &client_on_close;

    if (!(client->socket = sc_socket_new(&client->watcher)))
        fatal("sc_socket_new");

    client->active = false;
}

static struct server *server_new(struct uv_loop_s *loop)
{
    struct server *server = malloc(sizeof(*server));
    if (!server) fatal("not enough memory");
    server->uv.loop = loop;
    return server;
}

static void server_del(struct server *server)
{
    sc_socket_del(server->client.socket);
    sc_socket_del(server->socket);
    free(server);
}

static void server_bind_and_listen(struct server *server, char const *uri)
{
    if (sc_socket_bind(server->socket, uri)) fatal("sc_socket_bind");

    if (sc_socket_listen(server->socket, 4)) fatal("sc_socket_listen");
}

int main(int argc, char **argv)
{
    parse_args(argc, argv);

    struct server *server = server_new(uv_default_loop());

    sc_init(SC_UV, &server->uv, alloc_record, free_record);

    server_init(server);
    client_init(&server->client, server);

    server_bind_and_listen(server, uri);

    if (uv_run(server->uv.loop, UV_RUN_DEFAULT)) fatal("uv_run");
    if (uv_loop_close(server->uv.loop)) fatal("uv_loop_close");

    server_del(server);
    fclose(output);

    return 0;
}
