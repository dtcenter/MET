# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the vx_grib_classes library
#
###############################################################################

TARGETS  = libvx_grib_classes.a

OBJECTS  =  grib_classes.o \
	grib_strings.o

OPT_FLAGS=

FAKES    = vx_grib_classes_all vx_grib_classes_lib clean

LIB_DIR  = ..


vx_grib_classes_all:
	@ echo
	@ echo "*** Making vx_grib_classes Library ***"
	@ echo
	@ $(MAKE) vx_grib_classes_lib  $(PRINT_OPTS)
	cp $(TARGETS) ../.

vx_grib_classes_lib: libvx_grib_classes.a
	@ echo

libvx_grib_classes.a: $(OBJECTS)
	ar -rs libvx_grib_classes.a $(OBJECTS)
	ranlib libvx_grib_classes.a

grib_classes.o: grib_classes.cc
	$(CXX) grib_classes.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -I$(LIB_DIR)

grib_strings.o: grib_strings.cc
	$(CXX) grib_strings.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

clean:
	rm -f *.a *.o temp junk core a.out $(TARGETS) ../$(TARGETS)

.PHONY: $(FAKES)


