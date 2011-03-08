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


OBJECTS     = vx_pbm.o        \
              vx_pcm.o        \
              vx_pgm.o        \
              vx_ppm.o        \
              vx_pxm_base.o   \
              vx_pxm_utils.o


LIBRARIES   = libvx_pxm.a


########################################################################


HEADERS     = vx_pbm.h        \
              vx_pcm.h        \
              vx_pgm.h        \
              vx_ppm.h        \
              vx_pxm_base.h   \
              vx_pxm.h        \
              vx_pxm_utils.h


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished making vx_pxm Library ***"
	@ echo


########################################################################


   ##
   ##  objects
   ##


vx_pbm.o: vx_pbm.cc
	$(CXX) vx_pbm.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)

vx_pcm.o: vx_pcm.cc
	$(CXX) vx_pcm.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)

vx_pgm.o: vx_pgm.cc
	$(CXX) vx_pgm.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)

vx_ppm.o: vx_ppm.cc
	$(CXX) vx_ppm.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)

vx_pxm_base.o: vx_pxm_base.cc
	$(CXX) vx_pxm_base.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)

vx_pxm_utils.o: vx_pxm_utils.cc
	$(CXX) vx_pxm_utils.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_pxm.a: $(OBJECTS)
	ar -rs libvx_pxm.a $(OBJECTS)
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


