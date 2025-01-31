## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_data2d_nc_met_ra.a
libvx_data2d_nc_met_ra_a_SOURCES = \
              met_ra_file.cc met_ra_file.h \
              var_info_nc_met_ra.cc var_info_nc_met_ra.h \
              data2d_nc_met_ra.cc data2d_nc_met_ra.h \
              vx_data2d_nc_met_ra.h
libvx_data2d_nc_met_ra_a_CPPFLAGS = ${MET_CPPFLAGS}
