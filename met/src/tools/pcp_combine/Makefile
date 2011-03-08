# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research(UCAR)
# ** National Center for Atmospheric Research(NCAR)DERS     =

# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the pcp_combine application
#
########################################################################


EXECUTABLES =  pcp_combine

OBJECTS     =

LIBRARIES   =


########################################################################


HEADERS     =


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making pcp_combine application ***"
	@ echo


########################################################################


	##
	## objects
	##


########################################################################


	##
	## libraries
	##


########################################################################


	##
	## executables
	##

pcp_combine: pcp_combine.cc
	@echo
	$(CXX) -o pcp_combine pcp_combine.cc \
	$(CXX_FLAGS) $(OPT_FLAGS) -DMET_BASE=\"$(MET_BASE)\" \
	-I$(MET_INC_DIR) $(NETCDF_INCS) $(GSL_INCS) \
	-L$(NET_LIB_DIR) $(NETCDF_LIBS) $(GSL_LIBS) \
	-lvx_contable -lvx_met_util -lvx_analysis_util \
	-lvx_gdata -lvx_wrfdata -lvx_contable -lvx_met_util -lvx_grib_classes \
	-lvx_econfig -lvx_gsl_prob -lgsl \
	-lvx_plot_util  -lvx_render -lvx_pxm -lvx_color -lvx_ps -lvx_afm \
	-lvx_data_grids -lvx_gnomon -lvx_nav -lvx_cal -lvx_util -lvx_math -lm \
	-lnetcdf_c++ -lnetcdf \
	$(CXX_LIBS)
	cp pcp_combine $(MET_BIN_DIR)


########################################################################


clean:
	rm -r -f *.a *.o junk temp core log c.ps a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	cd $(MET_BIN_DIR) ; rm -f $(EXECUTABLES)


########################################################################


.PHONY: all clean


########################################################################


