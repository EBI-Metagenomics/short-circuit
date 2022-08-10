#ifndef SC_SOCKET_H
#define SC_SOCKET_H

#include "sc/callbacks.h"

struct sc_msg;
struct sc_socket;

struct sc_socket *sc_socket_new(void);
void sc_socket_del(struct sc_socket *);

int sc_socket_connect(struct sc_socket *, char const *uri);
void sc_socket_accept(struct sc_socket *server, struct sc_socket *client);

int sc_socket_bind(struct sc_socket *, char const *uri);
int sc_socket_listen(struct sc_socket *, int backlog);

int sc_socket_send(struct sc_socket *, struct sc_msg const *);

int sc_socket_close(struct sc_socket *);

void sc_socket_set_userdata(struct sc_socket *, void *);
void *sc_socket_get_userdata(struct sc_socket *);

void sc_socket_on_connect(struct sc_socket *, sc_on_connect_fn_t *);
void sc_socket_on_connection(struct sc_socket *, sc_on_connection_fn_t *);
void sc_socket_on_accept(struct sc_socket *, sc_on_accept_fn_t *);
void sc_socket_on_recv(struct sc_socket *, sc_on_recv_fn_t *);
void sc_socket_on_send(struct sc_socket *, sc_on_send_fn_t *);
void sc_socket_on_close(struct sc_socket *, sc_on_close_fn_t *);

#endif
