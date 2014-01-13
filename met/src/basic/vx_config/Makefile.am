## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Test programs

noinst_PROGRAMS = test_config \
              test_lookup \
              test_lookup2 \
              test_lookup3 \
              test_string

test_config_SOURCES = test_config.cc
test_config_CPPFLAGS = -I${includedir}
test_config_LDFLAGS = -L${libdir}
test_config_LDADD = -lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm

test_lookup_SOURCES = test_lookup.cc
test_lookup_CPPFLAGS = -I${includedir}
test_lookup_LDFLAGS = -L${libdir}
test_lookup_LDADD = -lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm

test_lookup2_SOURCES = test_lookup2.cc
test_lookup2_CPPFLAGS = -I${includedir}
test_lookup2_LDFLAGS = -L${libdir}
test_lookup2_LDADD = -lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm

test_lookup3_SOURCES = test_lookup3.cc
test_lookup3_CPPFLAGS = -I${includedir}
test_lookup3_LDFLAGS = -L${libdir}
test_lookup3_LDADD = -lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm

test_string_SOURCES = test_string.cc
test_string_CPPFLAGS = -I${includedir}
test_string_LDFLAGS = -L${libdir}
test_string_LDADD = -lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm

# The library itself

AM_YFLAGS	= -d -p config
AM_LFLAGS	= --prefix=config --outfile=lex.yy.c

lib_LIBRARIES = libvx_config.a
libvx_config_a_SOURCES = config.tab.yy config_scanner.ll \
              configobjecttype_to_string.cc \
              grdfiletype_to_string.cc \
              dictionary.cc \
              config_file.cc \
              config_util.cc
libvx_config_a_CPPFLAGS = -I${includedir}

include_HEADERS = config_file.h \
              configobjecttype_to_string.h \
              grdfiletype_to_string.h \
              data_file_type.h \
              config.tab.h \
              dictionary.h \
              object_types.h \
              scanner_stuff.h \
              config_util.h \
              config_constants.h \
              vx_config.h

# Build rule for configobjecttype_to_string.cc
configobjecttype_to_string.h configobjecttype_to_string.cc: object_types.h
	${bindir}/enum_to_string -concat_string object_types.h

# Build rule for configobjecttype_to_string.cc
grdfiletype_to_string.h grdfiletype_to_string.cc: data_file_type.h
	${bindir}/enum_to_string -concat_string -reverse data_file_type.h

# Clean up extra local files
clean-local:
	-rm -f config.tab.cc
	-rm -f config.tab.h
	-rm -f config_scanner.cc
	-rm -f configobjecttype_to_string.h
	-rm -f configobjecttype_to_string.cc
	-rm -f grdfiletype_to_string.h
	-rm -f grdfiletype_to_string.cc
