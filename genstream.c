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
#include "args.h"


const char *argv0;
uint8_t tag = 0x0;
bool_t tag_flag = FALSE;
bool_t creator_flag = FALSE;

volatile int signalled = 0;

static void
handle_sig(int sig)
{
    signalled++;
}


/*
 * Emits a stream of data which is a sequence of 8-byte records
 * comprising three fields: a 5-byte byte offset from the start of
 * the stream, a 1 byte tag, and a 2-byte checksum.  The checksum is
 * calculated so that the UDP checksum of every 8-byte record is zero.
 * The offset field is in network byte order (of course).  The tag
 * is used to distinguish multiple streams; the default value 0
 * is chosen so that it happens to match streams created by previous
 * versions of genstream.
 */

static void
generate_stream(stream_t *st, uint64_t length, uint64_t seek)
{
    uint64_t i, off;
    size_t record_size = (creator_flag ? RECORD_SIZE_CREATOR : RECORD_SIZE);
    uint64_t record_mask = (creator_flag ? RECORD_MASK_CREATOR : RECORD_MASK);
    record_t *rec;
    uint32_t creator_bits[2];

    if (seek && stream_seek(st, seek) < 0)
	fatal("%s: stream_seek failed", st->name);

    if (creator_flag)
    {
	struct timeval now;
	uint64_t creator;

	gettimeofday(&now, 0);
	creator = creator_make(getpid(), &now);
	fprintf(stderr, "%s: pid %u started at %s\n",
		argv0,
		creator_get_pid(creator),
		creator_to_timestamp_str(creator));
	creator_bits[0] = htonl(creator >> 32);
	creator_bits[1] = htonl(creator & 0xffffffffULL);
    }

    length &= ~record_mask;	/* has to be multiple of record size */

    for (i = 0 ; !signalled && i < length ; i += record_size)
    {
	off = i + seek;
	rec = (record_t *)stream_inline_write(st, record_size);
	if (rec == 0)
	{
	    if (signalled)
	    	break;
	    fatal("%s: stream_inline_write failed", st->name);
	}
	if (tag_flag)
	{
	    rec->w8[2] = tag;
	    rec->w8[3] = (off >> 32) & 0xff;
	}
	else
	{
	    rec->w16[1] = htons((off >> 32) & 0xffff);
	}
	rec->w32[1] = htonl(off & 0xffffffff);
	if (creator_flag)
	{
	    rec->w32[2] = creator_bits[0];
	    rec->w32[3] = creator_bits[1];
	    rec->w16[0] = aligned_ip_checksum_7(&rec->w16[1]);
	}
	else
	{
	    rec->w16[0] = aligned_ip_checksum_3(&rec->w16[1]);
	}
    }
}

static const char usage_str[] =
"Usage: genstream [options] SIZE file\n"
"       genstream [options] SIZE > file\n"
"       genstream [options] --protocol=tcp size host\n"
"options:\n"
"    -S, --sync     	    	open files with O_SYNC\n"
"    -D, --direct     	    	open files with O_DIRECT\n"
"    -M, --mmap     	    	use mmap instead of the read() syscall\n"
"    -b SIZE, --blocksize=SIZE 	block size for read() syscalls\n"
"    -s SIZE, --seek=SIZE   	seek to the given offset before writing\n"
"    -t, --no-truncate   	don't truncate the output file (if file given)\n"
"    --retry-eagain       	handle EAGAIN errors on write by retrying with backoff\n"
"    -u, --unlink		unlink file after opening\n"
"    -c, --close		close file descriptor after mmaping\n"
"    -T NUM, --tag=N     	generate given 8-bit tag value in stream\n"
"    -C, --record-creator       record start time & pid in stream\n"
"SIZE arguments may be specified as nnn[KMGT]\n"
;

static void
usage(void)
{
    fputs(usage_str, stderr);
    fflush(stderr);	/* JIC */
    exit(1);
}

static const args_desc_t arg_desc[] =
{
    {'C', "creator",    ARG_UNVALUED},
    {'S', "sync", 	ARG_UNVALUED},
    {'D', "direct", 	ARG_UNVALUED},
    {'M', "mmap", 	ARG_UNVALUED},
    {'b', "blocksize", 	ARG_VALUED},
    {'c', "close", 	ARG_UNVALUED},
    {'s', "seek",   	ARG_VALUED},
    {'t', "no-truncate",ARG_UNVALUED},
    {'T', "tag",	ARG_VALUED},
    {'u', "unlink",	ARG_UNVALUED},
    {'P', "protocol",	ARG_VALUED},
    {'p', "port",	ARG_VALUED},
    {'V', "version",	ARG_UNVALUED},
    {ARGS_NOSHORT(0), "retry-eagain", ARG_UNVALUED},
    {0, 0, 0}
};

