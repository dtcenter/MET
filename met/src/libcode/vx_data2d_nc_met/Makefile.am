## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

lib_LIBRARIES = libvx_data2d_nc_met.a
libvx_data2d_nc_met_a_SOURCES = get_met_grid.cc \
              met_file.cc \
              var_info_nc_met.cc \
              data2d_nc_met.cc
libvx_data2d_nc_met_a_CPPFLAGS = -I${includedir}

include_HEADERS = get_met_grid.h \
              met_file.h \
              var_info_nc_met.h \
              data2d_nc_met.h \
              vx_data2d_nc_met.h
