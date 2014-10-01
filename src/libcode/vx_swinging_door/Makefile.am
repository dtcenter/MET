## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_swinging_door.a
libvx_swinging_door_a_SOURCES = \
               compute_swinging_door.cc compute_swinging_door.h \
               vx_swinging_door.h
libvx_swinging_door_a_CPPFLAGS = ${MET_CPPFLAGS}
