#include "record.h"
#include "ctb/ctb.h"
#include "sc/record.h"

sc_record_alloc_fn_t *record_alloc = 0;
sc_record_free_fn_t *record_free = 0;

void record_init(struct sc_record *record)
{
    record->size_bigend = 0;
    record->data[0] = 0;
}

void record_init_allocator(sc_record_alloc_fn_t *rec_alloc,
                           sc_record_free_fn_t *rec_free)
{
    record_alloc = rec_alloc;
    record_free = rec_free;
}

void sc_record_set_size(struct sc_record *record, unsigned size)
{
    record->size_bigend = ctb_htonl(size);
}

unsigned sc_record_size(struct sc_record const *record)
{
    return ctb_ntohl(record->size_bigend);
}
