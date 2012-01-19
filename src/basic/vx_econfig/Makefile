# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2012
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_econfig library
#
########################################################################


EXECUTABLES = econfig_codegen


OBJECTS     = pwl.o \
              algline.o \
              icode.o \
              idstack.o \
              symtab.o \
              builtin.o \
              machine.o \
              array.o \
              celltype_to_string.o \
              stetype_to_string.o \
              builtin_to_string.o \
              econfig.tab.o \
              lex.econfig.o \
              resulttype_to_string.o \
              result.o \
              icodecell_to_result.o \
              gen.o


LIBRARIES   = libvx_econfig.a


GEN_SOURCES = celltype_to_string.h celltype_to_string.cc \
              stetype_to_string.h stetype_to_string.cc \
              builtin_to_string.h builtin_to_string.cc \
              resulttype_to_string.h resulttype_to_string.cc


########################################################################


HEADERS     = algline.h \
              array.h \
              builtin.h \
              icode.h \
              idstack.h \
              machine.h \
              pwl.h \
              symtab.h \
              result.h \
              resulttype_to_string.h \
              builtin_to_string.h \
              celltype_to_string.h \
              stetype_to_string.h \
              gen.h \
              icodecell_to_result.h


########################################################################


YACC        = bison -d -t -v -p econfig

LEX         = flex --prefix=econfig


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_econfig Library ***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##  call make all since this library does source code generation
   ##


gen_sources: $(GEN_SOURCES)
	@ $(MAKE) all


celltype_to_string.h celltype_to_string.cc: icode.h
	@ rm -f celltype_to_string.h celltype_to_string.cc
	$(ENUM_TO_STRING) -concat_string icode.h

stetype_to_string.h stetype_to_string.cc: symtab.h
	@ rm -f stetype_to_string.h stetype_to_string.cc
	$(ENUM_TO_STRING) -concat_string symtab.h

builtin_to_string.h builtin_to_string.cc: builtin.h
	@ rm -f builtin_to_string.h builtin_to_string.cc
	$(ENUM_TO_STRING) -concat_string builtin.h

resulttype_to_string.h resulttype_to_string.cc: result.h
	@ rm -f resulttype_to_string.h resulttype_to_string.cc
	$(ENUM_TO_STRING) -concat_string result.h


########################################################################


   ##
   ##  objects
   ##


celltype_to_string.o: celltype_to_string.h celltype_to_string.cc
	$(CXX) celltype_to_string.cc   -c $(CXX_FLAGS) -I$(MET_INC_DIR)

stetype_to_string.o: stetype_to_string.h stetype_to_string.cc
	$(CXX) stetype_to_string.cc    -c $(CXX_FLAGS) -I$(MET_INC_DIR)

builtin_to_string.o: builtin_to_string.h builtin_to_string.cc
	$(CXX) builtin_to_string.cc    -c $(CXX_FLAGS) -I$(MET_INC_DIR)

idstack.o: idstack.h idstack.cc
	$(CXX) idstack.cc              -c $(CXX_FLAGS) -I$(MET_INC_DIR)

icode.o: icode.h icode.cc celltype_to_string.o pwl.o builtin.o
	$(CXX) icode.cc                -c $(CXX_FLAGS) -I$(MET_INC_DIR)

symtab.o: symtab.h symtab.cc stetype_to_string.o celltype_to_string.o pwl.o algline.o builtin.o array.o
	$(CXX) symtab.cc               -c $(CXX_FLAGS) -I$(MET_INC_DIR)

array.o: array.h array.cc icode.o
	$(CXX) array.cc                -c $(CXX_FLAGS) -I$(MET_INC_DIR)

builtin.o: builtin.h builtin.cc
	$(CXX) builtin.cc              -c $(CXX_FLAGS)

pwl.o: pwl.h pwl.cc
	$(CXX) pwl.cc                  -c $(CXX_FLAGS) -I$(MET_INC_DIR)

algline.o: algline.h algline.cc
	$(CXX) algline.cc              -c $(CXX_FLAGS) -I$(MET_INC_DIR)

machine.o: machine.h machine.cc symtab.o icode.o celltype_to_string.o stetype_to_string.o builtin_to_string.o builtin.o
	$(CXX) machine.cc              -c $(CXX_FLAGS) -I$(MET_INC_DIR)

econfig.tab.c: econfig.y icode.o symtab.o machine.o idstack.o builtin.o celltype_to_string.o
	$(YACC) econfig.y

econfig.tab.o: econfig.tab.c
	@ echo
	$(CXX) econfig.tab.c           -c $(CXX_FLAGS) -I$(MET_INC_DIR)

lex.econfig.c: econfig.l econfig.tab.o icode.o symtab.o machine.o
	$(LEX) econfig.l
	rm -f temp
	mv lex.econfig.c temp
	sed "s/static void yyunput/void yy_econfig_unput/" temp > lex.econfig.c
	rm -f temp

lex.econfig.o: lex.econfig.c
	$(CXX) lex.econfig.c           -c $(CXX_FLAGS) -I$(MET_INC_DIR)

resulttype_to_string.o: resulttype_to_string.h resulttype_to_string.cc
	$(CXX) resulttype_to_string.cc -c $(CXX_FLAGS) -I$(MET_INC_DIR)

result.o: result.h result.cc
	$(CXX) result.cc               -c $(CXX_FLAGS) -I$(MET_INC_DIR)

icodecell_to_result.o: icodecell_to_result.h icodecell_to_result.cc icode.o result.o celltype_to_string.o resulttype_to_string.o
	$(CXX) icodecell_to_result.cc  -c $(CXX_FLAGS) -I$(MET_INC_DIR)

gen.o: gen.h gen.cc machine.o result.o
	$(CXX) gen.cc                  -c $(CXX_FLAGS) -I$(MET_INC_DIR)

########################################################################


   ##
   ##  libraries
   ##


libvx_econfig.a: $(OBJECTS)
	ar -rs libvx_econfig.a $(OBJECTS) 2>&1
	ranlib libvx_econfig.a
	rm -f $(MET_LIB_DIR)/libvx_econfig.a
	cp libvx_econfig.a $(MET_LIB_DIR)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)


########################################################################


   ##
   ##  executables
   ##

econfig_codegen: econfig_codegen.cc libvx_econfig.a
	@ echo
	$(CXX) econfig_codegen.cc -o econfig_codegen $(CXX_FLAGS) -I$(MET_INC_DIR) -L$(MET_LIB_DIR) \
	-lvx_econfig \
	-lvx_cal \
	-lvx_util \
	-lvx_log \
	-lm
	rm -f $(MET_BIN_DIR)/econfig_codegen
	cp econfig_codegen $(MET_BIN_DIR)


########################################################################


clean:
	@ echo
	rm -f *.a *.o temp junk core log a.out out out2 $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	rm -f econfig.output econfig.tab.c econfig.tab.h
	rm -f lex.econfig.c
	rm -f $(MET_BIN_DIR)/econfig_codegen
	rm -f $(MET_LIB_DIR)/libvx_econfig.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)


########################################################################


.PHONY: all clean gen_sources


########################################################################

