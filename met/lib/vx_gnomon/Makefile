# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the vx_gnomon library
#
###############################################################################

TARGETS  = libvx_gnomon.a

OBJECTS  =  gnomon.o

OPT_FLAGS=

FAKES    = vx_gnomon_all vx_gnomon_lib clean

LIB_DIR  = ..

vx_gnomon_all:
	@ echo
	@ echo "*** Making vx_gnomon Library ***"
	@ echo
	@ $(MAKE) vx_gnomon_lib  $(PRINT_OPTS)
	cp $(TARGETS) ../.

vx_gnomon_lib: libvx_gnomon.a
	@ echo

libvx_gnomon.a: $(OBJECTS)
	ar -rs libvx_gnomon.a $(OBJECTS)
	ranlib libvx_gnomon.a

gnomon.o: gnomon.cc
	$(CXX) gnomon.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

clean:
	rm -f *.a *.o temp junk core a.out $(TARGETS) ../$(TARGETS)

.PHONY: $(FAKES)
