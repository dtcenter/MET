# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2012
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_shapedata library
#
########################################################################


EXECUTABLES =


OBJECTS     = moments.o   \
              node.o      \
              polyline.o  \
              shapedata.o \
              set.o       \
              interest.o  \
              mode_Conf.o \
              engine.o    \

LIBRARIES   = libvx_shapedata.a


GEN_SOURCES = mode_Conf.h \
              mode_Conf.cc


########################################################################


HEADERS     = moments.h \
              node.h \
              polyline.h \
              shape.h \
              shapedata.h \
              set.h \
              interest.h \
              mode_columns.h \
              engine.h \
              mode_Conf.h \
              vx_shapedata.h


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_shapedata Library ***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##


gen_sources: $(GEN_SOURCES)
	@ echo

mode_Conf.h mode_Conf.cc: $(CONFIG_TEMPLATE_DIR)/MODEConfig_default
	@ echo
	@ rm -f mode_Conf.h mode_Conf.cc
	$(ECONFIG_CODEGEN) -class mode_Conf -outfile mode_Conf -allow_multiple_reads $(CONFIG_TEMPLATE_DIR)/MODEConfig_default


########################################################################


   ##
   ##  objects
   ##


moments.o: moments.h moments.cc
	$(CXX) moments.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

node.o: node.h node.cc
	$(CXX) node.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

polyline.o: polyline.h polyline.cc
	$(CXX) polyline.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

shapedata.o: shapedata.h shapedata.cc
	$(CXX) shapedata.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) -I.

set.o: set.h set.cc
	$(CXX) set.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

interest.o: interest.h interest.cc shapedata.o
	$(CXX) interest.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

mode_Conf.o: mode_Conf.h mode_Conf.cc
	$(CXX) mode_Conf.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

engine.o: engine.h engine.cc
	$(CXX) engine.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -DMET_BASE=\"$(MET_BASE_DIR)\" -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_shapedata.a: $(OBJECTS)
	ar -rs libvx_shapedata.a $(OBJECTS) 2>&1
	ranlib libvx_shapedata.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_shapedata.a
	cp libvx_shapedata.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_shapedata.a


########################################################################


.PHONY: all clean gen_sources


########################################################################

