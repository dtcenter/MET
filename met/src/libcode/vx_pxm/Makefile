# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_pxm library
#
########################################################################


EXECUTABLES =


OBJECTS     = pbm.o        \
              pcm.o        \
              pgm.o        \
              ppm.o        \
              pxm_base.o   \
              pxm_utils.o


LIBRARIES   = libvx_pxm.a


########################################################################


HEADERS     = pbm.h        \
              pcm.h        \
              pgm.h        \
              ppm.h        \
              pxm_base.h   \
              pxm_utils.h  \
              vx_pxm.h


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_pxm Library ***"
	@ echo


########################################################################


   ##
   ##  objects
   ##


pbm.o: pbm.cc
	$(CXX) pbm.cc $(CXX_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)

pcm.o: pcm.cc
	$(CXX) pcm.cc $(CXX_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)

pgm.o: pgm.cc
	$(CXX) pgm.cc $(CXX_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)

ppm.o: ppm.cc
	$(CXX) ppm.cc $(CXX_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)

pxm_base.o: pxm_base.cc
	$(CXX) pxm_base.cc $(CXX_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)

pxm_utils.o: pxm_utils.cc
	$(CXX) pxm_utils.cc $(CXX_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_pxm.a: $(OBJECTS)
	ar -rs libvx_pxm.a $(OBJECTS) 2>&1
	ranlib libvx_pxm.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_pxm.a
	cp libvx_pxm.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_pxm.a


########################################################################


.PHONY: all clean


########################################################################


