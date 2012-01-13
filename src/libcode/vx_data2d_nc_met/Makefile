# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_data2d_nc_met library
#
########################################################################


EXECUTABLES = test_read_nc_met
              
OBJECTS     = get_met_grid.o        \
              met_file.o            \
              var_info_nc_met.o     \
              data2d_nc_met.o       \


LIBRARIES   = libvx_data2d_nc_met.a


GEN_SOURCES =


########################################################################


HEADERS     = get_met_grid.h        \
              met_file.h            \
              var_info_nc_met.h     \
              data2d_nc_met.h       \
              vx_data2d_nc_met.h

#data2d_nc_met_utils.h \


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_data2d_nc_met Library ***"
	@ echo


########################################################################


gen_sources: $(GEN_SOURCES)
	@ echo


########################################################################


   ##
   ##  objects
   ##


get_met_grid.o: get_met_grid.cc
	$(CXX) get_met_grid.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)

met_file.o: met_file.cc
	$(CXX) met_file.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)

var_info_nc_met.o: var_info_nc_met.cc
	$(CXX) var_info_nc_met.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

data2d_nc_met.o: data2d_nc_met.cc
	$(CXX) data2d_nc_met.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)


########################################################################


   ##
   ##  libraries
   ##


libvx_data2d_nc_met.a: $(OBJECTS)
	ar -rs libvx_data2d_nc_met.a $(OBJECTS) 2>&1
	ranlib libvx_data2d_nc_met.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_data2d_nc_met.a
	cp libvx_data2d_nc_met.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##

test_read_nc_met: $(LIBRARIES) test_read_nc_met.cc
	$(CXX) -o test_read_nc_met test_read_nc_met.cc \
	$(CXX_FLAGS) \
	-I$(MET_INC_DIR) $(NETCDF_INCS) \
	-L$(MET_LIB_DIR) $(NETCDF_LIBS) \
	-lvx_data2d_nc_met -lvx_nc_util \
	-lvx_data2d -lvx_grid -lvx_util \
	-lvx_math -lvx_cal -lvx_log \
	-lnetcdf_c++ -lnetcdf


########################################################################

clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_data2d_nc_met.a


########################################################################


.PHONY: all clean


########################################################################
