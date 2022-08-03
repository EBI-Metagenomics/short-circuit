#ifndef SC_WATCHER_H
#define SC_WATCHER_H

struct sc_record;
struct sc_watcher;

typedef void sc_watcher_on_connect_success_fn_t(struct sc_watcher *);
typedef void sc_watcher_on_connect_failure_fn_t(struct sc_watcher *);

typedef void sc_watcher_on_connection_success_fn_t(struct sc_watcher *);
typedef void sc_watcher_on_connection_failure_fn_t(struct sc_watcher *);

typedef void sc_watcher_on_accept_success_fn_t(struct sc_watcher *);
typedef void sc_watcher_on_accept_failure_fn_t(struct sc_watcher *);

typedef void sc_watcher_on_recv_success_fn_t(struct sc_watcher *,
                                             struct sc_record *);
typedef void sc_watcher_on_recv_failure_fn_t(struct sc_watcher *);
typedef void sc_watcher_on_recv_eof_fn_t(struct sc_watcher *);

typedef void sc_watcher_on_send_success_fn_t(struct sc_watcher *);
typedef void sc_watcher_on_send_failure_fn_t(struct sc_watcher *);
typedef void sc_watcher_on_send_eof_fn_t(struct sc_watcher *);

typedef void sc_watcher_on_close_fn_t(struct sc_watcher *);

struct sc_watcher
{
    void *data;

    sc_watcher_on_connect_success_fn_t *on_connect_success;
    sc_watcher_on_connect_failure_fn_t *on_connect_failure;

    sc_watcher_on_connection_success_fn_t *on_connection_success;
    sc_watcher_on_connection_failure_fn_t *on_connection_failure;

    sc_watcher_on_accept_success_fn_t *on_accept_success;
    sc_watcher_on_accept_failure_fn_t *on_accept_failure;

    sc_watcher_on_recv_success_fn_t *on_recv_success;
    sc_watcher_on_recv_failure_fn_t *on_recv_failure;
    sc_watcher_on_recv_eof_fn_t *on_recv_eof;

    sc_watcher_on_send_success_fn_t *on_send_success;
    sc_watcher_on_send_failure_fn_t *on_send_failure;
    sc_watcher_on_send_eof_fn_t *on_send_eof;

    sc_watcher_on_close_fn_t *on_close;
};

void sc_watcher_init(struct sc_watcher *);

#endif
