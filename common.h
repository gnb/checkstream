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
#ifndef _CHECKSTREAM_COMMON_H_
#define _CHECKSTREAM_COMMON_H_ 1

#ifdef HAVE_CONFIG_H
#include <config.h> /* for aspen tree autoconf defines */
#endif
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef sgi
#include <stdint.h>
#endif
#include <stdarg.h>
#include <unistd.h>
#include <memory.h>
#include <assert.h>
#include <netinet/in.h>
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

/*
 * This trick is straight from the gcc manual.  On non-gcc
 * compilers, define away the __attribute__ keyword; thus
 * we can use it to add gcc-specific compiler hints without
 * having to ifdef it everywhere.
 */
#ifndef __GNUC__
#define __attribute__(x)
#endif


extern int parse_length(const char *str, uint64_t *lengthp);
extern int parse_tag(const char *str, uint8_t *tagp);
extern int parse_protocol(const char *str, int *protp);
extern int parse_tcp_port(const char *str, uint16_t *portp);
/* compose and return a string in IEC standard notation e.g. 124KiB */
extern char *iec_sizestr(uint64_t sz, char *buf, int maxlen);
const char *tail(const char *);
extern void perrorf(const char *fmt, ...) __attribute__(( format(printf,1,2) ));
extern void fatal(const char *fmt, ...) __attribute__(( format(printf,1,2), noreturn ));
extern void error(const char *fmt, ...) __attribute__(( format(printf,1,2) ));
extern void message(const char *fmt, ...) __attribute__(( format(printf,1,2) ));
extern void *xmalloc(size_t sz) __attribute__(( malloc ));
extern char *xstrdup(const char *s) __attribute__(( malloc ));
extern void *xvalloc(size_t sz) __attribute__(( malloc ));
extern void xfree(void *x);

static inline uint16_t
finish_checksum(uint32_t sum)
{
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16) & 1;

    return htons(~sum);
}


static inline uint16_t
aligned_ip_checksum_3(const uint16_t *buf)
{
    uint32_t sum;

    sum  = (uint32_t)ntohs(buf[0]);
    sum += (uint32_t)ntohs(buf[1]);
    sum += (uint32_t)ntohs(buf[2]);

    return finish_checksum(sum);
}

static inline uint16_t
aligned_ip_checksum_4(const uint16_t *buf)
{
    uint32_t sum;

    sum  = (uint32_t)ntohs(buf[0]);
    sum += (uint32_t)ntohs(buf[1]);
    sum += (uint32_t)ntohs(buf[2]);
    sum += (uint32_t)ntohs(buf[3]);

    return finish_checksum(sum);
}

static inline uint16_t
aligned_ip_checksum_7(const uint16_t *buf)
{
    uint32_t sum;

    sum  = (uint32_t)ntohs(buf[0]);
    sum += (uint32_t)ntohs(buf[1]);
    sum += (uint32_t)ntohs(buf[2]);
    sum += (uint32_t)ntohs(buf[3]);
    sum += (uint32_t)ntohs(buf[4]);
    sum += (uint32_t)ntohs(buf[5]);
    sum += (uint32_t)ntohs(buf[6]);

    return finish_checksum(sum);
}

static inline uint16_t
aligned_ip_checksum_8(const uint16_t *buf)
{
    uint32_t sum;

    sum  = (uint32_t)ntohs(buf[0]);
    sum += (uint32_t)ntohs(buf[1]);
    sum += (uint32_t)ntohs(buf[2]);
    sum += (uint32_t)ntohs(buf[3]);
    sum += (uint32_t)ntohs(buf[4]);
    sum += (uint32_t)ntohs(buf[5]);
    sum += (uint32_t)ntohs(buf[6]);
    sum += (uint32_t)ntohs(buf[7]);

    return finish_checksum(sum);
}




typedef union
{
    /* sized for the largest possible record */
    uint32_t w32[4];
    uint16_t w16[8];
    uint8_t w8[16];
} record_t;

