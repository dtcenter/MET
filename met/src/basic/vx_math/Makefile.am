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
               vx_math.h
libvx_math_a_CPPFLAGS = ${MET_CPPFLAGS}

# Build rule for viewgravity_to_string.cc
affine.cc viewgravity_to_string.cc viewgravity_to_string.h: affine.h
	${ENUM_TO_STRING} -concat_string affine.h

# Extra files to be included in the distribution
EXTRA_DIST = viewgravity_to_string.h

# Clean up extra local files
clean-local:
	-rm -f viewgravity_to_string.h
	-rm -f viewgravity_to_string.cc
