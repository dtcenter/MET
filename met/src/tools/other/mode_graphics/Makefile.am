## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The programs

bin_PROGRAMS    = plot_mode_field

plot_mode_field_SOURCES = cgraphbase_plottype_to_string.cc cgraphbase_plottype_to_string.h \
	color_stack.cc color_stack.h \
	cgraph_font.cc cgraph_font.h \
	cgraph_main.cc cgraph_main.h \
	gs_ps_map.h \
	plot_mode_field.cc \
	mode_nc_output_file.cc mode_nc_output_file.h \
	cgraph.h
plot_mode_field_CPPFLAGS = ${MET_CPPFLAGS}
plot_mode_field_LDFLAGS = -L. ${MET_LDFLAGS}
plot_mode_field_LDADD = -lvx_config \
	-lvx_plot_util \
	-lvx_data2d_nc_met \
	$(PYTHON_LIBS) \
	-lvx_grid \
	-lvx_nc_util \
	-lvx_ps \
	-lvx_afm \
	-lvx_pxm \
	-lvx_render \
	-lvx_color \
	-lvx_util \
	-lvx_cal \
	-lvx_log \
	-lvx_math \
	-lnetcdf_c++4 \
	-lnetcdf \
	-lcairo \
	-lfreetype \
	-lm \
	-lz

# If we are in development mode, generate the "to_string" files and
# clean them up in a "make clean".  We don't want to do either of these
# in a distribution because the enum_to_string utility isn't available.

if ENABLE_DEVELOPMENT
    cgraphbase_plottype_to_string.cc cgraphbase_plottype_to_string.h: cgraph_main.h
	${ENUM_TO_STRING} -prefix -concat_string cgraph_main.h

    clean-local:
	-rm -f cgraphbase_plottype_to_string.cc
	-rm -f cgraphbase_plottype_to_string.h
endif
