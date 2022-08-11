#ifndef SC_SOCKET_H
#define SC_SOCKET_H

#include "sc/callbacks.h"
#include "sc/export.h"

struct sc_msg;
struct sc_socket;

SC_API struct sc_socket *sc_socket_new(void);
SC_API void sc_socket_del(struct sc_socket *);

SC_API int sc_socket_connect(struct sc_socket *, char const *uri);
SC_API void sc_socket_accept(struct sc_socket *server,
                             struct sc_socket *client);

SC_API int sc_socket_bind(struct sc_socket *, char const *uri);
SC_API int sc_socket_listen(struct sc_socket *, int backlog);

SC_API int sc_socket_send(struct sc_socket *, struct sc_msg const *);

SC_API int sc_socket_close(struct sc_socket *);

SC_API void sc_socket_set_userdata(struct sc_socket *, void *);
SC_API void *sc_socket_get_userdata(struct sc_socket *);

SC_API void sc_socket_on_connect(struct sc_socket *, sc_on_connect_fn_t *);
SC_API void sc_socket_on_connection(struct sc_socket *,
                                    sc_on_connection_fn_t *);
SC_API void sc_socket_on_accept(struct sc_socket *, sc_on_accept_fn_t *);
SC_API void sc_socket_on_recv(struct sc_socket *, sc_on_recv_fn_t *);
SC_API void sc_socket_on_send(struct sc_socket *, sc_on_send_fn_t *);
SC_API void sc_socket_on_close(struct sc_socket *, sc_on_close_fn_t *);

#endif
