# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the vx_econfig library
#
###############################################################################

TARGETS  = libvx_econfig.a

OBJECTS  =  algline.o \
	array.o \
	builtin.o \
	builtin_to_string.o \
	celltype_to_string.o \
	econfig.o \
	econfigl.o \
	gen.o \
	icode.o \
	icodecell_to_result.o \
	idstack.o \
	machine.o \
	pwl.o \
	result.o \
	resulttype_to_string.o \
	stetype_to_string.o \
	symtab.o

OPT_FLAGS=

FAKES    = vx_econfig_all vx_econfig_lib clean

LIB_DIR  = ..

vx_econfig_all:
	@ echo
	@ echo "*** Making vx_econfig Library ***"
	@ echo
	@ $(MAKE) vx_econfig_lib  $(PRINT_OPTS)
	cp $(TARGETS) ../.

vx_econfig_lib: libvx_econfig.a
	@ echo

libvx_econfig.a: $(OBJECTS)
	ar -rs libvx_econfig.a $(OBJECTS)
	ranlib libvx_econfig.a

algline.o: algline.cc
	$(CXX) algline.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

array.o: array.cc
	$(CXX) array.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

builtin.o: builtin.cc
	$(CXX) builtin.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

builtin_to_string.o: builtin_to_string.cc
	$(CXX) builtin_to_string.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

celltype_to_string.o: celltype_to_string.cc
	$(CXX) celltype_to_string.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

econfig.o: econfig.cc
	$(CXX) econfig.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

econfigl.o: econfigl.cc
	$(CXX) econfigl.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

gen.o: gen.cc
	$(CXX) gen.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)
	
icode.o: icode.cc
	$(CXX) icode.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

icodecell_to_result.o: icodecell_to_result.cc
	$(CXX) icodecell_to_result.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

idstack.o: idstack.cc
	$(CXX) idstack.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

machine.o: machine.cc
	$(CXX) machine.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

pwl.o: pwl.cc
	$(CXX) pwl.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

result.o: result.cc
	$(CXX) result.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

resulttype_to_string.o: resulttype_to_string.cc
	$(CXX) resulttype_to_string.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

stetype_to_string.o: stetype_to_string.cc
	$(CXX) stetype_to_string.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

symtab.o: symtab.cc
	$(CXX) symtab.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

clean:
	rm -f *.a *.o temp junk core a.out $(TARGETS) ../$(TARGETS)

.PHONY: $(FAKES)


