# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2013
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


OBJECTS     = afwa_file.o \
              afwa_cp_file.o \
              afwa_pt_file.o \
              interp_base.o \
              min_interp.o \
              max_interp.o \
              ave_interp.o \
              nearest_interp.o \
              gridhemisphere_to_string.o \
              wwmca_ref.o \
              nc_output.o


LIBRARIES   =


GEN_SOURCES = gridhemisphere_to_string.h \
              gridhemisphere_to_string.cc


########################################################################


HEADERS     =


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the wwmca_regrid and wwmca_plot Tools ***"
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


########################################################################


   ##
   ##  objects
   ##


afwa_file.o: afwa_file.h afwa_file.cc
	@ echo
	$(CXX) afwa_file.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)


afwa_cp_file.o: afwa_cp_file.h afwa_cp_file.cc
	@ echo
	$(CXX) afwa_cp_file.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)


afwa_pt_file.o: afwa_pt_file.h afwa_pt_file.cc
	@ echo
	$(CXX) afwa_pt_file.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)


interp_base.o: interp_base.h interp_base.cc
	@ echo
	$(CXX) interp_base.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)


min_interp.o: min_interp.h min_interp.cc interp_base.o
	@ echo
	$(CXX) min_interp.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)


max_interp.o: max_interp.h max_interp.cc interp_base.o
	@ echo
	$(CXX) max_interp.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)


ave_interp.o: ave_interp.h ave_interp.cc interp_base.o
	@ echo
	$(CXX) ave_interp.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)


nearest_interp.o: nearest_interp.h nearest_interp.cc interp_base.o
	@ echo
	$(CXX) nearest_interp.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)


gridhemisphere_to_string.o: gridhemisphere_to_string.h gridhemisphere_to_string.cc
	@ echo
	$(CXX) gridhemisphere_to_string.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)


wwmca_ref.o: wwmca_ref.h wwmca_ref.cc gridhemisphere_to_string.o
	@ echo
	$(CXX) wwmca_ref.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)


nc_output.o: nc_output.cc wwmca_ref.o
	@ echo
	$(CXX) nc_output.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS) $(GSL_INCS)


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
	$(CXX_FLAGS) -DMET_BASE=\"$(MET_BASE_DIR)\" \
	-I$(MET_INC_DIR) $(NETCDF_INCS) $(GSL_INCS) $(GRIB2C_INCS) \
	-L$(MET_LIB_DIR) $(NETCDF_LIBS) $(GSL_LIBS) $(GRIB2C_LIBS) \
	-lvx_pxm \
	-lvx_plot_util \
	-lvx_nav \
	-lvx_render \
	-lvx_ps \
	-lvx_afm \
	-lvx_stat_out \
	-lvx_statistics \
	-lvx_shapedata \
	-lvx_gsl_prob \
	-lvx_analysis_util \
	-lvx_data2d_factory \
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_data2d_nc_met \
	-lvx_data2d_nc_pinterp \
	-lvx_data2d_nccf \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_grid \
	-lvx_config \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	-lm -lnetcdf_c++ -lnetcdf -lgsl -lgslcblas \
	$(CXX_LIBS)
	cp wwmca_regrid $(MET_BIN_DIR)


wwmca_plot: wwmca_plot.cc $(OBJECTS)
	@ echo
	$(CXX) -o wwmca_plot wwmca_plot.cc $(OBJECTS) \
	$(CXX_FLAGS) \
	-I$(MET_INC_DIR) $(NETCDF_INCS) $(GSL_INCS) $(GRIB2C_INCS) \
	-L$(MET_LIB_DIR) $(NETCDF_LIBS) $(GSL_LIBS) $(GRIB2C_LIBS) \
	-lvx_pxm \
	-lvx_plot_util \
	-lvx_nav \
	-lvx_render \
	-lvx_ps \
	-lvx_render \
	-lvx_afm \
	-lvx_render \
	-lvx_stat_out \
	-lvx_statistics \
	-lvx_shapedata \
	-lvx_gsl_prob \
	-lvx_analysis_util \
	-lvx_data2d_factory \
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_data2d_nc_met \
	-lvx_data2d_nc_pinterp \
	-lvx_data2d_nccf \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_grid \
	-lvx_config \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	-lm -lz -lnetcdf_c++ -lnetcdf -lgsl -lgslcblas \
	$(CXX_LIBS)
	cp wwmca_plot $(MET_BIN_DIR)


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	rm -f $(MET_BIN_DIR)/wwmca_regrid
	rm -f $(MET_BIN_DIR)/wwmca_plot


########################################################################


.PHONY: all clean gen_sources


########################################################################

