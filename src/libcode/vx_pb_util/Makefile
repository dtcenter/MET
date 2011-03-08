# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_pb_util library
#
########################################################################


EXECUTABLES =


OBJECTS     =  pblock.o        \
               do_blocking.o   \
               do_unblocking.o \
               copy_bytes.o


LIBRARIES   = libvx_pb_util.a


########################################################################


HEADERS     =  pblock.h        \
               do_blocking.h   \
               do_unblocking.h \
               copy_bytes.h    \
               vx_pb_util.h


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished making vx_pb_util Library ***"
	@ echo


########################################################################


   ##
   ##  objects
   ##


pblock.o: pblock.cc
	$(CXX) pblock.cc $(ARCH_FLAGS) $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)

do_blocking.o: do_blocking.cc
	$(CXX) do_blocking.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)

do_unblocking.o: do_unblocking.cc
	$(CXX) do_unblocking.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)

copy_bytes.o: copy_bytes.cc
	$(CXX) copy_bytes.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_pb_util.a: $(OBJECTS)
	ar -rs libvx_pb_util.a $(OBJECTS)
	ranlib libvx_pb_util.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_pb_util.a
	cp libvx_pb_util.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_pb_util.a


########################################################################


.PHONY: all clean


########################################################################


