# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2012
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the ascii2nc tool
#
########################################################################


EXECUTABLES = ascii2nc


OBJECTS     =


LIBRARIES   =


GEN_SOURCES =


########################################################################


HEADERS     =


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the ascii2nc Tool ***"
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


ascii2nc: ascii2nc.cc $(OBJECTS)
	@ echo
	$(CXX) -o $(EXECUTABLES) ascii2nc.cc $(OBJECTS) \
	$(CXX_FLAGS) -DMET_BASE=\"$(MET_BASE_DIR)\" \
	-I$(MET_INC_DIR) $(NETCDF_INCS) $(GSL_INCS) \
	-L$(MET_LIB_DIR) $(NETCDF_LIBS) $(GSL_LIBS) \
	-lvx_nc_util \
	-lvx_grid \
	-lvx_math \
	-lvx_cal \
	-lvx_util \
	-lvx_log \
	-lm -lnetcdf_c++ -lnetcdf \
	$(CXX_LIBS)
	cp $(EXECUTABLES) $(MET_BIN_DIR)


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	rm -f $(MET_BIN_DIR)/ascii2nc


########################################################################


.PHONY: all clean gen_sources


########################################################################

