#include "sc/sc.h"
#include <uv.h>

static struct sc_msg hello = SC_MSG_INIT(5, "Hello");

static void on_connect(struct sc_socket *socket, int errcode)
{
    sc_socket_send(socket, &hello);
    (void)errcode;
}

int main(void)
{
    struct sc_backend_uv_data uv = {.loop = uv_default_loop()};
    sc_init(SC_LIBUV, &uv, 0, 0);

    struct sc_socket *socket = sc_socket_new();
    sc_socket_on_connect(socket, &on_connect);
    sc_socket_connect(socket, "sc+tcp://127.0.0.1:8765");

    uv_run(uv.loop, UV_RUN_DEFAULT);

    sc_socket_del(socket);
    return 0;
}
