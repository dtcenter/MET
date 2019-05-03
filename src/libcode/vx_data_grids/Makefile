# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_data_grids library
#
########################################################################


EXECUTABLES =


OBJECTS     = find_grid_by_name.o \
              grid_base.o         \
              latlon_grid.o       \
              lc_grid.o           \
              merc_grid.o         \
              st_grid.o


LIBRARIES   = libvx_data_grids.a


########################################################################


HEADERS     = find_grid_by_name.h \
              grid_base.h         \
              grid.h              \
              latlon_grid_defs.h  \
              latlon_grid.h       \
              lc_grid_defs.h      \
              lc_grid.h           \
              merc_grid_defs.h    \
              merc_grid.h         \
              st_grid_defs.h      \
              st_grid.h


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_data_grids Library ***"
	@ echo

########################################################################


   ##
   ##  objects
   ##


find_grid_by_name.o: find_grid_by_name.cc
	$(CXX) find_grid_by_name.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

grid_base.o: grid_base.cc
	$(CXX) grid_base.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

latlon_grid.o: latlon_grid.cc
	$(CXX) latlon_grid.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

lc_grid.o: lc_grid.cc
	$(CXX) lc_grid.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

merc_grid.o: merc_grid.cc
	$(CXX) merc_grid.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

st_grid.o: st_grid.cc
	$(CXX) st_grid.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_data_grids.a: $(OBJECTS)
	ar -rs libvx_data_grids.a $(OBJECTS)
	ranlib libvx_data_grids.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_data_grids.a
	cp libvx_data_grids.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_data_grids.a


########################################################################


.PHONY: all clean


########################################################################


