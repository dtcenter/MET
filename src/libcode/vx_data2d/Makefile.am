## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# The library

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

# Generated source files

leveltype_to_string.h leveltype_to_string.cc: level_info.h
	${bindir}/enum_to_string -concat_string -reverse level_info.h

# Test programs

noinst_PROGRAMS = test_table_read \
              dump_default_table

test_table_read_SOURCES = test_table_read.cc
test_table_read_CPPFLAGS = -I${includedir}
test_table_read_LDFLAGS = -L${libdir} -L.
test_table_read_LDADD = -lvx_config \
	-lvx_cal \
	-lvx_log \
	-lvx_data2d \
	-lvx_grid  \
	-lvx_util \
	-lvx_math

dump_default_table_SOURCES = dump_default_table.cc
dump_default_table_CPPFLAGS = -I${includedir}
dump_default_table_LDFLAGS = -L${libdir} -L.
dump_default_table_LDADD = -lvx_config \
	-lvx_cal \
	-lvx_log \
	-lvx_data2d \
	-lvx_grid \
	-lvx_util \
	-lvx_math

# Clean up extra local files

clean-local:
	-rm -f leveltype_to_string.h
	-rm -f leveltype_to_string.cc
