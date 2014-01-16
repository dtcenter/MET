## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# The library

lib_LIBRARIES = libvx_ps.a
libvx_ps_a_SOURCES = documentmedia_to_string.cc \
              documentorientation_to_string.cc \
              vx_ps.cc \
              ps_text.cc
libvx_ps_a_CPPFLAGS = -I${includedir}

include_HEADERS = documentmedia_to_string.h \
              documentorientation_to_string.h \
              vx_ps.h \
              ps_text.h

# Generated source files

documentmedia_to_string.h \
documentmedia_to_string.cc \
documentorientation_to_string.h \
documentorientation_to_string.cc : vx_ps.h
	${bindir}/enum_to_string -concat_string vx_ps.h

# Test programs

noinst_PROGRAMS = test_ps

test_ps_SOURCES = test_ps.cc
test_ps_CPPFLAGS = -I${includedir}
test_ps_LDFLAGS = -L${libdir} -L.
test_ps_LDADD = -lvx_ps \
	-lvx_afm \
	-lvx_util \
	-lvx_log \
	-lm

# Clean up extra local files

clean-local:
	-rm -f documentmedia_to_string.h
	-rm -f documentmedia_to_string.cc
	-rm -r documentorientation_to_string.h
	-rm -r documentorientation_to_string.cc




