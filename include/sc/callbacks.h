#ifndef SC_CALLBACKS_H
#define SC_CALLBACKS_H

struct sc_msg;
struct sc_socket;

typedef void sc_on_connect_fn_t(struct sc_socket *, int errcode);
typedef void sc_on_connection_fn_t(struct sc_socket *, int errcode);
typedef void sc_on_accept_fn_t(struct sc_socket *, int errcode);
typedef void sc_on_recv_fn_t(struct sc_socket *, struct sc_msg *, int errcode);
typedef void sc_on_send_fn_t(struct sc_socket *, int errcode);
typedef void sc_on_close_fn_t(struct sc_socket *);

#endif
