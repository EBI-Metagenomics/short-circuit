#ifndef BACKEND_UV_H
#define BACKEND_UV_H

struct backend;
struct backend_uv_data;

struct backend const *backend_uv_init(struct backend_uv_data *);

#endif
