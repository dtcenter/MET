MAINTAINERCLEANFILES	= Makefile.in

include ${top_srcdir}/Make-include

bin_PROGRAMS = mode_multivar \

mode_multivar_SOURCES =  \
     objects_from_netcdf.h  \
     objects_from_netcdf.cc  \
     combine_boolplanes.h  \
     combine_boolplanes.cc  \
     mode_multivar.cc

mode_multivar_CPPFLAGS = ${MET_CPPFLAGS} -I../../../libcode/vx_bool_calc
mode_multivar_LDFLAGS = ${MET_LDFLAGS} -L../../../libcode/vx_bool_calc
mode_multivar_LDADD = \
	-lvx_bool_calc \
     -lvx_pxm \
     -lvx_color \
	-lvx_config \
	-lvx_log \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_nc_util \
	-lm -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas -lz

EXTRA_DIST = mode_multivar_conf_info.h \
	mode_multivar.h
