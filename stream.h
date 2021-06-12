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
#include <sys/fcntl.h>

typedef struct stream stream_t;
typedef struct stream_ops stream_ops_t;

struct stream
{
    char *name;			/* used for error reporting only */
    unsigned char *current;
    unsigned char *buffer;
    int64_t remain;
    uint64_t bufsize;
    int oflags;
#define STREAM_UNLINK	(1<<0)
#define STREAM_CLOSE	(1<<1)
#define STREAM_NOMSYNC	(1<<2)
#define STREAM_RETRY_EAGAIN	(1<<3)
    int xflags;
    int fd;
    struct stream_ops *ops;
    struct
    {
	uint64_t nblocks;   	/* number of blocks read or written */
	uint64_t nbytes;
    } stats;
};

struct stream_ops
{
    int (*pull)(stream_t *);	/* read a new buffer from the underlying file */
    int (*push)(stream_t *);	/* write a full buffer to the underlying file */
    int (*seek)(stream_t *, uint64_t offset);
    int (*close)(stream_t *);
};

extern stream_t *stream_mmap_open(const char *filename, int oflags,
				  int xflags, uint64_t bsize);
extern stream_t *stream_unix_open(const char *filename, int oflags,
				  int xflags, uint64_t bsize);
extern stream_t *stream_unix_dopen(int fd, int oflags, int xflags, uint64_t bsize);
extern stream_t *stream_client_open(const char *hostname, int protocol,
				    int port, int xflags, int bsize);
extern stream_t *stream_server_open(int protocol, int port,
				    int xflags, int bsize);
extern int stream_read(stream_t *, char *buf, int len);
extern int stream_write(stream_t *, char *buf, int len);
extern int stream_flush(stream_t *);
extern int stream_seek(stream_t *, uint64_t);
extern int stream_close(stream_t *);

/* internal functions */
extern int stream_push(stream_t *s);
extern int stream_pull(stream_t *s);

static inline char *
_stream_inline_bytes(stream_t *s, int len)
{
    char *p = (char *)s->current;
    s->current += len;
    s->remain -= len;
    return p;
}

static inline char *
stream_inline_write(stream_t *s, int len)
{
    if (len > s->bufsize)
	return 0;
    if (s->remain < len)
    {
	if (stream_push(s))
	    return 0;
    }
    return _stream_inline_bytes(s, len);
}

static inline const char *
stream_inline_read(stream_t *s, int len)
{
    if (len > s->bufsize)
	return 0;
    if (s->remain < len)
    {
	if (stream_pull(s) < 0 || s->remain < len)
	    return 0;
    }
    return _stream_inline_bytes(s, len);
}



