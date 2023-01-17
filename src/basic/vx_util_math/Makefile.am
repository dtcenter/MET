## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_util_math.a
libvx_util_math_a_SOURCES = so3.cc so3.h

libvx_util_math_a_CPPFLAGS = ${MET_CPPFLAGS}

# If we are in development mode, generate the "to_string" files and
# clean them up in a "make clean".  We don't want to do either of these
# in a distribution because the enum_to_string utility isn't available.

# Build the library when making a distribution so that
# we can make chk4copyright to add the copyright notices.

dist-hook: $(noinst_LIBRARIES)
