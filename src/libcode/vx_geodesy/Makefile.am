## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_geodesy.a
libvx_geodesy_a_SOURCES = \
              spheroid.cc spheroid.h \
              vx_geodesy.h 
libvx_geodesy_a_CPPFLAGS = ${MET_CPPFLAGS}
