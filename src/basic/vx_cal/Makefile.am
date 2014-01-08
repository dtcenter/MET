## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

INCLUDES        = -I$(includedir)

include_HEADERS = time_array.h \
              vx_cal.h

lib_LIBRARIES = libvx_cal.a

libvx_cal_a_SOURCES = date_to_mjd.cc date_to_mjd.h \
              day_dif.cc day_dif.h \
              day_of_week.cc day_of_week.h \
              doyhms_to_unix.cc doyhms_to_unix.h \
              is_dst.cc is_dst.h \
              is_leap_year.cc is_leap_year.h \
              mdyhms_to_unix.cc mdyhms_to_unix.h \
              mjd_to_date.cc mjd_to_date.h \
              time_array.cc time_array.h \
              time_strings.cc time_strings.h \
              unix_to_mdyhms.cc unix_to_mdyhms.h
