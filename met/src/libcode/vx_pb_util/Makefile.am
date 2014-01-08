## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

lib_LIBRARIES = libvx_pb_util.a
libvx_pb_util_a_SOURCES = pblock.cc \
               do_blocking.cc \
               do_unblocking.cc \
               copy_bytes.cc
libvx_pb_util_a_CPPFLAGS = -I${includedir}

include_HEADERS = pblock.h \
               do_blocking.h \
               do_unblocking.h \
               copy_bytes.h \
               vx_pb_util.h
