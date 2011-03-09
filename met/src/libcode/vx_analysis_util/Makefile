# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_analysis_util library
#
########################################################################


EXECUTABLES =


OBJECTS     = analysis_utils.o         \
              box.o                    \
              by_case_info.o           \
              data_line.o              \
              mask_poly.o              \
              mode_atts.o              \
              mode_job.o               \
              mode_line.o              \
              stat_job.o               \
              stat_line.o              \
              time_series.o


LIBRARIES   = libvx_analysis_util.a


########################################################################


HEADERS     = analysis_utils.h          \
              box.h                     \
              by_case_info.h            \
              data_line.h               \
              mask_poly.h               \
              mode_atts.h               \
              mode_analysis_columns.h   \
              mode_job.h                \
              mode_line.h               \
              stat_job.h                \
              stat_line.h               \
              stat_offsets.h            \
              time_series.h             \
              vx_analysis_util.h


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished making vx_analysis_util Library ***"
	@ echo


########################################################################


   ##
   ##  objects
   ##


analysis_utils.o: analysis_utils.cc
	$(CXX) analysis_utils.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

box.o: box.cc
	$(CXX) box.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

by_case_info.o: by_case_info.cc
	$(CXX) by_case_info.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

data_line.o: data_line.cc
	$(CXX) data_line.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

mask_poly.o: mask_poly.cc
	$(CXX) mask_poly.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

mode_atts.o: mode_atts.cc
	$(CXX) mode_atts.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

mode_job.o: mode_job.cc
	$(CXX) mode_job.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

mode_line.o: mode_line.cc
	$(CXX) mode_line.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

stat_job.o: stat_job.cc
	$(CXX) stat_job.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS)

stat_line.o: stat_line.cc
	$(CXX) stat_line.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS)

time_series.o: time_series.cc
	$(CXX) time_series.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_analysis_util.a: $(OBJECTS)
	ar -rs libvx_analysis_util.a $(OBJECTS)
	ranlib libvx_analysis_util.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_analysis_util.a
	cp libvx_analysis_util.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_analysis_util.a


########################################################################


.PHONY: all clean


########################################################################


