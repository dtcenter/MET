## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# Test programs

noinst_PROGRAMS = test_velocity

test_velocity_SOURCES = test_velocity.cc
test_velocity_CPPFLAGS = ${MET_CPPFLAGS} -I${top_builddir}/src/tools/other/mode_time_domain
test_velocity_LDFLAGS = -L. ${MET_LDFLAGS} -L${top_builddir}/src/tools/other/mode_time_domain
test_velocity_LDADD = \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-2d_att_array.o \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-2d_att.o \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-2d_moments.o \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-3d_att.o \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-3d_att_pair_array.o \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-3d_att_single_array.o \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-3d_conv.o \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-3d_moments.o \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-fo_graph.o \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-fo_node_array.o \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-fo_node.o \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-interest_calc.o \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-mm_engine.o \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-mtd_config_info.o \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-mtd_file_base.o \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-mtd_file_float.o \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-mtd_file_int.o \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-mtdfiletype_to_string.o \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-mtd_nc_output.o \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-mtd_partition.o \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-mtd_read_data.o \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-mtd_txt_output.o \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-nc_grid.o \
	${top_builddir}/src/tools/other/mode_time_domain/mtd-nc_utils_local.o \
	-lvx_pxm \
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
