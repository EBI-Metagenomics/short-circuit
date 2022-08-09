#include "sc/socket.h"
#include "backend.h"
#include "uri.h"
#include <assert.h>
#include <stdlib.h>

struct sc_socket
{
    void *data;
    struct uri *uri;
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
    socket->uri = 0;

    return socket;
}

void sc_socket_del(struct sc_socket *socket)
{
    if (socket->uri) uri_del(socket->uri);
    backend->free(socket->data);
    free(socket);
}

int sc_socket_connect(struct sc_socket *socket, char const *uri)
{
    assert(!socket->uri);
    if (!(socket->uri = uri_new(uri))) return 1;
    return backend->connect(socket->data, socket->uri);
}

int sc_socket_accept(struct sc_socket *server, struct sc_socket *client)
{
    return backend->accept(server->data, client->data);
}

int sc_socket_bind(struct sc_socket *socket, char const *uri)
{
    assert(!socket->uri);
    if (!(socket->uri = uri_new(uri))) return 1;
    return backend->bind(socket->data, socket->uri);
}

int sc_socket_listen(struct sc_socket *socket, int backlog)
{
    return backend->listen(socket->data, backlog);
}

int sc_socket_send(struct sc_socket *socket, struct sc_msg const *msg)
{
    return backend->send(socket->data, msg);
}

int sc_socket_close(struct sc_socket *socket)
{
    int rc = backend->close(socket->data);
    if (socket->uri)
    {
        uri_del(socket->uri);
        socket->uri = 0;
    }
    return rc;
}
