dnl Process this file with autoconf to produce a configure script.
dnl You may need the --localdir option to get the included .m4's
dnl
dnl checkstream - A filesystem and network corruption detector
dnl Copyright (c) 2004-2009 Silicon Graphics, Inc. All rights reserved.
dnl Copyright (c) 2010 Greg Banks <gnb@fmeh.org>
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
dnl

AC_INIT(checkstream, 1.0)
AC_CONFIG_SRCDIR(checkstream.c)
AM_INIT_AUTOMAKE()
AM_CONFIG_HEADER(config.h)

AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC

dnl Stuff for gettext
dnl ALL_LINGUAS="la"
dnl AM_GNU_GETTEXT

dnl Checks for header files.
dnl AC_HEADER_STDC
dnl AC_HEADER_SYS_WAIT
dnl AC_CHECK_HEADERS(malloc.h unistd.h memory.h)

dnl Checks for typedefs, structures, and compiler characteristics.

dnl Checks for library functions.
dnl AC_CHECK_FUNCS(putenv regcomp strchr)

dnl AC_SUBST(ALL_LINGUAS)
AC_SUBST(PACKAGE)
AC_SUBST(VERSION)
AC_OUTPUT([
    Makefile
])
dnl intl/Makefile po/Makefile.in
