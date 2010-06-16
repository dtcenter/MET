
#// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
#// ** Copyright UCAR (c) 1992 - 2007
#// ** University Corporation for Atmospheric Research (UCAR)
#// ** National Center for Atmospheric Research (NCAR)
#// ** Research Applications Lab (RAL)
#// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
#// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################


TARGETS   = libvx_data_grids.a


########################################################################


OBJECTS     = exp_grid.o           \
              grid_base.o          \
              latlon_grid.o        \
              lc_grid.o            \
              st_grid.o            \
              merc_grid.o          \
              find_grid_by_name.o  \


########################################################################


HEADERS     = exp_grid.h           \
              grid_base.h          \
              grid.h               \
              latlon_grid.h        \
              lc_grid.h            \
              st_grid.h            \
              merc_grid.h          \
              st_grid_defs.h       \
              lc_grid_defs.h       \
              exp_grid_defs.h      \
              latlon_grid_defs.h   \
              merc_grid_defs.h     \
              find_grid_by_name.h  \


########################################################################


all: $(OBJECTS) $(LIBRARIES)
	@ echo


tests: $(EXECUTABLES)
	@ echo


########################################################################


   ##
   ##  objects
   ##


grid_base.o: grid_base.h grid_base.cc
	@ echo
	$(COMPILER) grid_base.cc          -c $(FLAGS) -I$(INC_DIR)


st_grid.o: st_grid.cc st_grid.h grid_base.o st_grid_defs.h
	@ echo
	$(COMPILER) st_grid.cc            -c $(FLAGS) -I$(INC_DIR)


lc_grid.o: lc_grid.cc lc_grid.h grid_base.o lc_grid_defs.h
	@ echo
	$(COMPILER) lc_grid.cc            -c $(FLAGS) -I$(INC_DIR)


exp_grid.o: exp_grid.cc exp_grid.h grid_base.o exp_grid_defs.h
	@ echo
	$(COMPILER) exp_grid.cc           -c $(FLAGS) -I$(INC_DIR)


latlon_grid.o: latlon_grid.cc latlon_grid.h grid_base.o latlon_grid_defs.h
	@ echo
	$(COMPILER) latlon_grid.cc        -c $(FLAGS) -I$(INC_DIR)


merc_grid.o: merc_grid.cc merc_grid.h grid_base.o merc_grid_defs.h
	@ echo
	$(COMPILER) merc_grid.cc          -c $(FLAGS) -I$(INC_DIR)


find_grid_by_name.o: find_grid_by_name.cc find_grid_by_name.h grid_base.h exp_grid.h latlon_grid.h lc_grid.h st_grid.h merc_grid.h
	@ echo
	$(COMPILER) find_grid_by_name.cc  -c $(FLAGS) -I$(INC_DIR)


########################################################################


   ##
   ##  libs
   ##


libvx_data_grids.a: $(OBJECTS)
	@ echo
	ar -rs libvx_data_grids.a $(OBJECTS) 2>&1
	ranlib libvx_data_grids.a


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	@ echo
	rm -f *.a *.o *.ps *.ppm *.pgm *.png temp junk core log a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)


########################################################################




