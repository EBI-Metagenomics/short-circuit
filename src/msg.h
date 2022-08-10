#ifndef MSG_H
#define MSG_H

#include "sc/msg.h"

struct sc_msg;

void sc_msg_init(struct sc_msg *);
void sc_msg_init_allocator(sc_msg_alloc_fn_t *, sc_msg_free_fn_t *);

#endif
