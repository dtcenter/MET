# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_contable library
#
########################################################################


EXECUTABLES = 

OBJECTS     =  vx_contable.o       \
	            vx_contable_stats.o \
	            vx_nx2.o

LIBRARIES   =  libvx_contable.a


########################################################################


HEADERS     =  vx_contable.h 


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished making vx_contable Library ***"
	@ echo


########################################################################


	##
	## objects
	##


vx_contable.o: vx_contable.cc
	$(CXX) vx_contable.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

vx_contable_stats.o: vx_contable_stats.cc
	$(CXX) vx_contable_stats.cc $(CXX_FLAGS) $(OPT_FLAGS) -c $(GSL_INCS) $(NETCDF_INCS)

vx_nx2.o: vx_nx2.cc
	$(CXX) vx_nx2.cc $(CXX_FLAGS) $(OPT_FLAGS) -c $(GSL_INCS) $(NETCDF_INCS)


########################################################################


	##
	## libraries
	##


libvx_contable.a: $(OBJECTS)
   ar -rs libvx_contable.a $(OBJECTS)
   ranlib libvx_contable.a
   cd $(MET_INC_DIR) ; rm -f $(HEADERS)
   cp $(HEADERS) $(MET_INC_DIR)
   rm -f $(MET_LIB_DIR)/libvx_contable.a
   cp libvx_contable.a $(MET_LIB_DIR)


########################################################################


	##
	## executables
	##



########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_contable.a


########################################################################


.PHONY: all clean


########################################################################


