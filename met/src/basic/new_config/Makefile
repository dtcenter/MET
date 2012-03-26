

########################################################################


EXECUTABLES = test_config  \


########################################################################


OBJECTS     = configobjecttype_to_string.o  \
              dictionary.o                  \
              config.tab.o                  \
              lex.config.o                  \
              config_file.o                 \


########################################################################


GEN_SOURCES = configobjecttype_to_string.h   \
              configobjecttype_to_string.cc  \


########################################################################


LIBRARIES   =


########################################################################


COMPILER  = g++

FLAGS     = -g -Wall -Wshadow


ARCHIVE   = ar r


########################################################################


LEX_PREFIX = config

YACC       = bison -d -t -v -p $(LEX_PREFIX)

LEX        = flex --prefix=$(LEX_PREFIX)

ARRAY_GEN  = $(CODETREE_DIR)/randy/bin/arraygen


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) 
	@ echo


########################################################################


   ##
   ##  objects
   ##


configobjecttype_to_string.h configobjecttype_to_string.cc: object_types.h
	@ echo
	@ rm -f configobjecttype_to_string.h configobjecttype_to_string.cc
	@ $(ENUM_TO_STRING) -concat_string object_types.h


configobjecttype_to_string.o: configobjecttype_to_string.h configobjecttype_to_string.cc
	@ echo
	$(COMPILER) configobjecttype_to_string.cc -c $(FLAGS) -I$(MET_INC_DIR)


dictionary.o: dictionary.h dictionary.cc configobjecttype_to_string.o
	@ echo
	$(COMPILER) dictionary.cc -c $(FLAGS) -I$(MET_INC_DIR)


config.tab.c: config.y scanner_stuff.h dictionary.o
	@ echo
	$(YACC) config.y


config.tab.o: config.tab.c
	@ echo
	$(COMPILER) config.tab.c -c $(FLAGS) -I$(MET_INC_DIR)


lex.config.c: config.l config.tab.o scanner_stuff.h
	@ echo
	$(LEX) config.l

lex.config.o: lex.config.c
	@ echo
	$(COMPILER) lex.config.c -c -g -I$(MET_INC_DIR)


config_file.o: config_file.h config_file.cc configobjecttype_to_string.o dictionary.o config.tab.o lex.config.o
	@ echo
	$(COMPILER) config_file.cc -c -g -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


########################################################################


   ##
   ##  executables
   ##


test_config: test_config.cc $(OBJECTS)
	@ echo
	$(COMPILER) test_config.cc config_file.o dictionary.o configobjecttype_to_string.o lex.config.o config.tab.o $(FLAGS) -o test_config -I$(MET_INC_DIR) -L$(MET_LIB_DIR) -lvx_util -lvx_math -lvx_cal -lvx_log -lm


########################################################################


clean:
	@ echo
	rm -f config.output config.tab.c config.tab.h lex.config.c
	rm -f *.a *.o temp junk core log a.out $(OBJECTS) $(EXECUTABLES) $(LIBRARIES)


########################################################################




