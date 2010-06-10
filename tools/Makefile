# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2005
# ** University Corporation for Atmospheric Research(UCAR)
# ** National Center for Atmospheric Research(NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the tools directory
#
###############################################################################

# Optional flags to disable the compilation of MET tools
# Specify a non-zero value to disable the compilation of the tool
DISABLE_PLOT_POINT_OBS = 0

###############################################################################

PLOT_POINT_OBS	= plot_point_obs

###############################################################################

all:
	@ echo
	@ echo "*** Making tools directory ***"
	@ echo
	@ $(MAKE) targets $(PRINT_OPTS)
	@ echo
	@ echo "*** Finished Making tools directory ***"
	@ echo

targets:
ifeq ($(DISABLE_PLOT_POINT_OBS), 0)
	@ cd $(PLOT_POINT_OBS);	 $(MAKE) $(PRINT_OPTS)
endif

clean:
	@ cd $(PLOT_POINT_OBS);	 $(MAKE) clean $(PRINT_OPTS)

.PHONY: $(FAKES)
