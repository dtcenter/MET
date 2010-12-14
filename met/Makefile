# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2005
# ** University Corporation for Atmospheric Research(UCAR)
# ** National Center for Atmospheric Research(NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the Model Evaluation Tools Project
# for use with the GNU Compilers
#
###############################################################################

include user_defs.mk
include met_defs.mk
include export.mk

###############################################################################

all:
	@ echo
	@ echo "*** Making the Model Evaluation Tools Project ***"
	@ echo
	@ cd src ; $(MAKE) $(PRINT_OPTS)
	@ echo
	@ echo "*** Finished Making the Model Evaluation Tools Project ***"
	@ echo


###############################################################################

libs:
	@ cd $(VX_AFM);           $(MAKE) $(PRINT_OPTS)
	@ cd $(VX_ANALYSIS_UTIL); $(MAKE) $(PRINT_OPTS)
	@ cd $(VX_CAL);           $(MAKE) $(PRINT_OPTS)
	@ cd $(VX_COLOR);         $(MAKE) $(PRINT_OPTS)
	@ cd $(VX_CONTABLE);      $(MAKE) $(PRINT_OPTS)
	@ cd $(VX_DATA_GRIDS);    $(MAKE) $(PRINT_OPTS)
	@ cd $(VX_ECONFIG);       $(MAKE) $(PRINT_OPTS)
	@ cd $(VX_GDATA);	  $(MAKE) $(PRINT_OPTS)
	@ cd $(VX_GNOMON);        $(MAKE) $(PRINT_OPTS)
	@ cd $(VX_GRIB_CLASSES);  $(MAKE) $(PRINT_OPTS)
	@ cd $(VX_GSL_PROB);      $(MAKE) $(PRINT_OPTS)
	@ cd $(VX_MATH);          $(MAKE) $(PRINT_OPTS)
	@ cd $(VX_MET_UTIL);      $(MAKE) $(PRINT_OPTS)
	@ cd $(VX_NAV);           $(MAKE) $(PRINT_OPTS)
	@ cd $(VX_PLOT_UTIL);     $(MAKE) $(PRINT_OPTS)
	@ cd $(VX_PS);            $(MAKE) $(PRINT_OPTS)
	@ cd $(VX_PXM);           $(MAKE) $(PRINT_OPTS)
	@ cd $(VX_RENDER);        $(MAKE) $(PRINT_OPTS)
	@ cd $(VX_UTIL);          $(MAKE) $(PRINT_OPTS)
	@ cd $(VX_PB_UTIL);       $(MAKE) $(PRINT_OPTS)
	@ cd $(VX_WRFDATA);       $(MAKE) $(PRINT_OPTS)
	@ cd $(VX_WRFMODE);       $(MAKE) $(PRINT_OPTS)

targets:
ifeq ($(DISABLE_PCP_COMBINE), 0)
	@ cd $(PCP_COMBINE);	 $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(DISABLE_GEN_POLY_MASK), 0)
	@ cd $(GEN_POLY_MASK);	 $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(DISABLE_MODE), 0)
	@ cd $(MODE);		 $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(DISABLE_GRID_STAT), 0)
	@ cd $(GRID_STAT);	 $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(DISABLE_PB2NC), 0)
	@ cd $(PB2NC);		 $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(DISABLE_ASCII2NC), 0)
	@ cd $(ASCII2NC);	 $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(DISABLE_POINT_STAT), 0)
	@ cd $(POINT_STAT);	 $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(DISABLE_WAVELET_STAT), 0)
	@ cd $(WAVELET_STAT);	 $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(DISABLE_ENSEMBLE_STAT), 0)
	@ cd $(ENSEMBLE_STAT);	 $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(DISABLE_STAT_ANALYSIS), 0)
	@ cd $(STAT_ANALYSIS);	 $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(DISABLE_MODE_ANALYSIS), 0)
	@ cd $(MODE_ANALYSIS);	 $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(DISABLE_TOOLS), 0)
	@ cd $(TOOLS_DIR);	 $(MAKE) $(PRINT_OPTS)
endif

clean:
	cd src ; $(MAKE) clean $(PRINT_OPTS)
	rm -r -f *.a *.o junk temp core log c.ps a.out
	rm -f include/*.h

########################################################################

.PHONY: all clean libs targets gen_sources


########################################################################
