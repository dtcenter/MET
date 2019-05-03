## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_data2d.a
libvx_data2d_a_SOURCES = \
              leveltype_to_string.cc leveltype_to_string.h \
              level_info.cc level_info.h \
              var_info.cc var_info.h \
              data_class.cc data_class.h \
              data2d_utils.cc data2d_utils.h \
              table_lookup.cc table_lookup.h \
              vx_data2d.h
libvx_data2d_a_CPPFLAGS = ${MET_CPPFLAGS}

# If we are in development mode, generate the "to_string" files and
# clean them up in a "make clean".  We don't want to do either of these
# in a distribution because the enum_to_string utility isn't available.

if ENABLE_DEVELOPMENT
    leveltype_to_string.cc leveltype_to_string.h: level_info.h
	${ENUM_TO_STRING} -concat_string -reverse level_info.h

    clean-local:
	-rm -f leveltype_to_string.cc
	-rm -f leveltype_to_string.h
endif
