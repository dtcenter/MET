# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the vx_plot_util library
#
###############################################################################

TARGETS  = libvx_plot_util.a

OBJECTS  =  vx_plot_util.o

OPT_FLAGS=

FAKES    = vx_plot_util_all vx_plot_util_lib clean

LIB_DIR  = ..


vx_plot_util_all:
	@ echo
	@ echo "*** Making vx_plot_util Library ***"
	@ echo
	@ $(MAKE) vx_plot_util_lib  $(PRINT_OPTS)
	cp $(TARGETS) ../.

vx_plot_util_lib: libvx_plot_util.a
	@ echo

libvx_plot_util.a: $(OBJECTS)
	ar -rs libvx_plot_util.a $(OBJECTS)
	ranlib libvx_plot_util.a

vx_plot_util.o: vx_plot_util.cc
	$(CXX) vx_plot_util.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -DMET_BASE=\"$(MET_BASE)\" -I$(LIB_DIR)

clean:
	rm -f *.a *.o temp junk core a.out $(TARGETS) ../$(TARGETS)

.PHONY: $(FAKES)


