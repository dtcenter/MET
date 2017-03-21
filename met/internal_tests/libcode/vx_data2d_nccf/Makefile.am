## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# Test programs

noinst_PROGRAMS = test_read_nccf

test_read_nccf_SOURCES = test_read_nccf.cc
test_read_nccf_CPPFLAGS = ${MET_CPPFLAGS}
test_read_nccf_LDFLAGS = -L. ${MET_LDFLAGS}
test_read_nccf_LDADD = -lvx_data2d_nccf \
	-lvx_nc_util \
	-lvx_data2d \
	-lvx_grid \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lnetcdf_c++4 -lnetcdf

