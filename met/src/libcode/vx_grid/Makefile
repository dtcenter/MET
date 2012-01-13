# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_grid library
#
########################################################################


EXECUTABLES =


OBJECTS     = find_grid_by_name.o \
              grid_base.o         \
              latlon_grid.o       \
              lc_grid.o           \
              merc_grid.o         \
              st_grid.o


LIBRARIES   = libvx_grid.a


########################################################################


HEADERS     = find_grid_by_name.h \
              grid_base.h         \
              vx_grid.h           \
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
	@ echo "*** Finished Making the vx_grid Library ***"
	@ echo

########################################################################


   ##
   ##  objects
   ##


find_grid_by_name.o: find_grid_by_name.cc
	$(CXX) find_grid_by_name.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

grid_base.o: grid_base.cc
	$(CXX) grid_base.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

latlon_grid.o: latlon_grid.cc
	$(CXX) latlon_grid.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

lc_grid.o: lc_grid.cc
	$(CXX) lc_grid.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

merc_grid.o: merc_grid.cc
	$(CXX) merc_grid.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

st_grid.o: st_grid.cc
	$(CXX) st_grid.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_grid.a: $(OBJECTS)
	ar -rs libvx_grid.a $(OBJECTS) 2>&1
	ranlib libvx_grid.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_grid.a
	cp libvx_grid.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_grid.a


########################################################################


.PHONY: all clean


########################################################################


