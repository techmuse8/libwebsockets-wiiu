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

// TODO: Split these functions up into their own files

#include <wut.h>
#include <string.h>
#include "private-lib-core.h"
#if defined(LWS_WITH_MBEDTLS)
#if defined(LWS_HAVE_MBEDTLS_NET_SOCKETS)
#include "mbedtls/net_sockets.h"
#else
#include "mbedtls/net.h"
#endif
#endif
extern const struct lws_event_loop_ops event_loop_ops_poll;

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

int lws_plat_init(struct lws_context *cx,
                  const struct lws_context_creation_info *info)
{
    if (!cx->event_loop_ops)
        cx->event_loop_ops = &event_loop_ops_poll;

    if (!cx->max_fds) {
        unsigned int limit = info->fd_limit_per_thread ? info->fd_limit_per_thread : 64;
        cx->max_fds = limit;
    }

    if (!cx->lws_lookup) {
        cx->lws_lookup = (struct lws **)lws_malloc(
            sizeof(struct lws *) * cx->max_fds, "lws_lookup");
        if (!cx->lws_lookup) {
            lwsl_err("alloc lws_lookup failed (%u)\n", cx->max_fds);
            return 1;
        }
        memset(cx->lws_lookup, 0, sizeof(struct lws *) * cx->max_fds);
    }

    for (int n = 0; n < cx->count_threads; n++) {
        cx->pt[n].pipe_wsi = NULL;
    }

    return 0;
}



int
lws_plat_drop_app_privileges(struct lws_context *context, int actually_drop)
{
    return 0;
}

/* time / rand */

lws_usec_t lws_now_usecs(void) {
    OSTime ticks = OSGetTime();
    uint64_t usec_since_2000 = (ticks * 1000000ULL) / OSTimerClockSpeed;
    return usec_since_2000 + 946684800ULL * 1000000ULL; // convert Wii U time to Unix time
}

size_t
lws_get_random(struct lws_context *context, void *buf, size_t len)
{
    uint8_t *p = (uint8_t*)buf;
    for (size_t i = 0; i < len; i++) {
        p[i] = (uint8_t)(rand() & 0xFF);
    }
    return len;
}

/* Poll / fd management */

int
insert_wsi(const struct lws_context *context, struct lws *wsi)
{
	struct lws **p, **done;

	if (sanity_assert_no_wsi_traces(context, wsi))
		return 0;

	if (!context->max_fds_unrelated_to_ulimit) {
		assert(context->lws_lookup[wsi->desc.sockfd -
		                           lws_plat_socket_offset()] == 0);

		context->lws_lookup[wsi->desc.sockfd - \
				  lws_plat_socket_offset()] = wsi;

		return 0;
	}

	/* slow fds handling */

	p = context->lws_lookup;
	done = &p[context->max_fds];

	/* confirm fd isn't already in use by a wsi */

	if (sanity_assert_no_sockfd_traces(context, wsi->desc.sockfd))
		return 0;

	p = context->lws_lookup;

	/* find an empty slot */

	while (p != done && *p)
		p++;

	if (p == done) {
		lwsl_err("%s: reached max fds\n", __func__);
		return 1;
	}

	*p = wsi;

	return 0;
}

void
delete_from_fdwsi(const struct lws_context *context, struct lws *wsi)
{

	struct lws **p, **done;

	if (!context->max_fds_unrelated_to_ulimit)
		return;


	/* slow fds handling */

	p = context->lws_lookup;
	done = &p[context->max_fds];

	/* find the match */

	while (p != done && (!*p || (*p) != wsi))
		p++;

	if (p != done)
		*p = NULL;
}

struct lws *
wsi_from_fd(const struct lws_context *context, int fd)
{
	struct lws **p, **done;

	if (!context->max_fds_unrelated_to_ulimit)
		return context->lws_lookup[fd - lws_plat_socket_offset()];

	/* slow fds handling */

	p = context->lws_lookup;
	done = &p[context->max_fds];

	while (p != done) {
		if (*p && (*p)->desc.sockfd == fd)
			return *p;
		p++;
	}

	return NULL;
}

