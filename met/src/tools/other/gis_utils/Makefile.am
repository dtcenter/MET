## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The programs

bin_PROGRAMS	= gis_dump_shp \
	            gis_dump_shx \
	            gis_dump_dbf

gis_dump_shp_SOURCES = gis_dump_shp.cc

gis_dump_shp_CPPFLAGS = ${MET_CPPFLAGS}
gis_dump_shp_LDFLAGS = ${MET_LDFLAGS}
gis_dump_shp_LDADD = -lvx_config \
	-lvx_gis \
	-lvx_util \
	-lvx_cal \
	-lvx_log \
	-lm

gis_dump_shx_SOURCES = gis_dump_shx.cc 

gis_dump_shx_CPPFLAGS = ${MET_CPPFLAGS}
gis_dump_shx_LDFLAGS = ${MET_LDFLAGS}
gis_dump_shx_LDADD = -lvx_config \
	-lvx_gis \
	-lvx_util \
	-lvx_cal \
	-lvx_log \
	-lm

gis_dump_dbf_SOURCES = gis_dump_dbf.cc

gis_dump_dbf_CPPFLAGS = ${MET_CPPFLAGS}
gis_dump_dbf_LDFLAGS = ${MET_LDFLAGS}
gis_dump_dbf_LDADD = -lvx_config \
	-lvx_gis \
	-lvx_util \
	-lvx_cal \
	-lvx_log \
	-lm

