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

# If we are in development mode, generate the "to_string" files and
# clean them up in a "make clean".  We don't want to do either of these
# in a distribution because the enum_to_string utility isn't available.

if ENABLE_DEVELOPMENT
    documentmedia_to_string.cc documentmedia_to_string.h \
    documentorientation_to_string.cc documentorientation_to_string.h : vx_ps.h
	${ENUM_TO_STRING} -concat_string vx_ps.h

    clean-local:
	-rm -f documentmedia_to_string.cc
	-rm -f documentmedia_to_string.h
	-rm -f documentorientation_to_string.cc
	-rm -f documentorientation_to_string.h
endif
