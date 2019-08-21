MAINTAINERCLEANFILES	= Makefile.in

include ${top_srcdir}/Make-include

bin_PROGRAMS = grid_diag
grid_diag_SOURCES = grid_diag.cc \
	grid_diag_conf_info.cc
grid_diag_CPPFLAGS = ${MET_CPPFLAGS}
grid_diag_LDFLAGS = ${MET_LDFLAGS}
grid_diag_LDADD = \
	-lvx_config \
	-lvx_log \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_nc_util \
        -lm -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas -lz

EXTRA_DIST = grid_diag_conf_info.h \
	grid_diag.h
