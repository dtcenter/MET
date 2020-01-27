## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# Check for BUFRLIB_NAME

if BUFRLIB_NAME_SET
BLIB_NAME = ${BUFRLIB_NAME}
else
BLIB_NAME = -lbufr
endif


# The program

bin_PROGRAMS  = pb2nc
pb2nc_SOURCES = pb2nc.cc \
	pb2nc_conf_info.cc \
	calcape.f \
	calpbl.f \
	numpbmsg.f \
	openpb.f \
	closepb.f \
	readpb.f \
	dumppb.f
pb2nc_CPPFLAGS = ${MET_CPPFLAGS}
pb2nc_LDFLAGS = ${MET_LDFLAGS}
pb2nc_LDADD = -lvx_stat_out \
	-lvx_statistics \
	-lvx_shapedata \
	-lvx_gsl_prob \
	-lvx_analysis_util \
	-lvx_data2d_factory \
	-lvx_data2d_nc_met \
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_data2d_nc_pinterp \
	-lvx_data2d_nccf \
	$(PYTHON_LIBS) \
	-lvx_data2d \
	-lvx_nc_obs \
	-lvx_nc_util \
	-lvx_regrid \
	-lvx_grid \
	-lvx_config \
	-lvx_gsl_prob \
	-lvx_pb_util \
	-lvx_cal \
	-lvx_util \
	$(PYTHON_LIBS) \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	-lvx_summary \
	-lm -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas $(BLIB_NAME) \
	$(FLIBS)

EXTRA_DIST = readpb.prm \
	pb2nc_conf_info.h
