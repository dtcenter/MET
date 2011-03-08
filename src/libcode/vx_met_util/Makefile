# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_met_util library
#
########################################################################


EXECUTABLES =


OBJECTS     = apply_mask.o


LIBRARIES   = libvx_met_util.a


########################################################################


HEADERS     = apply_mask.h


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished making vx_met_util Library ***"
	@ echo


########################################################################


   ##
   ##  objects
   ##


apply_mask.o: apply_mask.cc
	$(CXX) apply_mask.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -DMET_BASE=\"$(MET_BASE)\" -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)


########################################################################


   ##
   ##  libraries
   ##


libvx_met_util.a: $(OBJECTS)
	ar -rs libvx_met_util.a $(OBJECTS)
	ranlib libvx_met_util.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_met_util.a
	cp libvx_met_util.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_met_util.a


########################################################################


.PHONY: all clean


########################################################################


