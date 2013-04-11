# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2012
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the modis_regrid tool
#
########################################################################


#HDF_BASE_DIR     = $(OTHERLIBS_DIR)/hdf/HDF4.2r3
#HDF_EOS_BASE_DIR = $(OTHERLIBS_DIR)/hdf_eos/hdfeos


EXECUTABLES = modis_regrid \


OBJECTS     = sat_utils.o             \
              cloudsat_swath_file.o   \
              data_plane_to_netcdf.o  \
              data_averager.o         \
              modis_file.o            \



LIBRARIES   =


GEN_SOURCES =


########################################################################


HEADERS     = cloudsat_swath_file.h   \
              data_averager.h         \
              modis_file.h            \
              sat_utils.h             \
              data_averager.h         \
              data_plane_to_netcdf.h  \


########################################################################


HDF_INC_DIR      = $(HDF_BASE_DIR)/include

HDF_LIB_DIR      = $(HDF_BASE_DIR)/lib

HDF_EOS_INC_DIR  = $(HDF_EOS_BASE_DIR)/include

HDF_EOS_LIB_DIR  = $(HDF_EOS_BASE_DIR)/lib


########################################################################


HDF_INCS = -I$(HDF_INC_DIR) -I$(HDF_EOS_INC_DIR)


########################################################################


MET_LITTLE_Ls    = -lvx_pxm \
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
                   -lvx_data2d_grib -lvx_data2d_grib2 -lgrib2c -ljasper -lpng -lz \
                   -lvx_data2d_nc_pinterp \
                   -lvx_data2d \
                   -lvx_nc_util \
                   -lvx_grid \
                   -lvx_config \
                   -lvx_cal \
                   -lvx_util \
                   -lvx_math \
                   -lvx_color \
                   -lvx_log \
                   -lm -lnetcdf_c++ -lnetcdf -lgsl -lgslcblas


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the modis_regrid Tool ***"
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


data_plane_to_netcdf.o: data_plane_to_netcdf.h data_plane_to_netcdf.cc
	@ echo
	$(CXX) data_plane_to_netcdf.cc $(FLAGS)   -c -I$(MET_INC_DIR)


data_averager.o: data_averager.h data_averager.cc
	@ echo
	$(CXX) data_averager.cc $(FLAGS)          -c -I$(MET_INC_DIR)


sat_utils.o: sat_utils.h sat_utils.cc
	@ echo
	$(CXX) sat_utils.cc $(FLAGS)              -c -I$(MET_INC_DIR) $(HDF_INCS)


cloudsat_swath_file.o: cloudsat_swath_file.h cloudsat_swath_file.cc
	@ echo
	$(CXX) cloudsat_swath_file.cc $(FLAGS)    -c -I$(MET_INC_DIR) $(HDF_INCS)


modis_file.o: modis_file.h modis_file.cc
	@ echo
	$(CXX) modis_file.cc $(FLAGS)             -c -I$(MET_INC_DIR) -I$(HDF_INC_DIR) -I$(HDF_EOS_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


########################################################################


   ##
   ##  executables
   ##


modis_regrid: modis_regrid.cc $(OBJECTS)
	@ echo
	$(CXX) modis_regrid.cc modis_file.o cloudsat_swath_file.o sat_utils.o data_averager.o data_plane_to_netcdf.o \
        -o modis_regrid \
        $(FLAGS) \
        -I$(MET_INC_DIR) \
        -I$(HDF_INC_DIR) \
        -I$(HDF_EOS_INC_DIR) \
        $(NETCDF_INCS) \
        $(GSL_INCS) \
        -L$(MET_LIB_DIR) \
        -L$(HDF_EOS_LIB_DIR) \
        -L$(HDF_LIB_DIR) \
        $(NETCDF_LIBS) \
        $(GSL_LIBS) \
        $(MET_LITTLE_Ls) \
        -lhdfeos \
        -lmfhdf \
        -ldf \
        -lGctp \
        -ljpeg \
        -lz \
        -lm
	cp modis_regrid $(MET_BIN_DIR)

########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	rm -f $(MET_BIN_DIR)/modis_regrid


########################################################################


.PHONY: all clean gen_sources


########################################################################

