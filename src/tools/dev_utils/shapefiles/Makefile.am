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
	make_mapfiles

dump_shp_SOURCES = dump_shp.cc

dump_shp_CPPFLAGS = ${MET_CPPFLAGS}
dump_shp_LDFLAGS = ${MET_LDFLAGS}
dump_shp_LDADD = -lvx_config \
	-lvx_gis \
	-lvx_util \
	-lvx_cal \
	-lvx_log \
	-lm

dump_shx_SOURCES = dump_shx.cc 

dump_shx_CPPFLAGS = ${MET_CPPFLAGS}
dump_shx_LDFLAGS = ${MET_LDFLAGS}
dump_shx_LDADD = -lvx_config \
	-lvx_gis \
	-lvx_util \
	-lvx_cal \
	-lvx_log \
	-lm

dump_dbf_SOURCES = dump_dbf.cc

dump_dbf_CPPFLAGS = ${MET_CPPFLAGS}
dump_dbf_LDFLAGS = ${MET_LDFLAGS}
dump_dbf_LDADD = -lvx_config \
	-lvx_gis \
	-lvx_util \
	-lvx_cal \
	-lvx_log \
	-lm

make_mapfiles_SOURCES = make_mapfiles.cc

make_mapfiles_CPPFLAGS = ${MET_CPPFLAGS}
make_mapfiles_LDFLAGS = ${MET_LDFLAGS}
make_mapfiles_LDADD = -lvx_config \
	-lvx_gis \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm

