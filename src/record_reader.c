#include "record_reader.h"
#include "ctb/ctb.h"
#include "sc/record.h"
#include <assert.h>

void record_reader_init(struct record_reader *reader, struct sc_record *record)
{
    reader->record = record;
    reader->head_pos = (unsigned char *)&record->size_be;
    reader->body_pos = (unsigned char *)record->data;
}

static unsigned skip_pos(unsigned char **pos, unsigned avail, unsigned size)
{
    unsigned skip = avail < size ? avail : size;
    *pos += skip;
    return (unsigned)(size - skip);
}

unsigned record_reader_parse_head(struct record_reader *reader, unsigned size)
{
    unsigned avail = record_reader_avail_head_size(reader);
    return skip_pos(&reader->head_pos, avail, size);
}

unsigned record_reader_parse_body(struct record_reader *reader, unsigned size)
{
    unsigned avail = record_reader_avail_body_size(reader);
    return skip_pos(&reader->body_pos, avail, size);
}

unsigned char *record_reader_head_pos(struct record_reader *reader)
{
    return reader->head_pos;
}

unsigned char *record_reader_body_pos(struct record_reader *reader)
{
    return reader->body_pos;
}

unsigned record_reader_avail_head_size(struct record_reader const *reader)
{
    unsigned char const *start = (unsigned char *)&reader->record->size_be;
    unsigned used = (unsigned)(reader->head_pos - start);
    return (unsigned)(SC_RECORD_SIZE_BYTES - used);
}

unsigned record_reader_avail_body_size(struct record_reader const *reader)
{
    unsigned used = (unsigned)(reader->body_pos - reader->record->data);
    return (unsigned)(sc_record_size(reader->record) - used);
}

bool record_reader_finished(struct record_reader const *reader)
{
    return record_reader_avail_body_size(reader) == 0;
}
