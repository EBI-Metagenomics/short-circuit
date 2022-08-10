#ifndef URI_H
#define URI_H

#include "proto.h"
#include <stdbool.h>

struct uri;

struct uri *sc_uri_new(char const *str, int *errcode);
void sc_uri_del(struct uri const *uri);

enum proto sc_uri_scheme_protocol(struct uri const *uri);
char const *sc_uri_pipe_filepath(struct uri const *uri);
char const *sc_uri_tcp_ip4(struct uri const *uri);
unsigned sc_uri_tcp_port(struct uri const *uri);

#endif
