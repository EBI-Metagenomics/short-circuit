#ifndef SC_SOCKET_H
#define SC_SOCKET_H

struct sc_record;
struct sc_socket;
struct sc_watcher;

struct sc_socket *sc_socket_new(struct sc_watcher *);
void sc_socket_del(struct sc_socket *);

int sc_socket_connect(struct sc_socket *, char const *addr);
int sc_socket_accept(struct sc_socket *server, struct sc_socket *client);

int sc_socket_bind(struct sc_socket *, char const *addr);
int sc_socket_listen(struct sc_socket *, int backlog);

int sc_socket_send(struct sc_socket *, struct sc_record const *);

int sc_socket_close(struct sc_socket *);

#endif
