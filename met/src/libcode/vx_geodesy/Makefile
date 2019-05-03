# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2013
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_geodesy library
#
########################################################################


EXECUTABLES = test_spheroid  \


OBJECTS     = spheroid.o  \


LIBRARIES   = libvx_geodesy.a


GEN_SOURCES =


########################################################################


HEADERS     = vx_geodesy.h  \
              spheroid.h    \


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_geodesy Library ***"
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


spheroid.o: spheroid.cc spheroid.h
	$(CXX) spheroid.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_geodesy.a: $(OBJECTS)
	ar -rs libvx_geodesy.a $(OBJECTS) 2>&1
	ranlib libvx_geodesy.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_geodesy.a
	cp libvx_geodesy.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


test_spheroid: test_spheroid.cc $(LIBRARIES)
	$(CXX) test_spheroid.cc $(CXX_FLAGS) -o test_spheroid -I$(MET_INC_DIR) -L$(MET_LIB_DIR) \
	-lvx_geodesy  \
	-lvx_util     \
	-lvx_log      \
	-lm           \


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_geodesy.a


########################################################################


.PHONY: all clean gen_sources


########################################################################

