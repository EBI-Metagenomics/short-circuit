#ifndef SC_BACKEND_UV_H
#define SC_BACKEND_UV_H

struct uv_loop_s;

struct sc_backend_uv_data
{
    struct uv_loop_s *loop;
};

#endif
