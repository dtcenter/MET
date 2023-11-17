## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The program

bin_PROGRAMS    = grid_stat
grid_stat_SOURCES = grid_stat.cc \
	grid_stat_conf_info.cc
grid_stat_CPPFLAGS = ${MET_CPPFLAGS}
grid_stat_LDFLAGS = ${MET_LDFLAGS}
grid_stat_LDADD = -lvx_stat_out \
	-lvx_statistics \
	-lvx_shapedata \
	-lvx_gsl_prob \
	-lvx_analysis_util \
	-lvx_data2d_factory \
	-lvx_data2d_nc_met \
	-lvx_data2d_grib \
	$(GRIB2_MET_LIBS) \
	-lvx_data2d_nc_pinterp \
	-lvx_data2d_nccf \
	$(UGRID_MET_LIBS) \
	$(PYTHON_MET_LIBS) \
	-lvx_statistics \
	-lvx_data2d \
	-lvx_seeps \
	-lvx_nc_util \
	-lvx_regrid \
	-lvx_grid \
	-lvx_geodesy \
	-lvx_seeps \
	-lvx_config \
	-lvx_cal \
	-lvx_util_math \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	$(GRIB2_DEP_LIBS) $(UGRID_DEP_LIBS) $(PYTHON_DEP_LIBS) \
	-lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas

EXTRA_DIST = grid_stat.h \
	grid_stat_conf_info.h
