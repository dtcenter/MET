## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

INCLUDES        = -I$(includedir)

include_HEADERS = time_array.h \
              vx_cal.h

lib_LIBRARIES = libvx_cal.a

libvx_cal_a_SOURCES = date_to_mjd.cc \
              day_dif.cc \
              day_of_week.cc \
              doyhms_to_unix.cc \
              is_dst.cc \
              is_leap_year.cc \
              mdyhms_to_unix.cc \
              mjd_to_date.cc \
              time_array.cc \
              time_strings.cc \
              unix_to_mdyhms.cc
