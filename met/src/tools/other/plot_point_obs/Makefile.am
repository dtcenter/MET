## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The program

bin_PROGRAMS    = plot_point_obs
plot_point_obs_SOURCES = plot_point_obs.cc
plot_point_obs_CPPFLAGS = ${MET_CPPFLAGS}
plot_point_obs_LDFLAGS = ${MET_LDFLAGS}
plot_point_obs_LDADD = -lvx_data2d_factory \
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_data2d_nc_met \
	-lvx_data2d_nc_pinterp \
	$(PYTHON_LIBS) \
	-lvx_data2d_nccf \
	-lvx_nc_obs \
	-lvx_nc_util \
	-lvx_data2d \
	-lvx_plot_util \
	-lvx_pxm \
	-lvx_color \
	-lvx_ps \
	-lvx_afm \
	-lvx_render \
	-lvx_pb_util \
	-lvx_nav \
	-lvx_gnomon \
	-lvx_gsl_prob \
	-lvx_grid \
	-lvx_config \
	-lvx_gsl_prob \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_log \
	-lm -lz -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas
