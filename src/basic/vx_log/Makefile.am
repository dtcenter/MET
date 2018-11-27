## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_log.a
libvx_log_a_SOURCES = concat_string.cc concat_string.h \
               file_fxns.cc file_fxns.h \
               indent.cc indent.h \
               logger.cc logger.h \
               string_array.cc string_array.h \
               vx_log.h

# Build the library when making a distribution so that
# we can make enum_to_string and chk4copyright.

dist-hook: $(noinst_LIBRARIES)
