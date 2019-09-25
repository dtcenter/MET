MAINTAINERCLEANFILES	= Makefile.in

include ${top_srcdir}/Make-include

bin_PROGRAMS = grid_diag
grid_diag_SOURCES = grid_diag.cc \
	grid_diag_conf_info.cc
grid_diag_CPPFLAGS = ${MET_CPPFLAGS}
grid_diag_LDFLAGS = ${MET_LDFLAGS}
grid_diag_LDADD = \
	-lvx_series_data \
	-lvx_data2d_factory \
	-lvx_data2d_nc_met \
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_data2d_nc_pinterp \
	-lvx_data2d_nccf \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_grid \
	-lvx_config \
	-lvx_config \
	-lvx_log \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	$(PYTHON_LIBS) \
        -lm -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas -lz

EXTRA_DIST = grid_diag_conf_info.h \
	grid_diag.h
