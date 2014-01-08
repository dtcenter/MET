## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

lib_LIBRARIES = libvx_data2d_nc_pinterp.a
libvx_data2d_nc_pinterp_a_SOURCES = get_pinterp_grid.cc \
              pinterp_file.cc \
              var_info_nc_pinterp.cc \
              data2d_nc_pinterp.cc
libvx_data2d_nc_pinterp_a_CPPFLAGS = -I${includedir}

include_HEADERS = get_pinterp_grid.h \
              pinterp_file.h \
              var_info_nc_pinterp.h \
              data2d_nc_pinterp.h \
              vx_data2d_nc_pinterp.h
