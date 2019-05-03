# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the vx_pxm library
#
###############################################################################

TARGETS  = libvx_pxm.a

OBJECTS  =  vx_pbm.o \
	vx_pcm.o \
	vx_pgm.o \
	vx_ppm.o \
	vx_pxm_base.o \
	vx_pxm_utils.o

OPT_FLAGS=

FAKES    = vx_pxm_all vx_pxm_lib clean

LIB_DIR  = ..


vx_pxm_all:
	@ echo
	@ echo "*** Making vx_pxm Library ***"
	@ echo
	@ $(MAKE) vx_pxm_lib  $(PRINT_OPTS)
	cp $(TARGETS) ../.

vx_pxm_lib: libvx_pxm.a
	@ echo

libvx_pxm.a: $(OBJECTS)
	ar -rs libvx_pxm.a $(OBJECTS)
	ranlib libvx_pxm.a

vx_pbm.o: vx_pbm.cc
	$(CXX) vx_pbm.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

vx_pcm.o: vx_pcm.cc
	$(CXX) vx_pcm.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

vx_pgm.o: vx_pgm.cc
	$(CXX) vx_pgm.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

vx_ppm.o: vx_ppm.cc
	$(CXX) vx_ppm.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

vx_pxm_base.o: vx_pxm_base.cc
	$(CXX) vx_pxm_base.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

vx_pxm_utils.o: vx_pxm_utils.cc
	$(CXX) vx_pxm_utils.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

clean:
	rm -f *.a *.o temp junk core a.out $(TARGETS) ../$(TARGETS)

.PHONY: $(FAKES)


