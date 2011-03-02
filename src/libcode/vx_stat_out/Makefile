# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_stat_out library
#
########################################################################


EXECUTABLES =


OBJECTS     = stat_columns.o       \
		stat_hdr_columns.o \
		write_netcdf.o


LIBRARIES   = libvx_stat_out.a


########################################################################


HEADERS     = stat_columns.h       \
		stat_hdr_columns.h \
		write_netcdf.h


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)


########################################################################


   ##
   ##  objects
   ##


stat_columns.o: stat_columns.cc
	$(CXX) stat_columns.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

stat_hdr_columns.o: stat_hdr_columns.cc
	$(CXX) stat_hdr_columns.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

write_netcdf.o: write_netcdf.cc
	$(CXX) write_netcdf.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_stat_out.a: $(OBJECTS)
	ar -rs libvx_stat_out.a $(OBJECTS)
	ranlib libvx_stat_out.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_stat_out.a
	cp libvx_stat_out.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_stat_out.a


########################################################################


.PHONY: all clean


########################################################################


