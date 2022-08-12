#include "backend_uv.h"
#include "backend.h"
#include "ctb/ctb.h"
#include "msg.h"
#include "msg_reader.h"
#include "proto.h"
#include "sc/backend_uv.h"
#include "sc/errcode.h"
#include "uri.h"
#include "uv.h"
#include "warn.h"
#include "watcher.h"
#include <assert.h>
#include <stdlib.h>

union stream
{
    struct uv_pipe_s pipe;
    struct uv_tcp_s tcp;
};

struct socket_uv
{
    struct watcher *watcher;
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
static void buv_init(void *, struct watcher *);
static void buv_free(void *);
static int buv_connect(void *client, struct uri const *);
static void buv_accept(void *server, void *client);
static int buv_bind(void *server, struct uri const *);
static int buv_listen(void *server, int backlog);
static int buv_send(void *socket, struct sc_msg const *);
static int buv_close(void *socket);
static char const *buv_strerror(int errcode);

static struct sc_backend_uv_data *buv_data = 0;
static struct backend buv_funcs = {
    buv_alloc, buv_init,   buv_free, buv_connect, buv_accept,
    buv_bind,  buv_listen, buv_send, buv_close,   buv_strerror};

struct backend const *backend_uv_init(struct sc_backend_uv_data *data)
{
    buv_data = data;
    return &buv_funcs;
}

static void *buv_alloc(void) { return malloc(sizeof(struct socket_uv)); }

static void buv_init(void *socket, struct watcher *watcher)
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

static void on_connect_wrap(struct uv_connect_s *request, int errcode)
{
    struct socket_uv *uv = request->data;
    if (errcode)
    {
        (*uv->watcher->on_connect)(uv->watcher->socket, errcode);
        return;
    }

    (*uv->watcher->on_connect)(uv->watcher->socket, errcode);
}

static int connect_pipe(struct socket_uv *clt, char const *filepath)
{
    int errcode = uv_pipe_init(buv_data->loop, &clt->stream.pipe, 0);
    if (errcode) return errcode;
    uv_pipe_connect(&clt->connect_request, &clt->stream.pipe, filepath,
                    &on_connect_wrap);
    return errcode;
}

static int connect_tcp(struct socket_uv *clt, char const *ip4, unsigned port)
{
    int errcode = uv_tcp_init(buv_data->loop, &clt->stream.tcp);
    if (errcode) return errcode;

    errcode = uv_ip4_addr(ip4, (int)port, &clt->tcp_addr);
    if (errcode) return errcode;

    struct sockaddr const *addr = (struct sockaddr const *)&clt->tcp_addr;
    errcode = uv_tcp_connect(&clt->connect_request, &clt->stream.tcp, addr,
                             &on_connect_wrap);
    return errcode;
}

static int buv_connect(void *client, struct uri const *uri)
{
    struct socket_uv *clt = client;
    enum proto proto = sc_uri_protocol(uri);
    if (proto == PROTO_PIPE)
        return connect_pipe(clt, sc_uri_pipe_filepath(uri));
    if (proto == PROTO_TCP)
        return connect_tcp(clt, sc_uri_tcp_ip4(uri), sc_uri_tcp_port(uri));
    sc_warn("invalid protocol");
    return SC_EINVPROTO;
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
        sc_msg_init(uv->msg);
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
    int errcode = nread >= 0 || nread == UV_EOF ? 0 : (int)nread;

    if (nread < 0)
    {
        if (nread == UV_EOF)
        {
            assert(errcode == 0);
            (*uv->watcher->on_recv)(uv->watcher->socket, 0, errcode);
        }
        else
        {
            assert(errcode != 0);
            (*uv->watcher->on_recv)(uv->watcher->socket, 0, errcode);
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
            assert(errcode == 0);
            (*uv->watcher->on_recv)(uv->watcher->socket, uv->msg, errcode);
            sc_msg_free(uv->msg);
            uv->msg = 0;
        }
    }
}

static void buv_accept(void *server, void *client)
{
    struct socket_uv *srv = server;
    struct socket_uv *clt = client;

    assert(srv->proto == PROTO_PIPE || srv->proto == PROTO_TCP);

    int errcode = 0;
    if (srv->proto == PROTO_PIPE)
    {
        errcode = uv_pipe_init(buv_data->loop, &clt->stream.pipe, 0);
    }
    else
    {
        errcode = uv_tcp_init(buv_data->loop, &clt->stream.tcp);
    }

    if (errcode)
    {
        (*clt->watcher->on_accept)(clt->watcher->socket, errcode);
        return;
    }

    clt->proto = srv->proto;

    struct uv_stream_s *srv_stream = (struct uv_stream_s *)(&srv->stream);
    struct uv_stream_s *clt_stream = (struct uv_stream_s *)(&clt->stream);

    if ((errcode = uv_accept(srv_stream, clt_stream)))
    {
        (*clt->watcher->on_accept)(clt->watcher->socket, errcode);
        return;
    }

    if ((errcode = uv_read_start(clt_stream, alloc_wrap, read_wrap)))
    {
        (*clt->watcher->on_accept)(clt->watcher->socket, errcode);
        return;
    }

    assert(errcode == 0);
    (*clt->watcher->on_accept)(clt->watcher->socket, errcode);
}

static int bind_pipe(struct socket_uv *srv, char const *filepath)
{
    int errcode = uv_pipe_init(buv_data->loop, &srv->stream.pipe, 0);
    if (errcode) return errcode;

    return uv_pipe_bind(&srv->stream.pipe, filepath);
}

static int bind_tcp(struct socket_uv *srv, char const *ip4, unsigned port)
{
    int errcode = uv_tcp_init(buv_data->loop, &srv->stream.tcp);
    if (errcode) return errcode;

    errcode = uv_ip4_addr(ip4, (int)port, &srv->tcp_addr);
    if (errcode) return errcode;

    struct sockaddr const *addr = (struct sockaddr const *)&srv->tcp_addr;
    return uv_tcp_bind(&srv->stream.tcp, addr, 0);
}

static int buv_bind(void *server, struct uri const *uri)
{
    struct socket_uv *srv = server;
    srv->proto = sc_uri_protocol(uri);
    assert(srv->proto == PROTO_PIPE || srv->proto == PROTO_TCP);

    int errcode = 0;
    if (srv->proto == PROTO_PIPE)
    {
        errcode = bind_pipe(srv, sc_uri_pipe_filepath(uri));
    }
    else
    {
        errcode = bind_tcp(srv, sc_uri_tcp_ip4(uri), sc_uri_tcp_port(uri));
    }

    return errcode;
}

static void on_connection_wrap(struct uv_stream_s *stream, int errcode)
{
    struct socket_uv *uv = stream->data;
    (*uv->watcher->on_connection)(uv->watcher->socket, errcode);
}

static int buv_listen(void *server, int backlog)
{
    struct socket_uv *srv = server;
    struct uv_stream_s *stream = (struct uv_stream_s *)(&srv->stream);

    return uv_listen(stream, backlog, &on_connection_wrap);
}

static void write_wrap(struct uv_write_s *request, int errcode)
{
    struct socket_uv *uv = request->data;
    (*uv->watcher->on_send)(uv->watcher->socket, errcode);
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

    return uv_write(request, stream, uv->write_buffers, 2, &write_wrap);
}

static void close_wrap(struct uv_handle_s *handle)
{
    struct socket_uv *uv = handle->data;
    (*uv->watcher->on_close)(uv->watcher->socket);
}

static int buv_close(void *socket)
{
    struct socket_uv *uv = socket;
    struct uv_handle_s *handle = (struct uv_handle_s *)&uv->stream;
    uv_close(handle, close_wrap);
    return 0;
}

// Copied from libuv 1.44.3
#define BUV_ERRNO_MAP(XX)                                                      \
    XX(E2BIG, "argument list too long")                                        \
    XX(EACCES, "permission denied")                                            \
    XX(EADDRINUSE, "address already in use")                                   \
    XX(EADDRNOTAVAIL, "address not available")                                 \
    XX(EAFNOSUPPORT, "address family not supported")                           \
    XX(EAGAIN, "resource temporarily unavailable")                             \
    XX(EAI_ADDRFAMILY, "address family not supported")                         \
    XX(EAI_AGAIN, "temporary failure")                                         \
    XX(EAI_BADFLAGS, "bad ai_flags value")                                     \
    XX(EAI_BADHINTS, "invalid value for hints")                                \
    XX(EAI_CANCELED, "request canceled")                                       \
    XX(EAI_FAIL, "permanent failure")                                          \
    XX(EAI_FAMILY, "ai_family not supported")                                  \
    XX(EAI_MEMORY, "out of memory")                                            \
    XX(EAI_NODATA, "no address")                                               \
    XX(EAI_NONAME, "unknown node or service")                                  \
    XX(EAI_OVERFLOW, "argument buffer overflow")                               \
    XX(EAI_PROTOCOL, "resolved protocol is unknown")                           \
    XX(EAI_SERVICE, "service not available for socket type")                   \
    XX(EAI_SOCKTYPE, "socket type not supported")                              \
    XX(EALREADY, "connection already in progress")                             \
    XX(EBADF, "bad file descriptor")                                           \
    XX(EBUSY, "resource busy or locked")                                       \
    XX(ECANCELED, "operation canceled")                                        \
    XX(ECHARSET, "invalid Unicode character")                                  \
    XX(ECONNABORTED, "software caused connection abort")                       \
    XX(ECONNREFUSED, "connection refused")                                     \
    XX(ECONNRESET, "connection reset by peer")                                 \
    XX(EDESTADDRREQ, "destination address required")                           \
    XX(EEXIST, "file already exists")                                          \
    XX(EFAULT, "bad address in system call argument")                          \
    XX(EFBIG, "file too large")                                                \
    XX(EHOSTUNREACH, "host is unreachable")                                    \
    XX(EINTR, "interrupted system call")                                       \
    XX(EINVAL, "invalid argument")                                             \
    XX(EIO, "i/o error")                                                       \
    XX(EISCONN, "socket is already connected")                                 \
    XX(EISDIR, "illegal operation on a directory")                             \
    XX(ELOOP, "too many symbolic links encountered")                           \
    XX(EMFILE, "too many open files")                                          \
    XX(EMSGSIZE, "message too long")                                           \
    XX(ENAMETOOLONG, "name too long")                                          \
    XX(ENETDOWN, "network is down")                                            \
    XX(ENETUNREACH, "network is unreachable")                                  \
    XX(ENFILE, "file table overflow")                                          \
    XX(ENOBUFS, "no buffer space available")                                   \
    XX(ENODEV, "no such device")                                               \
    XX(ENOENT, "no such file or directory")                                    \
    XX(ENOMEM, "not enough memory")                                            \
    XX(ENONET, "machine is not on the network")                                \
    XX(ENOPROTOOPT, "protocol not available")                                  \
    XX(ENOSPC, "no space left on device")                                      \
    XX(ENOSYS, "function not implemented")                                     \
    XX(ENOTCONN, "socket is not connected")                                    \
    XX(ENOTDIR, "not a directory")                                             \
    XX(ENOTEMPTY, "directory not empty")                                       \
    XX(ENOTSOCK, "socket operation on non-socket")                             \
    XX(ENOTSUP, "operation not supported on socket")                           \
    XX(EOVERFLOW, "value too large for defined data type")                     \
    XX(EPERM, "operation not permitted")                                       \
    XX(EPIPE, "broken pipe")                                                   \
    XX(EPROTO, "protocol error")                                               \
    XX(EPROTONOSUPPORT, "protocol not supported")                              \
    XX(EPROTOTYPE, "protocol wrong type for socket")                           \
    XX(ERANGE, "result too large")                                             \
    XX(EROFS, "read-only file system")                                         \
    XX(ESHUTDOWN, "cannot send after transport endpoint shutdown")             \
    XX(ESPIPE, "invalid seek")                                                 \
    XX(ESRCH, "no such process")                                               \
    XX(ETIMEDOUT, "connection timed out")                                      \
    XX(ETXTBSY, "text file is busy")                                           \
    XX(EXDEV, "cross-device link not permitted")                               \
    XX(UNKNOWN, "unknown error")                                               \
    XX(EOF, "end of file")                                                     \
    XX(ENXIO, "no such device or address")                                     \
    XX(EMLINK, "too many links")                                               \
    XX(EHOSTDOWN, "host is down")                                              \
    XX(EREMOTEIO, "remote I/O error")                                          \
    XX(ENOTTY, "inappropriate ioctl for device")                               \
    XX(EFTYPE, "inappropriate file type or format")                            \
    XX(EILSEQ, "illegal byte sequence")                                        \
    XX(ESOCKTNOSUPPORT, "socket type not supported")

#define BUV_STRERROR_GEN(name, msg)                                            \
    case UV_##name:                                                            \
        return msg;
static char const *buv_strerror(int errcode)
{
    switch (errcode)
    {
        BUV_ERRNO_MAP(BUV_STRERROR_GEN);
    }
    return "unknown error code";
}
#undef BUV_STRERROR_GEN
