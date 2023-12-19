## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

OPT_PYTHON_SOURCES =

if ENABLE_PYTHON
OPT_PYTHON_SOURCES += python_handler.h python_handler.cc
endif

# The program

bin_PROGRAMS     = ascii2nc
ascii2nc_SOURCES = ascii2nc.cc \
	ascii2nc_conf_info.cc ascii2nc_conf_info.h \
	file_handler.cc file_handler.h \
	little_r_handler.cc little_r_handler.h \
	met_handler.cc met_handler.h \
	surfrad_handler.cc surfrad_handler.h \
	wwsis_handler.cc wwsis_handler.h \
	airnow_handler.cc airnow_handler.h \
	ndbc_handler.cc ndbc_handler.h \
	ndbc_locations.cc ndbc_locations.h \
	airnow_locations.cc airnow_locations.h \
	aeronet_handler.cc aeronet_handler.h $(OPT_PYTHON_SOURCES)

ascii2nc_CPPFLAGS = ${MET_CPPFLAGS} -I../../../basic/vx_log
ascii2nc_LDFLAGS = ${MET_LDFLAGS}
ascii2nc_LDADD = -lvx_stat_out \
	-lvx_statistics \
	-lvx_shapedata \
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
	-lvx_summary \
	-lvx_pb_util \
	-lvx_config \
	-lvx_gsl_prob \
	-lvx_color \
	-lvx_util_math \
	-lvx_util \
	-lvx_cal \
	-lvx_math \
	-lvx_log \
	$(GRIB2_DEP_LIBS) $(UGRID_DEP_LIBS) $(PYTHON_DEP_LIBS) \
	-lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas



