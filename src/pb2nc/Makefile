# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2005
# ** University Corporation for Atmospheric Research(UCAR)
# ** National Center for Atmospheric Research(NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the pb2nc application
#
###############################################################################

CXX         = /usr/bin/g++
CXX_FLAGS   = -Wall -static -g
SVNROOT     = ../../../..
MET_LIB_DIR = $(SVNROOT)/trunk/met/lib

###############################################################################

TARGETS		= pb2nc

OBJECTS		= pb2nc_Conf.o \
		  numpbmsg.o \
		  openpb.o \
		  readpb.o \
		  dumppb.o

OPT_FLAGS	=

FAKES		= all clean objects targets

LIB_DIR		= ../../lib

INC_DIR		= ../../lib

VX_ECONFIG_CLASS = pb2nc
VX_ECONFIG_EXAMPLE_PARM = PB2NCConfig_default
VX_ECONFIG_IS_RAP_LIB = false
VX_ECONFIG_HEADER_SUFFIX = h
VX_ECONFIG_PANIC = true

MODULE_TYPE = progcpp

#
# vx_econfig macros and targets
#
include $(SVNROOT)/build/scripts/rap_make_vx_econfig_macros
include $(SVNROOT)/build/scripts/rap_make_vx_econfig_targets

all:
	@ echo
	@ echo "*** Making pb2nc application ***"
	@ echo
	@ $(MAKE) objects $(PRINT_OPTS)
	@ $(MAKE) targets $(PRINT_OPTS)
	@ echo
	@ echo "*** Finished Making pb2nc application ***"
	@ echo

objects: $(OBJECTS)
	@ echo

targets: $(TARGETS)
	@ echo

pb2nc: pb2nc.cc
	@echo
	$(CXX) -o pb2nc pb2nc.cc $(OBJECTS) \
	$(CXX_FLAGS) $(OPT_FLAGS) -DMET_BASE=\"$(MET_BASE)\" \
	-I$(LIB_DIR) $(NETCDF_INCS) $(GSL_INCS) $(BUFR_INCS) $(F2C_INCS) \
	-L$(LIB_DIR) $(NETCDF_LIBS) $(GSL_LIBS) $(BUFR_LIBS) $(F2C_LIBS) \
	-lbufr -lvx_pb_util \
	-lvx_contable -lvx_met_util -lvx_analysis_util \
	-lvx_wrfdata -lvx_contable -lvx_met_util -lvx_grib_classes \
	-lvx_econfig -lvx_gsl_prob -lgsl \
	-lvx_plot_util  -lvx_render -lvx_pxm -lvx_color -lvx_ps -lvx_afm \
	-lvx_data_grids -lvx_gnomon -lvx_nav -lvx_cal -lvx_util -lvx_math -lm \
	-lnetcdf_c++ -lnetcdf \
	$(F2C_LIBNAME) $(CXX_LIBS) $(FC_LIBS)
	cp $(TARGETS) ../../bin/.

pb2nc_Conf.o: pb2nc_Conf.cc
	$(CXX) -c pb2nc_Conf.cc $(CXX_FLAGS) $(OPT_FLAGS) \
	-I$(INC_DIR)

numpbmsg.o: numpbmsg.f
	$(FC) -c numpbmsg.f $(FC_FLAGS) $(OPT_FLAGS)

openpb.o: openpb.f
	$(FC) -c openpb.f $(FC_FLAGS) $(OPT_FLAGS)

readpb.o: readpb.f
	$(FC) -c readpb.f $(FC_FLAGS) $(OPT_FLAGS)

dumppb.o: dumppb.f
	$(FC) -c dumppb.f $(FC_FLAGS) $(OPT_FLAGS)

clean:
	rm -r -f *.a *.o junk temp core log c.ps a.out $(TARGETS) ../../bin/$(TARGETS)

.PHONY: $(FAKES)
