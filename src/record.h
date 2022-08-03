#ifndef RECORD_H
#define RECORD_H

#include "sc/record.h"

struct sc_record;

void record_init(struct sc_record *);
void record_init_allocator(sc_record_alloc_fn_t *, sc_record_free_fn_t *);

extern sc_record_alloc_fn_t *record_alloc;
extern sc_record_free_fn_t *record_free;

#endif
