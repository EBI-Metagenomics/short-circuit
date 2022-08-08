#include "record.h"
#include "ctb/ctb.h"
#include "sc/record.h"
#include <stdlib.h>

static struct sc_record *default_heap_alloc(unsigned size)
{
    void *ptr = malloc(sizeof(struct sc_record) + size);
    struct sc_record *record = ptr;
    record->data = ((unsigned char *)ptr) + sizeof(struct sc_record);
    return record;
}

static void default_heap_free(struct sc_record *record) { free(record); }

sc_record_alloc_fn_t *record_alloc = &default_heap_alloc;
sc_record_free_fn_t *record_free = &default_heap_free;

struct sc_record *sc_record_alloc(unsigned size)
{
    return (*record_alloc)(size);
}

void sc_record_free(struct sc_record *record) { (*record_free)(record); }

void sc_record_set_size(struct sc_record *record, unsigned size)
{
    record->size_be = ctb_htonl(size);
}

unsigned sc_record_size(struct sc_record const *record)
{
    return ctb_ntohl(record->size_be);
}

void record_init(struct sc_record *record)
{
    record->size_be = 0;
    record->data[0] = 0;
}

void record_init_allocator(sc_record_alloc_fn_t *rec_alloc,
                           sc_record_free_fn_t *rec_free)
{
    if (rec_alloc) record_alloc = rec_alloc;
    if (rec_free) record_free = rec_free;
}
