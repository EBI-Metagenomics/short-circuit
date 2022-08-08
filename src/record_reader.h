#ifndef RECORD_READER_H
#define RECORD_READER_H

#include "record.h"
#include <stdbool.h>

struct record_reader
{
    struct sc_record *record;
    unsigned char *head_pos;
    unsigned char *body_pos;
};

void record_reader_init(struct record_reader *, struct sc_record *);

unsigned record_reader_parse_head(struct record_reader *reader, unsigned size);
unsigned record_reader_parse_body(struct record_reader *reader, unsigned size);

unsigned char *record_reader_head_pos(struct record_reader *reader);
unsigned char *record_reader_body_pos(struct record_reader *reader);

unsigned record_reader_avail_head_size(struct record_reader const *);
unsigned record_reader_avail_body_size(struct record_reader const *);

bool record_reader_finished(struct record_reader const *);

#endif
