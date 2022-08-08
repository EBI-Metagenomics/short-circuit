#include "backend_uv.h"
#include "backend.h"
#include "ctb/ctb.h"
#include "msg.h"
#include "msg_reader.h"
#include "proto.h"
#include "sc/backend_uv.h"
#include "sc/watcher.h"
#include "uri.h"
#include "warn.h"
#include <stdlib.h>
#include <uv.h>

union stream
{
    struct uv_pipe_s pipe;
    struct uv_tcp_s tcp;
};

struct socket_uv
{
    struct sc_watcher *watcher;
    union stream stream;
    struct uv_connect_s connect_request;
    struct uv_write_s write_request;
    uv_buf_t write_buffers[2];
    struct sc_msg *msg;
    struct msg_reader reader;
    enum proto proto;
    struct sockaddr_in tcp_addr;
};

static void *buv_alloc(void);
static void buv_init(void *, struct sc_watcher *);
static void buv_free(void *);
static int buv_connect(void *client, struct uri const *);
static int buv_accept(void *server, void *client);
static int buv_bind(void *server, struct uri const *);
static int buv_listen(void *server, int backlog);
static int buv_send(void *socket, struct sc_msg const *);
static int buv_close(void *socket);
static void buv_error(char const *msg, int status);

static struct backend_uv_data *buv_data = 0;
static struct backend buv_funcs = {buv_alloc,   buv_init,   buv_free,
                                   buv_connect, buv_accept, buv_bind,
                                   buv_listen,  buv_send,   buv_close};

struct backend const *backend_uv_init(struct backend_uv_data *data)
{
    buv_data = data;
    return &buv_funcs;
}

static void *buv_alloc(void) { return malloc(sizeof(struct socket_uv)); }

static void buv_init(void *socket, struct sc_watcher *watcher)
{
    struct socket_uv *uv = socket;
    uv->watcher = watcher;

    ((struct uv_handle_s *)(&uv->stream.pipe))->data = uv;
    ((struct uv_stream_s *)(&uv->stream.pipe))->data = uv;
    uv->connect_request.data = uv;

    uv->write_request.data = uv;
    uv->write_buffers[0] = uv_buf_init(0, 0);
    uv->write_buffers[1] = uv_buf_init(0, 0);
    uv->msg = 0;
    uv->proto = PROTO_NOSET;
    ctb_bzero(&uv->tcp_addr, sizeof(uv->tcp_addr));
}

static void buv_free(void *socket) { free(socket); }

static void on_connect_wrap(struct uv_connect_s *request, int status)
{
    struct socket_uv *uv = request->data;
    if (status)
    {
        buv_error("connect failed", status);
        (*uv->watcher->on_connect_failure)(uv->watcher);
        return;
    }

    (*uv->watcher->on_connect_success)(uv->watcher);
}

static int connect_pipe(struct socket_uv *clt, char const *filepath)
{
    int r = uv_pipe_init(buv_data->loop, &clt->stream.pipe, 0);
    if (r)
    {
        buv_error("pipe init error", r);
        return r;
    }
    uv_pipe_connect(&clt->connect_request, &clt->stream.pipe, filepath,
                    &on_connect_wrap);
    return r;
}

static int connect_tcp(struct socket_uv *clt, char const *ip4, unsigned port)
{
    int r = uv_tcp_init(buv_data->loop, &clt->stream.tcp);
    if (r)
    {
        buv_error("tcp init error", r);
        return r;
    }
    r = uv_ip4_addr(ip4, (int)port, &clt->tcp_addr);
    if (r)
    {
        buv_error("ip4 addr error", r);
        return r;
    }

    struct sockaddr const *addr = (struct sockaddr const *)&clt->tcp_addr;
    r = uv_tcp_connect(&clt->connect_request, &clt->stream.tcp, addr,
                       &on_connect_wrap);
    if (r)
    {
        buv_error("tcp connect error", r);
        return r;
    }

    return r;
}

static int buv_connect(void *client, struct uri const *uri)
{
    struct socket_uv *clt = client;
    enum proto proto = uri_scheme_protocol(uri);
    if (proto == PROTO_PIPE) return connect_pipe(clt, uri_pipe_filepath(uri));
    if (proto == PROTO_TCP)
        return connect_tcp(clt, uri_tcp_ip4(uri), uri_tcp_port(uri));
    return 1;
}

static void alloc_wrap(uv_handle_t *handle, size_t suggested_size,
                       uv_buf_t *buffer)
{
    struct socket_uv *uv = handle->data;
    struct msg_reader *reader = &uv->reader;
    unsigned sz = (unsigned)suggested_size;

    if (!uv->msg)
    {
        uv->msg = sc_msg_alloc(sz);
        msg_init(uv->msg);
        msg_reader_init(&uv->reader, uv->msg);
    }

    unsigned hsize = msg_reader_avail_head_size(reader);
    unsigned avail = 0;
    if (hsize > 0)
    {
        buffer->base = (char *)msg_reader_head_pos(reader);
        avail = hsize;
    }
    else
    {
        buffer->base = (char *)msg_reader_body_pos(reader);
        avail = msg_reader_avail_body_size(reader);
    }

    buffer->len = avail < sz ? avail : sz;
}

