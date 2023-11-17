## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The program

bin_PROGRAMS    = pcp_combine
pcp_combine_SOURCES = pcp_combine.cc
pcp_combine_CPPFLAGS = ${MET_CPPFLAGS}
pcp_combine_LDFLAGS = ${MET_LDFLAGS}
pcp_combine_LDADD = -lvx_data2d_factory \
	-lvx_data2d_nc_met \
	-lvx_data2d_grib \
	$(GRIB2_MET_LIBS) \
	-lvx_data2d_nc_pinterp \
	-lvx_data2d_nccf \
	$(UGRID_MET_LIBS) \
	$(PYTHON_MET_LIBS) \
	-lvx_statistics \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_regrid \
	-lvx_grid \
	-lvx_geodesy \
	-lvx_config \
	-lvx_cal \
	-lvx_util_math \
	-lvx_util \
	-lvx_gsl_prob \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	$(GRIB2_DEP_LIBS) $(UGRID_DEP_LIBS) $(PYTHON_DEP_LIBS) \
	-lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas
