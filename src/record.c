#include "record.h"
#include "sc/record.h"

sc_record_alloc_fn_t *record_alloc = 0;
sc_record_free_fn_t *record_free = 0;

void record_init(struct sc_record *record)
{
    record->size = 0;
    record->data[0] = 0;
}

void record_init_allocator(sc_record_alloc_fn_t *rec_alloc,
                           sc_record_free_fn_t *rec_free)
{
    record_alloc = rec_alloc;
    record_free = rec_free;
}
