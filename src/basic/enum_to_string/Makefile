

########################################################################


EXECUTABLES = enum_to_string

LIBRARIES   = 

OBJECTS     = enum.tab.o lex.yy.o info.o code.o scope.o

HEADERS     = code.h enum.tab.h info.h scope.h


########################################################################


YACC        = bison -d -t -v

LEX         = flex


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo


########################################################################


   ##
   ##  objects
   ##


info.o: info.h info.cc
	@ echo
	$(CXX) info.cc     -c $(FLAGS)


scope.o: scope.h scope.cc info.o
	@ echo
	$(CXX) scope.cc    -c $(FLAGS)


code.o: code.h code.cc info.o
	@ echo
	$(CXX) code.cc     -c $(FLAGS) -I$(MET_INC_DIR)


enum.tab.c: enum.y info.o code.o scope.o
	@ echo
	$(YACC) enum.y


enum.tab.o: enum.tab.c
	@ echo
	$(CXX) enum.tab.c  -c $(FLAGS)


lex.yy.c: enum.l enum.tab.o code.o scope.o
	@ echo
	$(LEX) enum.l
	rm -f temp
	mv lex.yy.c temp
	sed "s/static void yyunput/void yy_ets_unput/" temp > lex.yy.c
	rm -f temp


lex.yy.o: lex.yy.c
	@ echo
	$(CXX) lex.yy.c    -c $(FLAGS)


########################################################################


   ##
   ##  libs
   ##


########################################################################


   ##
   ##  executables
   ##


enum_to_string: enum_to_string.cc lex.yy.o enum.tab.o $(OBJECTS)
	@ echo
	$(CXX) enum_to_string.cc lex.yy.o enum.tab.o info.o code.o scope.o $(FLAGS) -o enum_to_string -I$(MET_INC_DIR) -L$(MET_LIB_DIR) -lcal -lm


########################################################################


clean:
	@ echo
	rm -f *.a *.o temp junk core log a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)


########################################################################




