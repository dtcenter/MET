## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_solar.a
libvx_solar_a_SOURCES = astro_constants.h siderial.cc siderial.h solar.cc solar.h
libvx_solar_a_CPPFLAGS = ${MET_CPPFLAGS}
