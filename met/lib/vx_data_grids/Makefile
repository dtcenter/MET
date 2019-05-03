# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the vx_data_grids library
#
###############################################################################

TARGETS  = libvx_data_grids.a

OBJECTS  =  exp_grid.o \
	grid_base.o \
	grid_defs.o \
	lc_grid.o \
	merc_grid.o \
	pc_grid.o \
	st_grid.o

OPT_FLAGS=

FAKES    = vx_data_grids_all vx_data_grids_lib clean

LIB_DIR  = ..


vx_data_grids_all:
	@ echo
	@ echo "*** Making vx_data_grids Library ***"
	@ echo
	@ $(MAKE) vx_data_grids_lib  $(PRINT_OPTS)
	cp $(TARGETS) ../.

vx_data_grids_lib: libvx_data_grids.a
	@ echo

libvx_data_grids.a: $(OBJECTS)
	ar -rs libvx_data_grids.a $(OBJECTS)
	ranlib libvx_data_grids.a

exp_grid.o: exp_grid.cc
	$(CXX) exp_grid.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

grid_base.o: grid_base.cc
	$(CXX) grid_base.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

grid_defs.o: grid_defs.cc
	$(CXX) grid_defs.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

lc_grid.o: lc_grid.cc
	$(CXX) lc_grid.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

merc_grid.o: merc_grid.cc
	$(CXX) merc_grid.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

pc_grid.o: pc_grid.cc
	$(CXX) pc_grid.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

st_grid.o: st_grid.cc
	$(CXX) st_grid.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

clean:
	rm -f *.a *.o temp junk core a.out $(TARGETS) ../$(TARGETS)

.PHONY: $(FAKES)


