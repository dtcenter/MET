## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_data2d_ugrid.a
libvx_data2d_ugrid_a_SOURCES = \
              ugrid_file.cc ugrid_file.h \
              var_info_ugrid.cc var_info_ugrid.h \
              data2d_ugrid.cc data2d_ugrid.h \
              vx_data2d_ugrid.h
libvx_data2d_ugrid_a_CPPFLAGS = ${MET_CPPFLAGS}

