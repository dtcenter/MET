## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# Build flags applicable for the entire Makefile

AM_CPPFLAGS = ${MET_CPPFLAGS}
AM_LDFLAGS = -L. ${MET_LDFLAGS}

# Test programs

noinst_PROGRAMS = test_config \
              test_user_func \
              test_lookup \
              test_lookup2 \
              test_lookup3 \
              test_string \
              test_thresh  \
              test_string_then_config  \
              test_met_478

test_config_SOURCES = test_config.cc
test_config_LDADD = -lvx_config \
	-lvx_gsl_prob \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm -lgsl -lgslcblas


test_user_func_SOURCES = test_user_func.cc
test_user_func_LDADD = -lvx_config \
	-lvx_gsl_prob \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm -lgsl -lgslcblas

test_lookup_SOURCES = test_lookup.cc
test_lookup_LDADD = -lvx_config \
	-lvx_gsl_prob \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm -lgsl -lgslcblas

test_lookup2_SOURCES = test_lookup2.cc
test_lookup2_LDADD = -lvx_config \
	-lvx_gsl_prob \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm -lgsl -lgslcblas

test_lookup3_SOURCES = test_lookup3.cc
test_lookup3_LDADD = -lvx_config \
	-lvx_gsl_prob \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm -lgsl -lgslcblas

test_string_SOURCES = test_string.cc
test_string_LDADD = -lvx_config \
	-lvx_gsl_prob \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm -lgsl -lgslcblas

test_thresh_SOURCES = test_thresh.cc
test_thresh_LDADD = -lvx_config \
	-lvx_gsl_prob \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm -lgsl -lgslcblas

test_string_then_config_SOURCES = test_string_then_config.cc
test_string_then_config_LDADD = -lvx_config \
	-lvx_gsl_prob \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm -lgsl -lgslcblas

test_met_478_SOURCES = test_met_478.cc
test_met_478_LDADD = -lvx_config \
	-lvx_gsl_prob \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm -lgsl -lgslcblas
