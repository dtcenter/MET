## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_data2d_nc_wrf.a
libvx_data2d_nc_wrf_a_SOURCES = \
              get_wrf_grid.cc get_wrf_grid.h \
              wrf_file.cc wrf_file.h \
              var_info_nc_wrf.cc var_info_nc_wrf.h \
              data2d_nc_wrf.cc data2d_nc_wrf.h \
              vx_data2d_nc_wrf.h
libvx_data2d_nc_wrf_a_CPPFLAGS = ${MET_CPPFLAGS}
