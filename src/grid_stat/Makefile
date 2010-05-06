# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2005
# ** University Corporation for Atmospheric Research(UCAR)
# ** National Center for Atmospheric Research(NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the grid_stat application
#
###############################################################################

CXX         = /usr/bin/g++
CXX_FLAGS   = -static -g
SVNROOT     = /home/johnhg/MET/svn/code
MET_LIB_DIR = $(SVNROOT)/trunk/met/lib

###############################################################################

TARGETS		= grid_stat

OBJECTS		= grid_stat_Conf.o grid_stat_conf_info.o

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
include $(SVNROOT)/build/scripts/rap_make_vx_econfig_macros
include $(SVNROOT)/build/scripts/rap_make_vx_econfig_targets

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
	$(CXX_FLAGS) $(OPT_FLAGS) -DMET_BASE=\"$(MET_BASE)\" \
	-I$(LIB_DIR) $(NETCDF_INCS) $(GSL_INCS) \
	-L$(LIB_DIR) $(NETCDF_LIBS) $(GSL_LIBS) \
	-lvx_met_util -lvx_analysis_util -lvx_wrfdata -lvx_met_util \
	-lvx_contable -lvx_grib_classes \
	-lvx_econfig -lvx_gsl_prob -lgsl \
	-lvx_plot_util  -lvx_render -lvx_pxm -lvx_color -lvx_ps -lvx_afm \
	-lvx_data_grids -lvx_gnomon -lvx_nav -lvx_cal -lvx_util -lvx_math -lm \
	-lnetcdf_c++ -lnetcdf \
	$(CXX_LIBS)
	cp $(TARGETS) ../../bin/.

grid_stat_Conf.o: grid_stat_Conf.cc
	$(CXX) -c grid_stat_Conf.cc $(CXX_FLAGS) $(OPT_FLAGS) \
	-I$(INC_DIR)

grid_stat_conf_info.o: grid_stat_conf_info.cc
	$(CXX) -c grid_stat_conf_info.cc $(CXX_FLAGS) $(OPT_FLAGS) \
	-I$(INC_DIR) $(NETCDF_INCS) $(GSL_INCS)

clean:
	rm -r -f *.a *.o junk temp core log c.ps a.out $(TARGETS) ../../bin/$(TARGETS)

.PHONY: $(FAKES)

