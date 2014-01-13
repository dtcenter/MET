## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

bin_PROGRAMS    = pb2nc
pb2nc_SOURCES = pb2nc.cc \
	pb2nc_conf_info.cc \
	numpbmsg.f \
	openpb.f \
	closepb.f \
	readpb.f \
	dumppb.f
pb2nc_CPPFLAGS = -DMET_BASE_DIR=\"$(abs_top_builddir)/..\" \
	-I${includedir}
pb2nc_LDFLAGS = -L${libdir}
pb2nc_LDADD = -lvx_stat_out \
	-lvx_statistics \
	-lvx_shapedata \
	-lvx_gsl_prob \
	-lvx_analysis_util \
	-lvx_data2d_grib \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_grid \
	-lvx_config \
	-lvx_pb_util \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	-lm -lnetcdf_c++ -lnetcdf -lgsl -lgslcblas -lbufr \
	-lgfortran
