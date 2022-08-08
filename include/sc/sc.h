#ifndef SC_SC_H
#define SC_SC_H

#include "sc/backend.h"
#include "sc/backend_uv.h"
#include "sc/msg.h"
#include "sc/socket.h"
#include "sc/watcher.h"

void sc_init(enum sc_backend, void *backend_data, sc_msg_alloc_fn_t *,
             sc_msg_free_fn_t *);

#endif
