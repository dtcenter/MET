# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_wrfdata library
#
########################################################################


EXECUTABLES =


OBJECTS     = filterbox.o  \
              interpmthd.o \
              moments.o    \
              node.o       \
              polyline.o   \
              vx_wrfdata.o


LIBRARIES   = libvx_wrfdata.a


########################################################################


HEADERS     = filterbox.h  \
              interpmthd.h \
              moments.h    \
              node.h       \
              polyline.h   \
              shape.h      \
              vx_wrfdata.h


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_wrfdata Library ***"
	@ echo

########################################################################


   ##
   ##  objects
   ##


filterbox.o: filterbox.cc
	$(CXX) filterbox.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

interpmthd.o: interpmthd.cc
	$(CXX) interpmthd.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

moments.o: moments.cc
	$(CXX) moments.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

node.o: node.cc
	$(CXX) node.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

polyline.o: polyline.cc
	$(CXX) polyline.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

vx_wrfdata.o: vx_wrfdata.cc
	$(CXX) vx_wrfdata.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_wrfdata.a: $(OBJECTS)
	ar -rs libvx_wrfdata.a $(OBJECTS)
	ranlib libvx_wrfdata.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_wrfdata.a
	cp libvx_wrfdata.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_wrfdata.a


########################################################################


.PHONY: all clean


########################################################################


