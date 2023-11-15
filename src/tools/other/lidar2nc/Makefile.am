## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The program

bin_PROGRAMS    = lidar2nc
lidar2nc_SOURCES = lidar2nc.cc  \
                   hdf_utils.h  \
                   hdf_utils.cc  \
                   calipso_5km.cc  \
                   calipso_5km.h
lidar2nc_CPPFLAGS = ${MET_CPPFLAGS} \
	-I../../../libcode/vx_data2d_factory
lidar2nc_LDFLAGS = ${MET_LDFLAGS}
lidar2nc_LDADD = -lvx_shapedata \
	-lvx_config  \
	-lvx_analysis_util \
	-lvx_data2d_factory \
	-lvx_data2d_nc_met \
	$(PYTHON_MET_LIBS) $(PYTHON_DEP_LIBS) \
	-lvx_data2d_nccf \
	$(UGRID_MET_LIBS) $(UGRID_DEP_LIBS) \
	-lvx_statistics \
	-lvx_nc_obs \
	-lvx_nc_util \
	-lvx_data2d \
	-lvx_render \
	-lvx_plot_util \
	-lvx_pxm \
	-lvx_color \
	-lvx_ps \
	-lvx_afm \
	-lvx_pb_util \
	-lvx_nav \
	-lvx_gnomon \
	-lvx_stat_out \
	-lvx_statistics \
	-lvx_grid \
	-lvx_geodesy \
	-lvx_util_math \
	-lvx_util \
	-lvx_config \
	-lvx_gsl_prob \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lvx_util \
	$(PYTHON_MET_LIBS) $(PYTHON_DEP_LIBS) \
	-lvx_summary \
	-lmfhdf -ldf -ljpeg \
	-lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas -lz