void
delete_from_fd(const struct lws_context *context, int fd)
{

	struct lws **p, **done;

	if (!context->max_fds_unrelated_to_ulimit) {
		if (context->lws_lookup) {
			assert((int)context->max_fds > fd - lws_plat_socket_offset());
			context->lws_lookup[fd - lws_plat_socket_offset()] = NULL;
		}

		return;
	}

	/* slow fds handling */

	p = context->lws_lookup;
	assert(p);

	done = &p[context->max_fds];

	/* find the match */

	while (p != done && (!*p || (*p)->desc.sockfd != fd))
		p++;

	if (p != done)
		*p = NULL;

#if defined(_DEBUG)
	p = context->lws_lookup;
	while (p != done && (!*p || (*p)->desc.sockfd != fd))
		p++;

	if (p != done) {
		lwsl_err("%s: fd %d in lws_lookup again at %d\n", __func__,
				fd, (int)(p - context->lws_lookup));
		assert(0);
	}
#endif
}

void
lws_plat_insert_socket_into_fds(struct lws_context *context, struct lws *wsi)
{
    struct lws_context_per_thread *pt = &context->pt[(int)wsi->tsi];
    struct lws_pollfd *pfd;

    if (wsi->desc.sockfd <= 0) {
        lwsl_notice("%s: skip invalid sockfd=%d\n", __func__, wsi->desc.sockfd);
        return;
    }

    pfd = &pt->fds[pt->fds_count++];
    pfd->fd      = wsi->desc.sockfd;          
    pfd->events  = LWS_POLLIN;
    pfd->revents = 0;

    lwsl_debug("%s: inserted fd=%d at idx=%d\n",
               __func__, pfd->fd, (int)pt->fds_count - 1);

    if (context->event_loop_ops->io)
        context->event_loop_ops->io(wsi, LWS_EV_START | LWS_EV_READ);
}

int 
lws_plat_change_pollfd(struct lws_context *context,
                           struct lws *wsi, struct lws_pollfd *pfd)
{
    struct lws_context_per_thread *pt = &context->pt[(int)wsi->tsi];
    for (unsigned int u = 0; u < pt->fds_count; u++) {
        if (pt->fds[u].fd == pfd->fd) {
            pt->fds[u].events = pfd->events;   /* critical */
            lwsl_debug("change_pollfd: fd=%d events=0x%x\n", pfd->fd, pfd->events);
            return 0;
        }
    }
    lwsl_err("change_pollfd: fd %d not found\n", pfd->fd);
    return 1;
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
        return 64;
    return -1;
}

void
lws_plat_delete_socket_from_fds(struct lws_context *context,
						struct lws *wsi, int m)
{
	struct lws_context_per_thread *pt = &context->pt[(int)wsi->tsi];

	if (context->event_loop_ops->io)
		context->event_loop_ops->io(wsi,
				LWS_EV_STOP | LWS_EV_READ | LWS_EV_WRITE);

	pt->fds_count--;
}

int
_lws_plat_service_forced_tsi(struct lws_context *context, int tsi)
{
	struct lws_context_per_thread *pt = &context->pt[tsi];
	int m, n, r;

	r = lws_service_flag_pending(context, tsi);

	/* any socket with events to service? */
	for (n = 0; n < (int)pt->fds_count; n++) {
		lws_sockfd_type fd = pt->fds[n].fd;

		if (!pt->fds[n].revents)
			continue;

		m = lws_service_fd_tsi(context, &pt->fds[n], tsi);
		if (m < 0) {
			lwsl_err("%s: lws_service_fd_tsi returned %d\n",
				 __func__, m);
			return -1;
		}

		/* if something closed, retry this slot since may have been
		 * swapped with end fd */
		if (m && pt->fds[n].fd != fd)
			n--;
	}

	lws_service_do_ripe_rxflow(pt);

	return r;
}

#define LWS_POLL_WAIT_LIMIT 2000000000

