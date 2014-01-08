## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

lib_LIBRARIES = libvx_data2d_grib.a
libvx_data2d_grib_a_SOURCES = grib_classes.cc \
              grib_strings.cc \
              grib_utils.cc \
              var_info_grib.cc \
              data2d_grib_utils.cc \
              data2d_grib.cc
libvx_data2d_grib_a_CPPFLAGS = -I${includedir}

include_HEADERS = grib_classes.h \
              grib_strings.h \
              grib_utils.h \
              var_info_grib.h \
              data2d_grib_utils.h \
              data2d_grib.h \
              vx_data2d_grib.h

# Build rule for leveltype_to_string.cc
leveltype_to_string.h leveltype_to_string.cc: level_info.h
	${bindir}/enum_to_string -concat_string level_info.h

# Clean up extra local files
clean-local:
	-rm -f leveltype_to_string.h
	-rm -f leveltype_to_string.cc
