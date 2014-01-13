## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Test programs

noinst_PROGRAMS = test_is_grib \
	test_factory

test_is_grib_SOURCES = test_is_grib.cc
test_is_grib_CPPFLAGS = -I${includedir}
test_is_grib_LDFLAGS = -L${libdir}
test_is_grib_LDADD = -lvx_data2d_factory \
	-lvx_cal \
	-lvx_config \
	-lvx_util \
	-lvx_log

test_factory_SOURCES = test_factory.cc
test_factory_CPPFLAGS = -I${includedir}
test_factory_LDFLAGS = -L${libdir}
test_factory_LDADD = -lvx_data2d_factory \
	-lvx_data2d_nc_met \
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_data2d_nc_pinterp \
	-lvx_data2d_nccf \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_grid \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm -lnetcdf_c++ -lnetcdf

# The library itself

lib_LIBRARIES = libvx_data2d_factory.a
libvx_data2d_factory_a_SOURCES = is_met_nc_file.cc \
              is_nccf_file.cc \
              is_netcdf_file.cc \
              is_pinterp_file.cc \
              is_grib_file.cc \
              is_bufr_file.cc \
              var_info_factory.cc \
              data2d_factory_utils.cc \
              data2d_factory.cc
libvx_data2d_factory_a_CPPFLAGS = -I${includedir}

include_HEADERS = is_met_nc_file.h \
              is_nccf_file.h \
              is_netcdf_file.h \
              is_pinterp_file.h \
              is_grib_file.h \
              is_bufr_file.h \
              var_info_factory.h \
              data2d_factory_utils.h \
              data2d_factory.h \
              vx_data2d_factory.h
