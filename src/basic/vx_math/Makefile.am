## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_math.a
libvx_math_a_SOURCES = viewgravity_to_string.cc viewgravity_to_string.h \
               angles.cc angles.h \
               hist.cc hist.h \
               nint.cc nint.h \
               nti.cc nti.h \
               ptile.cc ptile.h \
               pwl.cc pwl.h \
               affine.cc affine.h \
               math_constants.h \
               is_bad_data.h \
               trig.h \
               legendre.cc legendre.h \
               so3.h so3.cc \
               vx_vector.h vx_vector.cc \
               vx_math.h
libvx_math_a_CPPFLAGS = ${MET_CPPFLAGS}

# If we are in development mode, generate the "to_string" files and
# clean them up in a "make clean".  We don't want to do either of these
# in a distribution because the enum_to_string utility isn't available.

if ENABLE_DEVELOPMENT
    affine.cc viewgravity_to_string.cc viewgravity_to_string.h: affine.h
	${ENUM_TO_STRING} -concat_string affine.h

    clean-local:
	-rm -f affine.cc viewgravity_to_string.cc
	-rm -f affine.cc viewgravity_to_string.h
endif
