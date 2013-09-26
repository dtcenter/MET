# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2013
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the dev_utils tools (development utilities)
#
########################################################################


EXECUTABLES = chk4copyright \
              reformat_map_data \
              reformat_county_data \
              pbtime  \
              gribtab.dat_to_flat  \
              nceptab_to_flat  \
              insitu_nc_to_ascii  \


PB_OBJECTS  = openpb.o \
              readpb.o


INSITU_OBJECTS = insitu_nc_file.o


OBJECTS     = $(PB_OBJECTS) \
              $(INSITU_OBJECTS)


LIBRARIES   =


GEN_SOURCES =


########################################################################


HEADERS     =


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the dev_utils Tools ***"
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


openpb.o: ../other/pb2nc/openpb.f
	@ echo
	$(FC) ../other/pb2nc/openpb.f $(FC_FLAGS) -c


readpb.o: ../other/pb2nc/readpb.f
	@ echo
	$(FC) ../other/pb2nc/readpb.f $(FC_FLAGS) -c


insitu_nc_file.o: insitu_nc_file.cc
	$(CXX) insitu_nc_file.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) $(NETCDF_INCS)


########################################################################


   ##
   ##  libraries
   ##


########################################################################


   ##
   ##  executables
   ##


chk4copyright: chk4copyright.cc $(LIBRARIES)
	@ echo
	$(CXX) chk4copyright.cc -o chk4copyright $(CXX_FLAGS) \
	-I$(MET_INC_DIR) -L$(MET_LIB_DIR) \
	-lvx_util \
	-lvx_log
	cp chk4copyright $(MET_BIN_DIR)


reformat_map_data: reformat_map_data.cc $(LIBRARIES)
	@ echo
	$(CXX) reformat_map_data.cc -o reformat_map_data $(CXX_FLAGS) \
	-I$(MET_INC_DIR) -L$(MET_LIB_DIR) \
	-lvx_util \
	-lvx_log
	cp reformat_map_data $(MET_BIN_DIR)


reformat_county_data: reformat_county_data.cc $(LIBRARIES)
	@ echo
	$(CXX) reformat_county_data.cc -o reformat_county_data $(CXX_FLAGS) \
	-I$(MET_INC_DIR) -L$(MET_LIB_DIR) \
	-lvx_util \
	-lvx_log
	cp reformat_county_data $(MET_BIN_DIR)


pbtime: pbtime.cc $(PB_OBJECTS)
	@ echo
	$(CXX) -o pbtime pbtime.cc $(PB_OBJECTS) \
	$(CXX_FLAGS) \
	-I$(MET_INC_DIR) $(BUFR_INCS) $(F2C_INCS) \
	-L$(MET_LIB_DIR) $(BUFR_LIBS) $(F2C_LIBS) \
	-lvx_pb_util \
	-lvx_cal \
	-lvx_util \
	-lvx_log \
	-lbufr \
	$(F2C_LIBNAME) $(CXX_LIBS) $(FC_LIBS)
	cp pbtime $(MET_BIN_DIR)


gribtab.dat_to_flat: gribtab.dat_to_flat.cc $(PB_OBJECTS)
	@ echo
	$(CXX) -o gribtab.dat_to_flat gribtab.dat_to_flat.cc $(PB_OBJECTS) \
	$(CXX_FLAGS) \
	-I$(MET_INC_DIR) $(BUFR_INCS) $(F2C_INCS) \
	-L$(MET_LIB_DIR) $(BUFR_LIBS) $(F2C_LIBS) \
	-lvx_pb_util \
	-lvx_cal \
	-lvx_util \
	-lvx_log \
	-lbufr \
	$(F2C_LIBNAME) $(CXX_LIBS) $(FC_LIBS)
	cp gribtab.dat_to_flat $(MET_BIN_DIR)


nceptab_to_flat: nceptab_to_flat.cc $(PB_OBJECTS)
	@ echo
	$(CXX) -o nceptab_to_flat nceptab_to_flat.cc $(PB_OBJECTS) \
	$(CXX_FLAGS) \
	-I$(MET_INC_DIR) $(BUFR_INCS) $(F2C_INCS) \
	-L$(MET_LIB_DIR) $(BUFR_LIBS) $(F2C_LIBS) \
	-lvx_pb_util \
	-lvx_cal \
	-lvx_util \
	-lvx_log \
	-lbufr \
	$(F2C_LIBNAME) $(CXX_LIBS) $(FC_LIBS)
	cp nceptab_to_flat $(MET_BIN_DIR)


insitu_nc_to_ascii: insitu_nc_to_ascii.cc $(INSITU_OBJECTS) $(LIBRARIES)
	@ echo
	$(CXX) -o insitu_nc_to_ascii insitu_nc_to_ascii.cc $(INSITU_OBJECTS) \
	$(CXX_FLAGS) \
	-I$(MET_INC_DIR) $(NETCDF_INCS) \
	-L$(MET_LIB_DIR) $(NETCDF_LIBS) \
	-lvx_util \
	-lvx_log \
	-lm -lnetcdf_c++ -lnetcdf
	cp insitu_nc_to_ascii $(MET_BIN_DIR)


########################################################################


clean:
	rm -f *.a *.o *.ps temp junk core a.out $(PB_OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	rm -f $(MET_BIN_DIR)/chk4copyright
	rm -f $(MET_BIN_DIR)/reformat_map_data
	rm -f $(MET_BIN_DIR)/reformat_county_data
	rm -f $(MET_BIN_DIR)/pbtime
	rm -f $(MET_BIN_DIR)/gribtab.dat_to_flat
	rm -f $(MET_BIN_DIR)/nceptab_to_flat
	rm -f $(MET_BIN_DIR)/insitu_nc_to_ascii


########################################################################


.PHONY: all clean gen_sources


########################################################################

