/*
 * Copyright (c) 2004-2009 Silicon Graphics, Inc. All rights reserved.
 *         By Greg Banks <gnb@sgi.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include "common.h"
#include "stream.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#ifndef MIN
#define MIN(a,b)    ((a)<(b)?(a):(b))
#endif

#define sperror(s, call) \
    perrorf(#call "(\"%s\")", (s)->name);

static inline int64_t
_stream_used_len(stream_t *s)
{
    return s->bufsize - s->remain;
}

static inline unsigned char *
_stream_pull_buffer(stream_t *s)
{
    return s->current + s->remain;
}

static inline unsigned char *
_stream_push_buffer(stream_t *s)
{
    return s->buffer;
}

int
stream_pull(stream_t *s)
{
    int pulled;

    if (s->remain)
	memmove(s->buffer, s->current, s->remain);
    s->current = s->buffer;
    if ((pulled = (s->ops->pull)(s)) < 0)
	return -1;
    s->remain += pulled;
    s->stats.nblocks++;
    s->stats.nbytes += pulled;
    return pulled;
}

int
stream_read(stream_t *s, char *buf, int len)
{
    int ret = 0;

    do
    {
	unsigned long n = MIN(len, s->remain);

	if (n)
	{
	    memcpy(buf, s->current, n);
	    buf += n;
	    s->current += n;
	    s->remain -= n;
	    len -= n;
	    ret += n;
	}
	if (len && !s->remain)
	{
	    int pulled = stream_pull(s);
	    if (pulled < 0)
	    	return -1;
	    if (pulled == 0)
	    	break;
	}
    }
    while (len);

    return ret;
}

int
stream_push(stream_t *s)
{
    int pushed;

    if ((pushed = (s->ops->push)(s)) < 0)
	return -1;
    if (pushed < _stream_used_len(s))
    {
	/* TODO: handle short writes properly */
	fprintf(stderr, "short write!!!\n");
	exit(1);
    }
    s->stats.nblocks++;
    s->stats.nbytes += _stream_used_len(s);
    s->current = s->buffer;
    s->remain = s->bufsize;
    return 0;
}

int
stream_seek(stream_t *s, uint64_t off)
{
    if (s->ops->seek == 0)
	return -EOPNOTSUPP;
    return (*s->ops->seek)(s, off);
}

int
stream_write(stream_t *s, char *buf, int len)
{
    int ret = 0;

    do
    {
	unsigned long n = MIN(len, s->remain);

	assert(n > 0);

	memcpy(s->current, buf, n);
	buf += n;
	s->current += n;
	s->remain -= n;
	len -= n;
	ret += n;

	if (!s->remain)
	{
	    if (stream_push(s))
	    	return -1;
	}
    }
    while (len);

    return ret;
}

int
stream_flush(stream_t *s)
{
    if ((s->oflags & O_ACCMODE) != O_RDONLY)
	return stream_push(s);
    return 0;
}

int
stream_close(stream_t *s)
{
    if (stream_flush(s) < 0)
	return -1;
    return (*s->ops->close)(s);
}




static int
mmap_pull(stream_t *s)
{
    fprintf(stderr, "mmap_pull called\n");
    return -1;
}

static int
mmap_push(stream_t *s)
{
#if 0
    fprintf(stderr, "mmap_push called\n");
    exit(1);
#endif
    return _stream_used_len(s);
}

static int
mmap_seek(stream_t *s, uint64_t off)
{
    if (off >= s->bufsize)
	return -EINVAL;
    s->current = s->buffer + off;
    s->remain = s->bufsize - off;
    return 0;
}

static int
mmap_close(stream_t *s)
{
    int error = 0;

    if (!(s->xflags & STREAM_NOMSYNC))
    {
	if (msync(s->buffer, s->bufsize, MS_SYNC) < 0)
	    sperror(s, msync);
    }

    if (munmap(s->buffer, s->bufsize) < 0)
    {
	error = -errno;
	sperror(s, munmap);
    }

    if (s->fd >= 0 && close(s->fd) < 0)
    {
	error = -errno;
	sperror(s, close);
    }

    xfree(s->name);
    xfree(s);
    return error ? -1 : 0;
}

static stream_ops_t mmap_ops =
{
    mmap_pull,
    mmap_push,
    mmap_seek,
    mmap_close
};

