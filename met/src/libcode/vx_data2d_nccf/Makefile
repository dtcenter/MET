# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2013
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_data2d_nccf library
#
########################################################################


EXECUTABLES = test_read_nccf
              
OBJECTS     = nccf_file.o \
              var_info_nccf.o \
              data2d_nccf.o


LIBRARIES   = libvx_data2d_nccf.a


GEN_SOURCES =


########################################################################


HEADERS     = nccf_file.h \
              var_info_nccf.h \
              data2d_nccf.h \
              vx_data2d_nccf.h


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_data2d_nccf Library ***"
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


nccf_file.o: nccf_file.cc
	$(CXX) nccf_file.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)

var_info_nccf.o: var_info_nccf.cc
	$(CXX) var_info_nccf.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

data2d_nccf.o: data2d_nccf.cc
	$(CXX) data2d_nccf.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)


########################################################################


   ##
   ##  libraries
   ##


libvx_data2d_nccf.a: $(OBJECTS)
	ar -rs libvx_data2d_nccf.a $(OBJECTS) 2>&1
	ranlib libvx_data2d_nccf.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_data2d_nccf.a
	cp libvx_data2d_nccf.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##

test_read_nccf: $(LIBRARIES) test_read_nccf.cc
	$(CXX) -o test_read_nccf test_read_nccf.cc \
	$(CXX_FLAGS) \
	-I$(MET_INC_DIR) $(NETCDF_INCS) \
	-L$(MET_LIB_DIR) $(NETCDF_LIBS) \
	-lvx_data2d_nccf \
	-lvx_nc_util \
	-lvx_data2d \
	-lvx_grid \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lnetcdf_c++ -lnetcdf


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_data2d_nccf.a


########################################################################


.PHONY: all clean gen_sources


########################################################################

