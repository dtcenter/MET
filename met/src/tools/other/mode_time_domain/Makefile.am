## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The program
bin_PROGRAMS    = mtd
mtd_SOURCES = mtd.cc \
	mtdfiletype_to_string.cc mtdfiletype_to_string.h \
	nc_utils_local.cc nc_utils_local.h \
	nc_grid.cc nc_grid.h \
	3d_moments.cc 3d_moments.h \
	2d_moments.cc 2d_moments.h \
	fo_node.cc fo_node.h \
	fo_node_array.cc fo_node_array.h \
	fo_graph.cc fo_graph.h \
	mtd_config_info.cc mtd_config_info.h \
	mtd_file_base.cc mtd_file_base.h mtd_file.h \
	mtd_file_float.cc mtd_file_float.h \
	mtd_file_int.cc mtd_file_int.h \
	mtd_read_data.cc mtd_read_data.h \
	3d_att.cc 3d_att.h 3d_txt_header.h \
	3d_att_single_array.cc 3d_att_single_array.h \
	3d_att_pair_array.cc 3d_att_pair_array.h \
	2d_att.cc 2d_att.h \
	2d_att_array.cc 2d_att_array.h \
	3d_conv.cc \
	3d_pair_columns.h 3d_single_columns.h 2d_columns.h \
	mtd_txt_output.cc mtd_txt_output.h \
	mtd_nc_output.cc mtd_nc_output.h mtd_nc_defs.h \
	interest_calc.cc interest_calc.h \
	mtd_partition.cc mtd_partition.h \
	mm_engine.cc mm_engine.h
mtd_CPPFLAGS = ${MET_CPPFLAGS}
mtd_LDFLAGS = ${MET_LDFLAGS}
mtd_LDADD = -lvx_pxm \
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
	-lm -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas -lz

# If we are in development mode, generate the "to_string" files and
# clean them up in a "make clean".  We don't want to do either of these
# in a distribution because the enum_to_string utility isn't available.

if ENABLE_DEVELOPMENT
    mtdfiletype_to_string.cc mtdfiletype_to_string.h : mtd_file_base.h
	${ENUM_TO_STRING} -concat_string -reverse mtd_file_base.h

    clean-local:
	-rm -f mtdfiletype_to_string.cc
	-rm -f mtdfiletype_to_string.h
endif

#EXTRA_DIST = mtd_exec.h
