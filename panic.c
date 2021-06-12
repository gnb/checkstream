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
#include "panic.h"

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
#ifdef __linux__

#include <sys/klog.h>

static const char trigger_file[] = "/proc/sysrq-trigger";
static const char help_tag[] = "\n<6>SysRq : HELP :";
static char dump_char = '\0';

bool_t
panic_enable(void)
{
    FILE *fp;
    int n;
    bool_t help_found = FALSE;
    char *p, *q, buf[4096];

    if ((fp = fopen(trigger_file, "w")) == 0)
    {
	switch (errno)
	{
	case ENOENT:
	    error("%s does not exist. Perhaps your kernel been built "
		  "without CONFIG_MAGIC_SYSRQ?",
		  trigger_file);
	    return FALSE;
	case EACCES:
	    error("cannot write to %s.  This program needs to be "
	          "run as root for --kernel-dump-on-error to work",
		  trigger_file);
	    return FALSE;
	default:
	    perrorf("%s", trigger_file);
	    return FALSE;
	}
    }

    /*
     * Write a meaningless value to the trigger file
     * in order to trigger the help message, which we
     * then parse to see if 'd' will work later.  There
     * doesn't seem to be a better way to probe for
     * a specific functioning sysrq without invoking it.
     * If only /proc/sysrq-trigger had a .read method.
     */
    fputc('?', fp);
    fclose(fp);

    /* non-destructively read the last 4K of the buffer */
    n = klogctl(3, buf, sizeof(buf)-1);
    if (n < 0)
    {
	perrorf("klogctl(3)");
	return FALSE;
    }
    buf[n] = '\0';	/* JIC */

    for (p = buf ; (p = strstr(p, help_tag)) ; )
    {
	help_found = TRUE;
	/*
	 * Note: we want to look for the dump string in the *last*
	 * help message only, presumably the one we just caused,
	 * as the string's presence may be affected by loading
	 * or unloading kernel modules.
	 */
	dump_char = '\0';

	/* skip over the leadin tag */
	p += sizeof(help_tag)-1;

	/* temporarily nul-terminate the line */
	if ((q = strchr(p, '\n')))
	    *q = '\0';

	/* detect the help text for the 'd' or 'c' sysrqs */
	if (strstr(p, " Dump "))
	    dump_char = 'd';	    /* LKCD, e.g. on ia64 */
	else if (strstr(p, " Crashdump "))
	    dump_char = 'c';	    /* kdump, e.g. on x86_64 */

	/* undo the temporary nul-termination */
	if (q)
	{
	    *q = '\n';
	    p = q;
	}
    }

    if (!help_found)
    {
	error("couldn't find sysrq help text in dmesg, perhaps %s is broken?",
	      trigger_file);
	return FALSE;
    }
    if (!dump_char)
    {
	error("no kernel dump mechanism seems to be enabled on this machine, "
	      "perhaps you should install and configure kdump or lkcd?");
	return FALSE;
    }

    return TRUE;
}

void
panic(void)
{
    FILE *fp;

    if ((fp = fopen(trigger_file, "w")) == 0)
    {
	perrorf("%s", trigger_file);
	exit(1);
    }
    fputc(dump_char, fp);
    fclose(fp);

    fatal("wrote %s but that didn't cause a kernel panic", trigger_file);
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
#elif defined(sgi)

#include <sys/syssgi.h>


/*
 * On Irix, we need *both* a kernel tunable enable_upanic=1
 * and a flag which requires a privileged syssgi() to set.
 * It's as if they don't *want* us to panic the kernel!
 */

bool_t
panic_enable(void)
{
    int enabled;
    int retries = 0;

retry:
    if ((enabled = syssgi(SGI_UPANIC_SET, -1)) < 0)
    {
	switch (errno)
	{
	case EPERM:
	    error("this program needs to run as root (or to be given "
		  "CAP_DEVICE_MGT in some other way) in order for "
		  "--kernel-dump-on-error to work");
	    return FALSE;
	case EACCES:
	    if (!retries)
	    {
		message("setting the enable_upanic kernel tunable");
		system("echo y | systune -r enable_upanic 1");
	    }
	    else
	    {
		error("this machine needs the enable_upanic tunable "
		      "set in order for --kernel-dump-on-error to work, "
		      "but we don't seem to be able to set it using the "
		      "command \"echo y | systune -r enable_upanic 1\"");
	    }
	    retries++;
	    goto retry;
	default:
	    perrorf("cannot query enable_upanic flag, unexpected error");
	    return FALSE;
	}
    }

    if (!enabled)
    {
	message("enabling kernel upanic flag");
	if (syssgi(SGI_UPANIC_SET, 1) < 0)
	{
	    perrorf("syssgi(SGI_UPANIC_SET,1)");
	    return FALSE;
	}
    }

    return TRUE;
}

void
panic(void)
{
    if (syssgi(SGI_UPANIC) < 0)
    {
	perrorf("syssgi(SGI_UPANIC)");
	exit(1);
    }
    else
    {
	fatal("odd, syssgi(SGI_UPANIC) returned success!");
    }
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
#else

/* default implementation: do nothing */

bool_t
panic_enable(void)
{
    error("this platform does not support --kernel-dump-on-error");
    return FALSE;
}

void
panic(void)
{
    fatal("cannot cause kernel panic on this platform");
}

#endif
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
