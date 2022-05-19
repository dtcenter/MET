## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_regrid.a
libvx_regrid_a_SOURCES = \
              vx_regrid.h vx_regrid.cc \
              vx_regrid_budget.cc
libvx_regrid_a_CPPFLAGS = ${MET_CPPFLAGS}
