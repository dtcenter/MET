## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

OPT_PYTHON_SOURCES =

if ENABLE_PYTHON
LDFLAGS += -lvx_python3_utils
OPT_PYTHON_SOURCES += python_line.h python_line.cc
endif

# The library

noinst_LIBRARIES = libvx_util.a
libvx_util_a_SOURCES = ascii_table.cc ascii_table.h \
               asciitablejust_to_string.cc asciitablejust_to_string.h \
               ascii_header.cc ascii_header.h \
               check_endian.cc check_endian.h \
               comma_string.cc comma_string.h \
               conversions.cc conversions.h \
               data_line.cc data_line.h \
               grib_constants.cc grib_constants.h \
               is_number.cc is_number.h \
               crc_array.h \
               crr_array.h \
               ncrr_array.h \
               int_array.h \
               two_d_array.h \
               long_array.cc long_array.h \
               num_array.cc num_array.h \
               thresh_array.cc thresh_array.h \
               make_path.cc make_path.h \
               memory.cc memory.h \
               ordinal.cc ordinal.h \
               roman_numeral.cc roman_numeral.h \
               string_fxns.cc string_fxns.h \
               substring.cc substring.h \
               fix_float.cc fix_float.h \
               command_line.cc command_line.h \
               file_size.cc file_size.h \
               file_exists.cc file_exists.h \
               file_linecount.cc file_linecount.h \
               filename_suffix.cc filename_suffix.h \
               data_plane.cc data_plane.h \
               data_cube.cc data_cube.h \
               data_plane_util.cc data_plane_util.h \
               interp_mthd.cc interp_mthd.h \
               interp_util.cc interp_util.h \
               two_to_one.cc two_to_one.h \
               get_filenames.cc get_filenames.h \
               util_constants.h \
               bool_to_string.h \
               empty_string.h \
               polyline.h polyline.cc \
               mask_poly.h mask_poly.cc \
               read_fortran_binary.h read_fortran_binary.cc \
               met_buffer.h met_buffer.cc \
               smart_buffer.h smart_buffer.cc  \
               vx_util.h \
               CircularTemplate.h CircularTemplate.cc \
               GridTemplate.h GridTemplate.cc \
               GridPoint.h GridPoint.cc \
               GridOffset.h GridOffset.cc \
               observation.h observation.cc \
               stat_column_defs.h \
               RectangularTemplate.h RectangularTemplate.cc $(OPT_PYTHON_SOURCES)

libvx_util_a_CPPFLAGS = ${MET_CPPFLAGS}

# If we are in development mode, generate the "to_string" files and
# clean them up in a "make clean".  We don't want to do either of these
# in a distribution because the enum_to_string utility isn't available.

if ENABLE_DEVELOPMENT
    asciitablejust_to_string.cc asciitablejust_to_string.h: ascii_table.h
	${ENUM_TO_STRING} -concat_string ascii_table.h

    clean-local:
	-rm -f asciitablejust_to_string.cc
	-rm -f asciitablejust_to_string.h
endif

# Build the library when making a distribution so that
# we can make chk4copyright to add the copyright notices.

dist-hook: $(noinst_LIBRARIES)
