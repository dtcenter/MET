# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the pb2nc tool
#
########################################################################


EXECUTABLES = pb2nc


OBJECTS     = pb2nc_Conf.o  \
              numpbmsg.o    \
              openpb.o      \
              closepb.o     \
              readpb.o      \
              dumppb.o


LIBRARIES   =


GEN_SOURCES = pb2nc_Conf.h   \
              pb2nc_Conf.cc


########################################################################


HEADERS     =


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the pb2nc Tool ***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##

gen_sources: $(GEN_SOURCES)
	@ echo


pb2nc_Conf.h pb2nc_Conf.cc: $(CONFIG_TEMPLATE_DIR)/PB2NCConfig_default
	@ echo
	@ rm -f pb2nc_Conf.h pb2nc_Conf.cc
	$(ECONFIG_CODEGEN) -class pb2nc_Conf -outfile pb2nc_Conf -allow_multiple_reads $(CONFIG_TEMPLATE_DIR)/PB2NCConfig_default


########################################################################


   ##
   ##  objects
   ##


pb2nc_Conf.o: pb2nc_Conf.cc
	@ echo
	$(CXX) pb2nc_Conf.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR) $(GSL_INCS) $(NETCDF_INCS)


numpbmsg.o: numpbmsg.f
	@ echo
	$(FC) numpbmsg.f $(FC_FLAGS) $(OPT_FLAGS) -c


openpb.o: openpb.f
	@ echo
	$(FC) openpb.f $(FC_FLAGS) $(OPT_FLAGS) -c


closepb.o: closepb.f
	@ echo
	$(FC) closepb.f $(FC_FLAGS) $(OPT_FLAGS) -c


readpb.o: readpb.f
	@ echo
	$(FC) readpb.f $(FC_FLAGS) $(OPT_FLAGS) -c


dumppb.o: dumppb.f
	@ echo
	$(FC) dumppb.f $(FC_FLAGS) $(OPT_FLAGS) -c


########################################################################


   ##
   ##  libraries
   ##


########################################################################


   ##
   ##  executables
   ##


pb2nc: pb2nc.cc $(OBJECTS)
	$(CXX) -o $(EXECUTABLES) pb2nc.cc $(OBJECTS) \
	$(CXX_FLAGS) $(OPT_FLAGS) -DMET_BASE=\"$(MET_BASE_DIR)\" \
	-I$(MET_INC_DIR) $(NETCDF_INCS) $(GSL_INCS) $(BUFR_INCS) $(F2C_INCS) \
	-L$(MET_LIB_DIR) $(NETCDF_LIBS) $(GSL_LIBS) $(BUFR_LIBS) $(F2C_LIBS) \
	-lvx_analysis_util -lvx_met_util -lvx_wrfmode -lvx_render \
	-lvx_plot_util -lvx_pxm -lvx_color -lvx_ps -lvx_afm -lvx_pb_util \
	-lvx_nav -lvx_gnomon -lvx_gdata -lvx_stat_out -lvx_statistics \
	-lvx_gsl_prob -lvx_wrfdata -lvx_grib_classes -lvx_data_grids \
	-leconfig -lvx_util -lvx_math -lvx_cal \
	-lm -lnetcdf_c++ -lnetcdf -lgsl -lbufr \
        $(F2C_LIBNAME) $(CXX_LIBS) $(FC_LIBS)
	cp $(EXECUTABLES) $(MET_BIN_DIR)


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	rm -f $(MET_BIN_DIR)/pb2nc


########################################################################


.PHONY: all clean


########################################################################


