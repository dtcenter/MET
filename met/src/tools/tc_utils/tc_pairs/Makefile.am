## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The program

bin_PROGRAMS    = tc_pairs
tc_pairs_SOURCES = tc_pairs.cc \
	acerr.f \
	clip_shif.f \
	tc_pairs_conf_info.cc
tc_pairs_CPPFLAGS = ${MET_CPPFLAGS}
tc_pairs_LDFLAGS = ${MET_LDFLAGS}
tc_pairs_LDADD = -lvx_config \
	-lvx_data2d_nc_met \
	-lvx_analysis_util \
	-lvx_tc_util \
	-lvx_nc_util \
	-lvx_grid \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm \
	-lnetcdf_c++4 -lnetcdf \
	$(FLIBS)

EXTRA_DIST = tc_pairs_conf_info.h \
	tc_pairs.h
