## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

lib_LIBRARIES = libvx_data2d_nccf.a
libvx_data2d_nccf_a_SOURCES = nccf_file.cc \
              var_info_nccf.cc \
              data2d_nccf.cc
libvx_data2d_nccf_a_CPPFLAGS = -I${includedir}

include_HEADERS = nccf_file.h \
              var_info_nccf.h \
              data2d_nccf.h \
              vx_data2d_nccf.h
