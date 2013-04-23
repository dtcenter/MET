# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2013
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_color library
#
########################################################################


EXECUTABLES =


OBJECTS     = cfile.tab.o \
              lex.color.o \
              color.o \
              color_list.o \
              color_table.o


LIBRARIES   = libvx_color.a


GEN_SOURCES =


########################################################################


HEADERS     = cfile.tab.h \
              color.h \
              color_list.h \
              color_parser.h \
              vx_color.h


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_color Library ***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##


gen_sources: $(GEN_SOURCES)
	@ echo


########################################################################


   ##
   ##  objects
   ##


cfile.tab.o: cfile.tab.cc
	$(CXX) cfile.tab.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) > /dev/null 2>&1

lex.color.o: lex.color.cc
	$(CXX) lex.color.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) > /dev/null 2>&1

color.o: color.cc
	$(CXX) color.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

color_list.o: color_list.cc
	$(CXX) color_list.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

color_table.o: color_table.cc
	$(CXX) color_table.cc $(CXX_FLAGS)  -c -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_color.a: $(OBJECTS)
	ar -rs libvx_color.a $(OBJECTS) 2>&1
	ranlib libvx_color.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_color.a
	cp libvx_color.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_color.a


########################################################################


.PHONY: all clean gen_sources


########################################################################

