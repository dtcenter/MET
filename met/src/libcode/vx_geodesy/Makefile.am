## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Test programs

noinst_PROGRAMS = test_spheroid

test_spheroid_SOURCES = test_spheroid.cc
test_spheroid_CPPFLAGS = -I${includedir}
test_spheroid_LDFLAGS = -L${libdir}
test_spheroid_LDADD = -lvx_geodesy  \
	-lvx_util     \
	-lvx_log      \
	-lm

# The library itself

lib_LIBRARIES = libvx_geodesy.a
libvx_geodesy_a_SOURCES = spheroid.cc
libvx_geodesy_a_CPPFLAGS = -I${includedir}

include_HEADERS = vx_geodesy.h  \
              spheroid.h
