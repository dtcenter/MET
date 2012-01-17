# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2012
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


OBJECTS     = ensemble_stat_Conf.o \
              ensemble_stat_conf_info.o


LIBRARIES   =


GEN_SOURCES = ensemble_stat_Conf.h \
              ensemble_stat_Conf.cc


########################################################################


HEADERS     =


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the ensemble_stat Tool ***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##


gen_sources: $(GEN_SOURCES)
	@ echo


ensemble_stat_Conf.h ensemble_stat_Conf.cc: $(CONFIG_TEMPLATE_DIR)/EnsembleStatConfig_default
	@ echo
	@ rm -f ensemble_stat_Conf.h ensemble_stat_Conf.cc
	$(ECONFIG_CODEGEN) -class ensemble_stat_Conf -outfile ensemble_stat_Conf -allow_multiple_reads $(CONFIG_TEMPLATE_DIR)/EnsembleStatConfig_default


########################################################################


   ##
   ##  objects
   ##


ensemble_stat_Conf.o: ensemble_stat_Conf.cc
	@ echo
	$(CXX) ensemble_stat_Conf.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)


ensemble_stat_conf_info.o: ensemble_stat_conf_info.cc
	@ echo
	$(CXX) ensemble_stat_conf_info.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)


########################################################################


   ##
   ##  libraries
   ##


########################################################################


   ##
   ##  executables
   ##


ensemble_stat: ensemble_stat.cc $(OBJECTS)
	@ echo
	$(CXX) -o $(EXECUTABLES) ensemble_stat.cc $(OBJECTS) \
	$(CXX_FLAGS) -DMET_BASE=\"$(MET_BASE_DIR)\" \
	-I$(MET_INC_DIR) $(NETCDF_INCS) $(GSL_INCS) \
	-L$(MET_LIB_DIR) $(NETCDF_LIBS) $(GSL_LIBS) \
	-lvx_stat_out \
	-lvx_statistics \
	-lvx_shapedata \
	-lvx_gsl_prob \
	-lvx_analysis_util \
	-lvx_data2d_factory \
	-lvx_data2d_grib \
	-lvx_data2d_nc_met \
	-lvx_data2d_nc_pinterp \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_grid \
	-lvx_math \
	-lvx_cal \
	-lvx_econfig \
	-lvx_util \
	-lvx_color \
	-lvx_log \
	-lm -lnetcdf_c++ -lnetcdf -lgsl \
	$(CXX_LIBS)
	cp $(EXECUTABLES) $(MET_BIN_DIR)


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	rm -f $(MET_BIN_DIR)/ensemble_stat


########################################################################


.PHONY: all clean gen_sources


########################################################################

