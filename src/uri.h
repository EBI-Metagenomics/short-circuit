#ifndef URI_H
#define URI_H

#include "proto.h"
#include <stdbool.h>

struct uri;

struct uri *uri_new(char const *str);
void uri_del(struct uri const *uri);

enum proto uri_scheme_protocol(struct uri const *uri);
char const *uri_pipe_filepath(struct uri const *uri);
char const *uri_tcp_ip4(struct uri const *uri);
unsigned uri_tcp_port(struct uri const *uri);

#endif
