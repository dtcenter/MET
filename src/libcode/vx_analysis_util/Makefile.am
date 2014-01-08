## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

lib_LIBRARIES = libvx_analysis_util.a
libvx_analysis_util_a_SOURCES = analysis_utils.cc \
              by_case_info.cc \
              mask_poly.cc \
              mode_atts.cc \
              mode_job.cc \
              mode_line.cc \
              stat_job.cc \
              stat_line.cc \
              time_series.cc
libvx_analysis_util_a_CPPFLAGS = -I${includedir}

include_HEADERS = analysis_utils.h \
              by_case_info.h \
              mask_poly.h \
              mode_atts.h \
              mode_analysis_columns.h \
              mode_job.h \
              mode_line.h \
              stat_job.h \
              stat_line.h \
              stat_offsets.h \
              time_series.h \
              vx_analysis_util.h
