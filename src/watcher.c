#include "sc/watcher.h"
#include "warn.h"

static void on_connect_success_noop(struct sc_watcher *watcher)
{
    (void)watcher;
    warn("calling on_connect_success_noop");
}
static void on_connect_failure_noop(struct sc_watcher *watcher)
{
    (void)watcher;
    warn("on_connect_failure_noop");
}

static void on_connection_success_noop(struct sc_watcher *watcher)
{
    (void)watcher;
    warn("on_connection_success_noop");
}
static void on_connection_failure_noop(struct sc_watcher *watcher)
{
    (void)watcher;
    warn("on_connection_failure_noop");
}

static void on_accept_success_noop(struct sc_watcher *watcher)
{
    (void)watcher;
    warn("on_accept_success_noop");
}
static void on_accept_failure_noop(struct sc_watcher *watcher)
{
    (void)watcher;
    warn("on_accept_failure_noop");
}

static void on_recv_success_noop(struct sc_watcher *watcher,
                                 struct sc_record *record)
{
    (void)watcher;
    (void)record;
    warn("on_recv_success_noop");
}
static void on_recv_failure_noop(struct sc_watcher *watcher)
{
    (void)watcher;
    warn("on_recv_failure_noop");
}
static void on_recv_eof_noop(struct sc_watcher *watcher)
{
    (void)watcher;
    warn("on_recv_eof_noop");
}

static void on_send_success_noop(struct sc_watcher *watcher)
{
    (void)watcher;
    warn("on_send_success_noop");
}
static void on_send_failure_noop(struct sc_watcher *watcher)
{
    (void)watcher;
    warn("on_send_failure_noop");
}
static void on_send_eof_noop(struct sc_watcher *watcher)
{
    (void)watcher;
    warn("on_send_eof_noop");
}

static void on_close_noop(struct sc_watcher *watcher)
{
    (void)watcher;
    warn("on_close_noop");
}

void sc_watcher_init(struct sc_watcher *watcher)
{
    watcher->data = 0;

    watcher->on_connect_success = &on_connect_success_noop;
    watcher->on_connect_failure = &on_connect_failure_noop;

    watcher->on_connection_success = &on_connection_success_noop;
    watcher->on_connection_failure = &on_connection_failure_noop;

    watcher->on_accept_success = &on_accept_success_noop;
    watcher->on_accept_failure = &on_accept_failure_noop;

    watcher->on_recv_success = &on_recv_success_noop;
    watcher->on_recv_failure = &on_recv_failure_noop;
    watcher->on_recv_eof = &on_recv_eof_noop;

    watcher->on_send_success = &on_send_success_noop;
    watcher->on_send_failure = &on_send_failure_noop;
    watcher->on_send_eof = &on_send_eof_noop;

    watcher->on_close = &on_close_noop;
}
