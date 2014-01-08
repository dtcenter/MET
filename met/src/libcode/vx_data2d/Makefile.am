## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

lib_LIBRARIES = libvx_data2d.a
libvx_data2d_a_SOURCES = leveltype_to_string.cc \
              level_info.cc \
              var_info.cc \
              data_class.cc \
              data2d_utils.cc \
              table_lookup.cc
libvx_data2d_a_CPPFLAGS = -I${includedir}

include_HEADERS = leveltype_to_string.h \
              level_info.h \
              var_info.h \
              data_class.h \
              data2d_utils.h \
              table_lookup.h \
              vx_data2d.h

# Build rule for leveltype_to_string.cc
leveltype_to_string.h leveltype_to_string.cc: level_info.h
	${bindir}/enum_to_string -concat_string level_info.h

# Clean up extra local files
clean-local:
	-rm -f leveltype_to_string.h
	-rm -f leveltype_to_string.cc
