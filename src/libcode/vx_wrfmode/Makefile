# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_wrfmode library
#
########################################################################


EXECUTABLES =


OBJECTS     = WrfMode_Conf.o \
              engine.o       \
              interest.o     \
              set.o


LIBRARIES   = libvx_wrfmode.a


GEN_SOURCES = WrfMode_Conf.h  \
              WrfMode_Conf.cc


########################################################################


HEADERS = engine.h        \
          interest.h      \
          mode_columns.h  \
          set.h           \
          vx_wrfmode.h    \
          WrfMode_Conf.h


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished making vx_wrfmode Library ***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##

gen_sources: $(GEN_SOURCES)
	@ echo


WrfMode_Conf.h WrfMode_Conf.cc: $(CONFIG_TEMPLATE_DIR)/WrfModeConfig_default
	@ echo
	@ rm -f WrfMode_Conf.h WrfMode_Conf.cc
	$(ECONFIG_CODEGEN) -class WrfMode_Conf -outfile WrfMode_Conf $(CONFIG_TEMPLATE_DIR)/WrfModeConfig_default


########################################################################


   ##
   ##  objects
   ##

objects: $(OBJECTS)
	@ echo

WrfMode_Conf.o: WrfMode_Conf.h WrfMode_Conf.cc
	$(CXX) WrfMode_Conf.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)

engine.o: engine.cc
	$(CXX) engine.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -DMET_BASE=\"$(MET_BASE)\" -I$(MET_INC_DIR)

interest.o: interest.cc
	$(CXX) interest.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)

set.o: set.cc
	$(CXX) set.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_wrfmode.a: $(OBJECTS)
	ar -rs libvx_wrfmode.a $(OBJECTS)
	ranlib libvx_wrfmode.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_wrfmode.a
	cp libvx_wrfmode.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_wrfmode.a


########################################################################


.PHONY: all clean objects gen_sources


########################################################################


