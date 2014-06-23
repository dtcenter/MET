## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

bin_PROGRAMS    = wwmca_regrid \
	wwmca_plot

wwmca_regrid_SOURCES = wwmca_regrid.cc \
	afwa_file.cc \
	afwa_cp_file.cc \
	afwa_pt_file.cc \
	interp_base.cc \
	min_interp.cc \
	max_interp.cc \
	ave_interp.cc \
	nearest_interp.cc \
	gridhemisphere_to_string.cc \
	wwmca_ref.cc \
	nc_output.cc
wwmca_regrid_CPPFLAGS = -I${includedir}
wwmca_regrid_LDFLAGS = -L${libdir}
wwmca_regrid_LDADD = -lvx_pxm \
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
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_data2d_nc_met \
	-lvx_data2d_nc_pinterp \
	-lvx_data2d_nccf \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_grid \
	-lvx_config \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	-lm -lz -lnetcdf_c++ -lnetcdf -lgsl -lgslcblas

wwmca_plot_SOURCES = wwmca_plot.cc \
	afwa_file.cc \
	afwa_cp_file.cc \
	afwa_pt_file.cc \
	interp_base.cc \
	min_interp.cc \
	max_interp.cc \
	ave_interp.cc \
	nearest_interp.cc \
	gridhemisphere_to_string.cc \
	wwmca_ref.cc \
	nc_output.cc
wwmca_plot_CPPFLAGS = -I${includedir}
wwmca_plot_LDFLAGS = -L${libdir}
wwmca_plot_LDADD = -lvx_pxm \
	-lvx_plot_util \
	-lvx_nav \
	-lvx_ps \
	-lvx_afm \
	-lvx_render \
	-lvx_stat_out \
	-lvx_statistics \
	-lvx_shapedata \
	-lvx_gsl_prob \
	-lvx_analysis_util \
	-lvx_data2d_factory \
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_data2d_nc_met \
	-lvx_data2d_nc_pinterp \
	-lvx_data2d_nccf \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_grid \
	-lvx_config \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	-lm -lz -lnetcdf_c++ -lnetcdf -lgsl -lgslcblas

# Create generated files
gridhemisphere_to_string.h gridhemisphere_to_string.cc: wwmca_ref.h
	${bindir}/enum_to_string wwmca_ref.h

# Clean up extra local files
clean-local:
	-rm -f gridhemisphere_to_string.h
	-rm -f gridhemisphere_to_string.cc
