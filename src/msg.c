#include "msg.h"
#include "ctb/ctb.h"
#include "sc/msg.h"
#include <stdlib.h>

static struct sc_msg *default_heap_alloc(unsigned size)
{
    void *ptr = malloc(sizeof(struct sc_msg) + size);
    struct sc_msg *msg = ptr;
    msg->data = ((unsigned char *)ptr) + sizeof(struct sc_msg);
    return msg;
}

static void default_heap_free(struct sc_msg *msg) { free(msg); }

sc_msg_alloc_fn_t *msg_alloc = &default_heap_alloc;
sc_msg_free_fn_t *msg_free = &default_heap_free;

struct sc_msg *sc_msg_alloc(unsigned size)
{
    return (*msg_alloc)(size);
}

void sc_msg_free(struct sc_msg *msg) { (*msg_free)(msg); }

void sc_msg_set_size(struct sc_msg *msg, unsigned size)
{
    msg->size_be = ctb_htonl(size);
}

unsigned sc_msg_get_size(struct sc_msg const *msg)
{
    return ctb_ntohl(msg->size_be);
}

void sc_msg_init(struct sc_msg *msg)
{
    msg->size_be = 0;
    msg->data[0] = 0;
}

void sc_msg_init_allocator(sc_msg_alloc_fn_t *rec_alloc,
                           sc_msg_free_fn_t *rec_free)
{
    if (rec_alloc) msg_alloc = rec_alloc;
    if (rec_free) msg_free = rec_free;
}
