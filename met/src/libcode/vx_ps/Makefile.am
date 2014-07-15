## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_ps.a
libvx_ps_a_SOURCES = \
              documentmedia_to_string.cc documentmedia_to_string.h \
              documentorientation_to_string.cc documentorientation_to_string.h \
              fontfamily_to_string.cc fontfamily_to_string.h \
              vx_ps.cc vx_ps.h \
              ps_text.cc ps_text.h \
              table_helper.cc table_helper.h
libvx_ps_a_CPPFLAGS = ${MET_CPPFLAGS}

# Generated source files

documentmedia_to_string.h \
documentmedia_to_string.cc \
documentorientation_to_string.h \
documentorientation_to_string.cc : vx_ps.h
	${ENUM_TO_STRING} -concat_string vx_ps.h

# Extra files to be included in the distribution

EXTRA_DIST = fontfamily_to_string.h \
	documentorientation_to_string.h \
	documentmedia_to_string.h

# Clean up extra local files

clean-local:
	-rm -f documentmedia_to_string.h
	-rm -f documentmedia_to_string.cc
	-rm -r documentorientation_to_string.h
	-rm -r documentorientation_to_string.cc
	-rm -r fontfamily_to_string.h
	-rm -r fontfamily_to_string.cc




