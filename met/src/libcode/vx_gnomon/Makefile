# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2012
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_gnomon library
#
########################################################################


EXECUTABLES =


OBJECTS     = gnomon.o


LIBRARIES   = libvx_gnomon.a


GEN_SOURCES =


########################################################################


HEADERS     = gnomon.h


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_gnomon Library ***"
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


gnomon.o: gnomon.cc
	$(CXX) gnomon.cc $(CXX_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_gnomon.a: $(OBJECTS)
	ar -rs libvx_gnomon.a $(OBJECTS)  2>&1
	ranlib libvx_gnomon.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_gnomon.a
	cp libvx_gnomon.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_gnomon.a


########################################################################


.PHONY: all clean gen_sources


########################################################################

