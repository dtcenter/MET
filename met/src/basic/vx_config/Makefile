# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2012
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
              dictionary.o \
              config.tab.o \
              lex.config.o \
              config_file.o


LIBRARIES   = libvx_config.a


GEN_SOURCES = configobjecttype_to_string.h \
              configobjecttype_to_string.cc


########################################################################


HEADERS     = config_file.h \
              configobjecttype_to_string.h \
              config.tab.h \
              dictionary.h \
              object_types.h \
              scanner_stuff.h


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


########################################################################


   ##
   ##  objects
   ##


configobjecttype_to_string.o: configobjecttype_to_string.h configobjecttype_to_string.cc
	$(CXX) configobjecttype_to_string.cc -c $(CXX_FLAGS) -I$(MET_INC_DIR)


dictionary.o: dictionary.h dictionary.cc configobjecttype_to_string.o
	$(CXX) dictionary.cc -c $(CXX_FLAGS) -I$(MET_INC_DIR)


config.tab.c: config.y scanner_stuff.h dictionary.o
	$(YACC) config.y


config.tab.o: config.tab.c
	$(CXX) config.tab.c -c $(CXX_FLAGS) -I$(MET_INC_DIR)


lex.config.c: config.l config.tab.o scanner_stuff.h
	$(LEX) config.l

lex.config.o: lex.config.c
	$(CXX) lex.config.c -c -g -I$(MET_INC_DIR)


config_file.o: config_file.h config_file.cc configobjecttype_to_string.o dictionary.o config.tab.o lex.config.o
	$(CXX) config_file.cc -c -g -I$(MET_INC_DIR)


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
	$(CXX) test_lookup3.cc -o test_lookup3 $(CXX_FLAGS) -I$(MET_INC_DIR) -L$(MET_LIB_DIR) \
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
	rm -f config.output config.tab.c config.tab.h lex.config.c
	rm -f lex.econfig.cc
	rm -f $(MET_BIN_DIR)/econfig_codegen
	rm -f $(MET_LIB_DIR)/libvx_config.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)


########################################################################


.PHONY: all clean gen_sources


########################################################################

