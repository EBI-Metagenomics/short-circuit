#ifndef WATCHER_H
#define WATCHER_H

#include "sc/callbacks.h"

struct sc_socket;

struct sc_watcher
{
    struct sc_socket *socket;

    sc_on_connect_fn_t *on_connect;
    sc_on_connection_fn_t *on_connection;
    sc_on_accept_fn_t *on_accept;
    sc_on_recv_fn_t *on_recv;
    sc_on_send_fn_t *on_send;
    sc_on_close_fn_t *on_close;
};

void sc_watcher_init(struct sc_watcher *, struct sc_socket *);

#endif
