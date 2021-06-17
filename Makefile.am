#
# Copyright (c) 2004-2009 Silicon Graphics, Inc. All rights reserved.
#         By Greg Banks <gnb@sgi.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

SUBDIRS=        tests
bin_PROGRAMS=	genstream checkstream

COMMON= common.c common.h stream.c stream.h

genstream_SOURCES=	genstream.c $(COMMON)

checkstream_SOURCES=	checkstream.c panic.c panic.h $(COMMON)

AM_CPPFLAGS =	-D_LARGEFILE64_SOURCE

man_MANS=	genstream.1 checkstream.1
EXTRA_DIST=	$(man_MANS)

coverage:
	$(MAKE) clean
	$(MAKE) AM_CFLAGS="--coverage" AM_LDFLAGS="--coverage" all
	lcov -c -i -d $(srcdir) --include '$(abs_srcdir)/*.c' -o tests/base.info
	$(MAKE) check
	lcov -c -d $(srcdir) --include '$(abs_srcdir)/*.c' -o tests/tests.info
	lcov -a tests/base.info -a tests/tests.info -o tests/total.info
	lcov --summary tests/total.info
	mkdir -p build/report/coverage
	genhtml -o build/report/coverage tests/total.info
