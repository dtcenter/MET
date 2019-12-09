## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_python_utils.a
libvx_python_utils_a_SOURCES = \
              python3_script.h python3_script.cc \
              python3_numpy.h python3_numpy.cc \
              python3_util.h python3_util.cc
libvx_python_utils_a_CPPFLAGS = ${MET_CPPFLAGS} ${MET_PYTHON_CC} $(MET_PYTHON_LD)