#define RECORD_SIZE		8
#define RECORD_MASK		0x7ULL

#define RECORD_SIZE_CREATOR	16
#define RECORD_MASK_CREATOR	0xfULL

#define DEFAULT_PORT	5000

/*
 * Time functions for dealing with timevals (microseconds
 * since the UNIX epoch) as 64 bit ints; arithmetic trivial.
 */
#define MICROSEC	1000000
extern uint64_t time_now(void);
#define time_seconds(t)			((uint32_t)((t) / MICROSEC))
#define time_microseconds(t)		((uint32_t)((t) % MICROSEC))
#define time_double(t)			((double)(t) / (double)MICROSEC)


/*
 * Encoding and decoding the creator information.  We encode a
 * millisecond resolution time and a PID in 64 bits by cheating:
 *
 * - encode pid in only 25 bits, let's hope test machines
 *   are not up long enough for this to matter.
 *
 * - encode milliseconds only, not microseconds, in 10 bits.
 *
 * - encode seconds using an epoch of 2007-01-01 00:00:00 UTC
 *   and only 29 bits (overflows at 2024-01-05 18:48:31 UTC).
 */
#define _CREATOR_PID_SIZE   25
#define _CREATOR_PID_SHIFT  0
#define _CREATOR_PID_MASK   ((1ULL<<_CREATOR_PID_SIZE)-1)

#define _CREATOR_MS_SIZE    10
#define _CREATOR_MS_SHIFT   (_CREATOR_PID_SHIFT+_CREATOR_PID_SIZE)
#define _CREATOR_MS_MASK    ((1ULL<<_CREATOR_MS_SIZE)-1)

#define _CREATOR_SEC_SIZE   29
#define _CREATOR_SEC_SHIFT  (_CREATOR_MS_SHIFT+_CREATOR_MS_SIZE)
#define _CREATOR_SEC_MASK   ((1ULL<<_CREATOR_SEC_SIZE)-1)

#define _CREATOR_EPOCH	    1167609600UL

static inline unsigned int creator_get_pid(uint64_t x)
{
    return (x >> _CREATOR_PID_SHIFT) & _CREATOR_PID_MASK;
}
static inline time_t creator_get_seconds(uint64_t x)
{
    return ((x >> _CREATOR_SEC_SHIFT) & _CREATOR_SEC_MASK) + _CREATOR_EPOCH;
}
static inline unsigned int creator_get_milliseconds(uint64_t x)
{
    return (x >> _CREATOR_MS_SHIFT) & _CREATOR_MS_MASK;
}
static inline uint64_t creator_make(unsigned int pid, const struct timeval *now)
{
    return ((pid & _CREATOR_PID_MASK) << _CREATOR_PID_SHIFT) |
	   (((now->tv_sec - _CREATOR_EPOCH) & _CREATOR_SEC_MASK) << _CREATOR_SEC_SHIFT) |
	   (((now->tv_usec / 1000) & _CREATOR_MS_MASK) << _CREATOR_MS_SHIFT);
}
extern const char *creator_to_timestamp_str(uint64_t creator);

extern void hexdump(uint64_t off, const void *buf, size_t len);

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
typedef unsigned int	bool_t;

/*
 * Pretend we have the POSIX 64b API even when we don't.
 *
 * On Cygwin it turns out the fstat() call is 64 bit clean
 * already, so they didn't feel the need to provide an
 * fstat64().
 */
#ifndef HAVE_STAT64
/* both the function and the struct */
#define stat64		stat
#endif
#ifndef HAVE_FSTAT64
#define fstat64		fstat
#endif
#ifndef HAVE_FTRUNCATE64
#define ftruncate64	ftruncate
#endif
#ifndef HAVE_LSEEK64
#define lseek64		lseek
#endif

#endif /* _CHECKSTREAM_COMMON_H_ */
