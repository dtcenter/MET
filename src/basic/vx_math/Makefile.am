## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

lib_LIBRARIES = libvx_math.a
libvx_math_a_SOURCES = viewgravity_to_string.cc \
               angles.cc \
               hist.cc \
               nint.cc \
               nti.cc \
               ptile.cc \
               pwl.cc \
               affine.cc
libvx_math_a_CPPFLAGS = -I${includedir}

include_HEADERS = math_constants.h \
               is_bad_data.h \
               trig.h \
               angles.h \
               hist.h \
               nint.h \
               nti.h \
               ptile.h \
               pwl.h \
               affine.h \
               viewgravity_to_string.h \
               vx_math.h

# Build rule for viewgravity_to_string.cc
affine.cc viewgravity_to_string.cc viewgravity_to_string.h: affine.h
	${bindir}/enum_to_string -concat_string affine.h

# Extra files to be included in the distribution
EXTRA_DIST = viewgravity_to_string.h

# Clean up extra local files
clean-local:
	-rm -f viewgravity_to_string.h
	-rm -f viewgravity_to_string.cc
