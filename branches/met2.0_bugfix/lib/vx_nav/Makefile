# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the vx_nav library
#
###############################################################################

TARGETS  = libvx_nav.a

OBJECTS  =  nav.o

OPT_FLAGS=

FAKES    = vx_nav_all vx_nav_lib clean

LIB_DIR  = ..


vx_nav_all:
	@ echo
	@ echo "*** Making vx_nav Library ***"
	@ echo
	@ $(MAKE) vx_nav_lib  $(PRINT_OPTS)
	cp $(TARGETS) ../.

vx_nav_lib: libvx_nav.a
	@ echo

libvx_nav.a: $(OBJECTS)
	ar -rs libvx_nav.a $(OBJECTS)
	ranlib libvx_nav.a

nav.o: nav.cc
	$(CXX) nav.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

clean:
	rm -f *.a *.o temp junk core a.out $(TARGETS) ../$(TARGETS)

.PHONY: $(FAKES)


