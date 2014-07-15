## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_nc_util.a
libvx_nc_util_a_SOURCES = \
              nc_var_info.cc nc_var_info.h \
              nc_utils.cc nc_utils.h \
              write_netcdf.cc write_netcdf.h \
              grid_output.cc grid_output.h \
              nc_constants.h \
              vx_nc_util.h
libvx_nc_util_a_CPPFLAGS = ${MET_CPPFLAGS}
