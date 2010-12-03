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
              grid_output.o       \
              met_stats.o         \
              pair_data.o         \
              read_grib.o         \
              read_gridded_data.o \
              read_netcdf.o       \
              stat_columns.o      \
              stat_hdr_columns.o  \
              two_to_one.o        \
              write_netcdf.o
																																																	

LIBRARIES   = libvx_met_util.a


########################################################################


HEADERS     = apply_mask.h        \
              compute_ci.h        \
              constants.h         \
              conversions.h       \
              file_type.h         \
              grid_output.h       \
              met_stats.h         \
              pair_data.h         \
              read_grib.h         \
              read_gridded_data.h \
              read_netcdf.h       \
              stat_columns.h      \
              stat_hdr_columns.h  \
              two_to_one.h        \
              vx_met_util.h       \
              write_netcdf.h


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)


########################################################################


   ##
   ##  objects
   ##


apply_mask.o: apply_mask.cc
	$(CXX) apply_mask.o $(CXX_FLAGS) $(OPT_FLAGS) -c

compute_ci.o: compute_ci.cc
	$(CXX) compute_ci.o $(CXX_FLAGS) $(OPT_FLAGS) -c

conversions.o: conversions.cc
	$(CXX) conversions.o $(CXX_FLAGS) $(OPT_FLAGS) -c

file_type.o: file_type.cc
	$(CXX) file_type.o $(CXX_FLAGS) $(OPT_FLAGS) -c

grid_output.o: grid_output.cc
	$(CXX) grid_output.o $(CXX_FLAGS) $(OPT_FLAGS) -c

met_stats.o: met_stats.cc
	$(CXX) met_stats.o $(CXX_FLAGS) $(OPT_FLAGS) -c

pair_data.o: pair_data.cc
	$(CXX) pair_data.o $(CXX_FLAGS) $(OPT_FLAGS) -c

read_grib.o: read_grib.cc
	$(CXX) read_grib.o $(CXX_FLAGS) $(OPT_FLAGS) -c

read_gridded_data.o: read_gridded_data.cc
	$(CXX) read_gridded_data.o $(CXX_FLAGS) $(OPT_FLAGS) -c

read_netcdf.o: read_netcdf.cc
	$(CXX) read_netcdf.o $(CXX_FLAGS) $(OPT_FLAGS) -c

stat_columns.o: stat_columns.cc
	$(CXX) stat_columns.o $(CXX_FLAGS) $(OPT_FLAGS) -c

stat_hdr_columns.o: stat_hdr_columns.cc
	$(CXX) stat_hdr_columns.o $(CXX_FLAGS) $(OPT_FLAGS) -c

two_to_one.o: two_to_one.cc
	$(CXX) two_to_one.o $(CXX_FLAGS) $(OPT_FLAGS) -c

write_netcdf.o: write_netcdf.cc
	$(CXX) write_netcdf.o $(CXX_FLAGS) $(OPT_FLAGS) -c


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


