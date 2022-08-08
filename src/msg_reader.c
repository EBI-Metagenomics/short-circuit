#include "msg_reader.h"
#include "ctb/ctb.h"
#include "sc/msg.h"
#include <assert.h>

void msg_reader_init(struct msg_reader *reader, struct sc_msg *msg)
{
    reader->msg = msg;
    reader->head_pos = (unsigned char *)&msg->size_be;
    reader->body_pos = (unsigned char *)msg->data;
}

static unsigned skip_pos(unsigned char **pos, unsigned avail, unsigned size)
{
    unsigned skip = avail < size ? avail : size;
    *pos += skip;
    return (unsigned)(size - skip);
}

unsigned msg_reader_parse_head(struct msg_reader *reader, unsigned size)
{
    unsigned avail = msg_reader_avail_head_size(reader);
    return skip_pos(&reader->head_pos, avail, size);
}

unsigned msg_reader_parse_body(struct msg_reader *reader, unsigned size)
{
    unsigned avail = msg_reader_avail_body_size(reader);
    return skip_pos(&reader->body_pos, avail, size);
}

unsigned char *msg_reader_head_pos(struct msg_reader *reader)
{
    return reader->head_pos;
}

unsigned char *msg_reader_body_pos(struct msg_reader *reader)
{
    return reader->body_pos;
}

unsigned msg_reader_avail_head_size(struct msg_reader const *reader)
{
    unsigned char const *start = (unsigned char *)&reader->msg->size_be;
    unsigned used = (unsigned)(reader->head_pos - start);
    return (unsigned)(SC_MSG_SIZE_BYTES - used);
}

unsigned msg_reader_avail_body_size(struct msg_reader const *reader)
{
    unsigned used = (unsigned)(reader->body_pos - reader->msg->data);
    return (unsigned)(sc_msg_get_size(reader->msg) - used);
}

bool msg_reader_finished(struct msg_reader const *reader)
{
    return msg_reader_avail_body_size(reader) == 0;
}
