# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_gdata library
#
########################################################################


EXECUTABLES =


OBJECTS     = get_pinterp_grid.o   \
              is_met_nc_file.o     \
              is_netcdf_file.o     \
              is_pinterp_file.o    \
              met_file.o           \
              pinterp_file.o       \
              var_info.o           \
              vx_gdata_util.o      \
              read_grib.o          \
              read_gridded_data.o  \
              read_netcdf.o        \
              grid_output.o        \
              write_netcdf.o       \


LIBRARIES   = libvx_gdata.a


########################################################################


HEADERS     = get_pinterp_grid.h   \
              is_met_nc_file.h     \
              is_netcdf_file.h     \
              is_pinterp_file.h    \
              met_file.h           \
              pinterp_file.h       \
              var_info.h           \
              vx_gdata.h           \
              vx_gdata_util.h      \
              read_grib.h          \
              read_gridded_data.h  \
              read_netcdf.h        \
              grid_output.h        \
              write_netcdf.h       \


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished making vx_gdata Library ***"
	@ echo


########################################################################


   ##
   ##  objects
   ##


get_pinterp_grid.o: get_pinterp_grid.cc
	$(CXX) get_pinterp_grid.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)

is_met_nc_file.o: is_met_nc_file.cc
	$(CXX) is_met_nc_file.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)

is_netcdf_file.o: is_netcdf_file.cc
	$(CXX) is_netcdf_file.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

is_pinterp_file.o: is_pinterp_file.cc
	$(CXX) is_pinterp_file.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)

met_file.o: met_file.cc
	$(CXX) met_file.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)

pinterp_file.o: pinterp_file.cc
	$(CXX) pinterp_file.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)

var_info.o: var_info.cc
	$(CXX) var_info.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)

vx_gdata_util.o: vx_gdata_util.cc
	$(CXX) vx_gdata_util.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)


read_grib.o: read_grib.cc
	$(CXX) read_grib.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)

read_gridded_data.o: read_gridded_data.cc
	$(CXX) read_gridded_data.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)

read_netcdf.o: read_netcdf.cc
	$(CXX) read_netcdf.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)

grid_output.o: grid_output.cc
	$(CXX) grid_output.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)

write_netcdf.o: write_netcdf.cc
	$(CXX) write_netcdf.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)


########################################################################


   ##
   ##  libraries
   ##


libvx_gdata.a: $(OBJECTS)
	ar -rs libvx_gdata.a $(OBJECTS)
	ranlib libvx_gdata.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_gdata.a
	cp libvx_gdata.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_afm.a


########################################################################


.PHONY: all clean


########################################################################


