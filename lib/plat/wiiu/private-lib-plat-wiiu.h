 /*
 * libwebsockets - small server side websockets and web server implementation
 *
 * Copyright (C) 2010 - 2019 Andy Green <andy@warmcat.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Included from lib/private-lib-core.h if LWS_PLAT_WIIU
 */

#pragma once

/* Wii U platform stubs for libwebsockets */

#include <wut.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdint.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <coreinit/thread.h>
#include <coreinit/mutex.h>

/* No rlimit on Wii U afaik */
struct rlimit {
    unsigned int rlim_cur;
    unsigned int rlim_max;
};
#define RLIMIT_NOFILE 0
static inline int setrlimit(int resource, const struct rlimit *rl) { return 0; }

typedef OSMutex lws_mutex_t;
typedef OSThread *lws_tid_t;

#define lws_mutex_init(x)   OSInitMutex(&(x))
#define lws_mutex_lock(x)   OSLockMutex(&(x))
#define lws_mutex_unlock(x) OSUnlockMutex(&(x))
#define lws_mutex_destroy(x) 0

#define lws_thread_id OSGetCurrentThread

#define LWS_ERRNO errno
#define LWS_EAGAIN EAGAIN
#define LWS_EALREADY EALREADY
#define LWS_EINPROGRESS EINPROGRESS
#define LWS_EINTR EINTR
#define LWS_EISCONN EISCONN
#define LWS_ENOTCONN ENOTCONN
#define LWS_EWOULDBLOCK EWOULDBLOCK
#define LWS_EADDRINUSE EADDRINUSE
#define lws_set_blocking_send(wsi)
#define LWS_SOCK_INVALID (-1)

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#define compatible_close(fd) close(fd)

/* Wii U has no AF_UNIX / Unix domain sockets */
#ifndef AF_UNIX
#define AF_UNIX 0
#endif

#ifndef PF_UNIX
#define PF_UNIX AF_UNIX
#endif
// No IPv6 on the Wii U so this is a stub
#ifndef AF_INET6
#define AF_INET6 23
#endif

#ifndef SOMAXCONN
#define SOMAXCONN 8
#endif

struct lws;
struct lws_context;

static inline void delete_from_fd(struct lws_context *ctx, int fd) {
    (void)ctx;
    (void)fd;
}

// TODO: uninline this
static inline int lws_plat_socket_offset(void) { return 0; }
int insert_wsi(const struct lws_context *context, struct lws *wsi);