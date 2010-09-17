

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
            nc_output.o                 \


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
	$(CXX) wwmca_regrid.cc $(OBJECTS) $(CXX_FLAGS) -o wwmca_regrid -I$(INC_DIR) $(NETCDF_INCS) -L$(LIB_DIR) $(NETCDF_LIBS) -lnetcdf_c++ -lnetcdf -lvx_econfig -lvx_met_util -lvx_data_grids -lvx_cal -lvx_util -lvx_math -lm
	rm -f $(BIN_DIR)/wwmca_regrid
	cp wwmca_regrid $(BIN_DIR)


wwmca_plot: wwmca_plot.cc $(OBJECTS)
	@ echo
	$(CXX) wwmca_plot.cc afwa_file.o $(CXX_FLAGS) -o wwmca_plot -DMAP_DATA_DIR=\"$(MAP_DATA_DIR)\" -I$(INC_DIR) -L$(LIB_DIR) -lvx_plot_util -lvx_render -lvx_ps -lvx_afm -lvx_pxm -lvx_color -lvx_data_grids -lvx_nav -lvx_cal -lvx_util -lvx_math -lm
	rm -f $(BIN_DIR)/wwmca_plot
	cp wwmca_plot $(BIN_DIR)


########################################################################


   ##
   ##  cleanup
   ##


clean:
	@ echo
	rm -f *.a *.o *.ps *.png *.ppm *.pgm temp junk core log a.out $(OBJECTS) $(TARGETS)
	rm -f $(BIN_DIR)/wwmca_plot
	rm -f $(BIN_DIR)/wwmca_regrid


########################################################################


