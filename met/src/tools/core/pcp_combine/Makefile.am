## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The program

bin_PROGRAMS    = pcp_combine
pcp_combine_SOURCES = pcp_combine.cc
pcp_combine_CPPFLAGS = ${MET_CPPFLAGS}
pcp_combine_LDFLAGS = ${MET_LDFLAGS}
pcp_combine_LDADD = -lvx_data2d \
	-lvx_data2d_factory \
	-lvx_data2d_nc_met \
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_nc_util \
	-lvx_data2d \
	-lvx_data2d_nc_pinterp \
	-lvx_data2d_nccf \
	-lvx_grid \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lvx_color \
	-lm -lnetcdf_c++4 -lnetcdf
