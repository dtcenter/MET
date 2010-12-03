# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_gsl_prob library
#
########################################################################


EXECUTABLES =


OBJECTS     = gsl_bvn.o         \
              gsl_cdf.o         \
              gsl_randist.o     \
              gsl_statistics.o  \
              gsl_wavelet2d.o


LIBRARIES   = libvx_gsl_prob.a


########################################################################


HEADERS     = gsl_bvn.h         \
              gsl_cdf.h         \
              gsl_randist.h     \
              gsl_statistics.h  \
              gsl_wavelet2d.h   \
              vx_gsl_prob.h


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)


########################################################################


   ##
   ##  objects
   ##


gsl_bvn.o: gsl_bvn.cc
	$(CXX) gsl_bvn.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

gsl_cdf.o: gsl_cdf.cc
	$(CXX) gsl_cdf.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

gsl_randist.o: gsl_randist.cc
	$(CXX) gsl_randist.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

gsl_statistics.o: gsl_statistics.cc
	$(CXX) gsl_statistics.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

gsl_wavelet2d.o: gsl_wavelet2d.cc
	$(CXX) gsl_wavelet2d.cc $(CXX_FLAGS) $(OPT_FLAGS) -c


########################################################################


   ##
   ##  libraries
   ##


libvx_gsl_prob.a: $(OBJECTS)
	ar -rs libvx_gsl_prob.a $(OBJECTS)
	ranlib libvx_gsl_prob.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_gsl_prob.a
	cp libvx_gsl_prob.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_afm.a


########################################################################


.PHONY: all clean


########################################################################


