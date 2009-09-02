# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the vx_contable library
#
###############################################################################

TARGETS  = libvx_contable.a

OBJECTS  =  vx_contable.o \
	vx_contable_stats.o \
	vx_nx2.o

OPT_FLAGS=

FAKES    = vx_contable_all vx_contable_lib clean

LIB_DIR  = ..


vx_contable_all:
	@ echo
	@ echo "*** Making vx_contable Library ***"
	@ echo
	@ $(MAKE) vx_contable_lib  $(PRINT_OPTS)
	cp $(TARGETS) ../.

vx_contable_lib: libvx_contable.a
	@ echo

libvx_contable.a: $(OBJECTS)
	ar -rs libvx_contable.a $(OBJECTS)
	ranlib libvx_contable.a

vx_contable.o: vx_contable.cc
	$(CXX) vx_contable.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

vx_contable_stats.o: vx_contable_stats.cc
	$(CXX) vx_contable_stats.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(GSL_INCS) $(NETCDF_INCS)

vx_nx2.o: vx_nx2.cc
	$(CXX) vx_nx2.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(GSL_INCS) $(NETCDF_INCS)

clean:
	rm -f *.a *.o temp junk core a.out $(TARGETS) ../$(TARGETS)

.PHONY: $(FAKES)


