/*
 * Wii U platform stubs for libwebsockets by techmuse
 *
 * Copyright (C) 2010 - 2025 Andy Green <andy@warmcat.com>
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
 */

#include <wut.h>
#include <string.h>
#include "private-lib-core.h"

// TODO: Reimplment these stubs
int
rpl_entry(int argc, char **argv)
{
    return 0;
}

/* Context lifecycle */

int
lws_plat_context_early_init(void)
{
    return 0;
}

void
lws_plat_context_early_destroy(struct lws_context *context)
{
}

void
lws_plat_context_late_destroy(struct lws_context *context)
{
}

int
lws_plat_init(struct lws_context *context,
          const struct lws_context_creation_info *info)
{
    return 0;
}

int
lws_plat_drop_app_privileges(struct lws_context *context, int actually_drop)
{
    return 0;
}

/* Timing / RNG */

lws_usec_t
lws_now_usecs(void)
{
    /* TODO: reimplement with OSGetTime() */
    return 0;
}

size_t
lws_get_random(struct lws_context *context, void *buf, size_t len)
{
    memset(buf, 0, len);
    return len;
}

/* Poll / fd management */

int
insert_wsi(const struct lws_context *context, struct lws *wsi)
{
    return 0;
}

void
delete_from_fdwsi(const struct lws_context *context, struct lws *wsi)
{
}

struct lws *
wsi_from_fd(const struct lws_context *context, int fd)
{
    return NULL;
}

void
lws_plat_insert_socket_into_fds(struct lws_context *context, struct lws *wsi)
{
}

int
lws_plat_change_pollfd(struct lws_context *context,
               struct lws *wsi, struct lws_pollfd *pfd)
{
    return 0;
}

/* Event pipes */

int
lws_plat_pipe_create(struct lws *wsi)
{
    return 0;
}

int
lws_plat_pipe_signal(struct lws_context *ctx, int tsi)
{
    return 0;
}

void
lws_plat_pipe_close(struct lws *wsi)
{
}

int
lws_plat_pipe_is_fd_assocated(struct lws_context *cx, int tsi,
                  lws_sockfd_type fd)
{
    return 0;
}

/* sysconf stub */

long
sysconf(int name)
{
    if (name == _SC_OPEN_MAX)
        return 64; /* arbitrary fd limit */
    return -1;
}

void
lws_plat_delete_socket_from_fds(struct lws_context *context,
                struct lws *wsi, int m)
{
    
}

int
lws_plat_service(struct lws_context *context, int timeout_ms)
{
    return 0;
}

int
_lws_plat_service_tsi(struct lws_context *context, int timeout_ms, int tsi)
{
    return 0;
}

int
lws_plat_set_socket_options(struct lws_vhost *vh,
                lws_sockfd_type fd, int flags)
{
    return 0;
}

int
lws_plat_set_socket_options_ip(lws_sockfd_type fd, uint8_t pri, int lws_flags)
{
    return 0;
}

int
lws_plat_set_nonblocking(lws_sockfd_type fd)
{
    return 0;
}

int
lws_poll_listen_fd(struct lws_pollfd *fd)
{
    return 0;
}

int
lws_open(const char *filename, int oflag, ...)
{
    return -1; 
}

const char *
lws_plat_inet_ntop(int af, const void *src, char *dst, socklen_t cnt)
{
    return NULL;
}

int
lws_interface_to_sa(int ipv6, const char *ifname,
            struct sockaddr_in *addr, size_t addrlen)
{
    return -1;
}