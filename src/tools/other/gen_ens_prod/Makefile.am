## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The program

bin_PROGRAMS    = gen_ens_prod
gen_ens_prod_SOURCES = gen_ens_prod.cc \
	gen_ens_prod_conf_info.cc
gen_ens_prod_CPPFLAGS = ${MET_CPPFLAGS}
gen_ens_prod_LDFLAGS = ${MET_LDFLAGS}
gen_ens_prod_LDADD = -lvx_stat_out \
	-lvx_statistics \
	-lvx_shapedata \
	-lvx_gsl_prob \
	-lvx_analysis_util \
	-lvx_data2d_factory \
	-lvx_data2d_nc_met \
	-lvx_data2d_grib \
	$(GRIB2_MET_LIBS) \
	-lvx_data2d_nc_pinterp \
	-lvx_data2d_nccf \
	$(UGRID_MET_LIBS) \
	$(PYTHON_MET_LIBS) \
	-lvx_data2d \
	-lvx_nc_obs \
	-lvx_seeps \
	-lvx_nc_util \
	-lvx_regrid \
	-lvx_grid \
	-lvx_geodesy \
	-lvx_seeps \
	-lvx_config \
	-lvx_color \
	-lvx_util_math \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	$(GRIB2_DEP_LIBS) $(UGRID_DEP_LIBS) $(PYTHON_DEP_LIBS) \
	-lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas

EXTRA_DIST = gen_ens_prod.h \
	gen_ens_prod_conf_info.h
