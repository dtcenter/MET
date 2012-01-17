# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2012
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################


all:
	cd gen_poly_mask ; $(MAKE) $(PRINT_OPTS)
	cd plot_data_plane ; $(MAKE) $(PRINT_OPTS)
	cd plot_point_obs ; $(MAKE) $(PRINT_OPTS)
	cd ascii2nc ; $(MAKE) $(PRINT_OPTS)
	cd madis2nc ; $(MAKE) $(PRINT_OPTS)
	cd pb2nc ; $(MAKE) $(PRINT_OPTS)
	cd wwmca_tool ; $(MAKE) $(PRINT_OPTS)


########################################################################


clean:
	cd gen_poly_mask ; $(MAKE) $(PRINT_OPTS) clean
	cd plot_data_plane ; $(MAKE) $(PRINT_OPTS) clean
	cd plot_point_obs ; $(MAKE) $(PRINT_OPTS) clean
	cd ascii2nc ; $(MAKE) $(PRINT_OPTS) clean
	cd madis2nc ; $(MAKE) $(PRINT_OPTS) clean
	cd pb2nc ; $(MAKE) $(PRINT_OPTS) clean
	cd wwmca_tool ; $(MAKE) $(PRINT_OPTS) clean


########################################################################


gen_sources:
	cd gen_poly_mask ; $(MAKE) $(PRINT_OPTS) gen_sources
	cd plot_data_plane ; $(MAKE) $(PRINT_OPTS) gen_sources
	cd plot_point_obs ; $(MAKE) $(PRINT_OPTS) gen_sources
	cd ascii2nc ; $(MAKE) $(PRINT_OPTS) gen_sources
	cd madis2nc ; $(MAKE) $(PRINT_OPTS) gen_sources
	cd pb2nc ; $(MAKE) $(PRINT_OPTS) gen_sources
	cd wwmca_tool ; $(MAKE) $(PRINT_OPTS) gen_sources


########################################################################

