# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_ps library
#
########################################################################


EXECUTABLES =


OBJECTS     = vx_ps.o       \
              vx_ps_text.o  \


LIBRARIES   = libvx_ps.a


########################################################################


HEADERS     = vx_ps.h       \
              vx_ps_text.h  \


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo


########################################################################


   ##
   ##  objects
   ##


vx_ps.o: vx_ps.cc
	$(CXX) vx_ps.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -DMET_BASE=\"$(MET_BASE)\"

vx_ps_text.o: vx_ps_text.cc
	$(CXX) vx_ps_text.cc $(CXX_FLAGS) $(OPT_FLAGS) -c


########################################################################


   ##
   ##  libraries
   ##


libvx_ps.a: $(OBJECTS)
	ar -rs libvx_ps.a $(OBJECTS)
	ranlib libvx_ps.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_ps.a
	cp libvx_ps.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_ps.a


########################################################################


.PHONY: all clean


########################################################################


