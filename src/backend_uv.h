#ifndef BACKEND_UV_H
#define BACKEND_UV_H

struct backend;
struct sc_backend_uv_data;

struct backend const *backend_uv_init(struct sc_backend_uv_data *);

#endif
