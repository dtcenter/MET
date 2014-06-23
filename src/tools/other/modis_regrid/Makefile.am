## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

bin_PROGRAMS    = modis_regrid
modis_regrid_SOURCES = modis_regrid.cc \
	sat_utils.cc \
	cloudsat_swath_file.cc \
	data_plane_to_netcdf.cc \
	data_averager.cc \
	modis_file.cc
modis_regrid_CPPFLAGS = -I${includedir}
modis_regrid_LDFLAGS = -L${libdir}
modis_regrid_LDADD = -lvx_pxm \
	-lvx_plot_util \
	-lvx_nav \
	-lvx_render \
	-lvx_ps \
	-lvx_afm \
	-lvx_stat_out \
	-lvx_statistics \
	-lvx_shapedata \
	-lvx_gsl_prob \
	-lvx_analysis_util \
	-lvx_data2d_factory \
	-lvx_data2d_nc_met \
	-lvx_data2d_nccf \
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_data2d_nc_pinterp \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_grid \
	-lvx_config \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	-lnetcdf_c++ -lnetcdf -lgsl -lgslcblas \
	-lhdfeos \
	-lmfhdf \
	-ldf \
	-lGctp \
	-ljpeg \
	-lz \
	-lm