int
_lws_plat_service_tsi(struct lws_context *context, int timeout_ms, int tsi)
{
	volatile struct lws_foreign_thread_pollfd *ftp, *next;
	volatile struct lws_context_per_thread *vpt;
	struct lws_context_per_thread *pt;
	lws_usec_t timeout_us, us;
#if defined(LWS_WITH_WAKE_LOGGING)
	unsigned int u;
	char hu[25];
	lws_usec_t a1;
#endif
#if defined(LWS_WITH_SYS_METRICS) || defined(LWS_WITH_WAKE_LOGGING)
	lws_usec_t a, b = 0;
#endif
	int n;
#if (defined(LWS_ROLE_WS) && !defined(LWS_WITHOUT_EXTENSIONS)) || defined(LWS_WITH_TLS)
	int m;
#endif

	/* stay dead once we are dead */

	if (!context)
		return 1;

#if defined(LWS_WITH_SYS_METRICS)
	b =
#endif
			us = lws_now_usecs();

	pt = &context->pt[tsi];
	vpt = (volatile struct lws_context_per_thread *)pt;

	if (timeout_ms < 0)
		timeout_ms = 0;
	else
		/* force a default timeout of 23 days */
		timeout_ms = LWS_POLL_WAIT_LIMIT;
	timeout_us = ((lws_usec_t)timeout_ms) * LWS_US_PER_MS;

	if (context->event_loop_ops->run_pt)
		context->event_loop_ops->run_pt(context, tsi);

	if (!pt->service_tid_detected && context->vhost_list) {
		lws_fakewsi_def_plwsa(pt);

		lws_fakewsi_prep_plwsa_ctx(context);

		pt->service_tid = context->vhost_list->protocols[0].callback(
					(struct lws *)plwsa,
					LWS_CALLBACK_GET_THREAD_ID,
					context->vhost_list->protocols[0].user,
					NULL, 0);
		pt->service_tid_detected = 1;
	}

	lws_pt_lock(pt, __func__);
	/*
	 * service ripe scheduled events, and limit wait to next expected one
	 */
	us = __lws_sul_service_ripe(pt->pt_sul_owner, LWS_COUNT_PT_SUL_OWNERS, us);
	if (us && us < timeout_us)
		/*
		 * If something wants zero wait, that's OK, but if the next sul
		 * coming ripe is an interval less than our wait resolution,
		 * bump it to be the wait resolution.
		 */
		timeout_us = us < context->us_wait_resolution ?
					context->us_wait_resolution : us;

	lws_pt_unlock(pt);

	/*
	 * is there anybody with pending stuff that needs service forcing?
	 */
	if (!lws_service_adjust_timeout(context, 1, tsi))
		timeout_us = 0;

	/* ensure we don't wrap at 2^31 with poll()'s signed int ms */

	timeout_us /= LWS_US_PER_MS; /* ms now */

#if defined(LWS_WITH_SYS_METRICS) || defined(LWS_WITH_WAKE_LOGGING)
	a = lws_now_usecs() - b;
#endif
#if defined(LWS_WITH_WAKE_LOGGING)
	a1 = lws_now_usecs();
	lws_humanize(hu, sizeof(hu), (uint64_t)(timeout_us * LWS_US_PER_MS), humanize_schema_us);
	lwsl_cx_notice(context, "event loop: entering sleep... scheduled wake after %s", hu);
#endif
	vpt->inside_poll = 1;
	lws_memory_barrier();
	n = poll(pt->fds, pt->fds_count, (int)timeout_us /* ms now */ );
	vpt->inside_poll = 0;
	lws_memory_barrier();

#if defined(LWS_WITH_SYS_METRICS) || defined(LWS_WITH_WAKE_LOGGING)
	b = lws_now_usecs();
#endif
#if defined(LWS_WITH_WAKE_LOGGING)
	lws_humanize(hu, sizeof(hu), (uint64_t)(b - a1), humanize_schema_us);
	lwsl_cx_notice(context, "event loop: WOKE after %s, %d fds ready", hu, n);
	for (u = 0; u < pt->fds_count; u++) {
		struct lws *wsi;
		struct lws_pollfd *pfd = &vpt->fds[u];

		if (lws_socket_is_valid(pfd->fd) &&
		    (pfd->revents & (POLLIN | POLLOUT | POLLERR))) {
			wsi = wsi_from_fd(context, pfd->fd);
#if defined(LWS_WITH_SECURE_STREAMS)
			if (wsi->for_ss && wsi->a.opaque_user_data) {
				lws_ss_handle_t *fih = (lws_ss_handle_t *)wsi->a.opaque_user_data;

				lwsl_ss_notice(fih, "    ready fd %d, %s %s %s, SS policy %s", pfd->fd,
					pfd->revents & POLLIN ? "POLLIN" : "",
					pfd->revents & POLLOUT ? "POLLOUT" : "",
					pfd->revents & POLLERR ? "POLLERR": "",
					fih->policy ? fih->policy->streamtype : "(null)");
			} else
#endif
			lwsl_wsi_notice(wsi, "    ready fd %d, %s %s %s, protocol %s", pfd->fd,
					pfd->revents & POLLIN ? "POLLIN" : "",
					pfd->revents & POLLOUT ? "POLLOUT" : "",
					pfd->revents & POLLERR ? "POLLERR": "",
					wsi->a.protocol ? wsi->a.protocol->name : "(null)");
		}
	}
#endif

	/* Collision will be rare and brief.  Spin until it completes */
	while (vpt->foreign_spinlock)
		;

	/*
	 * At this point we are not inside a foreign thread pollfd
	 * change, and we have marked ourselves as outside the poll()
	 * wait.  So we are the only guys that can modify the
	 * lws_foreign_thread_pollfd list on the pt.  Drain the list
	 * and apply the changes to the affected pollfds in the correct
	 * order.
	 */

	lws_pt_lock(pt, __func__);

	ftp = vpt->foreign_pfd_list;
	//lwsl_notice("cleared list %p\n", ftp);
	while (ftp) {
		struct lws *wsi;
		struct lws_pollfd *pfd;

		next = ftp->next;
		pfd = &vpt->fds[ftp->fd_index];
		if (lws_socket_is_valid(pfd->fd)) {
			wsi = wsi_from_fd(context, pfd->fd);
			if (wsi)
				__lws_change_pollfd(wsi, ftp->_and,
						    ftp->_or);
		}
#if defined(LWS_WITH_WAKE_LOGGING)
		else
			lwsl_cx_notice(context, "*** WOKE on Invalid fd in foreign pfd list");
#endif
		lws_free((void *)ftp);
		ftp = next;
	}
	vpt->foreign_pfd_list = NULL;
	lws_memory_barrier();

	lws_pt_unlock(pt);

#if (defined(LWS_ROLE_WS) && !defined(LWS_WITHOUT_EXTENSIONS)) || defined(LWS_WITH_TLS)
	m = 0;
#endif
#if defined(LWS_ROLE_WS) && !defined(LWS_WITHOUT_EXTENSIONS)
	m |= !!pt->ws.rx_draining_ext_list;
#endif

#if defined(LWS_WITH_TLS)
	if (pt->context->tls_ops &&
	    pt->context->tls_ops->fake_POLLIN_for_buffered)
		m |= pt->context->tls_ops->fake_POLLIN_for_buffered(pt);
#endif

	if (
#if (defined(LWS_ROLE_WS) && !defined(LWS_WITHOUT_EXTENSIONS)) || defined(LWS_WITH_TLS)
		!m &&
#endif
		!n) /* nothing to do */
		lws_service_do_ripe_rxflow(pt);
	else
		if (_lws_plat_service_forced_tsi(context, tsi) < 0)
			return -1;

#if defined(LWS_WITH_SYS_METRICS)
	lws_metric_event(context->mt_service, METRES_GO,
			 (u_mt_t) (a + (lws_now_usecs() - b)));
#endif

	if (pt->destroy_self) {
		lws_context_destroy(pt->context);
		return -1;
	}

	return 0;
}

