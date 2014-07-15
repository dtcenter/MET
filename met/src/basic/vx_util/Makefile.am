## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_util.a
libvx_util_a_SOURCES = ascii_table.cc ascii_table.h \
               asciitablejust_to_string.cc asciitablejust_to_string.h \
               check_endian.cc check_endian.h \
               comma_string.cc comma_string.h \
               conversions.cc conversions.h \
               data_line.cc data_line.h \
               grib_constants.cc grib_constants.h \
               int_array.cc int_array.h \
               is_number.cc is_number.h \
               long_array.cc long_array.h \
               make_path.cc make_path.h \
               memory.cc memory.h \
               num_array.cc num_array.h \
               ordinal.cc ordinal.h \
               roman_numeral.cc roman_numeral.h \
               string_fxns.cc string_fxns.h \
               substring.cc substring.h \
               temp_file.cc temp_file.h \
               thresh_array.cc thresh_array.h \
               threshold.cc threshold.h \
               fix_float.cc fix_float.h \
               command_line.cc command_line.h \
               file_exists.cc file_exists.h \
               file_size.cc file_size.h \
               file_linecount.cc file_linecount.h \
               filename_suffix.cc filename_suffix.h \
               data_plane.cc data_plane.h \
               data_plane_util.cc data_plane_util.h \
               interp_mthd.cc interp_mthd.h \
               interp_util.cc interp_util.h \
               two_to_one.cc two_to_one.h \
               get_filenames.cc get_filenames.h \
               util_constants.h \
               bool_to_string.h \
               empty_string.h \
               vx_util.h
libvx_util_a_CPPFLAGS = ${MET_CPPFLAGS}

# Generated source files

asciitablejust_to_string.h asciitablejust_to_string.cc: ascii_table.h
	${ENUM_TO_STRING} -concat_string ascii_table.h

# Extra files to be included in the distribution

EXTRA_DIST = asciitablejust_to_string.h

# Clean up extra local files

clean-local:
	-rm -f asciitablejust_to_string.h
	-rm -f asciitablejust_to_string.cc
