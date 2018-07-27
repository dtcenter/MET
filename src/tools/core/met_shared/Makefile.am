## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libmet_shared.a
libmet_shared_a_SOURCES = \
              nc_tools.cc nc_tools.h \
              summary_nc.cc summary_nc.h
libmet_shared_a_CPPFLAGS = ${MET_CPPFLAGS}
