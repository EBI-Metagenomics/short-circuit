#ifndef MSG_READER_H
#define MSG_READER_H

#include "msg.h"
#include <stdbool.h>

struct msg_reader
{
    struct sc_msg *msg;
    unsigned char *head_pos;
    unsigned char *body_pos;
};

void msg_reader_init(struct msg_reader *, struct sc_msg *);

unsigned msg_reader_parse_head(struct msg_reader *reader, unsigned size);
unsigned msg_reader_parse_body(struct msg_reader *reader, unsigned size);

unsigned char *msg_reader_head_pos(struct msg_reader *reader);
unsigned char *msg_reader_body_pos(struct msg_reader *reader);

unsigned msg_reader_avail_head_size(struct msg_reader const *);
unsigned msg_reader_avail_body_size(struct msg_reader const *);

bool msg_reader_finished(struct msg_reader const *);

#endif
