## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_statistics.a
libvx_statistics_a_SOURCES = \
               apply_mask.cc apply_mask.h \
               compute_ci.cc compute_ci.h \
               contable.cc contable.h \
               contable_stats.cc \
               contable_nx2.cc \
               met_stats.cc met_stats.h \
               pair_base.cc pair_base.h \
               pair_data_point.cc pair_data_point.h \
               pair_data_ensemble.cc pair_data_ensemble.h \
               read_climo.cc read_climo.h \
               obs_error.cc obs_error.h \
               vx_statistics.h
libvx_statistics_a_CPPFLAGS = ${MET_CPPFLAGS}
