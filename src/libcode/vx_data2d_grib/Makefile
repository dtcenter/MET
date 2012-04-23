# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2012
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_data2d_grib library
#
########################################################################


EXECUTABLES = test_read_grib1 \
              test_vx_data2d_grib


OBJECTS     = grib_classes.o \
              grib_strings.o \
              grib_utils.o \
              var_info_grib.o \
              data2d_grib_utils.o \
              data2d_grib.o


LIBRARIES   = libvx_data2d_grib.a


GEN_SOURCES =


########################################################################


HEADERS     = grib_classes.h \
              grib_strings.h \
              grib_utils.h \
              var_info_grib.h \
              data2d_grib_utils.h \
              data2d_grib.h \
              vx_data2d_grib.h


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_data2d_grib Library ***"
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


grib_classes.o: grib_classes.cc
	$(CXX) grib_classes.cc $(CXX_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)

grib_strings.o: grib_strings.cc
	$(CXX) grib_strings.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

grib_utils.o: grib_utils.cc
	$(CXX) grib_utils.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

var_info_grib.o: var_info_grib.cc
	$(CXX) var_info_grib.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

data2d_grib_utils.o: data2d_grib_utils.h data2d_grib_utils.cc
	$(CXX) data2d_grib_utils.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

data2d_grib.o: data2d_grib.h data2d_grib.cc
	$(CXX) data2d_grib.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_data2d_grib.a: $(OBJECTS)
	ar -rs libvx_data2d_grib.a $(OBJECTS) 2>&1
	ranlib libvx_data2d_grib.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_data2d_grib.a
	cp libvx_data2d_grib.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##

test_vx_data2d_grib: $(LIBRARIES) test_vx_data2d_grib.cc
	$(CXX) -o test_vx_data2d_grib test_vx_data2d_grib.cc \
	$(CXX_FLAGS) -I$(MET_INC_DIR) \
	-I$(MET_INC_DIR) \
	-L$(MET_LIB_DIR) \
	-lvx_data2d_grib \
	-lvx_data2d \
	-lvx_grid \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log


test_read_grib1: test_read_grib1.cc $(LIBRARIES)
	@ echo
	$(CXX) test_read_grib1.cc $(CXX_FLAGS) -o test_read_grib1 -I$(MET_INC_DIR) -L$(MET_LIB_DIR) \
	-lvx_data2d_grib \
	-lvx_data2d \
	-lvx_grid \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES) $(GEN_SOURCES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_data2d_grib.a


########################################################################


.PHONY: all clean gen_sources


########################################################################

