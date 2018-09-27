## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_data2d_nccf.a
libvx_data2d_nccf_a_SOURCES = \
              nccf_file.cc nccf_file.h \
              var_info_nccf.cc var_info_nccf.h \
              data2d_nccf.cc data2d_nccf.h \
              vx_data2d_nccf.h
libvx_data2d_nccf_a_CPPFLAGS = ${MET_CPPFLAGS}

