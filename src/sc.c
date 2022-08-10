#include "sc/sc.h"
#include "backend.h"
#include "backend_uv.h"
#include "msg.h"
#include "sc/errcode.h"
#include "warn.h"

void sc_init(enum sc_backend backend, void *backend_data,
             sc_msg_alloc_fn_t *rec_alloc, sc_msg_free_fn_t *rec_free)
{
    if (backend == SC_LIBUV)
        backend_init(backend_uv_init(backend_data));
    else
        sc_warn("invalid backend");

    sc_msg_init_allocator(rec_alloc, rec_free);
}

char const *sc_strerror(int errcode)
{
    switch (errcode)
    {
    case SC_ENOMEM:
        return "not enough memory";
    case SC_EURIPARSE:
        return "invalid URI";
    case SC_EINVPROTO:
        return "invalid protocol";
    }
    return (*backend->strerror)(errcode);
}
