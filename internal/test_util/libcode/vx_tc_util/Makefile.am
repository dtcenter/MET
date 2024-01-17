## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

if ENABLE_PYTHON
LDFLAGS += -lvx_python3_utils
endif

# Test programs

noinst_PROGRAMS = test_read \
	test_read_prob \
	test_read_rmw

test_read_SOURCES = test_read.cc
test_read_CPPFLAGS = ${MET_CPPFLAGS}
test_read_LDFLAGS = -L. ${MET_LDFLAGS}
test_read_LDADD = -lvx_stat_out \
	-lvx_statistics \
	-lvx_shapedata \
	-lvx_gsl_prob \
	-lvx_analysis_util \
	-lvx_tc_util \
	-lvx_shapedata \
	-lvx_util \
	$(PYTHON_MET_LIBS) $(PYTHON_DEP_LIBS) \
	-lvx_statistics \
	-lvx_data2d_factory \
	-lvx_data2d_nc_met \
	-lvx_data2d_nc_wrf \
	$(PYTHON_MET_LIBS) $(PYTHON_DEP_LIBS) \
	-lvx_data2d_nc_cf \
	-lvx_data2d_grib $(GRIB2_MET_LIBS) $(GRIB2_DEP_LIBS) \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_regrid \
	-lvx_grid \
	-lvx_geodesy \
	-lvx_config \
	-lvx_gsl_prob \
	-lvx_cal \
	-lvx_nav \
	-lvx_util_math \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	-lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas -lvx_util

test_read_prob_SOURCES = test_read_prob.cc
test_read_prob_CPPFLAGS = ${MET_CPPFLAGS}
test_read_prob_LDFLAGS = -L. ${MET_LDFLAGS}
test_read_prob_LDADD = -lvx_stat_out \
	-lvx_statistics \
	-lvx_shapedata \
	-lvx_gsl_prob \
	-lvx_analysis_util \
	-lvx_tc_util \
	-lvx_shapedata \
	-lvx_util_math \
	-lvx_util \
	$(PYTHON_MET_LIBS) $(PYTHON_DEP_LIBS) \
	-lvx_statistics \
	-lvx_data2d_factory \
	-lvx_data2d_nc_met \
	-lvx_data2d_nc_wrf \
	$(PYTHON_MET_LIBS) $(PYTHON_DEP_LIBS) \
	-lvx_data2d_nc_cf \
	-lvx_data2d_grib $(GRIB2_MET_LIBS) $(GRIB2_DEP_LIBS) \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_regrid \
	-lvx_grid \
	-lvx_geodesy \
	-lvx_config \
	-lvx_gsl_prob \
	-lvx_cal \
	-lvx_nav \
	-lvx_util_math \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	-lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas -lvx_util

test_read_rmw_SOURCES = test_read_rmw.cc
test_read_rmw_CPPFLAGS = ${MET_CPPFLAGS}
test_read_rmw_LDFLAGS = -L. ${MET_LDFLAGS}
test_read_rmw_LDADD = -lvx_stat_out \
	-lvx_statistics \
	-lvx_shapedata \
	-lvx_gsl_prob \
	-lvx_analysis_util \
	-lvx_tc_util \
	-lvx_shapedata \
	-lvx_util_math \
	-lvx_util \
	$(PYTHON_MET_LIBS) $(PYTHON_DEP_LIBS) \
	-lvx_statistics \
	-lvx_data2d_factory \
	-lvx_data2d_nc_met \
	-lvx_data2d_nc_pinterp \
	$(PYTHON_MET_LIBS) $(PYTHON_DEP_LIBS) \
	-lvx_data2d_nccf \
	-lvx_data2d_grib $(GRIB2_MET_LIBS) $(GRIB2_DEP_LIBS) \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_regrid \
	-lvx_grid \
	-lvx_geodesy \
	-lvx_config \
	-lvx_gsl_prob \
	-lvx_cal \
	-lvx_nav \
	-lvx_util_math \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	-lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas -lvx_util

