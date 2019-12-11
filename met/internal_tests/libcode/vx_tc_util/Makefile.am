## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# Test programs

noinst_PROGRAMS = test_read \
	test_read_prob \
	test_read_rmw

test_read_SOURCES = test_read.cc
test_read_CPPFLAGS = ${MET_CPPFLAGS}
test_read_LDFLAGS = -L. ${MET_LDFLAGS}
test_read_LDADD = -lvx_tc_util \
	-lvx_config \
	-lvx_gsl_prob \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_log \
	-lgsl -lgslcblas

test_read_prob_SOURCES = test_read_prob.cc
test_read_prob_CPPFLAGS = ${MET_CPPFLAGS}
test_read_prob_LDFLAGS = -L. ${MET_LDFLAGS}
test_read_prob_LDADD = -lvx_tc_util \
	-lvx_config \
	-lvx_gsl_prob \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_log \
	-lgsl -lgslcblas

test_read_rmw_SOURCES = test_read_rmw.cc
test_read_rmw_CPPFLAGS = ${MET_CPPFLAGS}
test_read_rmw_LDFLAGS = -L. ${MET_LDFLAGS}
test_read_rmw_LDADD = -lvx_tc_util \
	-lvx_config \
	-lvx_cal \
	-lvx_util \
	-lvx_nc_util \
	-lvx_math \
	-lvx_log \
	-lm -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas
