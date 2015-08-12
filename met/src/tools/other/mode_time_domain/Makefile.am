## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The program

bin_PROGRAMS    = mtd
mtd_SOURCES = mtd.cc  \
              mtdfiletype_to_string.h   \
              mtdfiletype_to_string.cc  \
              nc_utils.cc  \
              nc_grid.cc  \
              3d_moments.h  \
              3d_moments.cc  \
              mtd_config_info.cc  \
              mtd_file_base.cc  \
              mtd_file_float.cc  \
              mtd_file_int.cc
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
	-lm -lnetcdf_c++ -lnetcdf -lgsl -lgslcblas -lz

# If we are in development mode, generate the "to_string" files and
# clean them up in a "make clean".  We don't want to do either of these
# in a distribution because the enum_to_string utility isn't available.

if ENABLE_DEVELOPMENT
    mtdfiletype_to_string.cc mtdfiletype_to_string.h : mtd_file.h
	${ENUM_TO_STRING} -concat_string -reverse mtd_file.h

    clean-local:
	-rm -f mtdfiletype_to_string.cc
	-rm -f mtdfiletype_to_string.h
endif

#EXTRA_DIST = mtd_exec.h
