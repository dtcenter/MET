# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research(UCAR)
# ** National Center for Atmospheric Research(NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_econfig_gen application
#
########################################################################


EXECUTABLES = vx_econfig_gen

OBJECTS     =

LIBRARIES   =


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo


########################################################################


   ##
   ##  objects
   ##


########################################################################


   ##
   ##  libraries
   ##


########################################################################


   ##
   ##  executables
   ##


vx_econfig_gen: vx_econfig_gen.cc
	@echo
	$(CXX) -o $(TARGETS) vx_econfig_gen.cc  \
	$(CXX_FLAGS) $(OPT_FLAGS) \
	-I$(LIB_DIR) -L$(LIB_DIR) -I$(MET_INC_DIR) -L$(MET_LIB_DIR) \
        -lvx_econfig -lvx_cal -lvx_util \
	$(CXX_LIBS)
	cp $(TARGETS) ../../bin/.


########################################################################

TARGETS		= vx_econfig_gen

OBJECTS		=

OPT_FLAGS	=

FAKES		= all clean objects targets

LIB_DIR		= ../../lib

INC_DIR		= ../../lib

all:
	@ echo
	@ echo "*** Making vx_econfig_gen application ***"
	@ echo
	@ $(MAKE) objects $(PRINT_OPTS)
	@ $(MAKE) targets $(PRINT_OPTS)
	@ echo
	@ echo "*** Finished Making vx_econfig_gen application ***"
	@ echo

objects: $(OBJECTS)
	@ echo

targets: $(TARGETS)
	@ echo

vx_econfig_gen: vx_econfig_gen.cc
	@echo
	$(CXX) -o $(TARGETS) vx_econfig_gen.cc $(OBJECTS) \
	$(CXX_FLAGS) $(OPT_FLAGS) \
	-I$(LIB_DIR) -L$(LIB_DIR) -I$(MET_INC_DIR) -L$(MET_LIB_DIR) \
        -lvx_econfig -lvx_cal -lvx_util \
	$(CXX_LIBS)
	cp $(TARGETS) ../../bin/.

clean:
	rm -r -f *.a *.o junk temp core log c.ps a.out $(TARGETS) ../../bin/$(TARGETS)

.PHONY: $(FAKES)


