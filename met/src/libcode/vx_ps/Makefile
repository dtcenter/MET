# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2013
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_ps library
#
########################################################################


EXECUTABLES = test_ps


OBJECTS     = documentmedia_to_string.o \
              documentorientation_to_string.o \
              vx_ps.o \
              ps_text.o


LIBRARIES   = libvx_ps.a


GEN_SOURCES = documentmedia_to_string.h \
              documentmedia_to_string.cc \
              documentorientation_to_string.h \
              documentorientation_to_string.cc


########################################################################


HEADERS     = documentmedia_to_string.h \
              documentorientation_to_string.h \
              vx_ps.h \
              ps_text.h


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_ps Library ***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##


gen_sources: $(GEN_SOURCES)
	@ echo

$(GEN_SOURCES): vx_ps.h
	@ echo
	@ rm -f $(GEN_SOURCES)
	$(ENUM_TO_STRING) -concat_string vx_ps.h


########################################################################


   ##
   ##  objects
   ##


documentmedia_to_string.o: documentmedia_to_string.h documentmedia_to_string.cc
	$(CXX) documentmedia_to_string.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) -DMET_BASE=\"$(MET_BASE_DIR)\"


documentorientation_to_string.o: documentorientation_to_string.h documentorientation_to_string.cc
	$(CXX) documentorientation_to_string.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) -DMET_BASE=\"$(MET_BASE_DIR)\"


vx_ps.o: vx_ps.cc $(GEN_SOURCES)
	$(CXX) vx_ps.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) -DMET_BASE=\"$(MET_BASE_DIR)\"


ps_text.o: ps_text.cc
	$(CXX) ps_text.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_ps.a: $(OBJECTS)
	ar -rs libvx_ps.a $(OBJECTS) 2>&1
	ranlib libvx_ps.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_ps.a
	cp libvx_ps.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


test_ps: test_ps.cc $(LIBRARIES)
	$(CXX) test_ps.cc $(CXX_FLAGS) -o test_ps -I$(MET_INC_DIR) -L$(MET_LIB_DIR) \
	-lvx_ps \
	-lvx_afm \
	-lvx_util \
	-lvx_log \
	-lm

########################################################################


clean:
	rm -f *.a *.o *.ps temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_ps.a


########################################################################


.PHONY: all clean gen_sources


########################################################################

