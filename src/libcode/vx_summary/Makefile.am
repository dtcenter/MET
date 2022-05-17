## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_summary.a
libvx_summary_a_SOURCES = \
              summary_calc.cc summary_calc.h \
              summary_calc_max.cc summary_calc_max.h \
              summary_calc_mean.cc summary_calc_mean.h \
              summary_calc_median.cc summary_calc_median.h \
              summary_calc_min.cc summary_calc_min.h \
              summary_calc_percentile.cc summary_calc_percentile.h \
              summary_calc_range.cc summary_calc_range.h \
              summary_calc_stdev.cc summary_calc_stdev.h \
              summary_key.cc summary_key.h \
              summary_obs.cc summary_obs.h \
              time_summary_interval.cc time_summary_interval.h \
              vx_summary.h
libvx_summary_a_CPPFLAGS = ${MET_CPPFLAGS}
