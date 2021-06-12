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

typedef struct
{
    int shortname;
    const char *longname;
    int flags;
#define ARG_UNVALUED	    0
#define ARG_VALUED	    (1<<0)
} args_desc_t;

#define _ARGS_NOSHORT_BASE  0x100
#define ARGS_NOSHORT(x)	    (_ARGS_NOSHORT_BASE+(x))

typedef struct
{
    int count;
    char **vec;
    int i;
    int j;
    const char *value;
    const args_desc_t *desc;
    const char **files;
    int nfiles;
} args_state_t;

extern void args_init(args_state_t *st, int ac, char **av, const args_desc_t *desc);
/* parse next argument, return short name or 0 for end or -1 for error */
extern int args_next(args_state_t *st);
const char *args_value(args_state_t *st);
extern int args_nfiles(args_state_t *st);
extern const char **args_files(args_state_t *st);
extern void args_clear(args_state_t *st);
