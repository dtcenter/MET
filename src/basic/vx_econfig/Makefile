

########################################################################


EXECUTABLES  = econfig_codegen


LIBRARIES    = libeconfig.a



OBJECTS      = pwl.o                   \
               algline.o               \
               icode.o                 \
               idstack.o               \
               symtab.o                \
               builtin.o               \
               machine.o               \
               array.o                 \
               celltype_to_string.o    \
               stetype_to_string.o     \
               builtin_to_string.o     \
               econfig.tab.o           \
               lex.econfig.o           \
               resulttype_to_string.o  \
               result.o                \
               icodecell_to_result.o   \
               gen.o



HEADERS      = algline.h               \
               array.h                 \
               builtin.h               \
               icode.h                 \
               idstack.h               \
               machine.h               \
               pwl.h                   \
               scanner_stuff.h         \
               symtab.h                \
               result.h                \
               resulttype_to_string.h  \
               builtin_to_string.h     \
               celltype_to_string.h    \
               stetype_to_string.h     \
               gen.h                   \
               icodecell_to_result.h


GEN_SOURCES = celltype_to_string.h celltype_to_string.cc      \
              stetype_to_string.h stetype_to_string.cc        \
              builtin_to_string.h builtin_to_string.cc        \
              resulttype_to_string.h resulttype_to_string.cc


########################################################################


YACC        = bison -d -t -v -p econfig

LEX         = flex --prefix=econfig


########################################################################


all: $(OBJECTS) $(LIBRARIES) econfig_codegen
	@ echo
	@ echo "*** Finished making econfig_codegen Library ***"
	@ echo


########################################################################


gen_sources: $(GEN_SOURCES)
	@ echo


celltype_to_string.h celltype_to_string.cc: icode.h
	@ rm -f celltype_to_string.h celltype_to_string.cc
	$(ENUM_TO_STRING) icode.h

stetype_to_string.h stetype_to_string.cc: symtab.h
	@ rm -f stetype_to_string.h stetype_to_string.cc
	$(ENUM_TO_STRING) symtab.h

builtin_to_string.h builtin_to_string.cc: builtin.h
	@ rm -f builtin_to_string.h builtin_to_string.cc
	$(ENUM_TO_STRING) builtin.h

resulttype_to_string.h resulttype_to_string.cc: result.h
	@ rm -f resulttype_to_string.h resulttype_to_string.cc
	$(ENUM_TO_STRING) result.h


########################################################################


   ##
   ##  objects
   ##


celltype_to_string.o: celltype_to_string.h celltype_to_string.cc
	$(CXX) celltype_to_string.cc   -c $(CXX_FLAGS) -I$(MET_INC_DIR)

stetype_to_string.o: stetype_to_string.h stetype_to_string.cc
	$(CXX) stetype_to_string.cc    -c $(CXX_FLAGS) -I$(MET_INC_DIR)

builtin_to_string.o: builtin_to_string.h builtin_to_string.cc
	$(CXX) builtin_to_string.cc    -c $(CXX_FLAGS)

idstack.o: idstack.h idstack.cc
	$(CXX) idstack.cc              -c $(CXX_FLAGS) -I$(MET_INC_DIR)

icode.o: icode.h icode.cc scanner_stuff.h celltype_to_string.o pwl.o builtin.o
	$(CXX) icode.cc                -c $(CXX_FLAGS) -I$(MET_INC_DIR)

symtab.o: symtab.h symtab.cc scanner_stuff.h stetype_to_string.o celltype_to_string.o pwl.o algline.o builtin.o array.o
	$(CXX) symtab.cc               -c $(CXX_FLAGS) -I$(MET_INC_DIR)

array.o: array.h array.cc icode.o
	$(CXX) array.cc                -c $(CXX_FLAGS) -I$(MET_INC_DIR)

builtin.o: builtin.h builtin.cc
	$(CXX) builtin.cc              -c $(CXX_FLAGS)

pwl.o: pwl.h pwl.cc
	$(CXX) pwl.cc                  -c $(CXX_FLAGS) -I$(MET_INC_DIR)

algline.o: algline.h algline.cc
	$(CXX) algline.cc              -c $(CXX_FLAGS)

machine.o: machine.h machine.cc symtab.o icode.o scanner_stuff.h celltype_to_string.o stetype_to_string.o builtin_to_string.o builtin.o
	$(CXX) machine.cc              -c $(CXX_FLAGS) -I$(MET_INC_DIR)

econfig.tab.c: econfig.y scanner_stuff.h icode.o symtab.o machine.o idstack.o builtin.o celltype_to_string.o
	$(YACC) econfig.y

econfig.tab.o: econfig.tab.c
	@ echo
	$(CXX) econfig.tab.c           -c $(CXX_FLAGS) -I$(MET_INC_DIR)

lex.econfig.c: econfig.l econfig.tab.o scanner_stuff.h icode.o symtab.o machine.o
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


libeconfig.a: $(OBJECTS)
	ar -rs libeconfig.a $(OBJECTS)
	ranlib libeconfig.a
	rm -f $(MET_LIB_DIR)/libeconfig.a
	cp libeconfig.a $(MET_LIB_DIR)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)


########################################################################


   ##
   ##  executables
   ##

econfig_codegen: econfig_codegen.cc libeconfig.a
	@ echo
	$(CXX) econfig_codegen.cc -o econfig_codegen $(CXX_FLAGS) -I$(MET_INC_DIR) -L$(MET_LIB_DIR) -leconfig -lvx_cal -lvx_util -lm
	rm -f $(MET_BIN_DIR)/econfig_codegen
	cp econfig_codegen $(MET_BIN_DIR)


########################################################################


clean:
	@ echo
	rm -f *.a *.o temp junk core log a.out out out2 $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	rm -f econfig.output econfig.tab.c econfig.tab.h
	rm -f lex.econfig.c
	rm -f $(MET_BIN_DIR)/econfig_codegen
	rm -f $(MET_LIB_DIR)/libeconfig.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)


########################################################################


