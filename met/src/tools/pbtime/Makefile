# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the pbtime tool
#
########################################################################


EXECUTABLES = pbtime


OBJECTS     = openpb.o      \
              readpb.o


LIBRARIES   =


########################################################################


HEADERS     =


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the pbtime Tool ***"
	@ echo


########################################################################


   ##
   ##  objects
   ##


openpb.o: ../pb2nc/openpb.f
	@ echo
	$(FC) ../pb2nc/openpb.f $(FC_FLAGS) $(OPT_FLAGS) -c


readpb.o: ../pb2nc/readpb.f
	@ echo
	$(FC) ../pb2nc/readpb.f $(FC_FLAGS) $(OPT_FLAGS) -c


########################################################################


   ##
   ##  libraries
   ##


########################################################################


   ##
   ##  executables
   ##


pbtime: pbtime.cc $(OBJECTS)
	$(CXX) -o $(EXECUTABLES) pbtime.cc $(OBJECTS) \
	$(CXX_FLAGS) $(OPT_FLAGS) \
	-I$(MET_INC_DIR) $(BUFR_INCS) $(F2C_INCS) \
	-L$(MET_LIB_DIR) $(BUFR_LIBS) $(F2C_LIBS) \
	-lvx_pb_util -lvx_cal -lvx_util -lbufr \
        $(F2C_LIBNAME) $(CXX_LIBS) $(FC_LIBS)
	cp $(EXECUTABLES) $(MET_BIN_DIR)


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	rm -f $(MET_BIN_DIR)/pbtime


########################################################################


.PHONY: all clean


########################################################################


