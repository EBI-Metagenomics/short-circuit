#include "record_reader.h"
#include "ctb/ctb.h"
#include "sc/record.h"
#include <assert.h>

void record_reader_init(struct record_reader *reader, struct sc_record *record)
{
    reader->record = record;
    reader->pos = (unsigned char *)record;
}

unsigned char *record_reader_pos(struct record_reader *reader)
{
    return reader->pos;
}

static unsigned parse_head(struct record_reader *reader, unsigned size);
static unsigned parse_body(struct record_reader *reader, unsigned size);

unsigned record_reader_parse(struct record_reader *reader, unsigned size)
{
    size = parse_head(reader, size);
    return parse_body(reader, size);
}

static unsigned parse_head(struct record_reader *reader, unsigned size)
{
    unsigned avail = record_reader_avail_head_size(reader);

    if (avail == 0) return size;

    unsigned skip = avail < size ? avail : size;
    reader->pos += skip;
    size = (unsigned)(size - skip);
    avail = record_reader_avail_head_size(reader);

    if (avail == 0) reader->record->size = ctb_ntohl(reader->record->size);

    return size;
}

static unsigned parse_body(struct record_reader *reader, unsigned size)
{
    if (record_reader_avail_head_size(reader) > 0) return size;

    unsigned avail = record_reader_avail_body_size(reader);

    unsigned skip = avail < size ? avail : size;
    reader->pos += skip;
    size = (unsigned)(size - skip);

    return size;
}

static unsigned used_size(struct record_reader const *reader)
{
    return (unsigned)(reader->pos - (unsigned char *)reader->record);
}

unsigned record_reader_avail_head_size(struct record_reader const *reader)
{
    unsigned used = used_size(reader);
    return used < SC_RECORD_SIZE_BYTES ? (unsigned)(SC_RECORD_SIZE_BYTES - used)
                                       : 0;
}

unsigned record_reader_avail_body_size(struct record_reader const *reader)
{
    if (record_reader_avail_head_size(reader) > 0) return reader->record->size;
    unsigned used = used_size(reader);
    return reader->record->size - (used - SC_RECORD_SIZE_BYTES);
}

bool record_reader_finished(struct record_reader const *reader)
{
    return record_reader_avail_body_size(reader) == 0;
}
