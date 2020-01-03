## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_data2d_python.a
libvx_data2d_python_a_SOURCES = \
              dataplane_from_numpy_array.h dataplane_from_numpy_array.cc \
              dataplane_from_xarray.h dataplane_from_xarray.cc \
              grid_from_python_dict.h grid_from_python_dict.cc \
              python_dataplane.h python_dataplane.cc \
              data2d_python.h data2d_python.cc \
              var_info_python.h var_info_python.cc \
              global_python.h
libvx_data2d_python_a_CPPFLAGS = ${MET_CPPFLAGS} -I../vx_python2_utils ${MET_PYTHON_CC} $(MET_PYTHON_LD)

