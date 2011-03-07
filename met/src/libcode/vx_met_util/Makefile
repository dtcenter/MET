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


OBJECTS     = apply_mask.o        \
              compute_ci.o        \
              conversions.o       \
              file_type.o         \
              met_stats.o         \
              pair_data.o         \


LIBRARIES   = libvx_met_util.a


########################################################################


HEADERS     = apply_mask.h        \
              compute_ci.h        \
              conversions.h       \
              file_type.h         \
              met_stats.h         \
              pair_data.h         \


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)


########################################################################


   ##
   ##  objects
   ##


apply_mask.o: apply_mask.cc
	$(CXX) apply_mask.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

compute_ci.o: compute_ci.cc
	$(CXX) compute_ci.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

conversions.o: conversions.cc
	$(CXX) conversions.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

file_type.o: file_type.cc
	$(CXX) file_type.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

met_stats.o: met_stats.cc
	$(CXX) met_stats.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

pair_data.o: pair_data.cc
	$(CXX) pair_data.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)


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


