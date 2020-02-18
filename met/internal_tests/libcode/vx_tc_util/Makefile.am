## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

if ENABLE_PYTHON
LDFLAGS += -lvx_python3_utils
endif

# Test programs

noinst_PROGRAMS = test_read \
	test_read_prob

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

if ENABLE_PYTHON
test_read_LDADD += $(MET_PYTHON_LD)
test_read_LDADD += -lvx_data2d_python -lvx_python3_utils
test_read_LDADD += -lvx_data2d_python -lvx_python3_utils
test_read_LDADD += -lvx_grid -lvx_util -lvx_config
test_read_LDADD += -lvx_data2d -lvx_gsl_prob -lvx_util -lvx_math -lvx_cal -lvx_config
endif

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

if ENABLE_PYTHON
test_read_prob_LDADD += $(MET_PYTHON_LD)
test_read_prob_LDADD += -lvx_data2d_python -lvx_python3_utils
test_read_prob_LDADD += -lvx_data2d_python -lvx_python3_utils
test_read_prob_LDADD += -lvx_grid -lvx_util -lvx_config
test_read_prob_LDADD += -lvx_data2d -lvx_gsl_prob -lvx_util -lvx_math -lvx_cal -lvx_config
endif
