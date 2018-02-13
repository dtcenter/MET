## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The libraries

noinst_LIBRARIES = libvx_gis.a

libvx_gis_a_SOURCES = shapetype_to_string.h   \
                      shapetype_to_string.cc  \
                      dbf_file.cc  \
                      dbf_file.h   \
                      shp_file.cc  \
                      shp_file.h   \
                      shx_file.cc  \
                      shx_file.h


libvx_gis_a_CPPFLAGS = ${MET_CPPFLAGS}

# If we are in development mode, generate the "to_string" files and
# clean them up in a "make clean".  We don't want to do either of these
# in a distribution because the enum_to_string utility isn't available.

if ENABLE_DEVELOPMENT
    shapetype_to_string.h shapetype_to_string.cc: shp_file.h
	${ENUM_TO_STRING} -concat_string shp_file.h

   clean-local:
	-rm -f shapetype_to_string.cc
	-rm -f shapetype_to_string.h
endif

