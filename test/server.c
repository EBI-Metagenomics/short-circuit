#include "fatal.h"
#include "sc/sc.h"
#include "uv.h"
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct server;

struct client
{
    struct server *server;
    struct sc_socket *socket;
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
    struct sc_backend_uv_data uv;
    struct uv_async_s async;
    struct uv_signal_s sigterm;
    struct uv_signal_s sigint;
    struct uv_idle_s idle;

    struct sc_socket *socket;

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

static void print_msg(struct sc_msg const *msg)
{
    char const *str = (char const *)msg->data;
    unsigned size = sc_msg_get_size(msg);
    outf("[%u](%.*s)\n", size, (int)size, str);
}

static void server_on_connection(struct sc_socket *socket, int errcode)
{
    if (errcode) fatal("errcode");
    struct server *server = sc_socket_get_userdata(socket);
    if (server->terminate) return;

    sc_socket_accept(socket, server->client.socket);
}

static void server_on_recv(struct sc_socket *socket, struct sc_msg *msg,
                           int errcode)
{
    (void)socket;
    if (errcode) fatal("errcode");
    print_msg(msg);
}

static void server_on_close(struct sc_socket *socket)
{
    struct server *server = sc_socket_get_userdata(socket);
    uv_close((struct uv_handle_s *)&server->sigterm, 0);
    uv_close((struct uv_handle_s *)&server->sigint, 0);
}

static void client_on_accept(struct sc_socket *socket, int errcode)
{
    if (errcode) fatal("errcode");
    struct client *client = sc_socket_get_userdata(socket);
    struct server *server = client->server;
    client->active = true;
    if (server->terminate) close_socket(client->socket);
}

static void client_on_recv(struct sc_socket *socket, struct sc_msg *msg,
                           int errcode)
{
    if (errcode) fatal("errcode");
    struct client *client = sc_socket_get_userdata(socket);
    if (!msg)
    {
        close_socket(client->socket);
        return;
    }
    print_msg(msg);
    struct server *server = client->server;
    if (server->terminate) close_socket(client->socket);
}

static void client_on_close(struct sc_socket *socket)
{
    struct client *client = sc_socket_get_userdata(socket);
    client->active = false;
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

    if (!(server->socket = sc_socket_new())) fatal("sc_socket_new");

    sc_socket_set_userdata(server->socket, server);
    sc_socket_on_connection(server->socket, &server_on_connection);
    sc_socket_on_recv(server->socket, &server_on_recv);
    sc_socket_on_close(server->socket, &server_on_close);
    server->terminate = NOTSET;
}

static void client_init(struct client *client, struct server *server)
{
    client->server = server;
    if (!(client->socket = sc_socket_new())) fatal("sc_socket_new");

    sc_socket_set_userdata(client->socket, client);
    sc_socket_on_accept(client->socket, &client_on_accept);
    sc_socket_on_recv(client->socket, &client_on_recv);
    sc_socket_on_close(client->socket, &client_on_close);

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

    sc_init(SC_LIBUV, &server->uv, 0, 0);

    server_init(server);
    client_init(&server->client, server);

    server_bind_and_listen(server, uri);

    if (uv_run(server->uv.loop, UV_RUN_DEFAULT)) fatal("uv_run");
    if (uv_loop_close(server->uv.loop)) fatal("uv_loop_close");

    server_del(server);
    fclose(output);

    return 0;
}
