# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research(UCAR)
# ** National Center for Atmospheric Research(NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the grid_stat application
#
########################################################################

CXX         = /usr/bin/g++
CXX_FLAGS   = -Wall -Wshadow -static -g
SVNROOT     = ../../../..
MET_LIB_DIR = $(SVNROOT)/code/trunk/met/lib

########################################################################

TARGETS		= grid_stat

OBJECTS		= grid_stat_Conf.o \
		  grid_stat_conf_info.o

OPT_FLAGS	=

FAKES		= all clean objects targets

LIB_DIR		= ../../lib

INC_DIR		= ../../lib

VX_ECONFIG_CLASS = grid_stat
VX_ECONFIG_EXAMPLE_PARM = GridStatConfig_default
VX_ECONFIG_IS_RAP_LIB = false
VX_ECONFIG_HEADER_SUFFIX = h
VX_ECONFIG_PANIC = true

MODULE_TYPE = progcpp

#
# vx_econfig macros and targets
#
include $(SVNROOT)/build/make_include/rap_make_vx_econfig_macros

all:
	@ echo
	@ echo "*** Making grid_stat application ***"
	@ echo
	@ $(MAKE) objects $(PRINT_OPTS)
	@ $(MAKE) targets $(PRINT_OPTS)
	@ echo
	@ echo "*** Finished Making grid_stat application ***"
	@ echo

objects: $(OBJECTS)
	@ echo

targets: $(TARGETS)
	@ echo

grid_stat: grid_stat.cc
	@echo
	$(CXX) -o $(TARGETS) grid_stat.cc $(OBJECTS) \
	$(CXX_FLAGS) $(OPT_FLAGS) -DMET_BASE=\"$(MET_BASE_DIR)\" \
	-I$(LIB_DIR) $(NETCDF_INCS) $(GSL_INCS) \
	-L$(LIB_DIR) $(NETCDF_LIBS) $(GSL_LIBS) \
	-lvx_analysis_util -lvx_met_util -lvx_wrfmode -lvx_render \
	-lvx_plot_util -lvx_pxm -lvx_ps -lvx_afm -lvx_pb_util \
	-lvx_nav -lvx_gnomon -lvx_gdata -lvx_stat_out -lvx_statistics \
	-lvx_gsl_prob -lvx_wrfdata -lvx_grib_classes -lvx_data_grids \
	-lvx_util -lvx_math -lvx_cal \
	-lm -lnetcdf_c++ -lnetcdf -lgsl \
	$(CXX_LIBS)
	cp $(EXECUTABLES) $(MET_BIN_DIR)

grid_stat_Conf.o: grid_stat_Conf.cc
	$(CXX) -c grid_stat_Conf.cc $(CXX_FLAGS) $(OPT_FLAGS) \
	-I$(INC_DIR)

grid_stat_conf_info.o: grid_stat_conf_info.cc
	$(CXX) -c grid_stat_conf_info.cc $(CXX_FLAGS) $(OPT_FLAGS) \
	-I$(INC_DIR) $(NETCDF_INCS) $(GSL_INCS)

include $(SVNROOT)/build/make_include/rap_make_vx_econfig_targets

clean:
	rm -r -f *.a *.o junk temp core log c.ps a.out $(TARGETS) ../../bin/$(TARGETS)

.PHONY: $(FAKES)


