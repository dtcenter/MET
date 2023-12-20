## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_data2d_nc_cf.a
libvx_data2d_nc_cf_a_SOURCES = \
              nc_cf_file.cc nc_cf_file.h \
              var_info_nc_cf.cc var_info_nc_cf.h \
              data2d_nc_cf.cc data2d_nc_cf.h \
              vx_data2d_nc_cf.h
libvx_data2d_nc_cf_a_CPPFLAGS = ${MET_CPPFLAGS}

