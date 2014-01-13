## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Test programs

noinst_PROGRAMS = test_read_nccf

test_read_nccf_SOURCES = test_read_nccf.cc
test_read_nccf_CPPFLAGS = -I${includedir}
test_read_nccf_LDFLAGS = -L${libdir}
test_read_nccf_LDADD = -lvx_data2d_nccf \
	-lvx_nc_util \
	-lvx_data2d \
	-lvx_grid \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lnetcdf_c++ -lnetcdf

# The library itself

lib_LIBRARIES = libvx_data2d_nccf.a
libvx_data2d_nccf_a_SOURCES = nccf_file.cc \
              var_info_nccf.cc \
              data2d_nccf.cc
libvx_data2d_nccf_a_CPPFLAGS = -I${includedir}

include_HEADERS = nccf_file.h \
              var_info_nccf.h \
              data2d_nccf.h \
              vx_data2d_nccf.h
