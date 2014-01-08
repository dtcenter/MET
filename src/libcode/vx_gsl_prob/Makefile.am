## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

lib_LIBRARIES = libvx_gsl_prob.a
libvx_gsl_prob_a_SOURCES = gsl_bvn.cc \
              gsl_cdf.cc \
              gsl_randist.cc \
              gsl_statistics.cc \
              gsl_wavelet2d.cc
libvx_gsl_prob_a_CPPFLAGS = -I${includedir}

include_HEADERS = gsl_bvn.h \
              gsl_cdf.h \
              gsl_randist.h \
              gsl_statistics.h \
              gsl_wavelet2d.h \
              vx_gsl_prob.h
