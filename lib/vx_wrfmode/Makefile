# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the vx_wrfmode library
#
###############################################################################

CXX         = /usr/bin/g++
CXX_FLAGS   = -Wall -static -g
SVNROOT     = ../../../..
MET_LIB_DIR = $(SVNROOT)/trunk/met/lib

###############################################################################

TARGETS  = libvx_wrfmode.a

OBJECTS  =  WrfMode_Conf.o \
	engine.o \
	interest.o \
	set.o

OPT_FLAGS=

FAKES    = vx_wrfmode_all vx_wrfmode_lib clean

LIB_DIR  = ..

VX_ECONFIG_LIBNAME = vx_wrfmode
VX_ECONFIG_CLASS = WrfMode
VX_ECONFIG_EXAMPLE_PARM = WrfModeConfig_default
VX_ECONFIG_IS_RAP_LIB = false
VX_ECONFIG_HEADER_SUFFIX = h
VX_ECONFIG_PANIC = true

TARGET_FILE = ../libvx_wrfmode.a

MODULE_TYPE=library

#
# vx_econfig macros and targets
#
include $(SVNROOT)/build/make_include/rap_make_vx_econfig_macros
include $(SVNROOT)/build/make_include/rap_make_vx_econfig_targets

vx_wrfmode_all:
	@ echo
	@ echo "*** Making vx_wrfmode Library ***"
	@ echo
	@ $(MAKE) vx_wrfmode_lib  $(PRINT_OPTS)
	cp $(TARGETS) ../.

vx_wrfmode_lib: libvx_wrfmode.a
	@ echo

libvx_wrfmode.a: $(OBJECTS)
	ar -rs libvx_wrfmode.a $(OBJECTS)
	ranlib libvx_wrfmode.a

WrfMode_Conf.o: WrfMode_Conf.cc
	$(CXX) WrfMode_Conf.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -DMET_BASE=\"$(MET_BASE)\" -I$(LIB_DIR) $(NETCDF_INCS)

engine.o: engine.cc
	$(CXX) engine.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -DMET_BASE=\"$(MET_BASE)\" -I$(LIB_DIR) $(NETCDF_INCS) $(GSL_INCS)

interest.o: interest.cc
	$(CXX) interest.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(NETCDF_INCS) $(GSL_INCS)

set.o: set.cc
	$(CXX) set.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(NETCDF_INCS)

clean:
	rm -f *.a *.o temp junk core a.out $(TARGETS) ../$(TARGETS)

.PHONY: $(FAKES)


