## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The programs

bin_PROGRAMS    = wwmca_regrid \
	wwmca_plot

wwmca_regrid_SOURCES = gridhemisphere_to_string.cc gridhemisphere_to_string.h \
	wwmca_regrid.cc \
	af_file.cc af_file.h \
	af_cp_file.cc af_cp_file.h \
	af_pt_file.cc af_pt_file.h \
	interp_base.cc interp_base.h \
	wwmca_ref.cc wwmca_ref.h \
	nc_output.cc
wwmca_regrid_CPPFLAGS = ${MET_CPPFLAGS}
wwmca_regrid_LDFLAGS = ${MET_LDFLAGS}
wwmca_regrid_LDADD = -lvx_pxm \
	-lvx_plot_util \
	-lvx_nav \
	-lvx_ps \
	-lvx_render \
	-lvx_afm \
	-lvx_stat_out \
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
	-lvx_statistics \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_regrid \
	-lvx_grid \
	-lvx_geodesy \
	-lvx_config \
	-lvx_gsl_prob \
	-lvx_cal \
	-lvx_util_math \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	$(GRIB2_DEP_LIBS) $(UGRID_DEP_LIBS) $(PYTHON_DEP_LIBS) \
	-lm -lproj -lz -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas

wwmca_plot_SOURCES = gridhemisphere_to_string.cc gridhemisphere_to_string.h \
	wwmca_plot.cc \
	af_file.cc af_file.h \
	af_cp_file.cc af_cp_file.h \
	af_pt_file.cc af_pt_file.h \
	interp_base.cc interp_base.h \
	wwmca_ref.cc wwmca_ref.h
wwmca_plot_CPPFLAGS = ${MET_CPPFLAGS}
wwmca_plot_LDFLAGS = ${MET_LDFLAGS}
wwmca_plot_LDADD = -lvx_pxm \
	-lvx_plot_util \
	-lvx_nav \
	-lvx_ps \
	-lvx_render \
	-lvx_afm \
	-lvx_stat_out \
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
	-lvx_nc_util \
	-lvx_regrid \
	-lvx_grid \
	-lvx_geodesy \
	-lvx_config \
	-lvx_gsl_prob \
	-lvx_cal \
	-lvx_util_math \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	$(GRIB2_DEP_LIBS) $(UGRID_DEP_LIBS) $(PYTHON_DEP_LIBS) \
	-lm -lproj -lz -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas

# If we are in development mode, generate the "to_string" files and
# clean them up in a "make clean".  We don't want to do either of these
# in a distribution because the enum_to_string utility isn't available.

if ENABLE_DEVELOPMENT

# BUILT_SOURCES so these are generated before anything that
# includes them is compiled.  Without it, a development build
# whose .deps are empty - as after the make clean that removes
# these very files - compiles sources in _SOURCES order and can
# reach a consumer before its header has been generated.

BUILT_SOURCES = gridhemisphere_to_string.cc \
                gridhemisphere_to_string.h

gridhemisphere_to_string.cc gridhemisphere_to_string.h: wwmca_ref.h
	${ENUM_TO_STRING} -concat_string wwmca_ref.h

clean-local:
	-rm -f gridhemisphere_to_string.cc
	-rm -f gridhemisphere_to_string.h
endif
