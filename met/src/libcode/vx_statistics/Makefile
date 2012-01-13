# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_statistics library
#
########################################################################


EXECUTABLES =

OBJECTS     =  apply_mask.o         \
               compute_ci.o         \
               contable.o           \
               contable_stats.o     \
               contable_nx2.o       \
               met_stats.o          \
               pair_base.o          \
               pair_data_point.o    \
               pair_data_ensemble.o \
                              


LIBRARIES   =  libvx_statistics.a


########################################################################


HEADERS     =  apply_mask.h         \
               compute_ci.h         \
               contable.h           \
               met_stats.h          \
               pair_base.h          \
               pair_data_point.h    \
               pair_data_ensemble.h \
               vx_statistics.h      \


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_statistics Library ***"
	@ echo


########################################################################


   ##
   ## objects
   ##


apply_mask.o: apply_mask.cc
	$(CXX) apply_mask.cc $(CXX_FLAGS) -c -DMET_BASE=\"$(MET_BASE_DIR)\" -I$(MET_INC_DIR) $(GSL_INCS)

compute_ci.o: compute_ci.cc
	$(CXX) compute_ci.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS)

contable.o: contable.cc
	$(CXX) contable.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

contable_stats.o: contable_stats.cc
	$(CXX) contable_stats.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)

contable_nx2.o: contable_nx2.cc
	$(CXX) contable_nx2.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)

met_stats.o: met_stats.cc
	$(CXX) met_stats.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS)

pair_base.o: pair_base.cc
	$(CXX) pair_base.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS)

pair_data_point.o: pair_data_point.cc
	$(CXX) pair_data_point.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS)

pair_data_ensemble.o: pair_data_ensemble.cc
	$(CXX) pair_data_ensemble.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS)


########################################################################


   ##
   ## libraries
   ##


libvx_statistics.a: $(OBJECTS)
	ar -rs libvx_statistics.a $(OBJECTS) 2>&1
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


