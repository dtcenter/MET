# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2012
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the Tropical Cyclone tc_pairs tool
#
########################################################################


EXECUTABLES = tc_pairs


OBJECTS     = acerr.o \
              tc_pairs_conf_info.o

LIBRARIES   =


GEN_SOURCES =


########################################################################


HEADERS     =


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the tc_pairs Tool ***"
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


acerr.o: acerr.f
	@ echo
	$(FC) acerr.f $(FC_FLAGS) -c


tc_pairs_conf_info.o: tc_pairs_conf_info.cc
	@ echo
	$(CXX) tc_pairs_conf_info.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)


########################################################################


   ##
   ##  libraries
   ##


########################################################################


   ##
   ##  executables
   ##


tc_pairs: tc_pairs.cc $(OBJECTS)
	@ echo
	$(CXX) -o $(EXECUTABLES) tc_pairs.cc $(OBJECTS) \
	$(CXX_FLAGS) -DMET_BASE=\"$(MET_BASE_DIR)\" \
	-I$(MET_INC_DIR) $(NETCDF_INCS) $(GSL_INCS) $(F2C_INCS) \
	-L$(MET_LIB_DIR) $(NETCDF_LIBS) $(GSL_LIBS) $(F2C_LIBS) \
	-lvx_config \
	-lvx_data2d_nc_met \
	-lvx_analysis_util \
	-lvx_tc_util \
	-lvx_nc_util \
	-lvx_grid \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm \
	-lnetcdf_c++ -lnetcdf \
	$(F2C_LIBNAME) $(CXX_LIBS) $(FC_LIBS)
	cp $(EXECUTABLES) $(MET_BIN_DIR)


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	rm -f $(MET_BIN_DIR)/tc_pairs


########################################################################


.PHONY: all clean gen_sources


########################################################################

