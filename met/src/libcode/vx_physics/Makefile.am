## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES = Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_physics.a
libvx_physics_a_SOURCES = \
    thermo.cc thermo.h
libvx_physics_a_CPPFLAGS = ${MET_CPPFLAGS}
