#ifndef URI_H
#define URI_H

#include <stdbool.h>

struct uri;

enum uri_scheme
{
    SC_PIPE,
    SC_TCP,
};

struct uri *uri_new(char const *str);
void uri_del(struct uri const *uri);

enum uri_scheme uri_scheme(struct uri const *uri);
char const *uri_pipe_filepath(struct uri const *uri);
char const *uri_tcp_ip4(struct uri const *uri);
unsigned uri_tcp_port(struct uri const *uri);

#endif
