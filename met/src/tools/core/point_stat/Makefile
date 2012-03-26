# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2012
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the point_stat tool
#
########################################################################


EXECUTABLES = point_stat


OBJECTS     = point_stat_Conf.o \
              point_stat_conf_info.o


LIBRARIES   =


GEN_SOURCES = point_stat_Conf.h \
              point_stat_Conf.cc


########################################################################


HEADERS     =


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the point_stat Tool ***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##


gen_sources: $(GEN_SOURCES)
	@ echo


point_stat_Conf.h point_stat_Conf.cc: $(CONFIG_TEMPLATE_DIR)/PointStatConfig_default
	@ echo
	@ rm -f point_stat_Conf.h point_stat_Conf.cc
	$(ECONFIG_CODEGEN) -class point_stat_Conf -outfile point_stat_Conf -allow_multiple_reads $(CONFIG_TEMPLATE_DIR)/PointStatConfig_default


########################################################################


   ##
   ##  objects
   ##


point_stat_Conf.o: point_stat_Conf.cc
	@ echo
	$(CXX) point_stat_Conf.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)


point_stat_conf_info.o: point_stat_conf_info.cc
	@ echo
	$(CXX) point_stat_conf_info.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)


########################################################################


   ##
   ##  libraries
   ##


########################################################################


   ##
   ##  executables
   ##


point_stat: point_stat.cc $(OBJECTS)
	$(CXX) -o $(EXECUTABLES) point_stat.cc $(OBJECTS) \
	$(CXX_FLAGS) $(OPT_FLAGS) -DMET_BASE=\"$(MET_BASE_DIR)\" \
	-I$(MET_INC_DIR) $(NETCDF_INCS) $(GSL_INCS) $(GRIB2C_INCS) \
	-L$(MET_LIB_DIR) $(NETCDF_LIBS) $(GSL_LIBS) $(GRIB2C_LIBS) \
	-lvx_stat_out \
	-lvx_statistics \
	-lvx_shapedata \
	-lvx_gsl_prob \
	-lvx_analysis_util \
	-lvx_data2d_factory \
	-lvx_data2d_grib $(OPT_LIBS) $(GRIB2_LIBS) \
	-lvx_data2d_nc_met \
	-lvx_data2d_nc_pinterp \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_grid \
	-lvx_cal \
	-lvx_econfig \
	-lvx_math \
	-lvx_util \
	-lvx_log \
	-lvx_color \
	-lm -lnetcdf_c++ -lnetcdf -lgsl -lgslcblas \
	$(CXX_LIBS)
	cp $(EXECUTABLES) $(MET_BIN_DIR)


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES) $(GEN_SOURCES)
	rm -f $(MET_BIN_DIR)/point_stat


########################################################################


.PHONY: all clean gen_sources


########################################################################

