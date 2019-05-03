# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the vx_wrfdata library
#
###############################################################################

TARGETS  = libvx_wrfdata.a

OBJECTS  =  filterbox.o \
	interpmthd.o \
	moments.o \
	node.o \
	polyline.o \
	vx_wrfdata.o

OPT_FLAGS=

FAKES    = vx_wrfdata_all vx_wrfdata_lib clean

LIB_DIR  = ..


vx_wrfdata_all:
	@ echo
	@ echo "*** Making vx_wrfdata Library ***"
	@ echo
	@ $(MAKE) vx_wrfdata_lib  $(PRINT_OPTS)
	cp $(TARGETS) ../.

vx_wrfdata_lib: libvx_wrfdata.a
	@ echo

libvx_wrfdata.a: $(OBJECTS)
	ar -rs libvx_wrfdata.a $(OBJECTS)
	ranlib libvx_wrfdata.a

filterbox.o: filterbox.cc
	$(CXX) filterbox.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(NETCDF_INCS)

interpmthd.o: interpmthd.cc
	$(CXX) interpmthd.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(NETCDF_INCS)

moments.o: moments.cc
	$(CXX) moments.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(NETCDF_INCS)

node.o: node.cc
	$(CXX) node.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(NETCDF_INCS)

polyline.o: polyline.cc
	$(CXX) polyline.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(NETCDF_INCS) $(GSL_INCS)

vx_wrfdata.o: vx_wrfdata.cc
	$(CXX) vx_wrfdata.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -I$(LIB_DIR) $(NETCDF_INCS) $(GSL_INCS)

clean:
	rm -f *.a *.o temp junk core a.out $(TARGETS) ../$(TARGETS)

.PHONY: $(FAKES)


