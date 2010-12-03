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


OBJECTS     = plot_util.o


LIBRARIES   = libvx_plot_util.a


########################################################################


HEADERS     = plot_util.h


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)


########################################################################


   ##
   ##  objects
   ##


plot_util.o: plot_util.cc
	$(CXX) plot_util.cc $(CXX_FLAGS) $(OPT_FLAGS) -c


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


