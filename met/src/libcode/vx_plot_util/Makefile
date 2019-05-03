# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_plot_util library
#
########################################################################


EXECUTABLES =


OBJECTS     = vx_plot_util.o


LIBRARIES   = libvx_plot_util.a


########################################################################


HEADERS     = vx_plot_util.h


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_plot_util Library ***"
	@ echo


########################################################################


   ##
   ##  objects
   ##


vx_plot_util.o: vx_plot_util.cc
	$(CXX) vx_plot_util.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -DMET_BASE=\"$(MET_BASE_DIR)\" -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_plot_util.a: $(OBJECTS)
	ar -rs libvx_plot_util.a $(OBJECTS)
	ranlib libvx_plot_util.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_plot_util.a
	cp libvx_plot_util.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_plot_util.a


########################################################################


.PHONY: all clean


########################################################################


