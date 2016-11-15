## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# Test programs

noinst_PROGRAMS = test_ra_dec

test_ra_dec_SOURCES = test_ra_dec.cc
test_ra_dec_CPPFLAGS = ${MET_CPPFLAGS} -I../../../src/libcode/vx_solar
test_ra_dec_LDFLAGS = -L. -L../../../src/libcode/vx_solar ${MET_LDFLAGS} 
test_ra_dec_LDADD =  \
	-lvx_math \
	-lvx_solar \
	-lvx_log \
	-lvx_cal \
	-lm 
