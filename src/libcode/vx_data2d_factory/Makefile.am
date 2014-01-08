## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

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
