## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The program

bin_PROGRAMS    = gen_vx_mask

gen_vx_mask_SOURCES = gen_vx_mask.cc  \
                      grid_closed_poly.cc
gen_vx_mask_CPPFLAGS = ${MET_CPPFLAGS}
gen_vx_mask_LDFLAGS = ${MET_LDFLAGS}
gen_vx_mask_LDADD = -lvx_shapedata \
	-lvx_analysis_util \
	-lvx_data2d_factory \
	-lvx_data2d_nccf \
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_data2d_nc_met \
	-lvx_data2d_nc_pinterp \
	$(PYTHON_LIBS) \
	-lvx_gis  \
	-lvx_nc_util \
	-lvx_data2d \
	-lvx_render \
	-lvx_plot_util \
	-lvx_pxm \
	-lvx_color \
	-lvx_ps \
	-lvx_afm \
	-lvx_pb_util \
	-lvx_nav \
	-lvx_gnomon \
	-lvx_stat_out \
	-lvx_statistics \
	-lvx_gsl_prob \
	-lvx_grid \
	-lvx_config \
	-lvx_gsl_prob \
	-lvx_util \
	-lvx_math \
	-lvx_solar \
	-lvx_cal \
	-lvx_log \
	-lm -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas

EXTRA_DIST = gen_vx_mask.h \
	grid_closed_poly.h
