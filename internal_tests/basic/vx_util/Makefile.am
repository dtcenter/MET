## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# Test programs

noinst_PROGRAMS = test_command_line \
	test_data_plane \
	test_add_rows

nodist_test_command_line_SOURCES = test_command_line.cc
test_command_line_CPPFLAGS = ${MET_CPPFLAGS}
test_command_line_LDFLAGS = -L. ${MET_LDFLAGS}
test_command_line_LDADD = -lvx_util \
	-lvx_log

nodist_test_data_plane_SOURCES = test_data_plane.cc
test_data_plane_CPPFLAGS = ${MET_CPPFLAGS}
test_data_plane_LDFLAGS = -L. ${MET_LDFLAGS}
test_data_plane_LDADD = -lvx_util \
	-lvx_cal \
	-lvx_log

nodist_test_add_rows_SOURCES = test_add_rows.cc
test_add_rows_CPPFLAGS = ${MET_CPPFLAGS}
test_add_rows_LDFLAGS = -L. ${MET_LDFLAGS}
test_add_rows_LDADD = -lvx_util \
	-lvx_cal \
	-lvx_log