int
lws_plat_service(struct lws_context *context, int timeout_ms)
{
	return _lws_plat_service_tsi(context, timeout_ms, 0);
}

int lws_plat_set_socket_options(struct lws_vhost *vh, lws_sockfd_type fd, int flags) {
    // Apply TCP_NODELAY when requested
    int one = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
    // Optionally keepalive, etc.
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
	return fcntl(fd, F_SETFL, O_NONBLOCK) < 0;
}

int
lws_poll_listen_fd(struct lws_pollfd *fd)
{
	return poll(fd, 1, 0);
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

int 
lws_plat_BINDTODEVICE(int sockfd, const char *ifname) 
{
    return -1;
}

#if defined(LWS_WITH_MBEDTLS)
int
lws_plat_mbedtls_net_send(void *ctx, const uint8_t *buf, size_t len)
{
	int fd = ((mbedtls_net_context *) ctx)->MBEDTLS_PRIVATE_V30_ONLY(fd);
	int ret;

	if (fd < 0)
		return MBEDTLS_ERR_NET_INVALID_CONTEXT;

	ret = (int)write(fd, buf, len);
	if (ret >= 0)
		return ret;

	if (errno == EAGAIN || errno == EWOULDBLOCK)
		return MBEDTLS_ERR_SSL_WANT_WRITE;

	if (errno == EPIPE || errno == ECONNRESET)
		return MBEDTLS_ERR_NET_CONN_RESET;

	if( errno == EINTR )
		return MBEDTLS_ERR_SSL_WANT_WRITE;

	return MBEDTLS_ERR_NET_SEND_FAILED;
}

int
lws_plat_mbedtls_net_recv(void *ctx, unsigned char *buf, size_t len)
{
	int fd = ((mbedtls_net_context *) ctx)->MBEDTLS_PRIVATE_V30_ONLY(fd);
	int ret;

	if (fd < 0)
		return MBEDTLS_ERR_NET_INVALID_CONTEXT;

	ret = (int)read(fd, buf, len);
	if (ret >= 0)
		return ret;

	if (errno == EAGAIN || errno == EWOULDBLOCK)
		return MBEDTLS_ERR_SSL_WANT_READ;

	if (errno == EPIPE || errno == ECONNRESET)
		return MBEDTLS_ERR_NET_CONN_RESET;

	if (errno == EINTR)
		return MBEDTLS_ERR_SSL_WANT_READ;

	return MBEDTLS_ERR_NET_RECV_FAILED;
}

#if defined(_DEBUG)
int
sanity_assert_no_wsi_traces(const struct lws_context *context, struct lws *wsi)
{
	struct lws **p, **done;

	if (!context->max_fds_unrelated_to_ulimit)
		/* can't tell */
		return 0;

	/* slow fds handling */

	p = context->lws_lookup;
	done = &p[context->max_fds];

	/* confirm the wsi doesn't already exist */

	while (p != done && *p != wsi)
		p++;

	if (p == done)
		return 0;

	assert(0); /* this wsi is still mentioned inside lws */

	return 1;
}

int
sanity_assert_no_sockfd_traces(const struct lws_context *context,
			       lws_sockfd_type sfd)
{
#if LWS_MAX_SMP > 1
	/*
	 * We can't really do this test... another thread can accept and
	 * reuse the closed fd
	 */
	return 0;
#else
	struct lws **p, **done;

	if (sfd == LWS_SOCK_INVALID || !context->lws_lookup)
		return 0;

	if (!context->max_fds_unrelated_to_ulimit &&
	    context->lws_lookup[sfd - lws_plat_socket_offset()]) {
		assert(0); /* the fd is still in use */
		return 1;
	}

	/* slow fds handling */

	p = context->lws_lookup;
	done = &p[context->max_fds];

	/* confirm the sfd not already in use */

	while (p != done && (!*p || (*p)->desc.sockfd != sfd))
		p++;

	if (p == done)
		return 0;

	assert(0); /* this fd is still in the tables */

	return 1;
#endif
}
#endif

#endif