# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2012
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


OBJECTS     = stat_columns.o \
              stat_hdr_columns.o


LIBRARIES   = libvx_stat_out.a


GEN_SOURCES =


########################################################################


HEADERS     = stat_columns.h \
              stat_hdr_columns.h \
              vx_stat_out.h


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_stat_out Library ***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##


gen_sources: $(GEN_SOURCES)
	@ echo


########################################################################


   ##
   ##  objects
   ##


stat_columns.o: stat_columns.cc
	$(CXX) stat_columns.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS)

stat_hdr_columns.o: stat_hdr_columns.cc
	$(CXX) stat_hdr_columns.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS)


########################################################################


   ##
   ##  libraries
   ##


libvx_stat_out.a: $(OBJECTS)
	ar -rs libvx_stat_out.a $(OBJECTS) 2>&1
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


.PHONY: all clean gen_sources


########################################################################

