## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_python_utils.a
libvx_python_utils_a_SOURCES = \
              python_dict.h python_dict.cc \
              python_tuple.h python_tuple.cc \
              python_numarray.h python_numarray.cc \
              vx_python_utils.h
libvx_python_utils_a_CPPFLAGS = ${MET_CPPFLAGS} ${MET_PYTHON_CC} $(MET_PYTHON_LD)
