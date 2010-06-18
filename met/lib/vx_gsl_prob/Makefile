# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the vx_gsl_prob library
#
###############################################################################

TARGETS  = libvx_gsl_prob.a

OBJECTS  = gsl_cdf.o \
	gsl_bvn.o \
	gsl_randist.o \
	gsl_statistics.o \
	gsl_wavelet2d.o

OPT_FLAGS=

FAKES    = vx_gsl_prob_all vx_gsl_prob_lib clean

LIB_DIR  = ..


vx_gsl_prob_all:
	@ echo
	@ echo "*** Making vx_gsl_prob Library ***"
	@ echo
	@ $(MAKE) vx_gsl_prob_lib  $(PRINT_OPTS)
	cp $(TARGETS) ../.

vx_gsl_prob_lib: libvx_gsl_prob.a
	@ echo

libvx_gsl_prob.a: $(OBJECTS)
	ar -rs libvx_gsl_prob.a $(OBJECTS)
	ranlib libvx_gsl_prob.a

gsl_cdf.o: gsl_cdf.cc
	$(CXX) gsl_cdf.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(GSL_INCS)

gsl_bvn.o: gsl_bvn.cc
	$(CXX) gsl_bvn.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(GSL_INCS)

gsl_randist.o: gsl_randist.cc
	$(CXX) gsl_randist.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(GSL_INCS)

gsl_statistics.o: gsl_statistics.cc
	$(CXX) gsl_statistics.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(GSL_INCS)

gsl_wavelet2d.o: gsl_wavelet2d.cc
	$(CXX) gsl_wavelet2d.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(GSL_INCS)

clean:
	rm -f *.a *.o temp junk core a.out $(TARGETS) ../$(TARGETS)

.PHONY: $(FAKES)
