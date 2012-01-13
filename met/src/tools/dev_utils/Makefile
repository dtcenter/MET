# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the dev_utils tools (development utilities)
#
########################################################################


EXECUTABLES = chk4copyright         \
              reformat_map_data     \
              reformat_county_data  \
	      pbtime


OBJECTS     = openpb.o \
	      readpb.o


LIBRARIES   =


########################################################################


HEADERS     =


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the dev_utils Tools ***"
	@ echo


########################################################################


   ##
   ##  objects
   ##


openpb.o: ../other/pb2nc/openpb.f
	@ echo
	$(FC) ../other/pb2nc/openpb.f $(FC_FLAGS) -c


readpb.o: ../other/pb2nc/readpb.f
	@ echo
	$(FC) ../other/pb2nc/readpb.f $(FC_FLAGS) -c


########################################################################


   ##
   ##  libraries
   ##


########################################################################


   ##
   ##  executables
   ##


chk4copyright: chk4copyright.cc $(LIBRARIES)
	@ echo
	$(CXX) chk4copyright.cc -o chk4copyright $(CXX_FLAGS) $(ARCH_FLAGS) \
	-I$(MET_INC_DIR) -L$(MET_LIB_DIR) -lvx_util -lvx_log
	cp chk4copyright $(MET_BIN_DIR)


reformat_map_data: reformat_map_data.cc $(LIBRARIES)
	@ echo
	$(CXX) reformat_map_data.cc -o reformat_map_data $(CXX_FLAGS) $(ARCH_FLAGS) \
	-I$(MET_INC_DIR) -L$(MET_LIB_DIR) -lvx_util -lvx_log
	cp reformat_map_data $(MET_BIN_DIR)


reformat_county_data: reformat_county_data.cc $(LIBRARIES)
	@ echo
	$(CXX) reformat_county_data.cc -o reformat_county_data $(CXX_FLAGS) $(ARCH_FLAGS) \
	-I$(MET_INC_DIR) -L$(MET_LIB_DIR) -lvx_util -lvx_log
	cp reformat_county_data $(MET_BIN_DIR)


pbtime: pbtime.cc $(OBJECTS)
	@ echo
	$(CXX) -o pbtime pbtime.cc $(OBJECTS) \
	$(CXX_FLAGS) \
	-I$(MET_INC_DIR) $(BUFR_INCS) $(F2C_INCS) \
	-L$(MET_LIB_DIR) $(BUFR_LIBS) $(F2C_LIBS) \
	-lvx_pb_util -lvx_cal -lvx_util -lvx_log -lbufr \
        $(F2C_LIBNAME) $(CXX_LIBS) $(FC_LIBS)
	cp pbtime $(MET_BIN_DIR)


########################################################################


clean:
	rm -f *.a *.o *.ps temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	rm -f $(MET_BIN_DIR)/chk4copyright
	rm -f $(MET_BIN_DIR)/reformat_map_data
	rm -f $(MET_BIN_DIR)/reformat_county_data
	rm -f $(MET_BIN_DIR)/pbtime


########################################################################


.PHONY: all clean


########################################################################


