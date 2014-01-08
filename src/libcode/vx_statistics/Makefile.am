## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

lib_LIBRARIES = libvx_statistics.a
libvx_statistics_a_SOURCES = apply_mask.cc \
               compute_ci.cc \
               contable.cc \
               contable_stats.cc \
               contable_nx2.cc \
               met_stats.cc \
               pair_base.cc \
               pair_data_point.cc \
               pair_data_ensemble.cc
libvx_statistics_a_CPPFLAGS = -I${includedir}

include_HEADERS = apply_mask.h \
               compute_ci.h \
               contable.h \
               met_stats.h \
               pair_base.h \
               pair_data_point.h \
               pair_data_ensemble.h \
               vx_statistics.h
