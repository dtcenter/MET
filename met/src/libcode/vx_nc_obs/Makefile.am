## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_nc_obs.a
libvx_nc_obs_a_SOURCES = \
              nc_obs_util.cc nc_obs_util.h \
              nc_summary.cc nc_summary.h
libvx_nc_obs_a_CPPFLAGS = ${MET_CPPFLAGS}
