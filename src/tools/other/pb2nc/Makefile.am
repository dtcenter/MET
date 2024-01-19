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
	-lvx_data2d_grib \
	$(GRIB2_MET_LIBS) \
	-lvx_data2d_nc_wrf \
	-lvx_data2d_nc_cf \
	$(UGRID_MET_LIBS) \
	$(PYTHON_MET_LIBS) \
	-lvx_data2d \
	-lvx_nc_obs \
	-lvx_nc_util \
	-lvx_regrid \
	-lvx_grid \
	-lvx_geodesy \
	-lvx_pb_util \
	-lvx_summary \
	-lvx_config \
	-lvx_gsl_prob \
	-lvx_color \
	-lvx_util_math \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	$(GRIB2_DEP_LIBS) $(UGRID_DEP_LIBS) $(PYTHON_DEP_LIBS) \
	-lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas $(BLIB_NAME) \
	$(FLIBS)

EXTRA_DIST = readpb.prm \
	pb2nc_conf_info.h
