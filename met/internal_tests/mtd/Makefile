

########################################################################


EXECUTABLES = test_velocity


OBJECTS = 


########################################################################


COMPILER = g++


FLAGS = -g -Wall -Wshadow


########################################################################


MTD_SOURCE_DIR = ../../src/tools/other/mode_time_domain


MTD_OBJECTS = $(MTD_SOURCE_DIR)/mtd-2d_att.o  \
              $(MTD_SOURCE_DIR)/mtd-2d_moments.o  \
              $(MTD_SOURCE_DIR)/mtd-3d_att.o  \
              $(MTD_SOURCE_DIR)/mtd-3d_att_pair_array.o  \
              $(MTD_SOURCE_DIR)/mtd-3d_att_single_array.o  \
              $(MTD_SOURCE_DIR)/mtd-3d_conv.o  \
              $(MTD_SOURCE_DIR)/mtd-3d_moments.o  \
              $(MTD_SOURCE_DIR)/mtd-interest_calc.o  \
              $(MTD_SOURCE_DIR)/mtd-mtd_config_info.o  \
              $(MTD_SOURCE_DIR)/mtd-mtd_file_base.o  \
              $(MTD_SOURCE_DIR)/mtd-mtd_file_float.o  \
              $(MTD_SOURCE_DIR)/mtd-mtd_file_int.o  \
              $(MTD_SOURCE_DIR)/mtd-mtdfiletype_to_string.o  \
              $(MTD_SOURCE_DIR)/mtd-mtd_partition.o  \
              $(MTD_SOURCE_DIR)/mtd-mtd_txt_output.o  \
              $(MTD_SOURCE_DIR)/mtd-nc_grid.o  \
              $(MTD_SOURCE_DIR)/mtd-nc_utils.o  \


TEST_INCS = -I.   \
            -I../..    \
            -I../../src/basic/vx_cal   \
            -I../../src/basic/vx_config   \
            -I../../src/basic/vx_log   \
            -I../../src/basic/vx_math   \
            -I../../src/basic/vx_util   \
            -I../../src/libcode/vx_afm   \
            -I../../src/libcode/vx_analysis_util   \
            -I../../src/libcode/vx_color   \
            -I../../src/libcode/vx_data2d   \
            -I../../src/libcode/vx_data2d_factory   \
            -I../../src/libcode/vx_data2d_grib   \
            -I../../src/libcode/vx_data2d_grib2   \
            -I../../src/libcode/vx_data2d_nccf   \
            -I../../src/libcode/vx_data2d_nc_met   \
            -I../../src/libcode/vx_data2d_nc_pinterp   \
            -I../../src/libcode/vx_geodesy   \
            -I../../src/libcode/vx_gnomon   \
            -I../../src/libcode/vx_grid   \
            -I../../src/libcode/vx_gsl_prob   \
            -I../../src/libcode/vx_nav   \
            -I../../src/libcode/vx_nc_util   \
            -I../../src/libcode/vx_pb_util   \
            -I../../src/libcode/vx_plot_util   \
            -I../../src/libcode/vx_ps   \
            -I../../src/libcode/vx_pxm   \
            -I../../src/libcode/vx_render   \
            -I../../src/libcode/vx_regrid   \
            -I../../src/libcode/vx_shapedata   \
            -I../../src/libcode/vx_statistics   \
            -I../../src/libcode/vx_stat_out   \
            -I../../src/libcode/vx_time_series   \
            -I../../src/libcode/vx_tc_util   \
            -I/d2/bullock/otherlibs/netcdf/netcdf-3.6.3/include   \
            -I/d2/bullock/otherlibs/grib2c/g2clib-1.4.0/include   \
            -I/d2/bullock/otherlibs/gsl/gsl-1.15/include   \
            -I/d2/bullock/otherlibs/hdf/HDF4.2r3/include   \
            -I/d2/bullock/otherlibs/hdf_eos/hdfeos/include   \
            -I/usr/include/cairo   \
            -I/usr/include/freetype2  \


BIG_LS = -L.   \
         -L../../src/basic/vx_cal   \
         -L../../src/basic/vx_config   \
         -L../../src/basic/vx_log   \
         -L../../src/basic/vx_math   \
         -L../../src/basic/vx_util   \
         -L../../src/libcode/vx_afm   \
         -L../../src/libcode/vx_analysis_util   \
         -L../../src/libcode/vx_color   \
         -L../../src/libcode/vx_data2d   \
         -L../../src/libcode/vx_data2d_factory   \
         -L../../src/libcode/vx_data2d_grib   \
         -L../../src/libcode/vx_data2d_grib2   \
         -L../../src/libcode/vx_data2d_nccf   \
         -L../../src/libcode/vx_data2d_nc_met   \
         -L../../src/libcode/vx_data2d_nc_pinterp   \
         -L../../src/libcode/vx_geodesy   \
         -L../../src/libcode/vx_gnomon   \
         -L../../src/libcode/vx_grid   \
         -L../../src/libcode/vx_gsl_prob   \
         -L../../src/libcode/vx_nav   \
         -L../../src/libcode/vx_nc_util   \
         -L../../src/libcode/vx_pb_util   \
         -L../../src/libcode/vx_plot_util   \
         -L../../src/libcode/vx_ps   \
         -L../../src/libcode/vx_pxm   \
         -L../../src/libcode/vx_render   \
         -L../../src/libcode/vx_regrid   \
         -L../../src/libcode/vx_shapedata   \
         -L../../src/libcode/vx_statistics   \
         -L../../src/libcode/vx_stat_out   \
         -L../../src/libcode/vx_time_series   \
         -L../../src/libcode/vx_tc_util   \
         -L/d2/bullock/otherlibs/netcdf/netcdf-3.6.3/lib   \
         -L/d2/bullock/otherlibs/grib2c/g2clib-1.4.0/lib   \
         -L/d2/bullock/otherlibs/gsl/gsl-1.15/lib   \
         -L/d2/bullock/bufrlib/bufrlib   \
         -L/d2/bullock/otherlibs/hdf/HDF4.2r3/lib   \
         -L/d2/bullock/otherlibs/hdf_eos/hdfeos/lib   \
         -L/usr/lib   \


LITTLE_LS = -lvx_pxm \
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
     -lvx_data2d_nc_met \
     -lvx_data2d_grib \
     -lvx_data2d_grib2 \
     -lvx_data2d_nc_pinterp \
     -lvx_data2d_nccf \
     -lvx_data2d \
     -lvx_nc_util \
     -lvx_regrid \
     -lvx_grid \
     -lvx_config \
     -lvx_cal \
     -lvx_util \
     -lvx_math \
     -lvx_color \
     -lvx_log \
     -lm -lnetcdf_c++ -lnetcdf -lgsl -lgslcblas -lz


########################################################################


all: $(OBJECTS) $(EXECUTABLES)
	@ echo


########################################################################


   ##
   ##  objects
   ##


########################################################################


   ##
   ##  executables
   ##


test_velocity: test_velocity.cc $(MTD_OBJECTS)
	@ echo
	$(COMPILER) test_velocity.cc $(MTD_OBJECTS) -o test_velocity -I$(MTD_SOURCE_DIR) -I$(TEST_INCS) $(FLAGS) $(BIG_LS) $(LITTLE_LS)


########################################################################


clean:
	rm -f *.o *.a temp junk core log a.out $(OBJECTS) $(EXECUTABLES)
	rm *.nc 


########################################################################


