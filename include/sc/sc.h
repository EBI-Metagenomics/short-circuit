#ifndef SC_SC_H
#define SC_SC_H

#include "sc/backend.h"
#include "sc/backend_uv.h"
#include "sc/record.h"
#include "sc/socket.h"
#include "sc/watcher.h"

void sc_init(enum sc_backend, void *backend_data, sc_record_alloc_fn_t *,
             sc_record_free_fn_t *);

#endif
