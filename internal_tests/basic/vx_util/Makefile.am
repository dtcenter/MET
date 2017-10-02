## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# Test programs

noinst_PROGRAMS = test_command_line \
	test_data_plane \
	test_add_rows   \
	test_table_float \
	test_ascii_header

test_command_line_SOURCES = test_command_line.cc
test_command_line_CPPFLAGS = ${MET_CPPFLAGS}
test_command_line_LDFLAGS = -L. ${MET_LDFLAGS}
test_command_line_LDADD = -lvx_util \
	-lvx_log

test_data_plane_SOURCES = test_data_plane.cc
test_data_plane_CPPFLAGS = ${MET_CPPFLAGS}
test_data_plane_LDFLAGS = -L. ${MET_LDFLAGS}
test_data_plane_LDADD = -lvx_util \
        -lvx_config \
        -lvx_cal \
        -lvx_util \
        -lvx_math \
        -lvx_log

test_add_rows_SOURCES = test_add_rows.cc
test_add_rows_CPPFLAGS = ${MET_CPPFLAGS}
test_add_rows_LDFLAGS = -L. ${MET_LDFLAGS}
test_add_rows_LDADD = -lvx_util \
        -lvx_config \
        -lvx_cal \
        -lvx_util \
        -lvx_math \
        -lvx_log 

test_table_float_SOURCES = test_table_float.cc
test_table_float_CPPFLAGS = ${MET_CPPFLAGS}
test_table_float_LDFLAGS = -L. ${MET_LDFLAGS}
test_table_float_LDADD = -lvx_util \
        -lvx_config \
        -lvx_cal \
        -lvx_util \
        -lvx_math \
        -lvx_log

test_ascii_header_SOURCES = test_ascii_header.cc
test_ascii_header_CPPFLAGS = ${MET_CPPFLAGS}
test_ascii_header_LDFLAGS = -L. ${MET_LDFLAGS}
test_ascii_header_LDADD = -lvx_util \
        -lvx_config \
        -lvx_cal \
        -lvx_util \
        -lvx_math \
        -lvx_log
 
