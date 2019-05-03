## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_pb_util.a
libvx_pb_util_a_SOURCES = \
               pblock.cc pblock.h \
               do_blocking.cc do_blocking.h \
               do_unblocking.cc do_unblocking.h \
               copy_bytes.cc copy_bytes.h \
               vx_pb_util.h
libvx_pb_util_a_CPPFLAGS = ${MET_CPPFLAGS}
