## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

lib_LIBRARIES = libvx_grid.a
libvx_grid_a_SOURCES = find_grid_by_name.cc \
              grid_base.cc \
              latlon_grid.cc \
              lc_grid.cc \
              merc_grid.cc \
              st_grid.cc
libvx_grid_a_CPPFLAGS = -I${includedir}

include_HEADERS = find_grid_by_name.h \
              grid_base.h \
              vx_grid.h \
              latlon_grid_defs.h \
              latlon_grid.h \
              lc_grid_defs.h \
              lc_grid.h \
              merc_grid_defs.h \
              merc_grid.h \
              st_grid_defs.h \
              st_grid.h
