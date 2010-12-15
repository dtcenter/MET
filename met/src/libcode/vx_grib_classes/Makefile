# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_grib_classes library
#
########################################################################


EXECUTABLES =


OBJECTS     =  grib_classes.o \
               grib_strings.o


LIBRARIES   = libvx_grib_classes.a


########################################################################


HEADERS     =  grib_classes.h \
               grib_strings.h


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished making vx_grib_classes Library ***"
	@ echo


########################################################################


   ##
   ##  objects
   ##


grib_classes.o: grib_classes.cc
	$(CXX) grib_classes.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)

grib_strings.o: grib_strings.cc
	$(CXX) grib_strings.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_grib_classes.a: $(OBJECTS)
	ar -rs libvx_grib_classes.a $(OBJECTS)
	ranlib libvx_grib_classes.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_grib_classes.a
	cp libvx_grib_classes.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_grib_classes.a


########################################################################


.PHONY: all clean


########################################################################





