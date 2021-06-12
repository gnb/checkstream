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
#include "args.h"

void
args_init(args_state_t *st, int ac, char **av, const args_desc_t *desc)
{
    st->count = ac;
    st->vec = av;
    st->i = 1;
    st->j = 0;
    st->desc = desc;
    st->files = 0;
    st->nfiles = 0;
}

static void
args_add_file(args_state_t *st, const char *f)
{
    if (st->files == 0)
    {
    	st->files = (const char **)malloc(sizeof(const char *) * (st->count+1));
	if (st->files == 0)
	{
	    fprintf(stderr, "%s: out of memory allocating arguments\n",
	    	    st->vec[0]);
	    exit(1);
	}
    }
    assert(st->nfiles < st->count);
    st->files[st->nfiles++] = f;
    st->files[st->nfiles] = 0;
}

static const args_desc_t *
args_find_short(args_state_t *st, char c)
{
    const args_desc_t *d;

    for (d = st->desc ; d->shortname ; d++)
    {
    	if (d->shortname < _ARGS_NOSHORT_BASE && d->shortname == c)
	    return d;
    }
    return 0;
}

static const args_desc_t *
args_find_long(args_state_t *st, const char *p, int len)
{
    const args_desc_t *d;

    for (d = st->desc ; d->shortname ; d++)
    {
    	int l = strlen(d->longname);
	if (l == len && !memcmp(d->longname, p, l))
	    return d;
    }
    return 0;
}

int
args_next(args_state_t *st)
{
    const char *arg;
    const char *end;
    const args_desc_t *desc;

    st->value = 0;

    for (;;)
    {
	if (st->j)
	{
    	    /* continuing a previously started multiple-short option */
	    arg = &st->vec[st->i][st->j++];
	    if (*arg == '\0')
	    {
		st->i++;
		st->j = 0;
		continue;
	    }

	    desc = args_find_short(st, *arg);
	    if (desc == 0)
	    {
		fprintf(stderr, "%s: bad option -%c\n", st->vec[0], *arg);
		return -1;
	    }

	    if ((desc->flags & ARG_VALUED))
	    {
		if (arg[1] != '\0' ||
		    (st->value = st->vec[++st->i]) == 0)
		{
	    	    fprintf(stderr, "%s: option -%c requires a value\n",
		    	    st->vec[0], *arg);
		    return -1;
		}
		st->i++;
		st->j = 0;
	    }

	    return desc->shortname;
	}

	if (st->i >= st->count)
    	    return 0;

	arg = st->vec[st->i];
	if (arg[0] != '-')
	{
	    args_add_file(st, arg);
	    st->i++;
	    continue;
	}

	if (arg[1] != '-')
	{
	    /* start of multiple short options */
	    st->j = 1;
	    continue;
	}
	st->i++;

	/* single long form argument */

	/* lookup the option */
	arg += 2;
	end = strchr(arg, '=');
	desc = args_find_long(st, arg, (end ? end-arg : strlen(arg)));
	if (desc == 0)
	{
	    fprintf(stderr, "%s: bad option --%s\n", st->vec[0], arg);
	    return -1;
	}

    	/* handle option's value */
	if ((desc->flags & ARG_VALUED))
	{
	    st->value = (end ? end+1 : st->vec[st->i++]);
	    if (st->value == 0)
	    {
	    	fprintf(stderr, "%s: option --%s requires a value\n",
		    	st->vec[0], arg);
	    	return -1;
	    }
	}
	else
	{
	    if (end != 0 && *end != '\0')
	    {
	    	fprintf(stderr, "%s: option --%s does not take a value\n",
		    	st->vec[0], arg);
		return -1;
	    }
	}

	return desc->shortname;
    }
}

const char *
args_value(args_state_t *st)
{
    return st->value;
}

int
args_nfiles(args_state_t *st)
{
    return st->nfiles;
}

const char **
args_files(args_state_t *st)
{
    return st->files;
}

void
args_clear(args_state_t *st)
{
    if (st->files != 0)
    {
    	free(st->files);
	st->files = 0;
    }
}
