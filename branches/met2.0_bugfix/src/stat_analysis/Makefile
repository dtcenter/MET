# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2005
# ** University Corporation for Atmospheric Research(UCAR)
# ** National Center for Atmospheric Research(NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the stat_analysis application
#
###############################################################################

TARGETS		= stat_analysis

OBJECTS		= aggr_stat_line.o \
		  parse_stat_line.o \
		  stat_analysis_Conf.o \
		  stat_analysis_job.o

OPT_FLAGS	=

FAKES		= all clean objects targets

LIB_DIR		= ../../lib

INC_DIR		= ../../lib

all:
	@ echo
	@ echo "*** Making stat_analysis application ***"
	@ echo
	@ $(MAKE) objects $(PRINT_OPTS)
	@ $(MAKE) targets $(PRINT_OPTS)
	@ echo
	@ echo "*** Finished Making stat_analysis application ***"
	@ echo

objects: $(OBJECTS)
	@ echo

targets: $(TARGETS)
	@ echo

stat_analysis: stat_analysis.cc
	@echo
	$(CXX) -o $(TARGETS) stat_analysis.cc $(OBJECTS) \
	$(CXX_FLAGS) $(OPT_FLAGS) -DMET_BASE=\"$(MET_BASE)\" \
	-I$(LIB_DIR) $(NETCDF_INCS) $(GSL_INCS) \
	-L$(LIB_DIR) $(NETCDF_LIBS) $(GSL_LIBS) \
	-lvx_met_util -lvx_analysis_util -lvx_wrfdata -lvx_met_util \
        -lvx_contable -lvx_grib_classes \
	-lvx_econfig -lvx_gsl_prob -lgsl \
	-lvx_plot_util  -lvx_render -lvx_pxm -lvx_color -lvx_ps -lvx_afm \
	-lvx_data_grids -lvx_gnomon -lvx_nav -lvx_cal -lvx_util -lvx_math -lm \
	-lnetcdf_c++ -lnetcdf \
	$(CXX_LIBS)
	cp $(TARGETS) ../../bin/.

aggr_stat_line.o: aggr_stat_line.cc
	$(CXX) -c aggr_stat_line.cc $(CXX_FLAGS) $(OPT_FLAGS) \
	-I$(INC_DIR) $(NETCDF_INCS) $(GSL_INCS)

parse_stat_line.o: parse_stat_line.cc
	$(CXX) -c parse_stat_line.cc $(CXX_FLAGS) $(OPT_FLAGS) \
	-I$(INC_DIR) $(NETCDF_INCS) $(GSL_INCS)

stat_analysis_Conf.o: stat_analysis_Conf.cc
	$(CXX) -c stat_analysis_Conf.cc $(CXX_FLAGS) $(OPT_FLAGS) \
	-I$(INC_DIR)

stat_analysis_job.o: stat_analysis_job.cc
	$(CXX) -c stat_analysis_job.cc $(CXX_FLAGS) $(OPT_FLAGS) \
	-I$(INC_DIR) $(NETCDF_INCS) $(GSL_INCS)

clean:
	rm -r -f *.a *.o junk temp core log c.ps a.out $(TARGETS) ../../bin/$(TARGETS)

.PHONY: $(FAKES)
