#include "watcher.h"
#include "warn.h"

#define NOTSET_WARNING(CB) CB " callback called but not set"

static void on_connect_noop(struct sc_socket *socket, int errcode)
{
    (void)socket;
    (void)errcode;
    sc_warn(NOTSET_WARNING("on_connect"));
}

static void on_connection_noop(struct sc_socket *socket, int errcode)
{
    (void)socket;
    (void)errcode;
    sc_warn(NOTSET_WARNING("on_connection"));
}

static void on_accept_noop(struct sc_socket *socket, int errcode)
{
    (void)socket;
    (void)errcode;
    sc_warn(NOTSET_WARNING("on_accept"));
}

static void on_recv_noop(struct sc_socket *socket, struct sc_msg *msg,
                         int errcode)
{
    (void)socket;
    (void)msg;
    (void)errcode;
    sc_warn(NOTSET_WARNING("on_recv"));
}

static void on_send_noop(struct sc_socket *socket, int errcode)
{
    (void)socket;
    (void)errcode;
    sc_warn(NOTSET_WARNING("on_send"));
}

static void on_close_noop(struct sc_socket *socket)
{
    (void)socket;
    sc_warn(NOTSET_WARNING("on_close"));
}

void sc_watcher_init(struct watcher *watcher, struct sc_socket *socket)
{
    watcher->socket = socket;
    watcher->on_connect = &on_connect_noop;
    watcher->on_connection = &on_connection_noop;
    watcher->on_accept = &on_accept_noop;
    watcher->on_recv = &on_recv_noop;
    watcher->on_send = &on_send_noop;
    watcher->on_close = &on_close_noop;
}
