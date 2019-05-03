# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the vx_afm library
#
###############################################################################

TARGETS  = libvx_afm.a

OBJECTS  =  afm.o \
	afm_keywords.o \
	afm_line.o \
	afm_token.o \
	afmkeyword_to_string.o \
	afmtokentype_to_string.o

OPT_FLAGS=

FAKES    = vx_afm_all vx_afm_lib clean

LIB_DIR  = ..

vx_afm_all:
	@ echo
	@ echo "*** Making vx_afm Library ***"
	@ echo
	@ $(MAKE) vx_afm_lib  $(PRINT_OPTS)
	cp $(TARGETS) ../.

vx_afm_lib: libvx_afm.a
	@ echo

libvx_afm.a: $(OBJECTS)
	ar -rs libvx_afm.a $(OBJECTS)
	ranlib libvx_afm.a

afm.o: afm.cc
	$(CXX) afm.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

afm_keywords.o: afm_keywords.cc
	$(CXX) afm_keywords.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

afm_line.o: afm_line.cc
	$(CXX) afm_line.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

afm_token.o: afm_token.cc
	$(CXX) afm_token.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

afmkeyword_to_string.o: afmkeyword_to_string.cc
	$(CXX) afmkeyword_to_string.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

afmtokentype_to_string.o: afmtokentype_to_string.cc
	$(CXX) afmtokentype_to_string.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

clean:
	rm -f *.a *.o temp junk core a.out $(TARGETS) ../$(TARGETS)

.PHONY: $(FAKES)


