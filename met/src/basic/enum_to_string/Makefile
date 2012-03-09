# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2012
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the enum_to_string library
#
########################################################################


EXECUTABLES = enum_to_string


OBJECTS     = enum.tab.o \
              lex.yy.o \
              info.o \
              code.o \
              scope.o


LIBRARIES   = 


GEN_SOURCES =


########################################################################


HEADERS     = code.h \
              enum.tab.h \
              info.h \
              scope.h


########################################################################


YACC        = bison -d -t -v

LEX         = flex


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making enum_to_string***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##  call make all since this library does source code generation
   ##


gen_sources: $(GEN_SOURCES)
	@ $(MAKE) all


########################################################################


   ##
   ##  objects
   ##


info.o: info.h info.cc
	$(CXX) info.cc     -c $(CXX_FLAGS)


scope.o: scope.h scope.cc info.o
	$(CXX) scope.cc    -c $(CXX_FLAGS)


code.o: code.h code.cc info.o
	$(CXX) code.cc     -c $(CXX_FLAGS) -I$(MET_INC_DIR) -DMET_BASE_DIR=\"$(MET_BASE_DIR)\"


enum.tab.c: enum.y info.o code.o scope.o
	$(YACC) enum.y


enum.tab.o: enum.tab.c
	$(CXX) enum.tab.c  -c $(CXX_FLAGS)


lex.yy.c: enum.l enum.tab.o code.o scope.o
	$(LEX) enum.l
	@ rm -f temp
	@ mv lex.yy.c temp
	@ sed "s/static void yyunput/void yy_ets_unput/" temp > lex.yy.c
	@ rm -f temp


lex.yy.o: lex.yy.c
	$(CXX) lex.yy.c    -c $(CXX_FLAGS) > /dev/null 2>&1


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
	$(CXX) enum_to_string.cc lex.yy.o enum.tab.o info.o code.o scope.o $(CXX_FLAGS) -o enum_to_string -I$(MET_INC_DIR) -L$(MET_LIB_DIR) -lvx_cal -lm
	rm -f $(MET_BIN_DIR)/enum_to_string
	cp enum_to_string $(MET_BIN_DIR)


########################################################################


clean:
	@ echo
	rm -f *.a *.o temp junk core log a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	rm -f enum.tab.c enum.output lex.yy.c
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_BIN_DIR)/enum_to_string


########################################################################


.PHONY: all clean gen_sources


########################################################################

