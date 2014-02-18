# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2013
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the plot_data_plane tool
#
########################################################################


EXECUTABLES = plot_data_plane


OBJECTS     =


LIBRARIES   =


GEN_SOURCES =


########################################################################


HEADERS     =


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the plot_data_plane Tool ***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##


gen_sources: $(GEN_SOURCES)
	@ echo


########################################################################


   ##
   ##  objects
   ##


########################################################################


   ##
   ##  libraries
   ##


########################################################################


   ##
   ##  executables
   ##


plot_data_plane: plot_data_plane.cc $(LIBRARIES) $(OBJECTS)
	@ echo
	$(CXX) plot_data_plane.cc -o plot_data_plane \
	$(CXX_FLAGS) -DMET_BASE=\"$(MET_BASE_DIR)\" \
	-I$(MET_INC_DIR) $(NETCDF_INCS) $(GRIB2C_INCS) \
	-L$(MET_LIB_DIR) $(NETCDF_LIBS) $(GRIB2C_LIBS) \
	-lvx_data2d_factory \
	-lvx_data2d_nc_met \
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_data2d_nc_pinterp \
	-lvx_data2d_nccf \
	-lvx_nc_util \
	-lvx_data2d \
	-lvx_plot_util \
	-lvx_render \
	-lvx_pxm \
	-lvx_color \
	-lvx_ps \
	-lvx_afm \
	-lvx_nav \
	-lvx_gnomon \
	-lvx_grid \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm -lz -lnetcdf_c++ -lnetcdf \
	$(CXX_LIBS)
	cp plot_data_plane $(MET_BIN_DIR)


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	rm -f $(MET_BIN_DIR)/plot_data_plane


########################################################################


.PHONY: all clean gen_sources


########################################################################

