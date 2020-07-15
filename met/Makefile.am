## Makefile.am -- Process this file with automake to produce Makefile.in
## Copyright (C) 2000, 2006 Gary V. Vaughan
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
## MA 02110-1301, USA.

ACLOCAL_AMFLAGS		= -I m4

SUBDIRS 		= data src scripts/Rscripts scripts/python

if ENABLE_DEVELOPMENT
  SUBDIRS += internal_tests
endif

MAINTAINERCLEANFILES 	= \
	Makefile.in \
	aclocal.m4 \
	configure \
	config.h.in

# Cleanup for the distribution
#   - Remove any SVN files
#   - Remove lex/yacc source files
#   - Run the check for copyright utility

dist-hook:
	-rm -rf `find ${distdir} -name .svn`
	-rm -rf `find ${distdir} -name "*.ll"`
	-rm -rf `find ${distdir} -name "*.yy"`
	${top_srcdir}/src/tools/dev_utils/chk4copyright -dir ${distdir} -notice ${top_srcdir}/data/copyright_notice.txt
	-rm -rf ${distdir}/internal_tests
	-rm -rf ${distdir}/src/tools/dev_utils
	-rm -rf ${distdir}/ylwrap

# Run the non-standard test suite

test:
	cd scripts; make

.PHONY: test

# Other files/directories that need to be included
# in the distribution

EXTRA_DIST = \
	docs \
	out \
	scripts \
	Make-include


