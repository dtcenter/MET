## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The program

bin_PROGRAMS = regrid_data_plane
regrid_data_plane_SOURCES = regrid_data_plane.cc
regrid_data_plane_CPPFLAGS = ${MET_CPPFLAGS}
regrid_data_plane_LDFLAGS = ${MET_LDFLAGS}
regrid_data_plane_LDADD = -lvx_statistics \
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
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	-lm -lnetcdf_c++4 -lnetcdf
