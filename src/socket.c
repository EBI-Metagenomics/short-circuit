#include "sc/socket.h"
#include "backend.h"
#include "uri.h"
#include "watcher.h"
#include <assert.h>
#include <stdlib.h>

struct sc_socket
{
    void *userdata;
    void *backend;
    struct uri *uri;
    struct watcher watcher;
};

struct sc_socket *sc_socket_new(void)
{
    struct sc_socket *socket = malloc(sizeof(struct sc_socket));
    if (!socket) return 0;
    socket->userdata = 0;

    if (!(socket->backend = backend->alloc()))
    {
        backend->free(socket->backend);
        return 0;
    }
    sc_watcher_init(&socket->watcher, socket);
    backend->init(socket->backend, &socket->watcher);
    socket->uri = 0;

    return socket;
}

void sc_socket_del(struct sc_socket *socket)
{
    if (socket->uri) sc_uri_del(socket->uri);
    backend->free(socket->backend);
    free(socket);
}

int sc_socket_connect(struct sc_socket *socket, char const *uri)
{
    int errcode = 0;
    if (!(socket->uri = sc_uri_new(uri, &errcode))) return errcode;
    return backend->connect(socket->backend, socket->uri);
}

void sc_socket_accept(struct sc_socket *server, struct sc_socket *client)
{
    backend->accept(server->backend, client->backend);
}

int sc_socket_bind(struct sc_socket *socket, char const *uri)
{
    int errcode = 0;
    if (!(socket->uri = sc_uri_new(uri, &errcode))) return errcode;
    return backend->bind(socket->backend, socket->uri);
}

int sc_socket_listen(struct sc_socket *socket, int backlog)
{
    return backend->listen(socket->backend, backlog);
}

int sc_socket_send(struct sc_socket *socket, struct sc_msg const *msg)
{
    return backend->send(socket->backend, msg);
}

int sc_socket_close(struct sc_socket *socket)
{
    int errcode = backend->close(socket->backend);
    if (socket->uri)
    {
        sc_uri_del(socket->uri);
        socket->uri = 0;
    }
    return errcode;
}

void sc_socket_set_userdata(struct sc_socket *socket, void *userdata)
{
    socket->userdata = userdata;
}

void *sc_socket_get_userdata(struct sc_socket *socket)
{
    return socket->userdata;
}

void sc_socket_on_connect(struct sc_socket *socket, sc_on_connect_fn_t *cb)
{
    socket->watcher.on_connect = cb;
}

void sc_socket_on_connection(struct sc_socket *socket,
                             sc_on_connection_fn_t *cb)
{
    socket->watcher.on_connection = cb;
}

void sc_socket_on_accept(struct sc_socket *socket, sc_on_accept_fn_t *cb)
{
    socket->watcher.on_accept = cb;
}

void sc_socket_on_recv(struct sc_socket *socket, sc_on_recv_fn_t *cb)
{
    socket->watcher.on_recv = cb;
}

void sc_socket_on_send(struct sc_socket *socket, sc_on_send_fn_t *cb)
{
    socket->watcher.on_send = cb;
}

void sc_socket_on_close(struct sc_socket *socket, sc_on_close_fn_t *cb)
{
    socket->watcher.on_close = cb;
}
