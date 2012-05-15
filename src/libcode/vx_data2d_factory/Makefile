# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2012
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_data2d_factory library
#
########################################################################


EXECUTABLES = test_is_grib \
              test_factory


OBJECTS     = is_met_nc_file.o \
              is_netcdf_file.o \
              is_pinterp_file.o \
              is_grib_file.o \
              is_bufr_file.o \
              var_info_factory.o \
              data2d_factory_utils.o \
              data2d_factory.o


LIBRARIES   = libvx_data2d_factory.a


GEN_SOURCES =


########################################################################


HEADERS     = is_met_nc_file.h \
              is_netcdf_file.h \
              is_pinterp_file.h \
              is_grib_file.h \
              is_bufr_file.h \
              var_info_factory.h \
              data2d_factory_utils.h \
              data2d_factory.h \
              vx_data2d_factory.h


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_data2d_factory Library ***"
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


is_met_nc_file.o: is_met_nc_file.h is_met_nc_file.cc
	$(CXX) is_met_nc_file.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)

is_netcdf_file.o: is_netcdf_file.h is_netcdf_file.cc
	$(CXX) is_netcdf_file.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

is_pinterp_file.o: is_pinterp_file.h is_pinterp_file.cc
	$(CXX) is_pinterp_file.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)

is_grib_file.o: is_grib_file.h is_grib_file.cc
	$(CXX) is_grib_file.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

is_bufr_file.o: is_bufr_file.h is_bufr_file.cc
	$(CXX) is_bufr_file.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

var_info_factory.o: var_info_factory.cc
	$(CXX) var_info_factory.cc $(CXX_FLAGS) $(ARCH_FLAGS) -c \
		-I$(MET_INC_DIR) $(GRIB2C_INCS) $(GRIB2C_LIBS) $(G2SUP_LIBS)

data2d_factory_utils.o: data2d_factory_utils.h data2d_factory_utils.cc
	$(CXX) data2d_factory_utils.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

data2d_factory.o: data2d_factory.cc
	$(CXX) data2d_factory.cc $(CXX_FLAGS) $(ARCH_FLAGS) -c \
		-I$(MET_INC_DIR) $(NETCDF_INCS) $(GRIB2C_INCS) $(GRIB2C_LIBS) $(G2SUP_LIBS)


########################################################################


   ##
   ##  libraries
   ##


libvx_data2d_factory.a: $(OBJECTS)
	ar -rs libvx_data2d_factory.a $(OBJECTS) 2>&1
	ranlib libvx_data2d_factory.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_data2d_factory.a
	cp libvx_data2d_factory.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


test_is_grib: $(LIBRARIES) test_is_grib.cc
	$(CXX) -o test_is_grib test_is_grib.cc \
	$(CXX_FLAGS) -I$(MET_INC_DIR) \
	-I$(MET_INC_DIR) \
	-L$(MET_LIB_DIR) \
	-lvx_data2d_factory \
	-lvx_cal \
	-lvx_config \
	-lvx_util \
	-lvx_log

test_factory: $(LIBRARIES) test_factory.cc
	$(CXX) -o test_factory test_factory.cc \
	$(CXX_FLAGS) -I$(MET_INC_DIR) \
	-I$(MET_INC_DIR) $(NETCDF_INCS) $(GRIB2C_INCS) \
	-L$(MET_LIB_DIR) $(NETCDF_LIBS) $(GRIB2C_LIBS) \
	-lvx_data2d_factory \
	-lvx_data2d_nc_met \
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_data2d_nc_pinterp \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_grid \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lgrib2c -ljasper -lpng -lz -lm -lnetcdf_c++ -lnetcdf


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_data2d_factory.a


########################################################################


.PHONY: all clean gen_sources


########################################################################

