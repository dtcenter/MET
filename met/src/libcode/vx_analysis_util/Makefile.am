## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_analysis_util.a
libvx_analysis_util_a_SOURCES = \
              analysis_utils.cc analysis_utils.h \
              by_case_info.cc by_case_info.h \
              mode_atts.cc mode_atts.h \
              mode_job.cc mode_job.h \
              mode_line.cc mode_line.h \
              stat_job.cc stat_job.h \
              stat_line.cc stat_line.h \
              time_series.cc time_series.h \
              vx_analysis_util.h
libvx_analysis_util_a_CPPFLAGS = ${MET_CPPFLAGS}
