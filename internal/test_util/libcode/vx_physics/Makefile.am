## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# Test programs

noinst_PROGRAMS = test_thermo

test_thermo_SOURCES = test_thermo.cc
test_thermo_CPPFLAGS = ${MET_CPPFLAGS} -I../../../src/libcode/vx_physics
test_thermo_LDFLAGS = -L. ${MET_LDFLAGS} -L../../../src/libcode/vx_physics
test_thermo_LDADD = \
	-lvx_config \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_log \
	-lvx_physics
