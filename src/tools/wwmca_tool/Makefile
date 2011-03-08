

########################################################################


TARGETS   = wwmca_regrid wwmca_plot


OBJECTS   = afwa_file.o                 \
            interp_base.o               \
            min_interp.o                \
            max_interp.o                \
            ave_interp.o                \
            nearest_interp.o            \
            wwmca_config.o              \
            gridhemisphere_to_string.o  \
            wwmca_ref.o                 \
            nc_output.o


GEN_SOURCES = gridhemisphere_to_string.h   \
              gridhemisphere_to_string.cc  \
              wwmca_config.h               \
              wwmca_config.cc


########################################################################


INC_DIR         = ../../lib

LIB_DIR         = ../../lib

BIN_DIR         = ../../bin

MAP_DATA_DIR    = $(MET_BASE)/data/map


########################################################################


   ##
   ##  the whole enchilada
   ##


all: $(OBJECTS) $(TARGETS)
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


wwmca_config.h wwmca_config.cc: $(CONFIG_TEMPLATE_DIR)/wwmca_config_template
	@ echo
	@ rm -f wwmca_config.h wwmca_config.cc
	$(ECONFIG_CODEGEN) wwmca_ref.h -class WwmcaConfig -outfile wwmca_config $(CONFIG_TEMPLATE_DIR)/wwmca_config_template


########################################################################


   ##
   ##  objects
   ##


afwa_file.o: afwa_file.h afwa_file.cc
	@ echo
	$(CXX) afwa_file.cc                 -c $(CXX_FLAGS) -I$(INC_DIR)


interp_base.o: interp_base.h interp_base.cc
	@ echo
	$(CXX) interp_base.cc               -c $(CXX_FLAGS) -I$(INC_DIR)


min_interp.o: min_interp.h min_interp.cc interp_base.o
	@ echo
	$(CXX) min_interp.cc                -c $(CXX_FLAGS) -I$(INC_DIR)


max_interp.o: max_interp.h max_interp.cc interp_base.o
	@ echo
	$(CXX) max_interp.cc                -c $(CXX_FLAGS) -I$(INC_DIR)


ave_interp.o: ave_interp.h ave_interp.cc interp_base.o
	@ echo
	$(CXX) ave_interp.cc                -c $(CXX_FLAGS) -I$(INC_DIR)


nearest_interp.o: nearest_interp.h nearest_interp.cc interp_base.o
	@ echo
	$(CXX) nearest_interp.cc            -c $(CXX_FLAGS) -I$(INC_DIR)


wwmca_config.o: wwmca_config.h wwmca_config.cc
	@ echo
	$(CXX) wwmca_config.cc              -c $(CXX_FLAGS) -I$(INC_DIR)


gridhemisphere_to_string.o: gridhemisphere_to_string.h gridhemisphere_to_string.cc
	@ echo
	$(CXX) gridhemisphere_to_string.cc  -c $(CXX_FLAGS) -I$(INC_DIR) $(NETCDF_INCS)


wwmca_ref.o: wwmca_ref.h wwmca_ref.cc gridhemisphere_to_string.o wwmca_config.o
	@ echo
	$(CXX) wwmca_ref.cc                 -c $(CXX_FLAGS) -I$(INC_DIR) $(NETCDF_INCS)


nc_output.o: nc_output.cc wwmca_ref.o
	@ echo
	$(CXX) nc_output.cc                 -c $(CXX_FLAGS) -I$(INC_DIR) $(NETCDF_INCS) $(GSL_INCS)


########################################################################


   ##
   ##  executables
   ##


wwmca_regrid: wwmca_regrid.cc $(OBJECTS)
	@ echo
	$(CXX) -o wwmca_regrid wwmca_regrid.cc $(OBJECTS) \
	$(CXX_FLAGS) $(OPT_FLAGS) \
	-I$(LIB_DIR) $(NETCDF_INCS) $(GSL_INCS) \
	-L$(LIB_DIR) $(NETCDF_LIBS) $(GSL_LIBS) \
	-lvx_analysis_util -lvx_met_util -lvx_wrfmode -lvx_render \
	-lvx_plot_util -lvx_pxm -lvx_ps -lvx_afm -lvx_pb_util \
	-lvx_nav -lvx_gnomon -lvx_gdata -lvx_stat_out -lvx_statistics \
	-lvx_gsl_prob -lvx_wrfdata -lvx_grib_classes -lvx_data_grids \
	-lvx_util -lvx_math -lvx_cal \
	-lm -lnetcdf_c++ -lnetcdf -lgsl \
	$(CXX_LIBS)
	cp wwmca_regrid $(MET_BIN_DIR)


wwmca_plot: wwmca_plot.cc $(OBJECTS)
	@ echo
	$(CXX) -o wwmca_plot wwmca_plot.cc afwa_file.o \
	$(CXX_FLAGS) $(OPT_FLAGS) -DMAP_DATA_DIR=\"$(MAP_DATA_DIR)\" \
	-I$(LIB_DIR) $(NETCDF_INCS) $(GSL_INCS) \
	-L$(LIB_DIR) $(NETCDF_LIBS) $(GSL_LIBS) \
	-lvx_analysis_util -lvx_met_util -lvx_wrfmode -lvx_render \
	-lvx_plot_util -lvx_pxm -lvx_ps -lvx_afm -lvx_pb_util \
	-lvx_nav -lvx_gnomon -lvx_gdata -lvx_stat_out -lvx_statistics \
	-lvx_gsl_prob -lvx_wrfdata -lvx_grib_classes -lvx_data_grids \
	-lvx_util -lvx_math -lvx_cal \
	-lm -lnetcdf_c++ -lnetcdf -lgsl \
	$(CXX_LIBS)
	cp wwmca_plot $(MET_BIN_DIR)


########################################################################


   ##
   ##  cleanup
   ##


clean:
	@ echo
	rm -f *.a *.o *.ps *.png *.ppm *.pgm temp junk core log a.out $(OBJECTS) $(TARGETS) $(GEN_SOURCES)
	rm -f $(BIN_DIR)/wwmca_plot
	rm -f $(BIN_DIR)/wwmca_regrid


########################################################################


