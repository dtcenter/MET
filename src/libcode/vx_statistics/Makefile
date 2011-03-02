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

OBJECTS     =  vx_contable.o        \
		vx_contable_stats.o \
		vx_nx2.o            \
		compute_ci.o        \
		met_stats.o         \
		pair_data.o


LIBRARIES   =  libvx_statistics.a


########################################################################


HEADERS     =  vx_contable.h      \
		compute_ci.h      \
		met_stats.h       \
		pair_data.h


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished making vx_statistics Library ***"
	@ echo


########################################################################


	##
	## objects
	##


vx_contable.o: vx_contable.cc
	$(CXX) vx_contable.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

vx_contable_stats.o: vx_contable_stats.cc
	$(CXX) vx_contable_stats.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)

vx_nx2.o: vx_nx2.cc
	$(CXX) vx_nx2.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)

compute_ci.o: compute_ci.cc
	$(CXX) compute_ci.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS)

met_stats.o: met_stats.cc
	$(CXX) met_stats.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS)

pair_data.o: pair_data.cc
	$(CXX) pair_data.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS)


########################################################################


	##
	## libraries
	##


libvx_statistics.a: $(OBJECTS)
	ar -rs libvx_statistics.a $(OBJECTS)
	ranlib libvx_statistics.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_statistics.a
	cp libvx_statistics.a $(MET_LIB_DIR)


########################################################################


	##
	## executables
	##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_statistics.a


########################################################################


.PHONY: all clean


########################################################################


