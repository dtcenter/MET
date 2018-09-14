## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The program

bin_PROGRAMS    = plot_data_plane
plot_data_plane_SOURCES = plot_data_plane.cc
plot_data_plane_CPPFLAGS = ${MET_CPPFLAGS}
plot_data_plane_LDFLAGS = ${MET_LDFLAGS}
plot_data_plane_LDADD = -lvx_data2d_factory \
	-lvx_data2d_nc_met \
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_data2d_nc_pinterp \
	$(PYTHON_LIBS) \
	-lvx_data2d_nccf \
	-lvx_nc_util \
	-lvx_data2d \
	-lvx_plot_util \
	-lvx_render \
	-lvx_pxm \
	-lvx_color \
	-lvx_ps \
	-lvx_afm \
	-lvx_nav \
	-lvx_gnomon \
	-lvx_grid \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm -lz -lnetcdf_c++4 -lnetcdf
