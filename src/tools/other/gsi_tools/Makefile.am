## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The programs

bin_PROGRAMS    = gsid2mpr \
	gsidens2orank

gsid2mpr_SOURCES = gsid2mpr.h \
	gsi_record.h \
	gsi_record.cc \
	conv_record.h \
	conv_record.cc \
	rad_record.h \
	rad_record.cc \
	gsi_util.h \
	gsi_util.cc \
	gsid2mpr.cc \
	conv_offsets.h \
	rad_offsets.h \
	rad_config.h  \
	rad_config.cc \
	ftto.h
gsid2mpr_CPPFLAGS = ${MET_CPPFLAGS}
gsid2mpr_LDFLAGS = ${MET_LDFLAGS}
gsid2mpr_LDADD = -lvx_stat_out \
	-lvx_statistics \
	-lvx_shapedata \
	-lvx_gsl_prob \
	-lvx_analysis_util \
	-lvx_shapedata \
	-lvx_util \
	-lvx_data2d_factory \
	-lvx_data2d_nc_met \
	-lvx_data2d_grib \
	$(GRIB2_MET_LIBS) \
	-lvx_data2d_nc_wrf \
	-lvx_data2d_nc_cf \
	$(UGRID_MET_LIBS) \
	$(PYTHON_MET_LIBS) \
	-lvx_statistics \
	-lvx_data2d \
	-lvx_seeps \
	-lvx_nc_util \
	-lvx_regrid \
	-lvx_grid \
	-lvx_geodesy \
	-lvx_seeps \
	-lvx_config \
	-lvx_gsl_prob \
	-lvx_cal \
	-lvx_util_math \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	$(GRIB2_DEP_LIBS) $(UGRID_DEP_LIBS) $(PYTHON_DEP_LIBS) \
	-lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas -lvx_util

gsidens2orank_SOURCES = gsidens2orank.h \
	gsi_record.h \
	gsi_record.cc \
	conv_record.h \
	conv_record.cc \
	rad_record.h \
	rad_record.cc \
	gsi_util.h \
	gsi_util.cc \
	gsidens2orank.cc \
	conv_offsets.h \
	rad_offsets.h \
	rad_config.h  \
	rad_config.cc \
	ftto.h
gsidens2orank_CPPFLAGS = ${MET_CPPFLAGS}
gsidens2orank_LDFLAGS = ${MET_LDFLAGS}
gsidens2orank_LDADD = -lvx_stat_out \
	-lvx_statistics \
	-lvx_shapedata \
	-lvx_gsl_prob \
	-lvx_analysis_util \
	-lvx_shapedata \
	-lvx_util \
	-lvx_data2d_factory \
	-lvx_data2d_nc_met \
	-lvx_data2d_grib \
	$(GRIB2_MET_LIBS) \
	-lvx_data2d_nc_wrf \
	-lvx_data2d_nc_cf \
	$(UGRID_MET_LIBS) \
	$(PYTHON_MET_LIBS) \
	-lvx_statistics \
	-lvx_data2d \
	-lvx_seeps \
	-lvx_nc_util \
	-lvx_regrid \
	-lvx_grid \
	-lvx_geodesy \
	-lvx_seeps \
	-lvx_config \
	-lvx_gsl_prob \
	-lvx_cal \
	-lvx_util_math \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	$(GRIB2_DEP_LIBS) $(UGRID_DEP_LIBS) $(PYTHON_DEP_LIBS) \
	-lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas -lvx_util

