#include "sc/socket.h"
#include "backend.h"
#include <stdlib.h>

struct sc_socket
{
    void *data;
};

struct sc_socket *sc_socket_new(struct sc_watcher *watcher)
{
    struct sc_socket *socket = malloc(sizeof(struct sc_socket));
    if (!socket) return 0;

    if (!(socket->data = backend->alloc()))
    {
        backend->free(socket->data);
        return 0;
    }
    backend->init(socket->data, watcher);

    return socket;
}

void sc_socket_del(struct sc_socket *socket)
{
    return backend->free(socket->data);
}

int sc_socket_connect(struct sc_socket *socket, char const *addr)
{
    return backend->connect(socket->data, addr);
}

int sc_socket_accept(struct sc_socket *server, struct sc_socket *client)
{
    return backend->accept(server->data, client->data);
}

int sc_socket_bind(struct sc_socket *socket, char const *addr)
{
    return backend->bind(socket->data, addr);
}

int sc_socket_listen(struct sc_socket *socket, int backlog)
{
    return backend->listen(socket->data, backlog);
}

int sc_socket_send(struct sc_socket *socket, struct sc_record const *record)
{
    return backend->send(socket->data, record);
}

int sc_socket_close(struct sc_socket *socket)
{
    return backend->close(socket->data);
}
