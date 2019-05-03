# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2013
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the Tropical Cyclone tc_stat tool
#
########################################################################


EXECUTABLES = tc_stat


OBJECTS     = tc_stat_conf_info.o \
              tc_stat_files.o \
              tc_stat_job.o

LIBRARIES   =


GEN_SOURCES =


########################################################################


HEADERS     = tc_stat_conf_info.h \
              tc_stat_files.h \
              tc_stat_job.h


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the tc_stat Tool ***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##


gen_sources: $(GEN_SOURCES)
	@ echo


########################################################################


   ##
   ##  objects
   ##


tc_stat_conf_info.o: tc_stat_conf_info.cc
	@ echo
	$(CXX) tc_stat_conf_info.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)


tc_stat_files.o: tc_stat_files.cc
	@ echo
	$(CXX) tc_stat_files.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)


tc_stat_job.o: tc_stat_job.cc
	@ echo
	$(CXX) tc_stat_job.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)


########################################################################


   ##
   ##  libraries
   ##


########################################################################


   ##
   ##  executables
   ##


tc_stat: tc_stat.cc $(OBJECTS)
	@ echo
	$(CXX) -o $(EXECUTABLES) tc_stat.cc $(OBJECTS) \
	$(CXX_FLAGS) -DMET_BASE=\"$(MET_BASE_DIR)\" \
	-I$(MET_INC_DIR) $(NETCDF_INCS) $(GSL_INCS) $(F2C_INCS) \
	-L$(MET_LIB_DIR) $(NETCDF_LIBS) $(GSL_LIBS) $(F2C_LIBS) \
	-lvx_config \
	-lvx_data2d_nc_met \
	-lvx_analysis_util \
	-lvx_tc_util \
	-lvx_nc_util \
	-lvx_statistics \
	-lvx_gsl_prob \
	-lvx_grid \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm \
	-lnetcdf_c++ -lnetcdf -lgsl -lgslcblas \
	$(F2C_LIBNAME) $(CXX_LIBS) $(FC_LIBS)
	cp $(EXECUTABLES) $(MET_BIN_DIR)


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	rm -f $(MET_BIN_DIR)/tc_stat


########################################################################


.PHONY: all clean gen_sources


########################################################################
