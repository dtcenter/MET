# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the vx_gdata library
#
###############################################################################

TARGETS  = libvx_gdata.a

OBJECTS  = get_pinterp_grid.o \
           is_met_nc_file.o   \
           is_netcdf_file.o   \
           is_pinterp_file.o  \
           met_file.o         \
           pinterp_file.o     \
           var_info.o         \
           vx_gdata_util.o

OPT_FLAGS=

FAKES    = vx_gdata_all vx_gdata_lib clean

LIB_DIR  = ..


vx_gdata_all:
	@ echo
	@ echo "*** Making vx_gdata Library ***"
	@ echo
	@ $(MAKE) vx_gdata_lib  $(PRINT_OPTS)
	cp $(TARGETS) ../.

vx_gdata_lib: libvx_gdata.a
	@ echo

libvx_gdata.a: $(OBJECTS)
	ar -rs libvx_gdata.a $(OBJECTS)
	ranlib libvx_gdata.a

get_pinterp_grid.o: get_pinterp_grid.cc
	$(CXX) get_pinterp_grid.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(NETCDF_INCS)

is_met_nc_file.o: is_met_nc_file.cc
	$(CXX) is_met_nc_file.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(NETCDF_INCS)

is_netcdf_file.o: is_netcdf_file.cc
	$(CXX) is_netcdf_file.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(NETCDF_INCS)

is_pinterp_file.o: is_pinterp_file.cc
	$(CXX) is_pinterp_file.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(NETCDF_INCS)

met_file.o: met_file.cc
	$(CXX) met_file.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(GSL_INCS) $(NETCDF_INCS)

pinterp_file.o: pinterp_file.cc
	$(CXX) pinterp_file.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(NETCDF_INCS)

var_info.o: var_info.cc
	$(CXX) var_info.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(NETCDF_INCS)

vx_gdata_util.o: vx_gdata_util.cc
	$(CXX) vx_gdata_util.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR) $(NETCDF_INCS)

clean:
	rm -f *.a *.o temp junk core a.out $(TARGETS) ../$(TARGETS)

.PHONY: $(FAKES)


