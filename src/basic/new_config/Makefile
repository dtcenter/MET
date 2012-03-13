

########################################################################


EXECUTABLES = test_read  \


########################################################################


OBJECTS     = config.tab.o  \
              lex.config.o  \


########################################################################


GEN_SOURCES =


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


config.tab.c: config.y scanner_stuff.h
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


########################################################################


   ##
   ##  libraries
   ##


########################################################################


   ##
   ##  executables
   ##


test_read: test_read.cc $(OBJECTS)
	@ echo
	$(COMPILER) test_read.cc lex.config.o config.tab.o $(FLAGS) -o test_read -L$(MET_LIB_DIR) -lvx_cal -lm


########################################################################


clean:
	@ echo
	rm -f config.output config.tab.c config.tab.h lex.config.c
	rm -f *.a *.o temp junk core log a.out $(OBJECTS) $(EXECUTABLES) $(LIBRARIES)


########################################################################




