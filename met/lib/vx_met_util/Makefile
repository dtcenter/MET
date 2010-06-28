# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the vx_met_util library
#
###############################################################################

TARGETS  = libvx_met_util.a

OBJECTS  = apply_mask.o        \
           compute_ci.o        \
           conversions.o       \
           file_type.o         \
           met_stats.o         \
           pair_data.o         \
           read_grib.o         \
           read_netcdf.o       \
           stat_columns.o      \
           stat_hdr_columns.o  \
           two_to_one.o        \
           write_netcdf.o      \
           grid_output.o       \


OPT_FLAGS=

FAKES    = vx_met_util_all vx_met_util_lib clean

LIB_DIR  = ..


vx_met_util_all:
	@ echo
	@ echo "*** Making vx_met_util Library ***"
	@ echo
	@ $(MAKE) vx_met_util_lib  $(PRINT_OPTS)
	cp $(TARGETS) ../.

vx_met_util_lib: libvx_met_util.a
	@ echo

libvx_met_util.a: $(OBJECTS)
	ar -rs libvx_met_util.a $(OBJECTS)
	ranlib libvx_met_util.a

apply_mask.o: apply_mask.cc
	$(CXX) apply_mask.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -DMET_BASE=\"$(MET_BASE)\" -I$(LIB_DIR) $(GSL_INCS) $(NETCDF_INCS)

compute_ci.o: compute_ci.cc
	$(CXX) compute_ci.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(GSL_INCS)

conversions.o: conversions.cc
	$(CXX) conversions.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(GSL_INCS)

file_type.o: file_type.cc
	$(CXX) file_type.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(GSL_INCS) $(NETCDF_INCS)

met_stats.o: met_stats.cc
	$(CXX) met_stats.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(GSL_INCS)

pair_data.o: pair_data.cc
	$(CXX) pair_data.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(GSL_INCS)

read_grib.o: read_grib.cc
	$(CXX) read_grib.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(GSL_INCS) $(NETCDF_INCS)

read_netcdf.o: read_netcdf.cc
	$(CXX) read_netcdf.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(NETCDF_INCS)

stat_columns.o: stat_columns.cc
	$(CXX) stat_columns.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(GSL_INCS)

stat_hdr_columns.o: stat_hdr_columns.cc
	$(CXX) stat_hdr_columns.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(GSL_INCS)

two_to_one.o: two_to_one.cc
	$(CXX) two_to_one.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

write_netcdf.o: write_netcdf.cc
	$(CXX) write_netcdf.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(GSL_INCS) $(NETCDF_INCS)

grid_output.o: grid_output.h grid_output.cc
	$(CXX) grid_output.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(NETCDF_INCS)


clean:
	rm -f *.a *.o temp junk core a.out $(TARGETS) ../$(TARGETS)

.PHONY: $(FAKES)


