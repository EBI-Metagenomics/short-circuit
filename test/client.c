#include "helper.h"
#include "sc/sc.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <uv.h>

enum
{
    RECORD_MAX_SIZE = 1024,
};

struct client
{
    struct backend_uv_data uv;
    struct uv_async_s async;
    struct uv_signal_s sigterm;
    struct uv_signal_s sigint;
    struct uv_idle_s idle;

    struct sc_record *record;
    struct sc_socket *socket;
    struct sc_watcher watcher;
    bool terminate;
    bool input_eof;
    bool ready_send;
};

static char const *uri = 0;
static FILE *input = 0;

static void close_socket(struct sc_socket *socket)
{
    if (sc_socket_close(socket)) fatal("sc_socket_close");
}

static void async_cb(struct uv_async_s *handle)
{
    struct client *client = handle->data;
    client->terminate = true;
    uv_close((struct uv_handle_s *)&client->async, 0);
}

static void signal_cb(struct uv_signal_s *handle, int signum)
{
    (void)signum;
    assert(signum == SIGTERM || signum == SIGINT);
    struct client *client = handle->data;
    if (client->terminate) return;
    if (uv_async_send(&client->async)) fatal("uv_async_send");
}

static void record_setup(struct sc_record *record, char const *msg)
{
    unsigned size = strlen(msg);
    sc_record_set_size(record, size);
    memcpy(record->data, msg, size);
}

static void idle_cb(struct uv_idle_s *handle)
{
    struct client *client = handle->data;
    if (client->terminate)
    {
        uv_close((struct uv_handle_s *)&client->idle, 0);
        close_socket(client->socket);
        return;
    }

    if (!client->input_eof && client->ready_send)
    {
        char *str = (char *)client->record->data;
        if (fgets(str, RECORD_MAX_SIZE, input))
        {
            record_setup(client->record, str);
            if (sc_socket_send(client->socket, client->record))
                fatal("sc_socket_send");
            client->ready_send = false;
            client->input_eof = feof(input);
        }
        else
        {
            if (ferror(input))
                fatal("input error");
            else
            {
                if (!(client->input_eof = feof(input))) fatal("input error");
                if (uv_async_send(&client->async)) fatal("uv_async_send");
            }
        }
    }
}

static void async_init(struct client *client)
{
    if (uv_async_init(client->uv.loop, &client->async, async_cb))
        fatal("uv_async_init");

    client->async.data = client;
}

static void signal_init(struct client *client, struct uv_signal_s *signal,
                        uv_signal_cb signal_cb, int signum)
{
    if (uv_signal_init(client->uv.loop, signal)) fatal("uv_signal_init");

    if (uv_signal_start(signal, signal_cb, signum)) fatal("uv_signal_start");

    signal->data = client;
}

static void idle_init(struct client *client)
{
    if (uv_idle_init(client->uv.loop, &client->idle)) fatal("uv_idle_init");

    if (uv_idle_start(&client->idle, idle_cb)) fatal("uv_idle_start");

    client->idle.data = client;
}

static void on_connect_success(struct sc_watcher *w)
{
    struct client *client = w->data;
    client->ready_send = true;
}

static void on_connect_failure(struct sc_watcher *w)
{
    (void)w;
    fatal("failed to connect");
}

static void on_recv_eof(struct sc_watcher *w)
{
    struct client *client = w->data;
    close_socket(client->socket);
}

static void on_send_success(struct sc_watcher *w)
{
    struct client *client = w->data;
    client->ready_send = true;
}

static void on_close(struct sc_watcher *w)
{
    struct client *client = w->data;
    uv_close((struct uv_handle_s *)&client->sigterm, 0);
    uv_close((struct uv_handle_s *)&client->sigint, 0);
}

static struct sc_record *alloc_record(uint32_t size)
{
    struct sc_record *record = malloc(sizeof(struct sc_record));
    record->data = malloc(size);
    return record;
}

static void free_record(struct sc_record *record)
{
    free(record->data);
    free(record);
}

static struct client *client_new(struct uv_loop_s *loop)
{
    struct client *client = malloc(sizeof(*client));
    if (!client) fatal("not enough memory");
    client->record = alloc_record(RECORD_MAX_SIZE);
    if (!client->record) fatal("not enough memory");
    client->uv.loop = loop;
    return client;
}

static void client_init(struct client *client)
{
    async_init(client);
    signal_init(client, &client->sigterm, signal_cb, SIGTERM);
    signal_init(client, &client->sigint, signal_cb, SIGINT);
    idle_init(client);

    sc_watcher_init(&client->watcher);
    client->watcher.data = client;
    client->watcher.on_connect_success = &on_connect_success;
    client->watcher.on_connect_failure = &on_connect_failure;
    client->watcher.on_recv_eof = &on_recv_eof;
    client->watcher.on_send_success = &on_send_success;
    client->watcher.on_close = &on_close;

    if (!(client->socket = sc_socket_new(&client->watcher)))
        fatal("sc_socket_new");

    client->terminate = false;
    client->input_eof = false;
    client->ready_send = false;
}

static void client_del(struct client *client)
{
    sc_socket_del(client->socket);
    free_record(client->record);
    free(client);
}

static void usage(void)
{
    puts("Usage: client URI INPUT_FILE");
    exit(1);
}

static void parse_args(int argc, char **argv)
{
    if (argc != 3) usage();
    uri = argv[1];
    if (!(input = fopen(argv[2], "r"))) fatal("failed to open input");
}

static void client_connect(struct client *client, char const *uri)
{
    if (sc_socket_connect(client->socket, uri)) fatal("sc_socket_connect");
}

int main(int argc, char **argv)
{
    parse_args(argc, argv);

    struct client *client = client_new(uv_default_loop());

    sc_init(SC_UV, &client->uv, alloc_record, free_record);

    client_init(client);

    client_connect(client, uri);

    if (uv_run(client->uv.loop, UV_RUN_DEFAULT)) fatal("uv_run");
    if (uv_loop_close(client->uv.loop)) fatal("uv_loop_close");

    client_del(client);
    fclose(input);

    return 0;
}
