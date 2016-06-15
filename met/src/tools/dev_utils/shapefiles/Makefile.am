## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The programs

bin_PROGRAMS	= dump_shp \
	dump_shx \
	dump_dbf \
	reformat_country_shapefile

dump_shp_SOURCES = dump_shp.cc \
	shapetype_to_string.cc shapetype_to_string.h \
	shp_file.cc shp_file.h

dump_shp_CPPFLAGS = ${MET_CPPFLAGS}
dump_shp_LDFLAGS = ${MET_LDFLAGS}
dump_shp_LDADD = -lvx_config \
	-lvx_util \
	-lvx_cal \
	-lvx_log \
	-lm

dump_shx_SOURCES = dump_shx.cc \
	shapetype_to_string.cc shapetype_to_string.h \
	shp_file.cc shp_file.h \
	shx_file.cc shx_file.h

dump_shx_CPPFLAGS = ${MET_CPPFLAGS}
dump_shx_LDFLAGS = ${MET_LDFLAGS}
dump_shx_LDADD = -lvx_config \
	-lvx_util \
	-lvx_cal \
	-lvx_log \
	-lm

dump_dbf_SOURCES = dump_dbf.cc \
	dbf_file.cc dbf_file.h

dump_dbf_CPPFLAGS = ${MET_CPPFLAGS}
dump_dbf_LDFLAGS = ${MET_LDFLAGS}
dump_dbf_LDADD = -lvx_config \
	-lvx_util \
	-lvx_cal \
	-lvx_log \
	-lm

reformat_country_shapefile_SOURCES = reformat_country_shapefile.cc \
	shapetype_to_string.cc shapetype_to_string.h \
	shp_file.cc shp_file.h \
	dbf_file.cc dbf_file.h

reformat_country_shapefile_CPPFLAGS = ${MET_CPPFLAGS}
reformat_country_shapefile_LDFLAGS = ${MET_LDFLAGS}
reformat_country_shapefile_LDADD = -lvx_config \
	-lvx_util \
	-lvx_cal \
	-lvx_log \
	-lm

# If we are in development mode, generate the "to_string" files and
# clean them up in a "make clean".  We don't want to do either of these
# in a distribution because the enum_to_string utility isn't available.

if ENABLE_DEVELOPMENT
    shapetype_to_string.h shapetype_to_string.cc: shp_file.h
	${ENUM_TO_STRING} -concat_string shp_file.h

   clean-local:
	-rm -f shapetype_to_string.cc
	-rm -f shapetype_to_string.h
endif

# Build chk4copyright when making a distribution so that we can
#  add missing copyrights to the source files

dist-hook: chk4copyright

