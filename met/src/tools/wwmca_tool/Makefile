# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the wwmca_regrid and wwmca_plot tools
#
########################################################################


EXECUTABLES = wwmca_regrid \
              wwmca_plot


OBJECTS     = afwa_file.o                 \
              interp_base.o               \
              min_interp.o                \
              max_interp.o                \
              ave_interp.o                \
              nearest_interp.o            \
              wwmca_regrid_Conf.o         \
              gridhemisphere_to_string.o  \
              wwmca_ref.o                 \
              nc_output.o


LIBRARIES   =


GEN_SOURCES = gridhemisphere_to_string.h   \
              gridhemisphere_to_string.cc  \
              wwmca_regrid_Conf.h          \
              wwmca_regrid_Conf.cc


########################################################################


HEADERS     = afwa_file.h                 \
              ave_interp.h                \
              gridhemisphere_to_string.h  \
              interp_base.h               \
              max_interp.h                \
              min_interp.h                \
              nearest_interp.h            \
              wwmca_regrid_Conf.          \
              wwmca_grids.h               \
              wwmca_ref.h                 \
              wwmca_utils.h

########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished making wwmca_regrid and wwmca_plot tools ***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##

gen_sources: $(GEN_SOURCES)
	@ echo


gridhemisphere_to_string.h gridhemisphere_to_string.cc: wwmca_ref.h
	@ echo
	@ rm -f gridhemisphere_to_string.h gridhemisphere_to_string.cc
	$(ENUM_TO_STRING) wwmca_ref.h


wwmca_regrid_Conf.h wwmca_regrid_Conf.cc: $(CONFIG_TEMPLATE_DIR)/WWMCAConfig_default
	@ echo
	@ rm -f wwmca_config.h wwmca_config.cc
	$(ECONFIG_CODEGEN) -class wwmca_regrid_Conf -outfile wwmca_regrid_Conf $(CONFIG_TEMPLATE_DIR)/WWMCAConfig_default


########################################################################

   ##
   ##  objects
   ##


afwa_file.o: afwa_file.h afwa_file.cc
	@ echo
	$(CXX) afwa_file.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)


interp_base.o: interp_base.h interp_base.cc
	@ echo
	$(CXX) interp_base.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)


min_interp.o: min_interp.h min_interp.cc interp_base.o
	@ echo
	$(CXX) min_interp.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)


max_interp.o: max_interp.h max_interp.cc interp_base.o
	@ echo
	$(CXX) max_interp.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)


ave_interp.o: ave_interp.h ave_interp.cc interp_base.o
	@ echo
	$(CXX) ave_interp.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)


nearest_interp.o: nearest_interp.h nearest_interp.cc interp_base.o
	@ echo
	$(CXX) nearest_interp.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)


wwmca_regrid_Conf.o: wwmca_regrid_Conf.h wwmca_regrid_Conf.cc
	@ echo
	$(CXX) wwmca_regrid_Conf.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)


gridhemisphere_to_string.o: gridhemisphere_to_string.h gridhemisphere_to_string.cc
	@ echo
	$(CXX) gridhemisphere_to_string.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)


wwmca_ref.o: wwmca_ref.h wwmca_ref.cc gridhemisphere_to_string.o wwmca_regrid_Conf.o
	@ echo
	$(CXX) wwmca_ref.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)


nc_output.o: nc_output.cc wwmca_ref.o
	@ echo
	$(CXX) nc_output.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS) $(GSL_INCS)


########################################################################


   ##
   ##  libraries
   ##


########################################################################


   ##
   ##  executables
   ##


wwmca_regrid: wwmca_regrid.cc $(OBJECTS)
	@ echo
	$(CXX) -o wwmca_regrid wwmca_regrid.cc $(OBJECTS) \
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
	cp wwmca_regrid $(MET_BIN_DIR)


wwmca_plot: wwmca_plot.cc $(OBJECTS)
	@ echo
	$(CXX) -o wwmca_plot wwmca_plot.cc $(OBJECTS) \
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
	cp wwmca_plot $(MET_BIN_DIR)


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_BIN_DIR)/wwmca_regrid
	rm -f $(MET_BIN_DIR)/wwmca_plot


########################################################################


.PHONY: all clean


########################################################################


