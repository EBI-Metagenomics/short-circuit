#include "watcher.h"
#include "warn.h"

static void on_connect_noop(struct sc_socket *socket, int errcode)
{
    (void)socket;
    (void)errcode;
    warn("calling on_connect_noop");
}

static void on_connection_noop(struct sc_socket *socket, int errcode)
{
    (void)socket;
    (void)errcode;
    warn("on_connection_noop");
}

static void on_accept_noop(struct sc_socket *socket, int errcode)
{
    (void)socket;
    (void)errcode;
    warn("on_accept_noop");
}

static void on_recv_noop(struct sc_socket *socket, struct sc_msg *msg,
                         int errcode)
{
    (void)socket;
    (void)msg;
    (void)errcode;
    warn("on_recv_noop");
}

static void on_send_noop(struct sc_socket *socket, int errcode)
{
    (void)socket;
    (void)errcode;
    warn("on_send_noop");
}

static void on_close_noop(struct sc_socket *socket)
{
    (void)socket;
    warn("on_close_noop");
}

void sc_watcher_init(struct sc_watcher *watcher, struct sc_socket *socket)
{
    watcher->socket = socket;
    watcher->on_connect = &on_connect_noop;
    watcher->on_connection = &on_connection_noop;
    watcher->on_accept = &on_accept_noop;
    watcher->on_recv = &on_recv_noop;
    watcher->on_send = &on_send_noop;
    watcher->on_close = &on_close_noop;
}
