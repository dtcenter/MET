# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the mode_analysis tool
#
########################################################################


EXECUTABLES = mode_analysis


OBJECTS     = mode_analysis_Conf.o      \
              config_to_att.o \
              mode_analysis.o


LIBRARIES   =


GEN_SOURCES = mode_analysis_Conf.h  \
              mode_analysis_Conf.cc


########################################################################


HEADERS     = mode_analysis_Conf.h \
              config_to_att.h


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished making mode_analysis tool ***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##

gen_sources: $(GEN_SOURCES)
	@ echo


mode_analysis_Conf.h mode_analysis_Conf.cc: $(CONFIG_TEMPLATE_DIR)/WrfModeConfig_default
	@ echo
	@ rm -f mode_analysis_Conf.h mode_analysis_Conf.cc
	$(ECONFIG_CODEGEN) -class mode_analysis_Conf -outfile mode_analysis_Conf $(CONFIG_TEMPLATE_DIR)/MODEAnalysisConfig_default


########################################################################

   ##
   ##  objects
   ##


mode_analysis_Conf.o: mode_analysis_Conf.cc
	$(CXX) mode_analysis_Conf.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

config_to_att.o: config_to_att.cc
	$(CXX) config_to_att.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

mode_analysis.o: mode_analysis.cc
	$(CXX) mode_analysis.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) -DMET_BASE=\"$(MET_BASE_DIR)\" $(GSL_INCS) $(NETCDF_INCS)


########################################################################


   ##
   ##  libraries
   ##


########################################################################


   ##
   ##  executables
   ##


mode_analysis: $(OBJECTS)
	$(CXX) -o $(EXECUTABLES) $(OBJECTS) \
	$(CXX_FLAGS) $(OPT_FLAGS) -DMET_BASE=\"$(MET_BASE_DIR)\" \
	-I$(MET_INC_DIR) $(NETCDF_INCS) $(GSL_INCS) \
	-L$(MET_LIB_DIR) $(NETCDF_LIBS) $(GSL_LIBS) \
	-lvx_analysis_util -lvx_met_util -lvx_wrfmode -lvx_render \
	-lvx_plot_util -lvx_pxm -lvx_color -lvx_ps -lvx_afm -lvx_pb_util \
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
	rm -f $(MET_BIN_DIR)/mode_analysis


########################################################################


.PHONY: all clean


########################################################################


