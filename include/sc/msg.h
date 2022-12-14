#ifndef SC_MSG_H
#define SC_MSG_H

#include "ctb/ctb.h"
#include "sc/export.h"

#define SC_MSG_SIZE_BYTES 4

struct sc_msg
{
    uint32_t size_be;
    unsigned char *data;
};

#define SC_MSG_INIT(sz, x)                                                     \
    (struct sc_msg) { .size_be = CTB_HTONL(sz), .data = (unsigned char *)(x) }

typedef struct sc_msg *(sc_msg_alloc_fn_t)(unsigned size);
typedef void sc_msg_free_fn_t(struct sc_msg *);

SC_API struct sc_msg *sc_msg_alloc(unsigned size);
SC_API void sc_msg_free(struct sc_msg *);

SC_API void sc_msg_set_size(struct sc_msg *, unsigned size);
SC_API unsigned sc_msg_get_size(struct sc_msg const *);

#endif
