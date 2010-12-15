# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_econfig library
#
########################################################################


EXECUTABLES =


OBJECTS     = algline.o               \
              array.o                 \
              builtin.o               \
              builtin_to_string.o     \
              celltype_to_string.o    \
              econfig.o               \
              econfigl.o              \
              gen.o                   \
              icode.o                 \
              icodecell_to_result.o   \
              idstack.o               \
              machine.o               \
              pwl.o                   \
              result.o                \
              resulttype_to_string.o  \
              stetype_to_string.o     \
              symtab.o                \


LIBRARIES   = libvx_econfig.a


########################################################################


HEADERS     = algline.h               \
              array.h                 \
              builtin.h               \
              builtin_to_string.h     \
              celltype_to_string.h    \
              econfig.h               \
              gen.h                   \
              icodecell_to_result.h   \
              icode.h                 \
              idstack.h               \
              machine.h               \
              pwl.h                   \
              result.h                \
              resulttype_to_string.h  \
              scanner_stuff.h         \
              stetype_to_string.h     \
              symtab.h                \


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo


########################################################################


   ##
   ##  objects
   ##

algline.o: algline.cc
	$(CXX) algline.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

array.o: array.cc
	$(CXX) array.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

builtin.o: builtin.cc
	$(CXX) builtin.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

builtin_to_string.o: builtin_to_string.cc
	$(CXX) builtin_to_string.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

celltype_to_string.o: celltype_to_string.cc
	$(CXX) celltype_to_string.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

econfig.o: econfig.cc
	$(CXX) econfig.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

econfigl.o: econfigl.cc
	$(CXX) econfigl.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

gen.o: gen.cc
	$(CXX) gen.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

icode.o: icode.cc
	$(CXX) icode.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

icodecell_to_result.o: icodecell_to_result.cc
	$(CXX) icodecell_to_result.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

idstack.o: idstack.cc
	$(CXX) idstack.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

machine.o: machine.cc
	$(CXX) machine.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

pwl.o: pwl.cc
	$(CXX) pwl.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

result.o: result.cc
	$(CXX) result.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

resulttype_to_string.o: resulttype_to_string.cc
	$(CXX) resulttype_to_string.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

stetype_to_string.o: stetype_to_string.cc
	$(CXX) stetype_to_string.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

symtab.o: symtab.cc
	$(CXX) symtab.cc $(CXX_FLAGS) $(OPT_FLAGS) -c


########################################################################


   ##
   ##  libraries
   ##


libvx_econfig.a: $(OBJECTS)
	ar -rs libvx_econfig.a $(OBJECTS)
	ranlib libvx_econfig.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_econfig.a
	cp libvx_econfig.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_econfig.a


########################################################################


.PHONY: all clean


########################################################################

