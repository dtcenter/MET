# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2013
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_config library
#
########################################################################


EXECUTABLES = test_config   \
              test_lookup   \
              test_lookup2  \
              test_lookup3  \
              test_string   \


OBJECTS     = configobjecttype_to_string.o \
              grdfiletype_to_string.o \
              dictionary.o \
              config.tab.o \
              lex.config.o \
              config_file.o \
              config_util.o \


LIBRARIES   = libvx_config.a


GEN_SOURCES = grdfiletype_to_string.cc \
              grdfiletype_to_string.h \
              configobjecttype_to_string.h \
              configobjecttype_to_string.cc \
              lex.config.cc \
              config.tab.h \
              config.tab.cc \


########################################################################


HEADERS     = config_file.h \
              configobjecttype_to_string.h \
              grdfiletype_to_string.h \
              data_file_type.h \
              config.tab.h \
              dictionary.h \
              object_types.h \
              scanner_stuff.h \
              config_util.h \
              config_constants.h \
              vx_config.h \


########################################################################


LEX_PREFIX = config

YACC       = bison -d -t -v -p $(LEX_PREFIX)

LEX        = flex --prefix=$(LEX_PREFIX)


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_config Library ***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##


gen_sources: $(GEN_SOURCES)
	@ echo


configobjecttype_to_string.h configobjecttype_to_string.cc: object_types.h
	@ rm -f configobjecttype_to_string.h configobjecttype_to_string.cc
	@ $(ENUM_TO_STRING) -concat_string object_types.h


grdfiletype_to_string.cc grdfiletype_to_string.h: data_file_type.h
	@ echo
	@ rm -f grdfiletype_to_string.cc grdfiletype_to_string.h
	$(ENUM_TO_STRING) -concat_string -reverse data_file_type.h


lex.config.cc: config.l config.tab.h scanner_stuff.h
	$(LEX) config.l
	@ mv lex.config.c lex.config.cc


config.tab.cc config.tab.h: config.y scanner_stuff.h dictionary.o
	$(YACC) config.y
	@ mv config.tab.c config.tab.cc


########################################################################


   ##
   ##  objects
   ##


configobjecttype_to_string.o: configobjecttype_to_string.h configobjecttype_to_string.cc
	$(CXX) configobjecttype_to_string.cc -c $(CXX_FLAGS) -I$(MET_INC_DIR)


grdfiletype_to_string.o: grdfiletype_to_string.h grdfiletype_to_string.cc
	$(CXX) grdfiletype_to_string.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)


dictionary.o: dictionary.h dictionary.cc configobjecttype_to_string.o
	$(CXX) dictionary.cc -c $(CXX_FLAGS) -I$(MET_INC_DIR)


config.tab.o: config.tab.cc
	$(CXX) config.tab.cc -c $(CXX_FLAGS) -I$(MET_INC_DIR)


lex.config.o: lex.config.cc
	$(CXX) lex.config.cc -c $(CXX_FLAGS) -I$(MET_INC_DIR)


config_file.o: config_file.h config_file.cc configobjecttype_to_string.o dictionary.o config.tab.o lex.config.o
	$(CXX) config_file.cc -c $(CXX_FLAGS) -I$(MET_INC_DIR)


config_util.o: config_util.h config_util.cc
	$(CXX) config_util.cc -c $(CXX_FLAGS) -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_config.a: $(OBJECTS)
	ar -rs libvx_config.a $(OBJECTS) 2>&1
	ranlib libvx_config.a
	rm -f $(MET_LIB_DIR)/libvx_config.a
	cp libvx_config.a $(MET_LIB_DIR)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)


########################################################################


   ##
   ##  executables
   ##


test_config: test_config.cc $(OBJECTS) $(LIBRARIES)
	$(CXX) test_config.cc -o test_config $(CXX_FLAGS) -I$(MET_INC_DIR) -L$(MET_LIB_DIR) \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm  \


test_lookup: test_lookup.cc $(OBJECTS) $(LIBRARIES)
	$(CXX) test_lookup.cc -o test_lookup $(CXX_FLAGS) -I$(MET_INC_DIR) -L$(MET_LIB_DIR) \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm  \


test_lookup2: test_lookup2.cc $(OBJECTS) $(LIBRARIES)
	$(CXX) test_lookup2.cc -o test_lookup2 $(CXX_FLAGS) -I$(MET_INC_DIR) -L$(MET_LIB_DIR) \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm  \


test_lookup3: test_lookup3.cc $(OBJECTS) $(LIBRARIES)
	$(CXX) test_lookup3.cc -g -o test_lookup3 $(CXX_FLAGS) -I$(MET_INC_DIR) -L$(MET_LIB_DIR) \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm  \


test_string: test_string.cc $(OBJECTS) $(LIBRARIES)
	$(CXX) test_string.cc -o test_string $(CXX_FLAGS) -I$(MET_INC_DIR) -L$(MET_LIB_DIR) \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm  \


########################################################################


clean:
	@ echo
	rm -f *.a *.o temp junk core log a.out out out2 $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	rm -f config.output
	rm -f $(MET_LIB_DIR)/libvx_config.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)


########################################################################


.PHONY: all clean gen_sources


########################################################################

