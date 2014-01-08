## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

lib_LIBRARIES = libvx_data2d_grib2.a
libvx_data2d_grib2_a_SOURCES =var_info_grib2.cc \
              data2d_grib2.cc
libvx_data2d_grib2_a_CPPFLAGS = -I${includedir}

include_HEADERS = var_info_grib2.h \
              data2d_grib2.h
