# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the vx_math library
#
###############################################################################

TARGETS  = libvx_math.a

OBJECTS  =  hist.o \
            nint.o \
            nti.o \
            ptile.o \
            pwl_interpolate.o  \
            affine.o  \
            viewgravity_to_string.o  \


OPT_FLAGS=

FAKES    = vx_math_all vx_math_lib clean

LIB_DIR  = ..


vx_math_all:
	@ echo
	@ echo "*** Making vx_math Library ***"
	@ echo
	@ $(MAKE) vx_math_lib  $(PRINT_OPTS)
	cp $(TARGETS) ../.

vx_math_lib: libvx_math.a
	@ echo

libvx_math.a: $(OBJECTS)
	ar -rs libvx_math.a $(OBJECTS)
	ranlib libvx_math.a

hist.o: hist.cc
	$(CXX) hist.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

nint.o: nint.cc
	$(CXX) nint.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

nti.o: nti.cc
	$(CXX) nti.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

ptile.o: ptile.cc
	$(CXX) ptile.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

pwl_interpolate.o: pwl_interpolate.cc
	$(CXX) pwl_interpolate.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

viewgravity_to_string.o: viewgravity_to_string.h viewgravity_to_string.cc
	$(CXX) viewgravity_to_string.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

affine.o: affine.h affine.cc
	$(CXX) affine.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)





clean:
	rm -f *.a *.o temp junk core a.out $(TARGETS) ../$(TARGETS)

.PHONY: $(FAKES)


