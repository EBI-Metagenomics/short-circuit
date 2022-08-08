#ifndef SC_MSG_H
#define SC_MSG_H

#include <assert.h>
#include <stdint.h>

#define SC_MSG_SIZE_BYTES 4

struct sc_msg
{
    uint32_t size_be;
    unsigned char *data;
};

typedef struct sc_msg *(sc_msg_alloc_fn_t)(unsigned size);
typedef void sc_msg_free_fn_t(struct sc_msg *);

struct sc_msg *sc_msg_alloc(unsigned size);
void sc_msg_free(struct sc_msg *);

void sc_msg_set_size(struct sc_msg *, unsigned size);
unsigned sc_msg_get_size(struct sc_msg const *);

#endif