int
main(int argc, char **argv)
{
    args_state_t as;
    uint64_t length;
    const char *filename = 0;	/* or hostname for TCP */
    bool_t mmap_flag = FALSE;
    int oflags = O_WRONLY|O_CREAT;
    int otrunc = O_TRUNC;
    int xflags = 0;
    uint64_t bsize = 0;
    stream_t *stream;
    const char **files;
    uint64_t seek = 0;
    int c;
    int protocol = 0;
    uint16_t port = 0;

#ifdef O_LARGEFILE
    oflags |= O_LARGEFILE;
#endif

    argv0 = tail(argv[0]);
    args_init(&as, argc, argv, arg_desc);

    while ((c = args_next(&as)))
    {
	if (c < 0)
	    usage();

	switch (c)
	{
	case 'S':
	    oflags |= O_SYNC;
	    break;

	case 'C':
	    creator_flag = TRUE;
	    break;

        case 'D':
#ifdef O_DIRECT
	    oflags |= O_DIRECT;
#else
	    fatal("O_DIRECT not implemented on this platform, failing");
#endif /*O_DIRECT*/
	    break;

	case 'M':
	    mmap_flag = TRUE;
	    break;

	case 'b':
	    if (!parse_length(args_value(&as), &bsize))
		fatal("cannot parse blocksize \"%s\"", args_value(&as));
	    break;

	case 'c':
	    xflags |= STREAM_CLOSE;
	    break;

	case 's':
	    if (!parse_length(args_value(&as), &seek))
		fatal("cannot parse seek \"%s\"", args_value(&as));
	    break;

	case 't':
	    otrunc = 0;
	    break;

	case 'u':
	    xflags |= STREAM_UNLINK;
	    break;

	case 'T':
	    if (!parse_tag(args_value(&as), &tag))
		fatal("cannot parse tag \"%s\"", args_value(&as));
	    tag_flag = TRUE;
	    break;

	case 'P':
	    if (!parse_protocol(args_value(&as), &protocol))
		fatal("cannot parse protocol \"%s\"", args_value(&as));
	    break;

	case 'p':
	    if (!parse_tcp_port(args_value(&as), &port))
		fatal("cannot parse port \"%s\"", args_value(&as));
	    break;

	case 'V':
	    fputs("genstream version " VERSION "\n", stdout);
	    fflush(stdout);
	    exit(0);

	case ARGS_NOSHORT(0): // retry-eagain
	    xflags |= STREAM_RETRY_EAGAIN;
	    break;
	}
    }
    oflags |= otrunc;

    files = args_files(&as);
    switch (args_nfiles(&as))
    {
    case 2:
	filename = files[1];
	/* fall through */
    case 1:
	if (!parse_length(files[0], &length))
	    fatal("cannot parse length \"%s\"", files[0]);
	break;
    default:
	usage();
    }
    args_clear(&as);

    if (port && !protocol)
	fatal("must specify --protocol=tcp with --port");

    if (filename == 0)
    {
	if (mmap_flag)
	    fatal("must specify a filename with --mmap option");
	if ((xflags & STREAM_UNLINK))
	    fatal("must specify a filename with --unlink option");
	if (oflags & O_SYNC)
	    fatal("must specify a filename with --sync option");
	if (protocol)
	    fatal("must specify a hostname with --protocol=tcp option");
    }
    if (protocol)
    {
	if (mmap_flag)
	    fatal("cannot use --protocol=tcp with --mmap option");
	if ((xflags & STREAM_UNLINK))
	    fatal("cannot use --protocol=tcp with --unlink option");
	if (oflags & O_SYNC)
	    fatal("cannot use --protocol=tcp with --sync option");
	if (!port)
	    port = DEFAULT_PORT;
    }

    if ((xflags & STREAM_CLOSE) && !mmap_flag)
	fatal("--close is not useful except with --mmap");

    /* ensure stats are dumped when we get a sigint */
    signal(SIGINT, handle_sig);
    signal(SIGTERM, handle_sig);

    if (protocol)
	stream = stream_client_open(filename, protocol, port, xflags, bsize);
    else if (filename == 0)
	stream = stream_unix_dopen(fileno(stdout), oflags, xflags, bsize);
    else if (mmap_flag)
	stream = stream_mmap_open(filename, oflags, xflags, length+seek);
    else
	stream = stream_unix_open(filename, oflags, xflags, bsize);

    if (stream == 0)
	exit(1);    /* error message printed at lower level in stream.c */


    generate_stream(stream, length, seek);
    stream_flush(stream);

    /* used for determining how many blocks have been read or written */
    fprintf(stderr, "%s: %s %llu blocks %llu bytes\n",
	    argv0,
	    stream->name,
	    (unsigned long long)stream->stats.nblocks,
	    (unsigned long long)stream->stats.nbytes);
    fflush(stderr); /* JIC */

    stream_close(stream);

    return !!signalled;
}

/* vim: set ts=8 sw=4 sts=4: */
