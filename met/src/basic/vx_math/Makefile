# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2012
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


OBJECTS     =  angles.o \
               hist.o \
               nint.o \
               nti.o \
               ptile.o \
               pwl.o \
               affine.o \
               viewgravity_to_string.o


LIBRARIES   = libvx_math.a


GEN_SOURCES = viewgravity_to_string.cc \
              viewgravity_to_string.h


########################################################################


HEADERS     =  math_constants.h \
               is_bad_data.h \
               trig.h \
               angles.h \
               hist.h \
               nint.h \
               nti.h \
               ptile.h \
               pwl.h \
               affine.h \
               viewgravity_to_string.h \
               vx_math.h


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_math Library ***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##  call make all since this library is used by source code
   ##  generation tools
   ##


gen_sources: $(GEN_SOURCES)
	@ $(MAKE) all

viewgravity_to_string.cc viewgravity_to_string.h: affine.h
	@ echo
	@ rm -f viewgravity_to_string.cc viewgravity_to_string.h
	$(ENUM_TO_STRING) -concat_string affine.h


########################################################################


   ##
   ##  objects
   ##


angles.o: angles.cc
	$(CXX) angles.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

hist.o: hist.cc
	$(CXX) hist.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

nint.o: nint.cc
	$(CXX) nint.cc $(CXX_FLAGS) -c

nti.o: nti.cc
	$(CXX) nti.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

ptile.o: ptile.cc
	$(CXX) ptile.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

pwl.o: pwl.h pwl.cc
	$(CXX) pwl.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

viewgravity_to_string.o: viewgravity_to_string.h viewgravity_to_string.cc
	$(CXX) viewgravity_to_string.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

affine.o: affine.h affine.cc viewgravity_to_string.cc viewgravity_to_string.h
	$(CXX) affine.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_math.a: $(OBJECTS)
	ar -rs libvx_math.a $(OBJECTS) 2>&1
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
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_math.a


########################################################################


.PHONY: all clean gen_sources


########################################################################


