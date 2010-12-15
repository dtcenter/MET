# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_math library
#
########################################################################


EXECUTABLES =


OBJECTS     =  hist.o \
               nint.o \
               nti.o \
               ptile.o \
               pwl_interpolate.o \
               affine.o \
               viewgravity_to_string.o \


LIBRARIES   = libvx_math.a


GEN_SOURCES = viewgravity_to_string.cc  \
              viewgravity_to_string.h   \


########################################################################


HEADERS     =  constants.h \
               is_bad_data.h \
               trig.h \
               hist.h \
               nint.h \
               nti.h \
               ptile.h \
               pwl_interpolate.h \
               affine.h \
               viewgravity_to_string.h \
               vx_math.h


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished making vx_math Library ***"
	@ echo


########################################################################


gen_sources: $(GEN_SOURCES)
	@ echo

viewgravity_to_string.cc viewgravity_to_string.h: affine.h
	@ echo
	@ rm -f viewgravity_to_string.cc viewgravity_to_string.h
	$(ENUM_TO_STRING) affine.h


########################################################################


   ##
   ##  objects
   ##


hist.o: hist.cc
	$(CXX) hist.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

nint.o: nint.cc
	$(CXX) nint.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

nti.o: nti.cc
	$(CXX) nti.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

ptile.o: ptile.cc
	$(CXX) ptile.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

pwl_interpolate.o: pwl_interpolate.cc
	$(CXX) pwl_interpolate.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

viewgravity_to_string.o: viewgravity_to_string.h viewgravity_to_string.cc
	$(CXX) viewgravity_to_string.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

affine.o: affine.h affine.cc viewgravity_to_string.cc viewgravity_to_string.h
	$(CXX) affine.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_math.a: $(OBJECTS)
	ar -rs libvx_math.a $(OBJECTS)
	ranlib libvx_math.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_math.a
	cp libvx_math.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_math.a
	rm -f viewgravity_to_string.cc viewgravity_to_string.h


########################################################################


.PHONY: all clean


########################################################################


