## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_grid.a
libvx_grid_a_SOURCES = \
              earth_rotation.cc earth_rotation.h  \
              find_grid_by_name.cc  find_grid_by_name.h \
              grid_base.cc grid_base.h \
              latlon_grid_defs.h \
              latlon_grid.cc latlon_grid.h \
              lc_grid_defs.h \
              lc_grid.cc lc_grid.h \
              merc_grid_defs.h \
              merc_grid.cc merc_grid.h \
              gaussian_grid_defs.h \
              gaussian_grid.cc gaussian_grid.h \
              st_grid_defs.h \
              st_grid.cc st_grid.h \
              latlon_xyz.h latlon_xyz.cc \
              rot_latlon_grid.cc rot_latlon_grid.h \
              goes_grid_defs.h goes_grid.cc goes_grid.h \
              vx_grid.h
libvx_grid_a_CPPFLAGS = ${MET_CPPFLAGS}
