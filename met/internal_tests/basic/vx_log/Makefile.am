## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# Test programs

noinst_PROGRAMS = test_logger

test_logger_SOURCES = test_logger.cc
test_logger_CPPFLAGS = ${MET_CPPFLAGS}
test_logger_LDFLAGS = -L. ${MET_LDFLAGS}
test_logger_LDADD = -lvx_log
