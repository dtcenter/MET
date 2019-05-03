# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the vx_color library
#
###############################################################################

TARGETS  = libvx_color.a

OBJECTS  =  cfile.tab.o \
	lex.color.o \
	vx_clist.o \
	vx_color.o \
	vx_ctable.o

OPT_FLAGS=

FAKES    = vx_color_all vx_color_lib clean

LIB_DIR  = ..


vx_color_all:
	@ echo
	@ echo "*** Making vx_color Library ***"
	@ echo
	@ $(MAKE) vx_color_lib  $(PRINT_OPTS)
	cp $(TARGETS) ../.

vx_color_lib: libvx_color.a
	@ echo

libvx_color.a: $(OBJECTS)
	ar -rs libvx_color.a $(OBJECTS)
	ranlib libvx_color.a

cfile.tab.o: cfile.tab.cc
	$(CXX) cfile.tab.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

lex.color.o: lex.color.cc
	$(CXX) lex.color.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

vx_clist.o: vx_clist.cc
	$(CXX) vx_clist.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

vx_color.o: vx_color.cc
	$(CXX) vx_color.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

vx_ctable.o: vx_ctable.cc
	$(CXX) vx_ctable.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

clean:
	rm -f *.a *.o temp junk core a.out $(TARGETS) ../$(TARGETS)

.PHONY: $(FAKES)


