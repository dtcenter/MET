## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_gsl_prob.a
libvx_gsl_prob_a_SOURCES = \
              gsl_bvn.cc gsl_bvn.h \
              gsl_cdf.cc gsl_cdf.h \
              gsl_randist.cc gsl_randist.h \
              gsl_statistics.cc gsl_statistics.h \
              gsl_wavelet2d.cc gsl_wavelet2d.h \
              vx_gsl_prob.h
libvx_gsl_prob_a_CPPFLAGS = ${MET_CPPFLAGS}
