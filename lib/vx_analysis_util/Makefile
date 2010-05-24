# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the vx_analysis_util library
#
###############################################################################

TARGETS  = libvx_analysis_util.a

OBJECTS  =  analysis_utils.o \
	box.o \
	by_case_info.o \
	data_line.o \
	mask_poly.o \
	mode_atts.o \
	mode_job.o \
	mode_line.o \
	time_series.o \
	stat_job.o \
	stat_line.o

OPT_FLAGS=

FAKES    = vx_analysis_util_all vx_analysis_util_lib clean

LIB_DIR  = ..

vx_analysis_util_all:
	@ echo
	@ echo "*** Making vx_analysis_util Library ***"
	@ echo
	@ $(MAKE) vx_analysis_util_lib  $(PRINT_OPTS)
	cp $(TARGETS) ../.

vx_analysis_util_lib: libvx_analysis_util.a
	@ echo

libvx_analysis_util.a: $(OBJECTS)
	ar -rs libvx_analysis_util.a $(OBJECTS)
	ranlib libvx_analysis_util.a

analysis_utils.o: analysis_utils.cc
	$(CXX) analysis_utils.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

box.o: box.cc
	$(CXX) box.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

by_case_info.o: by_case_info.cc
	$(CXX) by_case_info.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

data_line.o: data_line.cc
	$(CXX) data_line.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

mask_poly.o: mask_poly.cc
	$(CXX) mask_poly.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

mode_atts.o: mode_atts.cc
	$(CXX) mode_atts.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

mode_job.o: mode_job.cc
	$(CXX) mode_job.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

mode_line.o: mode_line.cc
	$(CXX) mode_line.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

time_series.o: time_series.cc
	$(CXX) time_series.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

stat_job.o: stat_job.cc
	$(CXX) stat_job.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(GSL_INCS)

stat_line.o: stat_line.cc
	$(CXX) stat_line.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(GSL_INCS)

clean:
	rm -f *.a *.o temp junk core a.out $(TARGETS) ../$(TARGETS)

.PHONY: $(FAKES)


