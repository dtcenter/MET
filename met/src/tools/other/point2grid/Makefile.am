## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The program

bin_PROGRAMS = point2grid
point2grid_SOURCES = point2grid.cc \
	point2grid_conf_info.h point2grid_conf_info.cc
point2grid_CPPFLAGS = ${MET_CPPFLAGS}
point2grid_LDFLAGS = ${MET_LDFLAGS}
point2grid_LDADD = -lvx_statistics \
	-lvx_data2d_factory \
	-lvx_data2d_nc_met \
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_data2d_nc_pinterp \
	$(PYTHON_LIBS) \
	-lvx_data2d_nccf \
	-lvx_nc_util \
	-lvx_data2d \
	-lvx_gnomon \
	-lvx_regrid \
	-lvx_grid \
	-lvx_config \
	-lvx_gsl_prob \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	-lm -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas
