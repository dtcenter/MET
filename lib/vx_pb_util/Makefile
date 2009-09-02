# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the vx_pb_util library
#
###############################################################################

TARGETS  = libvx_pb_util.a

OBJECTS  = pblock.o \
	do_blocking.o \
	do_unblocking.o \
	copy_bytes.o

OPT_FLAGS=

FAKES    = vx_pb_util_all vx_pb_util_lib clean

LIB_DIR  = ..


vx_pb_util_all:
	@ echo
	@ echo "*** Making vx_pb_util Library ***"
	@ echo
	@ $(MAKE) vx_pb_util_lib  $(PRINT_OPTS)
	cp $(TARGETS) ../.

vx_pb_util_lib: libvx_pb_util.a
	@ echo

libvx_pb_util.a: $(OBJECTS)
	ar -rs libvx_pb_util.a $(OBJECTS)
	ranlib libvx_pb_util.a

pblock.o: pblock.cc
	$(CXX) pblock.cc $(ARCH_FLAGS) $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

do_blocking.o: do_blocking.cc
	$(CXX) do_blocking.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

do_unblocking.o: do_unblocking.cc
	$(CXX) do_unblocking.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

copy_bytes.o: copy_bytes.cc
	$(CXX) copy_bytes.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

clean:
	rm -f *.a *.o temp junk core a.out $(TARGETS) ../$(TARGETS)

.PHONY: $(FAKES)
