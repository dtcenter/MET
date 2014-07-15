## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The vx_cgraph library

noinst_LIBRARIES = libvx_cgraph.a
libvx_cgraph_a_SOURCES = cgraphbase_plottype_to_string.cc \
	cgraphbase_fontfamily_to_string.cc \
	color_stack.cc \
	cgraph_font.cc \
	cgraph_main.cc
libvx_cgraph_a_CPPFLAGS = ${MET_CPPFLAGS}

# The plot_mode_field tool

bin_PROGRAMS    = plot_mode_field
nodist_plot_mode_field_SOURCES = plot_mode_field.cc mode_nc_output_file.cc
plot_mode_field_CPPFLAGS = ${MET_CPPFLAGS}
plot_mode_field_LDFLAGS = -L. ${MET_LDFLAGS}
plot_mode_field_LDADD = -lvx_cgraph \
        -lvx_config \
        -lvx_plot_util \
        -lvx_data2d_nc_met \
        -lvx_grid \
        -lvx_ps \
        -lvx_afm \
        -lvx_pxm \
        -lvx_render \
        -lvx_color \
        -lvx_util \
        -lvx_cal \
        -lvx_log \
        -lvx_math \
        -lnetcdf_c++ \
        -lnetcdf \
        -lcairo \
        -lfreetype \
        -lm  \
        -lz


# Create generated files
cgraphbase_plottype_to_string.h cgraphbase_plottype_to_string.cc \
cgraphbase_fontfamily_to_string.h cgraphbase_fontfamily_to_string.cc: cgraph_main.h
	${ENUM_TO_STRING} -prefix -concat_string cgraph_main.h

# Other files to be incoluded in the distribution
EXTRA_DIST = cgraphbase_fontfamily_to_string.h \
	cgraphbase_plottype_to_string.h

# Clean up extra local files
clean-local:
	-rm -f cgraphbase_plottype_to_string.h
	-rm -f cgraphbase_plottype_to_string.cc
	-rm -f cgraphbase_fontfamily_to_string.h
	-rm -f cgraphbase_fontfamily_to_string.cc
