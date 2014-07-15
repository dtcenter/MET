## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_data2d_grib2.a
libvx_data2d_grib2_a_SOURCES = \
              var_info_grib2.cc var_info_grib2.h \
              data2d_grib2.cc data2d_grib2.h
libvx_data2d_grib2_a_CPPFLAGS = ${MET_CPPFLAGS}
