## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# Test programs

noinst_PROGRAMS = test_get \
	test_numpy

test_get_SOURCES = test_get.cc
test_get_CPPFLAGS = ${MET_CPPFLAGS}
test_get_LDFLAGS = -L. ${MET_LDFLAGS}
test_get_LDADD = \
	-lvx_python3_utils \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	$(PYTHON_DEP_LIBS)

test_numpy_SOURCES = test_numpy.cc
test_numpy_CPPFLAGS = ${MET_CPPFLAGS}
test_numpy_LDFLAGS = -L. ${MET_LDFLAGS}
test_numpy_LDADD = \
	-lvx_python3_utils \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	$(PYTHON_DEP_LIBS)

