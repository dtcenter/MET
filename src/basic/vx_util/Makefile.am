## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# The library

lib_LIBRARIES = libvx_util.a
libvx_util_a_SOURCES = ascii_table.cc \
               asciitablejust_to_string.cc \
               check_endian.cc \
               comma_string.cc \
               conversions.cc \
               data_line.cc \
               grib_constants.cc \
               int_array.cc \
               is_number.cc \
               long_array.cc \
               make_path.cc \
               memory.cc \
               num_array.cc \
               ordinal.cc \
               roman_numeral.cc \
               string_fxns.cc \
               substring.cc \
               temp_file.cc \
               thresh_array.cc \
               threshold.cc \
               fix_float.cc \
               command_line.cc \
               file_exists.cc \
               file_size.cc \
               file_linecount.cc \
               filename_suffix.cc \
               data_plane.cc \
               data_plane_util.cc \
               interp_mthd.cc \
               interp_util.cc \
               two_to_one.cc \
               get_filenames.cc
libvx_util_a_CPPFLAGS = -I${includedir}

include_HEADERS = util_constants.h \
               ascii_table.h \
               asciitablejust_to_string.h \
               check_endian.h \
               comma_string.h \
               conversions.h \
               data_line.h \
               grib_constants.h \
               int_array.h \
               is_number.h \
               long_array.h \
               make_path.h \
               memory.h \
               num_array.h \
               ordinal.h \
               roman_numeral.h \
               string_fxns.h \
               substring.h \
               temp_file.h \
               thresh_array.h \
               threshold.h \
               fix_float.h \
               command_line.h \
               file_exists.h \
               file_size.h \
               file_linecount.h \
               filename_suffix.h \
               data_plane.h \
               data_plane_util.h \
               interp_mthd.h \
               two_to_one.h \
               interp_util.h \
               bool_to_string.h \
               empty_string.h \
               get_filenames.h \
               vx_util.h

# Generated source files

asciitablejust_to_string.h asciitablejust_to_string.cc: ascii_table.h
	${bindir}/enum_to_string -concat_string ascii_table.h

# Test programs

noinst_PROGRAMS = test_command_line \
	test_data_plane \
	test_add_rows

test_command_line_SOURCES = test_command_line.cc
test_command_line_CPPFLAGS = -I${includedir}
test_command_line_LDFLAGS = -L${libdir} -L.
test_command_line_LDADD = -lvx_util \
	-lvx_log

test_data_plane_SOURCES = test_data_plane.cc
test_data_plane_CPPFLAGS = -I${includedir}
test_data_plane_LDFLAGS = -L${libdir} -L.
test_data_plane_LDADD = -lvx_util \
	-lvx_cal \
	-lvx_log

test_add_rows_SOURCES = test_add_rows.cc
test_add_rows_CPPFLAGS = -I${includedir}
test_add_rows_LDFLAGS = -L${libdir} -L.
test_add_rows_LDADD = -lvx_util \
	-lvx_cal \
	-lvx_log

# Clean up extra local files

clean-local:
	-rm -f asciitablejust_to_string.h
	-rm -f asciitablejust_to_string.cc
