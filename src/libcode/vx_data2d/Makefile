# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2012
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_data2d library
#
########################################################################


EXECUTABLES =


OBJECTS     = level_info.o \
              var_info.o \
              grdfiletype_to_string.o \
              leveltype_to_string.o \
              data_class.o \
              data2d_utils.o


LIBRARIES   = libvx_data2d.a


GEN_SOURCES = grdfiletype_to_string.cc \
              grdfiletype_to_string.h \
              leveltype_to_string.cc \
              leveltype_to_string.h


########################################################################


HEADERS     = data_file_type.h \
              level_info.h \
              var_info.h \
              grdfiletype_to_string.h \
              leveltype_to_string.h \
              data_class.h \
              data2d_utils.h \
              vx_data2d.h


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_data2d Library ***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##


gen_sources: $(GEN_SOURCES)
	@ echo

grdfiletype_to_string.cc grdfiletype_to_string.h: data_file_type.h
	@ echo
	@ rm -f grdfiletype_to_string.cc grdfiletype_to_string.h
	$(ENUM_TO_STRING) -concat_string -reverse data_file_type.h

leveltype_to_string.cc leveltype_to_string.h: level_info.h
	@ echo
	@ rm -f leveltype_to_string.cc leveltype_to_string.h
	$(ENUM_TO_STRING) -concat_string -reverse level_info.h


########################################################################


   ##
   ##  objects
   ##


level_info.o: level_info.cc
	$(CXX) level_info.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

var_info.o: var_info.cc
	$(CXX) var_info.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

grdfiletype_to_string.o: grdfiletype_to_string.h grdfiletype_to_string.cc
	$(CXX) grdfiletype_to_string.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

leveltype_to_string.o: leveltype_to_string.h leveltype_to_string.cc
	$(CXX) leveltype_to_string.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

data_class.o: data_class.h data_class.cc
	$(CXX) data_class.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

data2d_utils.o: data2d_utils.h data2d_utils.cc
	$(CXX) data2d_utils.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_data2d.a: $(OBJECTS)
	ar -rs libvx_data2d.a $(OBJECTS) 2>&1
	ranlib libvx_data2d.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_data2d.a
	cp libvx_data2d.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_data2d.a


########################################################################


.PHONY: all clean gen_sources


########################################################################