stream_t *
stream_mmap_open(const char *filename, int oflags, int xflags, uint64_t size)
{
    stream_t *s;
    int prot;
    unsigned long page_size = sysconf(_SC_PAGESIZE);
    struct stat64 sb;


    switch (oflags & O_ACCMODE)
    {
    case O_RDONLY:
	prot = PROT_READ;
	break;
    case O_WRONLY:
	/* mmap() needs the mode to be O_RDWR */
	oflags &= ~O_ACCMODE;
	oflags |= O_RDWR;
	prot = PROT_READ|PROT_WRITE;
	break;
    default:
	fprintf(stderr, "stream_mmap_open: invalid access mode\n");
	return 0;
    }

    s = xmalloc(sizeof(stream_t));
    s->name = xstrdup(filename);
    s->bufsize = size;
    s->remain = size;
    s->ops = &mmap_ops;

    if ((s->fd = open(filename, oflags, 0600)) < 0)
    {
	sperror(s, open);
	goto failure;
    }

    if (fstat64(s->fd, &sb) < 0)
    {
	sperror(s, fstat64);
	goto failure;
    }
    if (size == 0)
    {
	size = sb.st_size;
    }
    else if (size > sb.st_size)
    {
	if (ftruncate64(s->fd, size) < 0)
	{
	    sperror(s, ftruncate64);
	    goto failure;
	}
    }
    size = (size + page_size-1) & ~(page_size-1);


    s->buffer = mmap(0, size, prot, MAP_SHARED, s->fd, /*offset*/0);
    if (s->buffer == MAP_FAILED)
    {
	sperror(s, mmap);
	goto failure;
    }
    s->current = s->buffer;
    s->oflags = oflags;
    s->xflags = xflags;

    if ((xflags & STREAM_UNLINK) && unlink(filename) < 0)
    {
	sperror(s, unlink);
	goto failure;
    }
    if ((xflags & STREAM_CLOSE))
    {
	if (close(s->fd) < 0)
	{
	    sperror(s, close);
	    s->fd = -1;
	    goto failure;
	}
	s->fd = -1;
    }

    return s;

failure:
    if (s->fd >= 0)
	close(s->fd);
    xfree(s->name);
    xfree(s);
    return 0;
}


static int
unix_pull(stream_t *s)
{
    int n = read(s->fd, _stream_pull_buffer(s), _stream_used_len(s));
    if (n < 0 && errno != EINTR)
	sperror(s, read);
    return n;
}

static int
unix_push(stream_t *s)
{
    int64_t retry_sleep_ns = 10000000; /* 10 ms */
    int retries_remaining = 10;
    int n;
    while ((n = write(s->fd, _stream_push_buffer(s), _stream_used_len(s))) < 0)
    {
	if (errno == EAGAIN &&
	    (s->xflags & STREAM_RETRY_EAGAIN) &&
	    --retries_remaining > 0)
	{
	    struct timespec delay;
	    delay.tv_sec = retry_sleep_ns / 1000000000;
	    delay.tv_nsec = retry_sleep_ns % 1000000000;
	    nanosleep(&delay, NULL);
	    retry_sleep_ns *= 2;
	    continue;
	}
	if (errno != EINTR)
	    sperror(s, write);
	break;
    }
    return n;
}

static int
unix_seek(stream_t *s, uint64_t off)
{
    if (stream_flush(s) < 0)
	return -1;
    if (lseek64(s->fd, off, SEEK_SET) < 0)
    {
	sperror(s, lseek64);
	return -1;
    }
    return 0;
}

static int
unix_close(stream_t *s)
{
    int ret = 0;

    if (s->fd >= 0 && close(s->fd) < 0)
    {
	sperror(s, close);
	ret = -1;
    }

    xfree(s->buffer);
    xfree(s->name);
    xfree(s);

    return ret;
}

static stream_ops_t unix_ops =
{
    unix_pull,
    unix_push,
    unix_seek,
    unix_close
};

static stream_t *
stream_unix_dopen_1(const char *name, int fd, int oflags,
		    int xflags, unsigned long bsize, bool_t ourfd)
{
    stream_t *s;

    s = xmalloc(sizeof(stream_t));

    if (bsize == 0)
	bsize = 4096;	/* simulate dumb stdio defaults */
    s->name = xstrdup(name);
    s->buffer = xvalloc(bsize);
    s->bufsize = bsize;
    s->remain = ((oflags & O_ACCMODE) == O_WRONLY) ? bsize : 0;
    s->current = s->buffer;
    s->oflags = oflags;
    s->xflags = xflags;
    s->fd = fd;
    s->ops = &unix_ops;

    return s;
}

