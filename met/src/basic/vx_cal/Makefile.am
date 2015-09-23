## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_cal.a

libvx_cal_a_SOURCES = date_to_mjd.cc \
              day_dif.cc \
              day_of_week.cc \
              doyhms_to_unix.cc \
              is_dst.cc \
              is_leap_year.cc \
              mdyhms_to_unix.cc \
              mjd_to_date.cc \
              time_array.cc time_array.h \
              time_strings.cc \
              unix_to_mdyhms.cc \
              unix_string.cc \
              vx_cal.h
libvx_cal_a_CPPFLAGS = ${MET_CPPFLAGS}

# Build the library when making a distribution so that
# we can make enum_to_string and create all of the generated
# sources.

dist-hook: $(noinst_LIBRARIES)
