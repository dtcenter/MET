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


EXECUTABLES        = plot_mode_field  \

#test_ft  \



CGRAPH_OBJECTS     = color_stack.o                      \
                     cgraph_font.o                      \
                     cgraph_main.o                      \
                     cgraphbase_plottype_to_string.o    \
                     cgraphbase_fontfamily_to_string.o  \


OTHER_OBJECTS      = mode_nc_file.o      \
                     fo_pair.o           \
                     fo_pair_array.o     \
                     met_int_array.o     \
                     met_double_array.o  \
                     mode_line_array.o   \
                     mode_run.o          \



OBJECTS            = $(CGRAPH_OBJECTS) $(OTHER_OBJECTS)



LIBRARIES          = libvx_cgraph.a


GEN_SOURCES        = cgraphbase_plottype_to_string.h     \
                     cgraphbase_plottype_to_string.cc    \
                     cgraphbase_fontfamily_to_string.h   \
                     cgraphbase_fontfamily_to_string.cc  \


########################################################################


HEADERS            = cgraph_font.h       \
                     cgraph.h            \
                     cgraph_main.h       \
                     color_stack.h       \
                     fo_pair_array.h     \
                     fo_pair.h           \
                     gs_ps_map.h         \
                     met_double_array.h  \
                     met_int_array.h     \
                     mode_line_array.h   \
                     mode_nc_file.h      \
                     mode_run.h          \
                     sincosd.h           \


########################################################################


CAIRO_INC_DIR           = /usr/include/cairo
// CAIRO_LIB_DIR           = /usr/lib
CAIRO_LIB_DIR           = /usr/lib/x86_64-linux-gnu

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
	$(CXX) cgraph_font.cc -c $(CXX_FLAGS) -DGS_FONT_DIR=\"$(GS_FONT_DIR)\" -I$(MET_INC_DIR) -I$(CAIRO_INC_DIR) -I$(FREETYPE_INC_DIR)


cgraph_main.o: cgraph_main.h cgraph_main.cc cgraphbase_plottype_to_string.o color_stack.o cgraph_font.o
	@ echo
	$(CXX) cgraph_main.cc -c $(CXX_FLAGS) -I$(MET_INC_DIR) -I$(CAIRO_INC_DIR) -I$(FREETYPE_INC_DIR)



fo_pair.o: fo_pair.h fo_pair.cc
	@ echo
	$(CXX) fo_pair.cc -c $(CXX_FLAGS) -I$(MET_INC_DIR)


fo_pair_array.o: fo_pair_array.h fo_pair_array.cc
	@ echo
	$(CXX) fo_pair_array.cc -c $(CXX_FLAGS) -I$(MET_INC_DIR)


mode_nc_file.o: mode_nc_file.h mode_nc_file.cc
	@ echo
	$(CXX) mode_nc_file.cc -c $(CXX_FLAGS) -I$(MET_INC_DIR) -I$(NETCDF_INCS)


met_int_array.o: met_int_array.h met_int_array.cc
	@ echo
	$(CXX) met_int_array.cc -c $(CXX_FLAGS) -I$(MET_INC_DIR)


met_double_array.o: met_double_array.h met_double_array.cc
	@ echo
	$(CXX) met_double_array.cc -c $(CXX_FLAGS) -I$(MET_INC_DIR)


mode_line_array.o: mode_line_array.h mode_line_array.cc
	@ echo
	$(CXX) mode_line_array.cc -c $(CXX_FLAGS) -I$(MET_INC_DIR)


mode_run.o: mode_run.h mode_run.cc
	@ echo
	$(CXX) mode_run.cc -c $(CXX_FLAGS) -I$(MET_INC_DIR)


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


test_ft: test_ft.cc
	@ echo
	$(CXX) test_ft.cc $(CXX_FLAGS) -o test_ft -I$(MET_INC_DIR) -I$(CAIRO_INC_DIR) -I$(FREETYPE_INC_DIR) -L$(MET_LIB_DIR) -L. -L$(CAIRO_LIB_DIR) -L$(FREETYPE_LIB_DIR) -lvx_cgraph -lvx_ps -lvx_afm -lvx_color -lvx_math -lvx_util -lvx_log -lm \
            -lcairo -lfreetype  \
            -lpixman-1  \
            -lcairo -lfreetype  \
            -lpthread  \
            -lpng  \
            -lz  \
            -lcairo -lfreetype  \
            -lfontconfig  \



plot_mode_field: $(OBJECTS) $(LIBRARIES) plot_mode_field.cc
	@ echo
	$(CXX) plot_mode_field.cc mode_nc_file.o $(CXX_FLAGS) -o plot_mode_field \
        -I$(MET_INC_DIR) \
        $(NETCDF_INCS) \
        -I/usr/include/cairo \
        -I/usr/include/freetype2  \
        $(NETCDF_LIBS) \
        -L/usr/lib/x86_64-linux-gnu \
        -L/usr/lib \
        -L. \
        -L$(MET_LIB_DIR) \
        -L$(CAIRO_LIB_DIR)  \
        -L$(FREETYPE_LIB_DIR)  \
        -lvx_config \
        -lvx_plot_util \
        -lvx_data2d_nc_met \
        -lvx_grid \
        -lvx_ps \
        -lvx_afm \
        -lvx_pxm \
        -lvx_color \
        -lvx_render \
        -lvx_math \
        -lvx_cgraph \
        -lvx_util \
        -lvx_cal \
        -lvx_log \
        -lcairo \
        -lfreetype \
        -lnetcdf_c++ \
        -lnetcdf \
        -lpthread  \
        -lpixman-1  \
        -lpng  \
        -lm  \
        -lz  \


#       -lfontconfig  \


#       -I$(NETCDF_INCS) \
#       -I$(CAIRO_INC_DIR) \
#       -I$(FREETYPE_INC_DIR) \
#       -L$(MET_LIB_DIR) \
#       -L. \
#       -L$(NETCDF_LIBS) \
#       -L$(CAIRO_LIB_DIR) \
#       -L$(FREETYPE_LIB_DIR) \
#       -lvx_cgraph \
#       -lvx_data2d_nc_met \
#       -lvx_grid \
#       -lvx_ps \
#       -lvx_afm \
#       -lvx_pxm \
#       -lvx_color \
#       -lvx_util \
#       -lvx_cal \
#       -lvx_log \
#       -lvx_math \
#       -lnetcdf_c++ -lnetcdf \
#       -lcairo -lfreetype \
#       -lm  \


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	rm -f $(MET_BIN_DIR)/modis_regrid


########################################################################


.PHONY: all clean gen_sources


########################################################################

