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
	min_interp.cc min_interp.h \
	max_interp.cc max_interp.h \
	ave_interp.cc ave_interp.h \
	nearest_interp.cc nearest_interp.h \
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
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_data2d_nc_pinterp \
	$(PYTHON_LIBS) \
	-lvx_data2d_nccf \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_regrid \
	-lvx_grid \
	-lvx_config \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	-lm -lz -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas

wwmca_plot_SOURCES = gridhemisphere_to_string.cc gridhemisphere_to_string.h \
	wwmca_plot.cc \
	af_file.cc af_file.h \
	af_cp_file.cc af_cp_file.h \
	af_pt_file.cc af_pt_file.h \
	interp_base.cc interp_base.h \
	min_interp.cc min_interp.h \
	max_interp.cc max_interp.h \
	ave_interp.cc ave_interp.h \
	nearest_interp.cc nearest_interp.h \
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
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_data2d_nc_pinterp \
	$(PYTHON_LIBS) \
	-lvx_data2d_nccf \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_regrid \
	-lvx_grid \
	-lvx_config \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	-lm -lz -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas

# If we are in development mode, generate the "to_string" files and
# clean them up in a "make clean".  We don't want to do either of these
# in a distribution because the enum_to_string utility isn't available.

if ENABLE_DEVELOPMENT
    gridhemisphere_to_string.cc gridhemisphere_to_string.h: wwmca_ref.h
	${ENUM_TO_STRING} wwmca_ref.h

   clean-local:
	-rm -f gridhemisphere_to_string.cc
	-rm -f gridhemisphere_to_string.h
endif
