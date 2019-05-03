# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2013
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_nc_util library
#
########################################################################


EXECUTABLES =


OBJECTS     = nc_var_info.o \
              nc_utils.o \
              write_netcdf.o \
              grid_output.o


LIBRARIES   = libvx_nc_util.a


GEN_SOURCES =


########################################################################


HEADERS     = nc_constants.h \
              nc_var_info.h \
              nc_utils.h \
              write_netcdf.h \
              grid_output.h \
              vx_nc_util.h


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_nc_util Library ***"
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


nc_var_info.o: nc_var_info.cc
	$(CXX) nc_var_info.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)


nc_utils.o: nc_utils.cc
	$(CXX) nc_utils.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)


grid_output.o: grid_output.h grid_output.cc
	$(CXX) grid_output.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)


write_netcdf.o: write_netcdf.h write_netcdf.cc grid_output.o
	$(CXX) write_netcdf.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)


########################################################################


   ##
   ##  libraries
   ##


libvx_nc_util.a: $(OBJECTS)
	ar -rs libvx_nc_util.a $(OBJECTS) 2>&1
	ranlib libvx_nc_util.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_nc_util.a
	cp libvx_nc_util.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_nc_util.a


########################################################################


.PHONY: all clean gen_sources


########################################################################

