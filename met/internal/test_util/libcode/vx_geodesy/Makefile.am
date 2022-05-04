## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# Test programs

noinst_PROGRAMS = test_spheroid

test_spheroid_SOURCES = test_spheroid.cc
test_spheroid_CPPFLAGS = ${MET_CPPFLAGS}
test_spheroid_LDFLAGS = -L. ${MET_LDFLAGS}
test_spheroid_LDADD = -lvx_geodesy  \
	-lvx_util     \
	-lvx_log      \
	-lm

