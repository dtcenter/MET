# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research(UCAR)
# ** National Center for Atmospheric Research(NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the gen_poly_mask application
#
###############################################################################


EXECUTABLES = gen_poly_mask

OBJECTS     =

LIBRARIES   =


###############################################################################


HEADERS     =


###############################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making gen_poly_mask application ***"
	@ echo


###############################################################################


   ##
   ## objects
   ##


###############################################################################


   ##
   ## libraries
   ##


###############################################################################


   ##
   ## executables
   ##

gen_poly_mask: gen_poly_mask.cc
	@echo
	$(CXX) -o gen_poly_mask gen_poly_mask.cc \
	$(CXX_FLAGS) $(OPT_FLAGS) -DMET_BASE=\"$(MET_BASE)\" \
	-I$(MET_INC_DIR) $(NETCDF_INCS) $(GSL_INCS) \
	-L$(MET_LIB_DIR) $(NETCDF_LIBS) $(GSL_LIBS) \
	-lvx_met_util -lvx_analysis_util -lvx_gdata -lvx_wrfdata -lvx_met_util \
	-lvx_contable -lvx_grib_classes \
	-lvx_econfig -lvx_gsl_prob -lgsl \
	-lvx_plot_util  -lvx_render -lvx_pxm -lvx_color -lvx_ps -lvx_afm \
	-lvx_data_grids -lvx_gnomon -lvx_nav -lvx_cal -lvx_util -lvx_math -lm \
	-lnetcdf_c++ -lnetcdf \
	$(CXX_LIBS)
	cp gen_poly_mask $(MET_BIN_DIR)


###############################################################################


clean:
   rm -r -f *.a *.o junk temp core log c.ps a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
   cd $(MET_BIN_DIR) ; rm -f $(EXECUTABLES)


###############################################################################


.PHONY: all clean


###############################################################################


