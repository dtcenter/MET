## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The programs

bin_PROGRAMS	= make_mapfiles

make_mapfiles_SOURCES = make_mapfiles.cc

make_mapfiles_CPPFLAGS = ${MET_CPPFLAGS}
make_mapfiles_LDFLAGS = ${MET_LDFLAGS}
make_mapfiles_LDADD = -lvx_config \
	-lvx_gis \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm

