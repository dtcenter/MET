# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2012
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_data2d_nc_pinterp library
#
########################################################################


EXECUTABLES =
              
OBJECTS     = get_pinterp_grid.o \
              pinterp_file.o \
              var_info_nc_pinterp.o \
              data2d_nc_pinterp.o


LIBRARIES   = libvx_data2d_nc_pinterp.a


GEN_SOURCES =


########################################################################


HEADERS     = get_pinterp_grid.h \
              pinterp_file.h \
              var_info_nc_pinterp.h \
              data2d_nc_pinterp.h \
              vx_data2d_nc_pinterp.h


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_data2d_nc_pinterp Library ***"
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


get_pinterp_grid.o: get_pinterp_grid.cc
	$(CXX) get_pinterp_grid.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)

pinterp_file.o: pinterp_file.cc
	$(CXX) pinterp_file.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)

var_info_nc_pinterp.o: var_info_nc_pinterp.cc
	$(CXX) var_info_nc_pinterp.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

data2d_nc_pinterp.o: data2d_nc_pinterp.cc
	$(CXX) data2d_nc_pinterp.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)


########################################################################


   ##
   ##  libraries
   ##


libvx_data2d_nc_pinterp.a: $(OBJECTS)
	ar -rs libvx_data2d_nc_pinterp.a $(OBJECTS) 2>&1
	ranlib libvx_data2d_nc_pinterp.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_data2d_nc_pinterp.a
	cp libvx_data2d_nc_pinterp.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_data2d_nc_pinterp.a


########################################################################


.PHONY: all clean gen_sources


########################################################################

