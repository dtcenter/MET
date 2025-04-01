## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_ioda.a
libvx_ioda_a_SOURCES = \
              ioda.cc ioda.h \
              ioda_data_conf_info.cc ioda_data_conf_info.h \
              vx_ioda.h
libvx_ioda_a_CPPFLAGS = ${MET_CPPFLAGS}
