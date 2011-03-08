# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
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


OBJECTS     = cfile.tab.o  \
              lex.color.o  \
              vx_clist.o   \
              vx_color.o   \
              vx_ctable.o


LIBRARIES   = libvx_color.a


########################################################################


HEADERS     = cfile.tab.h        \
              vx_clist.h         \
              vx_color.h         \
              vx_color_parser.h


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished making vx_color Library ***"
	@ echo


########################################################################


   ##
   ##  objects
   ##


cfile.tab.o: cfile.tab.cc
	$(CXX) cfile.tab.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)

lex.color.o: lex.color.cc
	$(CXX) lex.color.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)

vx_clist.o: vx_clist.cc
	$(CXX) vx_clist.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)

vx_color.o: vx_color.cc
	$(CXX) vx_color.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS)  -c -I$(MET_INC_DIR)

vx_ctable.o: vx_ctable.cc
	$(CXX) vx_ctable.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS)  -c -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_color.a: $(OBJECTS)
	ar -rs libvx_color.a $(OBJECTS)
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
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_color.a


########################################################################


.PHONY: all clean


########################################################################


