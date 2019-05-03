## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_data2d_grib.a
libvx_data2d_grib_a_SOURCES = \
              grib_classes.cc grib_classes.h \
              grib_strings.cc grib_strings.h \
              grib_utils.cc grib_utils.h \
              var_info_grib.cc var_info_grib.h \
              data2d_grib_utils.cc data2d_grib_utils.h \
              data2d_grib.cc data2d_grib.h \
              vx_data2d_grib.h
libvx_data2d_grib_a_CPPFLAGS = ${MET_CPPFLAGS}

