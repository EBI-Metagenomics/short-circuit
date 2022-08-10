#ifndef BACKEND_H
#define BACKEND_H

struct sc_msg;
struct sc_watcher;
struct uri;

typedef void *(alloc_fn_t)(void);
typedef void init_fn_t(void *, struct sc_watcher *);
typedef void free_fn_t(void *);

typedef int connect_fn_t(void *, struct uri const *);
typedef void accept_fn_t(void *server, void *client);

typedef int bind_fn_t(void *, struct uri const *);
typedef int listen_fn_t(void *, int backlog);

typedef int send_fn_t(void *, struct sc_msg const *);

typedef int close_fn_t(void *);

struct backend
{
    alloc_fn_t *alloc;
    init_fn_t *init;
    free_fn_t *free;

    connect_fn_t *connect;
    accept_fn_t *accept;

    bind_fn_t *bind;
    listen_fn_t *listen;

    send_fn_t *send;

    close_fn_t *close;
};

extern struct backend const *backend;

void backend_init(struct backend const *be);

#endif
