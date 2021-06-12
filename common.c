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
#include <sys/time.h>
#include <ctype.h>

extern const char *argv0;

int
parse_length(const char *str, uint64_t *lengthp)
{
    char *end = 0;

    if (str == 0 || *str == '\0')
    	return 0;

    *lengthp = strtoull(str, &end, 0);
    if (end == 0 || end == str)
    	return 0;   /* failed */

    switch (*end)
    {
    case '\0':
    	return 1;
    case 'b': case 'B':
	/* "bb" or "BB" suffix: length in basic blocks (512 byte units) */
	if (end[1] != end[0])
	    return 0;
	end++;
	*lengthp <<= 9;
	break;
    case 'k': case 'K':
    	*lengthp <<= 10;
	break;
    case 'm': case 'M':
    	*lengthp <<= 20;
	break;
    case 'g': case 'G':
    	*lengthp <<= 30;
	break;
    case 't': case 'T':
    	*lengthp <<= 40;
	break;
    default:
	return 0;
    }
    return (end[1] == '\0');
}

int
parse_tag(const char *str, uint8_t *tagp)
{
    char *end = 0;
    unsigned long v;

    if (str == 0 || *str == '\0')
	return 0;

    v = strtoul(str, &end, 0);
    if (end == 0 || *end != '\0' || end == str)
	return 0;   /* failed */
    if (v > 255)
	return 0;
    *tagp = (uint8_t)v;
    return 1;
}

int
parse_protocol(const char *str, int *protp)
{
    if (str == 0 || *str == '\0')
	return 0;
    if (!strcmp(str, "tcp"))
    {
	*protp = IPPROTO_TCP;
	return 1;
    }
    return 0;
}

int
parse_tcp_port(const char *str, uint16_t *portp)
{
    char *end = 0;
    unsigned long v;

    if (str == 0 || *str == '\0')
	return 0;

    v = strtoul(str, &end, 0);
    if (end == 0 || *end != '\0' || end == str)
	return 0;   /* failed */
    if (v == 0 || v > 65535)
	return 0;
    *portp = (uint16_t)v;
    return 1;
}

char *
iec_sizestr(uint64_t sz, char *buf, int maxlen)
{
    int shift = 0;
    static const char * const units[] = {
    	"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB"
    };
    static char defbuf[32];

    if (buf == 0)
    {
    	buf = defbuf;
	maxlen = sizeof(defbuf);
    }

    while (sz >= 1024)
    {
	sz >>= 10;
    	shift += 10;
    }
    snprintf(buf, maxlen, "%llu %s", (unsigned long long)sz, units[shift/10]);
    return buf;
}

uint64_t
time_now(void)
{
     struct timeval now;

     memset(&now, 0, sizeof(now));
     gettimeofday(&now, 0);
     return (uint64_t)now.tv_sec * MICROSEC + (uint64_t)now.tv_usec;
}

const char *
tail(const char *path)
{
    const char *t = strrchr(path, '/');
    return (t == 0 ? path : ++t);
}

void
perrorf(const char *fmt, ...)
{
    va_list args;
    int e = errno;

    va_start(args, fmt);
    fprintf(stderr, "%s[%d]: ", argv0, (int)getpid());
    vfprintf(stderr, fmt, args);
    fprintf(stderr, ": %d(%s)\n", e, strerror(e));
    fflush(stderr); /* JIC */
    va_end(args);
}

static void
verror(const char *fmt, va_list args)
{
    fprintf(stderr, "%s[%d]: ", argv0, (int)getpid());
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);
    fflush(stderr); /* JIC */
}

void
fatal(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    verror(fmt, args);
    va_end(args);

    exit(1);
}

void
error(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    verror(fmt, args);
    va_end(args);
}

void
message(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    verror(fmt, args);
    va_end(args);
}

static void _oom(void) __attribute__(( noreturn ));

static void
_oom(void)
{
    static const char msg[] = ": failed to allocate memory, exiting\n";
    fflush(stderr);
    /* not using fprintf() to avoid it needing to allocate memory */
    write(2, argv0, strlen(argv0));
    write(2, msg, sizeof(msg)-1);
    exit(1);
}

void *
xmalloc(size_t sz)
{
    void *x = malloc(sz);
    if (!x)
	_oom();
    memset(x, 0, sz);
    return x;
}

char *
xstrdup(const char *s)
{
    char *x;

    if (!s)
	return 0;
    if (!(x = strdup(s)))
	_oom();
    return x;
}

void *
xvalloc(size_t sz)
{
    void *x = valloc(sz);
    if (!x)
	_oom();
    return x;
}

void
xfree(void *x)
{
    if (x)
	free(x);
}


const char *
creator_to_timestamp_str(uint64_t creator)
{
    time_t sec = creator_get_seconds(creator);
    unsigned int millisec = creator_get_milliseconds(creator);
    const struct tm *tm = localtime(&sec);
    static char buf[35];

#ifdef __GLIBC__
    snprintf(buf, sizeof(buf)-1, "%04d/%02d/%02d %02d:%02d:%02d.%03d %s",
	    tm->tm_year+1900,
	    tm->tm_mon+1,
	    tm->tm_mday,
	    tm->tm_hour,
	    tm->tm_min,
	    tm->tm_sec,
	    millisec,
	    tm->tm_zone);
#else
    snprintf(buf, sizeof(buf)-1, "%04d/%02d/%02d %02d:%02d:%02d.%03d",
	    tm->tm_year+1900,
	    tm->tm_mon+1,
	    tm->tm_mday,
	    tm->tm_hour,
	    tm->tm_min,
	    tm->tm_sec,
	    millisec);
#endif

    return buf;
}

void
hexdump(uint64_t off, const void *buf, size_t len)
{
    unsigned int i;
    const unsigned char *ubuf = buf;

    fprintf(stderr, "%s: 0x%llx:", argv0, (unsigned long long)off);
    for (i = 0 ; i < len ; i++)
	fprintf(stderr, " %02x", (unsigned)ubuf[i]);
    fputs("    ", stderr);
    for (i = 0 ; i < len ; i++)
    {
	unsigned int c = (unsigned)ubuf[i];
	fprintf(stderr, "%c", (isprint(c) ? c : '.'));
    }
    fputc('\n', stderr);
}

