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


EXECUTABLES = test_table_read     \
              dump_default_table  \


OBJECTS     = level_info.o \
              var_info.o \
              leveltype_to_string.o \
              data_class.o \
              data2d_utils.o \
              table_lookup.o


LIBRARIES   = libvx_data2d.a


GEN_SOURCES = leveltype_to_string.cc \
              leveltype_to_string.h


########################################################################


HEADERS     = level_info.h \
              var_info.h \
              leveltype_to_string.h \
              data_class.h \
              data2d_utils.h \
              table_lookup.h \
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

leveltype_to_string.o: leveltype_to_string.h leveltype_to_string.cc
	$(CXX) leveltype_to_string.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

data_class.o: data_class.h data_class.cc
	$(CXX) data_class.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

data2d_utils.o: data2d_utils.h data2d_utils.cc
	$(CXX) data2d_utils.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

table_lookup.o: table_lookup.h table_lookup.cc
	$(CXX) table_lookup.cc $(CXX_FLAGS) -c -DMET_BASE_DIR=\"$(MET_BASE_DIR)\" -I$(MET_INC_DIR)


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


test_table_read: test_table_read.cc libvx_data2d.a
	@ echo
	$(CXX) -o test_table_read test_table_read.cc $(OBJECTS) \
	$(CXX_FLAGS) \
	-I$(MET_INC_DIR) \
	-L$(MET_LIB_DIR) \
	-lvx_cal \
	-lvx_config \
	-lvx_log \
	-lvx_data2d \
	-lvx_util \
	-lvx_math \
	$(CXX_LIBS)
	cp test_table_read $(MET_BIN_DIR)


dump_default_table: dump_default_table.cc libvx_data2d.a
	@ echo
	$(CXX) -o dump_default_table dump_default_table.cc $(OBJECTS) \
	$(CXX_FLAGS) \
	-I$(MET_INC_DIR) \
	-L$(MET_LIB_DIR) \
	-lvx_cal \
	-lvx_config \
	-lvx_log \
	-lvx_data2d \
	-lvx_util \
	-lvx_math \
	$(CXX_LIBS)
	cp dump_default_table $(MET_BIN_DIR)

########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES) $(GEN_SOURCES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_data2d.a


########################################################################


.PHONY: all clean gen_sources


########################################################################

