

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


COMPILER        = g++

FLAGS           = -g -Wall -Wshadow

INC_DIR         = ../../lib

LIB_DIR         = ../../lib

BIN_DIR         = ../..//bin


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
	$(COMPILER) afwa_file.cc                 -c $(FLAGS) -I$(INC_DIR)


interp_base.o: interp_base.h interp_base.cc
	@ echo
	$(COMPILER) interp_base.cc               -c $(FLAGS) -I$(INC_DIR)


min_interp.o: min_interp.h min_interp.cc interp_base.o
	@ echo
	$(COMPILER) min_interp.cc                -c $(FLAGS) -I$(INC_DIR)


max_interp.o: max_interp.h max_interp.cc interp_base.o
	@ echo
	$(COMPILER) max_interp.cc                -c $(FLAGS) -I$(INC_DIR)


ave_interp.o: ave_interp.h ave_interp.cc interp_base.o
	@ echo
	$(COMPILER) ave_interp.cc                -c $(FLAGS) -I$(INC_DIR)


nearest_interp.o: nearest_interp.h nearest_interp.cc interp_base.o
	@ echo
	$(COMPILER) nearest_interp.cc            -c $(FLAGS) -I$(INC_DIR)


wwmca_config.o: wwmca_config.h wwmca_config.cc
	@ echo
	$(COMPILER) wwmca_config.cc              -c $(FLAGS) -I$(INC_DIR)


gridhemisphere_to_string.o: gridhemisphere_to_string.h gridhemisphere_to_string.cc
	@ echo
	$(COMPILER) gridhemisphere_to_string.cc  -c $(FLAGS) -I$(INC_DIR) $(NETCDF_INCS)


wwmca_ref.o: wwmca_ref.h wwmca_ref.cc gridhemisphere_to_string.o wwmca_config.o
	@ echo
	$(COMPILER) wwmca_ref.cc                 -c $(FLAGS) -I$(INC_DIR) $(NETCDF_INCS)


nc_output.o: nc_output.cc wwmca_ref.o
	@ echo
	$(COMPILER) nc_output.cc                 -c $(FLAGS) -I$(INC_DIR) $(NETCDF_INCS)


########################################################################


   ##
   ##  executables
   ##


wwmca_regrid: wwmca_regrid.cc $(OBJECTS)
	@ echo
	$(COMPILER) wwmca_regrid.cc $(OBJECTS) $(FLAGS) -o wwmca_regrid -I$(INC_DIR) $(NETCDF_INCS) -L$(LIB_DIR) $(NETCDF_LIBS) -lnetcdf_c++ -lnetcdf -lvx_econfig -lvx_met_util -lvx_data_grids -lvx_cal -lvx_util -lvx_math -lm
	rm -f $(BIN_DIR)/wwmca_regrid
	cp wwmca_regrid $(BIN_DIR)


wwmca_plot: wwmca_plot.cc $(OBJECTS)
	@ echo
	$(COMPILER) wwmca_plot.cc afwa_file.o $(FLAGS) -o wwmca_plot -DDATA_DIR=\"$(DATA_DIR)\" -I$(INC_DIR) -L$(LIB_DIR) -lvx_plot_util -lvx_render -lvx_ps -lvx_afm -lvx_pxm -lvx_color -lvx_data_grids -lvx_nav -lvx_cal -lvx_util -lvx_math -lm
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