stream_t *
stream_unix_open(const char *filename, int oflags, int xflags, uint64_t bsize)
{
    int fd;
    stream_t *s;

    if ((fd = open(filename, oflags, 0600)) < 0)
    {
	perrorf("open(\"%s\")", filename);
	return 0;
    }
    if ((s = stream_unix_dopen_1(filename, fd, oflags, xflags, bsize, TRUE)) != 0)
    {
	if ((xflags & STREAM_UNLINK) && unlink(filename) < 0)
	{
	    sperror(s, unlink);
	    unix_close(s);
	    return 0;
	}
    }
    return s;
}

stream_t *
stream_unix_dopen(int fd, int oflags, int xflags, uint64_t bsize)
{
    char name[32];

    if (fd == STDIN_FILENO)
	strcpy(name, "-stdin-");
    else if (fd == STDOUT_FILENO)
	strcpy(name, "-stdout-");
    else
	snprintf(name, sizeof(name), "fd%d", fd);

    return stream_unix_dopen_1(name, fd, oflags, xflags, bsize, FALSE);
}



static int
client_pull(stream_t *s)
{
    fprintf(stderr, "client_pull called\n");
    return -1;
}

static int
client_seek(stream_t *s, uint64_t off)
{
    if (off >= s->bufsize)
	return -EINVAL;
    s->current = s->buffer + off;
    s->remain = s->bufsize - off;
    return 0;
}

static stream_ops_t client_ops =
{
    client_pull,
    unix_push,
    client_seek,
    unix_close
};

stream_t *
stream_client_open(const char *hostname, int protocol, int port,
		   int xflags, int bsize)
{
    struct hostent *he;
    struct sockaddr_in sin;
    int sock;
    stream_t *stream;

    if ((he = gethostbyname(hostname)) == 0)
    {
	herror(hostname);
	return 0;
    }
    if (he->h_addrtype != AF_INET)
    {
	fprintf(stderr, "%s: bad address type %d\n", hostname, he->h_addrtype);
	return 0;
    }

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    memcpy(&sin.sin_addr, he->h_addr, he->h_length);
    sin.sin_port = htons(port);

    assert(protocol == IPPROTO_TCP);
    sock = socket(PF_INET, SOCK_STREAM, protocol);
    if (sock < 0)
    {
	perrorf("socket");
	return 0;
    }

    if (connect(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
	perrorf("host %s", hostname);
	close(sock);
	return 0;
    }

    stream = stream_unix_dopen_1(he->h_name, sock, O_WRONLY, xflags, bsize, TRUE);
    stream->ops = &client_ops;
    return stream;
}


static int
server_push(stream_t *s)
{
    fprintf(stderr, "server_push called\n");
    return -1;
}

static int
server_seek(stream_t *s, uint64_t off)
{
    if (off >= s->bufsize)
	return -EINVAL;
    s->current = s->buffer + off;
    s->remain = s->bufsize - off;
    return 0;
}

static stream_ops_t server_ops =
{
    unix_pull,
    server_push,
    server_seek,
    unix_close
};

stream_t *stream_server_open(int protocol, int port, int xflags, int bsize)
{
    struct sockaddr_in sin;
    socklen_t slen = sizeof(sin);
    int rsock, sock;
    stream_t *stream;
    int reuse = 1;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    assert(protocol == IPPROTO_TCP);
    rsock = socket(PF_INET, SOCK_STREAM, protocol);
    if (rsock < 0)
    {
	perrorf("socket");
	return 0;
    }

    if (setsockopt(rsock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
	perrorf("setsockopt(SO_REUSEADDR)");
	close(rsock);
	return 0;
    }

    if (bind(rsock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
	perrorf("bind(%d)", (int)port);
	close(rsock);
	return 0;
    }

    if (listen(rsock, 5) < 0)
    {
	perrorf("listen");
	close(rsock);
	return 0;
    }

    sock = accept(rsock, (struct sockaddr *)&sin, &slen);
    if (sock < 0)
    {
	perrorf("accept");
	close(rsock);
	return 0;
    }
    close(rsock);

    stream = stream_unix_dopen_1(inet_ntoa(sin.sin_addr), sock, O_RDONLY, xflags, bsize, TRUE);
    stream->ops = &server_ops;
    return stream;
}

/* vim: set ts=8 sw=4 sts=4: */
