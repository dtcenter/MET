## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

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
