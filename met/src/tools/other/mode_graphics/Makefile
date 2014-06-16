# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2013
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the modis_regrid tool
#
########################################################################



########################################################################
#
#
#  Tarball of ghostscript fonts available at 
#
#       http://sourceforge.net/projects/gs-fonts/
#
#
########################################################################


EXECUTABLES        = test_png         \
                     plot_mode_field  \



CGRAPH_OBJECTS     = color_stack.o                      \
                     cgraph_font.o                      \
                     cgraph_main.o                      \
                     cgraphbase_plottype_to_string.o    \
                     cgraphbase_fontfamily_to_string.o  \


OTHER_OBJECTS      = mode_nc_output_file.o      \



OBJECTS            = $(CGRAPH_OBJECTS) $(OTHER_OBJECTS)



LIBRARIES          = libvx_cgraph.a


GEN_SOURCES        = cgraphbase_plottype_to_string.h     \
                     cgraphbase_plottype_to_string.cc    \
                     cgraphbase_fontfamily_to_string.h   \
                     cgraphbase_fontfamily_to_string.cc  \


########################################################################


HEADERS            = cgraph_font.h          \
                     cgraph.h               \
                     cgraph_main.h          \
                     color_stack.h          \
                     fo_pair_array.h        \
                     fo_pair.h              \
                     gs_ps_map.h            \
                     met_double_array.h     \
                     met_int_array.h        \
                     mode_line_array.h      \
                     mode_nc_output_file.h  \
                     mode_run.h             \
                     sincosd.h              \


########################################################################


CAIRO_INC_DIR           = /usr/include/cairo
CAIRO_LIB_DIR           = /usr/lib
// CAIRO_LIB_DIR           = /usr/lib/x86_64-linux-gnu

FREETYPE_INC_DIR        = /usr/include/freetype2
FREETYPE_LIB_DIR        = /usr/lib


########################################################################


ARCHIVE        = ar -rs


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the Mode Graiphcs Tools ***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##


gen_sources: $(GEN_SOURCES)
	@ echo


cgraphbase_plottype_to_string.h cgraphbase_plottype_to_string.cc: cgraph_main.h
	@ echo
	@ rm -f cgraphbase_plottype_to_string.h cgraphbase_plottype_to_string.cc
	$(ENUM_TO_STRING) -prefix -concat_string cgraph_main.h


cgraphbase_fontfamily_to_string.h cgraphbase_fontfamily_to_string.cc: cgraph_main.h
	@ echo
	@ rm -f cgraphbase_fontfamily_to_string.h cgraph_fontfamily_to_string.cc
	$(ENUM_TO_STRING) -prefix -concat_string cgraph_main.h


########################################################################


   ##
   ##  objects
   ##


color_stack.o: color_stack.h color_stack.cc
	@ echo
	$(CXX) color_stack.cc -c $(CXX_FLAGS) -I$(MET_INC_DIR)


cgraphbase_plottype_to_string.o: cgraphbase_plottype_to_string.h cgraphbase_plottype_to_string.cc
	@ echo
	$(CXX) cgraphbase_plottype_to_string.cc -c $(CXX_FLAGS) -I$(MET_INC_DIR) -I$(CAIRO_INC_DIR) -I$(FREETYPE_INC_DIR)


cgraphbase_fontfamily_to_string.o: cgraphbase_fontfamily_to_string.h cgraphbase_fontfamily_to_string.cc
	@ echo
	$(CXX) cgraphbase_fontfamily_to_string.cc -c $(CXX_FLAGS) -I$(MET_INC_DIR) -I$(CAIRO_INC_DIR) -I$(FREETYPE_INC_DIR)


cgraph_font.o: cgraph_font.h cgraph_font.cc
	@ echo
	$(CXX) cgraph_font.cc -c $(CXX_FLAGS) -DCGRAPH_FONT_DIR=\"$(CGRAPH_FONT_DIR)\" -I$(MET_INC_DIR) -I$(CAIRO_INC_DIR) -I$(FREETYPE_INC_DIR)


cgraph_main.o: cgraph_main.h cgraph_main.cc cgraphbase_plottype_to_string.o color_stack.o cgraph_font.o
	@ echo
	$(CXX) cgraph_main.cc -c $(CXX_FLAGS) -I$(MET_INC_DIR) -I$(CAIRO_INC_DIR) -I$(FREETYPE_INC_DIR)


mode_nc_output_file.o: mode_nc_output_file.h mode_nc_output_file.cc
	@ echo
	$(CXX) mode_nc_output_file.cc -c $(CXX_FLAGS) -I$(MET_INC_DIR) -I$(NETCDF_INCS)


########################################################################


   ##
   ##  libraries
   ##


libvx_cgraph.a: $(CGRAPH_OBJECTS)
	@ echo
	$(ARCHIVE) libvx_cgraph.a $(CGRAPH_OBJECTS) 2>&1
	@ ranlib libvx_cgraph.a


########################################################################


   ##
   ##  executables
   ##


test_png: $(OBJECTS) $(LIBRARIES) test_png.cc
	@ echo
	$(CXX) test_png.cc -g -Wall -Wshadow -o test_png \
        -I$(MET_INC_DIR) \
        -I$(CAIRO_INC_DIR) \
        -I$(FREETYPE_INC_DIR) \
        -L$(MET_LIB_DIR) \
        -L. \
        -L$(CAIRO_LIB_DIR) \
        -L$(FREETYPE_LIB_DIR) \
        -lvx_cgraph \
        -lvx_ps \
        -lvx_afm \
        -lvx_pxm \
        -lvx_render \
        -lvx_color \
        -lvx_util \
        -lvx_cal \
        -lvx_log \
        -lvx_math \
        -lcairo \
        -lfreetype \
        -lm \
        -lz \



plot_mode_field: $(OBJECTS) $(LIBRARIES) plot_mode_field.cc
	@ echo
	$(CXX) plot_mode_field.cc mode_nc_output_file.o $(FLAGS) -o plot_mode_field   \
        -I$(CAIRO_INC_DIR) \
        -I$(FREETYPE_INC_DIR) \
        -I$(MET_INC_DIR)   \
        -I$(NETCDF_INC_DIR)   \
        -I$(CAIRO_INC_DIR)   \
        -I$(FREETYPE_INC_DIR)   \
        -L$(MET_LIB_DIR)   \
        -L.   \
        -L$(NETCDF_LIB_DIR)   \
        -L$(CAIRO_LIB_DIR)   \
        -L$(FREETYPE_LIB_DIR)   \
        -lvx_cgraph   \
        -lvx_config   \
        -lvx_plot_util   \
        -lvx_data2d_nc_met   \
        -lvx_grid   \
        -lvx_ps   \
        -lvx_afm   \
        -lvx_pxm   \
        -lvx_render  \
        -lvx_color   \
        -lvx_util   \
        -lvx_cal   \
        -lvx_log   \
        -lvx_math   \
        -lnetcdf_c++   \
        -lnetcdf   \
        -lcairo   \
        -lfreetype   \
        -lm   \
        -lz  \


########################################################################


clean:
	rm -f *.a *.o *.png temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	rm -f $(MET_BIN_DIR)/modis_regrid


########################################################################


.PHONY: all clean gen_sources


########################################################################

