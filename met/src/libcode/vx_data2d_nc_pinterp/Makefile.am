## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_data2d_nc_pinterp.a
libvx_data2d_nc_pinterp_a_SOURCES = \
              get_pinterp_grid.cc get_pinterp_grid.h \
              pinterp_file.cc pinterp_file.h \
              var_info_nc_pinterp.cc var_info_nc_pinterp.h \
              data2d_nc_pinterp.cc data2d_nc_pinterp.h \
              vx_data2d_nc_pinterp.h
libvx_data2d_nc_pinterp_a_CPPFLAGS = ${MET_CPPFLAGS}
