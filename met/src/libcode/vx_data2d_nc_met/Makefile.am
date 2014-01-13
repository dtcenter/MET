## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Test programs

noinst_PROGRAMS = test_read_nc_met

test_read_nc_met_SOURCES = test_read_nc_met.cc
test_read_nc_met_CPPFLAGS = -I${includedir}
test_read_nc_met_LDFLAGS = -L${libdir}
test_read_nc_met_LDADD = -lvx_data2d_nc_met \
	-lvx_nc_util \
	-lvx_data2d \
	-lvx_grid \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lnetcdf_c++ -lnetcdf

# The library itself

lib_LIBRARIES = libvx_data2d_nc_met.a
libvx_data2d_nc_met_a_SOURCES = get_met_grid.cc \
              met_file.cc \
              var_info_nc_met.cc \
              data2d_nc_met.cc
libvx_data2d_nc_met_a_CPPFLAGS = -I${includedir}

include_HEADERS = get_met_grid.h \
              met_file.h \
              var_info_nc_met.h \
              data2d_nc_met.h \
              vx_data2d_nc_met.h
