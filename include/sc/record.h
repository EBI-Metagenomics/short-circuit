#ifndef SC_RECORD_H
#define SC_RECORD_H

#include <assert.h>
#include <stdint.h>

#define SC_RECORD_SIZE_BYTES 4

struct __attribute__((__packed__)) sc_record
{
    uint32_t size_bigend;
    unsigned char data[];
};

static_assert(SC_RECORD_SIZE_BYTES == sizeof(struct sc_record), "No padding.");

typedef struct sc_record *(sc_record_alloc_fn_t)(uint32_t size);
typedef void sc_record_free_fn_t(struct sc_record *);

void sc_record_set_size(struct sc_record *, unsigned size);
unsigned sc_record_size(struct sc_record const *);

#endif
