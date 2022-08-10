#include "fatal.h"
#include "sc/sc.h"
#include <uv.h>

static struct sc_socket *server = 0;
static struct sc_socket *client = 0;

static void close_socket(struct sc_socket *socket)
{
    int errcode = sc_socket_close(socket);
    if (errcode) fatal(sc_strerror(errcode));
}

static void echo(struct sc_msg const *msg)
{
    printf("%.*s\n", (int)sc_msg_get_size(msg), (char const *)msg->data);
}

static void on_connection(struct sc_socket *server, int errcode)
{
    if (errcode) fatal(sc_strerror(errcode));
    sc_socket_accept(server, client);
}

static void on_recv(struct sc_socket *client, struct sc_msg *msg, int errcode)
{
    if (errcode) fatal("errcode");
    if (!msg)
    {
        close_socket(client);
        close_socket(server);
        return;
    }
    echo(msg);
}

int main(void)
{
    struct sc_backend_uv_data uv = {.loop = uv_default_loop()};
    sc_init(SC_LIBUV, &uv, 0, 0);

    server = sc_socket_new();
    sc_socket_on_connection(server, &on_connection);

    sc_socket_bind(server, "sc+tcp://127.0.0.1:8777");
    sc_socket_listen(server, 1);

    client = sc_socket_new();
    sc_socket_on_recv(client, &on_recv);

    uv_run(uv.loop, UV_RUN_DEFAULT);

    sc_socket_del(server);
    sc_socket_del(client);
    return 0;
}
