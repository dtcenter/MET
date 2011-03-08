# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the ensemble_stat tool
#
########################################################################


EXECUTABLES = ensemble_stat


OBJECTS     = ensemble_stat_Conf.o      \
              ensemble_stat_conf_info.o \
              ensemble_stat.o


LIBRARIES   =


GEN_SOURCES = ensemble_stat_Conf.h  \
              ensemble_stat_Conf.cc


########################################################################


HEADERS     = ensemble_stat_conf_info.h \
              ensemble_stat.h


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished making ensemble_stat application ***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##

gen_sources: $(GEN_SOURCES)
	@ echo


ensemble_stat_Conf.h ensemble_stat_Conf.cc: $(CONFIG_TEMPLATE_DIR)/WrfModeConfig_default
	@ echo
	@ rm -f ensemble_stat_Conf.h ensemble_stat_Conf.cc
	$(ECONFIG_CODEGEN) -class ensemble_stat_Conf -outfile ensemble_stat_Conf $(CONFIG_TEMPLATE_DIR)/EnsembleStatConfig_default


########################################################################

   ##
   ##  objects
   ##


ensemble_stat_Conf.o: ensemble_stat_Conf.cc
	$(CXX) ensemble_stat_Conf.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)

ensemble_stat_conf_info.o: ensemble_stat_conf_info.cc
	$(CXX) ensemble_stat_conf_info.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)

ensemble_stat.o: ensemble_stat.cc
	$(CXX) ensemble_stat.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) -DMET_BASE=\"$(MET_BASE)\" $(GSL_INCS) $(NETCDF_INCS)


########################################################################


   ##
   ##  libraries
   ##


########################################################################


   ##
   ##  executables
   ##


ensemble_stat: $(OBJECTS)
	$(CXX) -o $(EXECUTABLES) $(OBJECTS) \
	$(CXX_FLAGS) $(OPT_FLAGS) -DMET_BASE=\"$(MET_BASE)\" \
	-I$(MET_LIB_DIR) $(NETCDF_INCS) $(GSL_INCS) \
	-L$(MET_LIB_DIR) $(NETCDF_LIBS) $(GSL_LIBS) \
	-lvx_analysis_util -lvx_met_util -lvx_wrfmode -lvx_render \
	-lvx_plot_util -lvx_pxm -lvx_ps -lvx_afm -lvx_pb_util \
	-lvx_nav -lvx_gnomon -lvx_gdata -lvx_stat_out -lvx_statistics \
	-lvx_gsl_prob -lvx_wrfdata -lvx_grib_classes -lvx_data_grids \
	-leconfig -lvx_util -lvx_math -lvx_cal \
	-lm -lnetcdf_c++ -lnetcdf -lgsl \
	$(CXX_LIBS)
	cp $(EXECUTABLES) $(MET_BIN_DIR)


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_BIN_DIR)/ensemble_stat


########################################################################


.PHONY: all clean


########################################################################


