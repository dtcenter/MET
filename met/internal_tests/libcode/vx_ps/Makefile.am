## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# Test programs

noinst_PROGRAMS = test_ps

test_ps_SOURCES = test_ps.cc
test_ps_CPPFLAGS = ${MET_CPPFLAGS}
test_ps_LDFLAGS = -L. ${MET_LDFLAGS}
test_ps_LDADD = -lvx_ps \
	-lvx_render \
	-lvx_afm \
	-lvx_util \
	-lvx_log \
	-lz \
	-lm
