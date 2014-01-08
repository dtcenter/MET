## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

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

# Build rule for enum_to_string generated files
documentmedia_to_string.h \
documentmedia_to_string.cc \
documentorientation_to_string.h \
documentorientation_to_string.cc : vx_ps.h
	${bindir}/enum_to_string -concat_string vx_ps.h

# Clean up extra local files
clean-local:
	-rm -f documentmedia_to_string.h
	-rm -f documentmedia_to_string.cc
	-rm -r documentorientation_to_string.h
	-rm -r documentorientation_to_string.cc




