# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the gen_poly_mask tool
#
########################################################################


EXECUTABLES = gen_poly_mask


OBJECTS     =


LIBRARIES   =


########################################################################


HEADERS     =


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the gen_poly_mask Tool ***"
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


gen_poly_mask: gen_poly_mask.cc $(OBJECTS)
	@ echo
	$(CXX) -o $(EXECUTABLES) gen_poly_mask.cc $(OBJECTS) \
	$(CXX_FLAGS) \
	-I$(MET_INC_DIR) $(NETCDF_INCS) \
	-L$(MET_LIB_DIR) $(NETCDF_LIBS) \
	-lvx_analysis_util  \
	-lvx_data2d_factory  \
	-lvx_data2d_grib \
	-lvx_data2d_nc_met  \
	-lvx_data2d_nc_pinterp  \
	-lvx_data2d  \
	-lvx_nc_util  \
	-lvx_grid  \
	-lvx_math  \
	-lvx_cal  \
	-lvx_log -lvx_util \
	-lm -lnetcdf_c++ -lnetcdf \
	$(CXX_LIBS)
	cp $(EXECUTABLES) $(MET_BIN_DIR)


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	rm -f $(MET_BIN_DIR)/gen_poly_mask


########################################################################


.PHONY: all clean


########################################################################


