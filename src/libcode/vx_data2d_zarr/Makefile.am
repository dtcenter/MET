## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_data2d_zarr.a
libvx_data2d_zarr_a_SOURCES = \
              var_info_zarr.cc var_info_zarr.h \
              data2d_zarr.cc data2d_zarr.h
libvx_data2d_zarr_a_CPPFLAGS = ${MET_CPPFLAGS}
