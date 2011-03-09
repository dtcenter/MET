# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the stat_analysis tool
#
########################################################################


EXECUTABLES = stat_analysis


OBJECTS     = stat_analysis_Conf.o \
              aggr_stat_line.o     \
              parse_stat_line.o    \
              stat_analysis_job.o  \
              stat_analysis.o


LIBRARIES   =


GEN_SOURCES = stat_analysis_Conf.h  \
              stat_analysis_Conf.cc


########################################################################


HEADERS     = stat_analysis_Conf.h \
              aggr_stat_line.h     \
              parse_stat_line.h    \
              stat_analysis_job.h  \
              stat_analysis.h


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished making stat_analysis tool ***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##

gen_sources: $(GEN_SOURCES)
	@ echo


stat_analysis_Conf.h stat_analysis_Conf.cc: $(CONFIG_TEMPLATE_DIR)/STATAnalysisConfig_default
	@ echo
	@ rm -f stat_analysis_Conf.h stat_analysis_Conf.cc
	$(ECONFIG_CODEGEN) -class stat_analysis_Conf -outfile stat_analysis_Conf $(CONFIG_TEMPLATE_DIR)/STATAnalysisConfig_default


########################################################################

   ##
   ##  objects
   ##


stat_analysis_Conf.o: stat_analysis_Conf.cc
	$(CXX) stat_analysis_Conf.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)

aggr_stat_line.o: aggr_stat_line.cc
	$(CXX) aggr_stat_line.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)

parse_stat_line.o: parse_stat_line.cc
	$(CXX) parse_stat_line.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)

stat_analysis_job.o: stat_analysis_job.cc
	$(CXX) stat_analysis_job.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)

stat_analysis.o: stat_analysis.cc
	$(CXX) stat_analysis.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)


########################################################################


   ##
   ##  libraries
   ##


########################################################################


   ##
   ##  executables
   ##


stat_analysis: $(OBJECTS)
	$(CXX) -o $(EXECUTABLES) $(OBJECTS) \
	$(CXX_FLAGS) $(OPT_FLAGS) \
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
	rm -f $(MET_BIN_DIR)/stat_analysis


########################################################################


.PHONY: all clean


########################################################################


