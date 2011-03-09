# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the wavelet_stat tool
#
########################################################################


EXECUTABLES = wavelet_stat


OBJECTS     = wavelet_stat_Conf.o      \
              wavelet_stat_conf_info.o \
              wavelet_stat.o


LIBRARIES   =


GEN_SOURCES = wavelet_stat_Conf.h  \
              wavelet_stat_Conf.cc


########################################################################


HEADERS     =


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished making wavelet_stat tool ***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##


gen_sources: $(GEN_SOURCES)
	@ echo


wavelet_stat_Conf.h wavelet_stat_Conf.cc: $(CONFIG_TEMPLATE_DIR)/WaveletStatConfig_default
	@ echo
	@ rm -f wavelet_stat_Conf.h wavelet_stat_Conf.cc
	$(ECONFIG_CODEGEN) -class wavelet_stat_Conf -outfile wavelet_stat_Conf $(CONFIG_TEMPLATE_DIR)/WaveletStatConfig_default


########################################################################

   ##
   ##  objects
   ##


wavelet_stat_Conf.o: wavelet_stat_Conf.cc
	@ echo
	$(CXX) wavelet_stat_Conf.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)


wavelet_stat_conf_info.o: wavelet_stat_conf_info.cc
	@ echo
	$(CXX) wavelet_stat_conf_info.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)


wavelet_stat.o: wavelet_stat.cc
	@ echo
	$(CXX) wavelet_stat.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) -DMET_BASE=\"$(MET_BASE_DIR)\" $(GSL_INCS) $(NETCDF_INCS)


########################################################################


   ##
   ##  libraries
   ##


########################################################################


   ##
   ##  executables
   ##


wavelet_stat: $(OBJECTS)
	@ echo
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
	rm -f $(MET_BIN_DIR)/wavelet_stat


########################################################################


.PHONY: all clean


########################################################################


