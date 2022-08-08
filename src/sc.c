#include "sc/sc.h"
#include "backend.h"
#include "backend_uv.h"
#include "die.h"
#include "msg.h"

void sc_init(enum sc_backend backend, void *backend_data,
             sc_msg_alloc_fn_t *rec_alloc, sc_msg_free_fn_t *rec_free)
{
    if (backend == SC_LIBUV)
        backend_init(backend_uv_init(backend_data));
    else
        die("invalid backend");

    msg_init_allocator(rec_alloc, rec_free);
}
