#
# Tests Copyright (c) 2021 Greg Banks <gnb@fmeh.org>
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

TESTS=              tbasic.sh ttag.sh tcreator.sh tmmap.sh tcheckerrors.sh \
                    tstdio.sh ttcp.sh c-unit-runner
EXTRA_DIST=         $(TESTS)
LOG_DRIVER=         env AM_TAP_AWK='$(AWK)' $(SHELL) $(top_srcdir)/autotools.aux.d/tap-driver.sh

check_PROGRAMS=             c-unit-runner

c_unit_runner_SOURCES=      c_unit_fw.c c_unit_fw.h \
                            tcommon.c
c_unit_runner_LDADD=        c_unit_tests.o $(top_srcdir)/common.o

c_unit_tests.c: $(c_unit_runner_OBJECTS) c-unit-processor.sh
	./c-unit-processor.sh -o $@ $(c_unit_runner_OBJECTS)

clean-local:
	-for t in $(TESTS) ; do tb=`basename $$t .sh` ; /bin/rm -f $$tb.*.dat ; done
