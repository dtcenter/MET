## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_tc_util.a
libvx_tc_util_a_SOURCES = \
              atcf_line_base.cc atcf_line_base.h atcf_offsets.h \
              atcf_track_line.cc atcf_track_line.h \
              atcf_prob_line.cc atcf_prob_line.h \
              track_point.cc track_point.h \
              track_info.cc track_info.h \
              track_pair_info.cc track_pair_info.h \
              prob_info_base.h prob_info_base.cc \
              prob_info_array.h prob_info_array.cc \
              prob_rirw_info.h prob_rirw_info.cc \
              prob_rirw_pair_info.h prob_rirw_pair_info.cc \
              genesis_info.cc genesis_info.h \
              tc_hdr_columns.cc tc_hdr_columns.h \
              tc_columns.cc tc_columns.h \
              tc_stat_line.cc tc_stat_line.h \
              vx_tc_nc_util.h vx_tc_nc_util.cc \
              vx_tc_util.h
libvx_tc_util_a_CPPFLAGS = ${MET_CPPFLAGS}
