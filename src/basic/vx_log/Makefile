# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_log library
#
########################################################################


EXECUTABLES =  test_logger


OBJECTS     =  concat_string.o \
               indent.o \
               logger.o \
               string_array.o


LIBRARIES   =  libvx_log.a


GEN_SOURCES =


########################################################################


HEADERS     =  concat_string.h \
               indent.h \
               logger.h \
               string_array.h \
               vx_log.h


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_log Library ***"
	@ echo


########################################################################


gen_sources: $(GEN_SOURCES)
	@ echo


########################################################################


   ##
   ##  objects
   ##


concat_string.o: concat_string.cc
	$(CXX) concat_string.cc $(CXX_FLAGS) -c

indent.o: indent.cc
	$(CXX) indent.cc $(CXX_FLAGS) -c

logger.o: logger.cc logger.h
	$(CXX) logger.cc $(CXX_FLAGS) $(ARCH_FLAGS) -c

string_array.o: string_array.cc
	$(CXX) string_array.cc $(CXX_FLAGS) -c


########################################################################


   ##
   ##  libraries
   ##


libvx_log.a: $(OBJECTS)
	ar -rs libvx_log.a $(OBJECTS) 2>&1
	ranlib libvx_log.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_log.a
	cp libvx_log.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


test_logger: test_logger.cc $(LIBRARIES)
	@ echo
	$(CXX) test_logger.cc -o test_logger $(CXX_FLAGS) $(ARCH_FLAGS) -I$(MET_INC_DIR) -L$(MET_LIB_DIR) -lvx_log


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_log.a


########################################################################


.PHONY: all clean


########################################################################

