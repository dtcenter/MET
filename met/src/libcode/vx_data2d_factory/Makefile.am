## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_data2d_factory.a
libvx_data2d_factory_a_SOURCES = \
              is_netcdf_file.cc is_netcdf_file.h \
              is_grib_file.cc is_grib_file.h \
              is_bufr_file.cc is_bufr_file.h \
              var_info_factory.cc var_info_factory.h \
              data2d_factory_utils.cc data2d_factory_utils.h \
              data2d_factory.cc data2d_factory.h \
              vx_data2d_factory.h
libvx_data2d_factory_a_CPPFLAGS = ${MET_CPPFLAGS} ${MET_PYTHON_CC} $(MET_PYTHON_LD)
