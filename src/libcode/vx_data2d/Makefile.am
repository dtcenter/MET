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

# Generated source files

leveltype_to_string.h leveltype_to_string.cc: level_info.h
	${ENUM_TO_STRING} -concat_string -reverse level_info.h

# Extra files to be included in the distribution

EXTRA_DIST = leveltype_to_string.h

# Clean up extra local files

clean-local:
	-rm -f leveltype_to_string.h
	-rm -f leveltype_to_string.cc
