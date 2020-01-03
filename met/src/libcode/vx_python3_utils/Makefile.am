## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

CPPFLAGS += -I../../../src/basic/vx_math

CPPFLAGS += -I../../../src/basic/vx_cal

# The library

noinst_LIBRARIES = libvx_python3_utils.a
libvx_python3_utils_a_SOURCES = \
              wchar_argv.h wchar_argv.cc \
              python3_dict.h python3_dict.cc \
              python3_list.h python3_list.cc \
              python3_script.h python3_script.cc \
              python3_numpy.h python3_numpy.cc \
              python3_util.h python3_util.cc \
              vx_python3_utils.h
libvx_python3_utils_a_CPPFLAGS = ${MET_CPPFLAGS} ${CPPFLAGS} ${MET_PYTHON_CC} $(MET_PYTHON_LD)