static void read_wrap(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
    (void)buf;
    struct socket_uv *uv = stream->data;
    if (nread < 0)
    {
        if (nread == UV_EOF)
            (*uv->watcher->on_recv_eof)(uv->watcher);
        else
        {
            buv_error("read error", (int)nread);
            (*uv->watcher->on_recv_failure)(uv->watcher);
        }
        sc_msg_free(uv->msg);
        uv->msg = 0;
    }
    else if (nread > 0)
    {
        struct msg_reader *reader = &uv->reader;
        if (msg_reader_avail_head_size(reader) > 0)
        {
            nread = (unsigned)msg_reader_parse_head(reader, (unsigned)nread);
        }
        else
        {
            nread = (unsigned)msg_reader_parse_body(reader, (unsigned)nread);
        }

        if (msg_reader_finished(reader))
        {
            (*uv->watcher->on_recv_success)(uv->watcher, uv->msg);
            sc_msg_free(uv->msg);
            uv->msg = 0;
        }
    }
}

static int buv_accept(void *server, void *client)
{
    struct socket_uv *srv = server;
    struct socket_uv *clt = client;

    int r = 0;
    if (srv->proto == PROTO_PIPE)
    {
        if ((r = uv_pipe_init(buv_data->loop, &clt->stream.pipe, 0)))
            buv_error("pipe init error", r);
    }
    else if (srv->proto == PROTO_TCP)
    {
        if ((r = uv_tcp_init(buv_data->loop, &clt->stream.tcp)))
            buv_error("tcp init error", r);
    }
    else
        assert(false);

    clt->proto = srv->proto;

    if (r)
    {
        (*clt->watcher->on_accept_failure)(clt->watcher);
        return r;
    }

    struct uv_stream_s *srv_stream = (struct uv_stream_s *)(&srv->stream);
    struct uv_stream_s *clt_stream = (struct uv_stream_s *)(&clt->stream);

    if ((r = uv_accept(srv_stream, clt_stream)))
    {
        buv_error("accept error", r);
        (*clt->watcher->on_accept_failure)(clt->watcher);
        return r;
    }

    if ((r = uv_read_start(clt_stream, alloc_wrap, read_wrap)))
    {
        buv_error("read start error", r);
        (*clt->watcher->on_accept_failure)(clt->watcher);
        return r;
    }

    (*clt->watcher->on_accept_success)(clt->watcher);

    return r;
}

static int bind_pipe(struct socket_uv *srv, char const *filepath)
{
    int r = uv_pipe_init(buv_data->loop, &srv->stream.pipe, 0);
    if (r)
    {
        buv_error("pipe init error", r);
        return r;
    }
    if ((r = uv_pipe_bind(&srv->stream.pipe, filepath)))
    {
        buv_error("pipe bind error", r);
        return r;
    }
    return r;
}

static int bind_tcp(struct socket_uv *srv, char const *ip4, unsigned port)
{
    int r = uv_tcp_init(buv_data->loop, &srv->stream.tcp);
    if (r)
    {
        buv_error("tcp init error", r);
        return r;
    }
    r = uv_ip4_addr(ip4, (int)port, &srv->tcp_addr);
    if (r)
    {
        buv_error("ip4 addr error", r);
        return r;
    }

    struct sockaddr const *addr = (struct sockaddr const *)&srv->tcp_addr;
    if ((r = uv_tcp_bind(&srv->stream.tcp, addr, 0)))
    {
        buv_error("tcp bind error", r);
        return r;
    }
    return r;
}

static int buv_bind(void *server, struct uri const *uri)
{
    struct socket_uv *srv = server;
    srv->proto = uri_scheme_protocol(uri);

    if (srv->proto == PROTO_PIPE) return bind_pipe(srv, uri_pipe_filepath(uri));
    if (srv->proto == PROTO_TCP)
        return bind_tcp(srv, uri_tcp_ip4(uri), uri_tcp_port(uri));

    assert(false);
}

static void on_connection_wrap(struct uv_stream_s *stream, int status)
{
    struct socket_uv *uv = stream->data;
    if (status)
    {
        buv_error("connection error", status);
        (*uv->watcher->on_connection_failure)(uv->watcher);
        return;
    }
    (*uv->watcher->on_connection_success)(uv->watcher);
}

static int buv_listen(void *server, int backlog)
{
    struct socket_uv *srv = server;

    int r = uv_listen((struct uv_stream_s *)(&srv->stream), backlog,
                      &on_connection_wrap);
    if (r)
    {
        buv_error("listen error", r);
        return r;
    }
    return r;
}

static void write_wrap(struct uv_write_s *request, int status)
{
    struct socket_uv *uv = request->data;
    if (status)
    {
        buv_error("write error", status);
        (*uv->watcher->on_send_failure)(uv->watcher);
        return;
    }
    (*uv->watcher->on_send_success)(uv->watcher);
}

static int buv_send(void *socket, struct sc_msg const *msg)
{
    struct socket_uv *uv = socket;

    uv->write_buffers[0].len = SC_MSG_SIZE_BYTES;
    uv->write_buffers[0].base = (char *)&msg->size_be;
    uv->write_buffers[1].len = sc_msg_get_size(msg);
    uv->write_buffers[1].base = (char *)msg->data;

    struct uv_stream_s *stream = (struct uv_stream_s *)&uv->stream;
    struct uv_write_s *request = &uv->write_request;
    int r = uv_write(request, stream, uv->write_buffers, 2, &write_wrap);
    if (r)
    {
        buv_error("send error", r);
        return r;
    }
    return r;
}

static void close_wrap(struct uv_handle_s *handle)
{
    struct socket_uv *uv = handle->data;
    (*uv->watcher->on_close)(uv->watcher);
}

static int buv_close(void *socket)
{
    struct socket_uv *uv = socket;
    struct uv_handle_s *handle = (struct uv_handle_s *)&uv->stream;
    uv_close(handle, close_wrap);
    return 0;
}

static void buv_error(char const *msg, int status)
{
    fprintf(stderr, "%s %s\n", msg, uv_err_name(status));
}
