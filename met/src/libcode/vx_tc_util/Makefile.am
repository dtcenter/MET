## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Test programs

noinst_PROGRAMS = test_read

test_read_SOURCES = test_read.cc
test_read_CPPFLAGS = -I${includedir}
test_read_LDFLAGS = -L${libdir}
test_read_LDADD = -lvx_tc_util \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_log

# The library itself

lib_LIBRARIES = libvx_tc_util.a
libvx_tc_util_a_SOURCES = atcf_line.cc \
              track_point.cc \
              track_info.cc \
              track_pair_info.cc \
              tc_hdr_columns.cc \
              tc_columns.cc \
              tc_stat_line.cc
libvx_tc_util_a_CPPFLAGS = -I${includedir}

include_HEADERS = atcf_line.h \
              track_point.h \
              track_info.h \
              track_pair_info.h \
              tc_hdr_columns.h \
              tc_columns.h \
              tc_stat_line.h \
              vx_tc_util.h
