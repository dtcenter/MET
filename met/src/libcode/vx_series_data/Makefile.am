## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES = Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_series_data.a
libvx_series_data_a_SOURCES = \
                series_pdf.cc series_pdf.h \
                series_data.cc series_data.h
libvx_series_data_a_CPPFLAGS = ${MET_CPPFLAGS}
