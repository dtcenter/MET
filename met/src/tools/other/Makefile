# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2012
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################


all:
ifeq ($(ENABLE_GEN_POLY_MASK), 1)
	cd gen_poly_mask ; $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(ENABLE_PLOT_DATA_PLANE), 1)
	cd plot_data_plane ; $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(ENABLE_PLOT_POINT_OBS), 1)
	cd plot_point_obs ; $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(ENABLE_ASCII2NC), 1)
	cd ascii2nc ; $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(ENABLE_MADIS2NC), 1)
	cd madis2nc ; $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(ENABLE_PB2NC), 1)
	cd pb2nc ; $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(ENABLE_WWMCA), 1)
	cd wwmca_tool ; $(MAKE) $(PRINT_OPTS)
endif


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

