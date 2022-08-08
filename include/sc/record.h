#ifndef SC_RECORD_H
#define SC_RECORD_H

#include <assert.h>
#include <stdint.h>

#define SC_RECORD_SIZE_BYTES 4

struct sc_record
{
    uint32_t size_be;
    unsigned char *data;
};

typedef struct sc_record *(sc_record_alloc_fn_t)(unsigned size);
typedef void sc_record_free_fn_t(struct sc_record *);

struct sc_record *sc_record_alloc(unsigned size);
void sc_record_free(struct sc_record *);

void sc_record_set_size(struct sc_record *, unsigned size);
unsigned sc_record_size(struct sc_record const *);

#endif
