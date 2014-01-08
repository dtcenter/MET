## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

lib_LIBRARIES = libvx_nc_util.a
libvx_nc_util_a_SOURCES = nc_var_info.cc \
              nc_utils.cc \
              write_netcdf.cc \
              grid_output.cc
libvx_nc_util_a_CPPFLAGS = -I${includedir}

include_HEADERS = nc_constants.h \
              nc_var_info.h \
              nc_utils.h \
              write_netcdf.h \
              grid_output.h \
              vx_nc_util.h
