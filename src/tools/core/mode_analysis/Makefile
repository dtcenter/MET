# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2012
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


OBJECTS     = mode_analysis_Conf.o \
              config_to_att.o


LIBRARIES   =


GEN_SOURCES = mode_analysis_Conf.h \
              mode_analysis_Conf.cc


########################################################################


HEADERS     =


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the mode_analysis Tool ***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##


gen_sources: $(GEN_SOURCES)
	@ echo


mode_analysis_Conf.h mode_analysis_Conf.cc: $(CONFIG_TEMPLATE_DIR)/MODEAnalysisConfig_build
	@ echo
	@ rm -f mode_analysis_Conf.h mode_analysis_Conf.cc
	$(ECONFIG_CODEGEN) -class mode_analysis_Conf -outfile mode_analysis_Conf -nopanic -allow_multiple_reads $(CONFIG_TEMPLATE_DIR)/MODEAnalysisConfig_build


########################################################################


   ##
   ##  objects
   ##


mode_analysis_Conf.o: mode_analysis_Conf.cc
	@ echo
	$(CXX) mode_analysis_Conf.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)


config_to_att.o: config_to_att.cc
	@ echo
	$(CXX) config_to_att.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)


########################################################################


   ##
   ##  libraries
   ##


########################################################################


   ##
   ##  executables
   ##


mode_analysis: mode_analysis.cc $(OBJECTS)
	@ echo
	$(CXX) -o $(EXECUTABLES) mode_analysis.cc $(OBJECTS) \
	$(CXX_FLAGS) -DMET_BASE=\"$(MET_BASE_DIR)\" \
	-I$(MET_INC_DIR) $(NETCDF_INCS) $(GSL_INCS) \
	-L$(MET_LIB_DIR) $(NETCDF_LIBS) $(GSL_LIBS) \
	-lvx_shapedata \
	-lvx_analysis_util \
	-lvx_data2d_factory \
	-lvx_data2d_grib \
	-lvx_data2d_nc_met \
	-lvx_data2d_nc_pinterp \
	-lvx_nc_util \
	-lvx_data2d \
	-lvx_render \
	-lvx_plot_util \
	-lvx_pxm \
	-lvx_color \
	-lvx_ps \
	-lvx_afm \
	-lvx_pb_util \
	-lvx_nav \
	-lvx_gnomon \
	-lvx_stat_out \
	-lvx_statistics \
	-lvx_gsl_prob \
	-lvx_econfig \
	-lvx_grid \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm -lnetcdf_c++ -lnetcdf -lgsl \
	$(CXX_LIBS)
	cp $(EXECUTABLES) $(MET_BIN_DIR)


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES) $(GEN_SOURCES)
	rm -f $(MET_BIN_DIR)/mode_analysis


########################################################################


.PHONY: all clean gen_sources


########################################################################

