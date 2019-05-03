# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the pcp_combine tool
#
########################################################################


EXECUTABLES = pcp_combine


OBJECTS     =


LIBRARIES   =


########################################################################


HEADERS     =


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the pcp_combine Tool ***"
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


pcp_combine: pcp_combine.cc $(OBJECTS)
	@ echo
	$(CXX) -o $(EXECUTABLES) pcp_combine.cc $(OBJECTS) \
	$(CXX_FLAGS) $(OPT_FLAGS) \
	-I$(MET_INC_DIR) $(NETCDF_INCS) $(GSL_INCS) \
	-L$(MET_LIB_DIR) $(NETCDF_LIBS) $(GSL_LIBS) \
	-lvx_analysis_util -lvx_met_util -lvx_wrfmode -lvx_render \
	-lvx_plot_util -lvx_pxm -lvx_color -lvx_ps -lvx_afm -lvx_pb_util \
	-lvx_nav -lvx_gnomon -lvx_gdata -lvx_stat_out -lvx_statistics \
	-lvx_gsl_prob -lvx_wrfdata -lvx_grib_classes -lvx_data_grids \
	-leconfig -lvx_util -lvx_math -lvx_cal \
	-lm -lnetcdf_c++ -lnetcdf -lgsl \
	$(CXX_LIBS)
	cp $(EXECUTABLES) $(MET_BIN_DIR)


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	rm -f $(MET_BIN_DIR)/pcp_combine


########################################################################


.PHONY: all clean


########################################################################


