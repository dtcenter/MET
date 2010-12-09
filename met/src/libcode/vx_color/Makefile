# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
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
              vx_ctable.o  \


LIBRARIES   = libvx_color.a


########################################################################


HEADERS     = cfile.tab.h        \
              vx_clist.h         \
              vx_color.h         \
              vx_color_parser.h  \


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo


########################################################################


   ##
   ##  objects
   ##


cfile.tab.o: cfile.tab.cc
	$(CXX) cfile.tab.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

lex.color.o: lex.color.cc
	$(CXX) lex.color.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

vx_clist.o: vx_clist.cc
	$(CXX) vx_clist.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

vx_color.o: vx_color.cc
	$(CXX) vx_color.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

vx_ctable.o: vx_ctable.cc
	$(CXX) vx_ctable.cc $(CXX_FLAGS) $(OPT_FLAGS) -c


########################################################################


   ##
   ##  libraries
   ##

libvx_color.a: $(OBJECTS)
	ar -rs libvx_color.a $(OBJECTS)
	ranlib libvx_color.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_color.o
	cp libvx_color.o $(MET_LIB_DIR)


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


