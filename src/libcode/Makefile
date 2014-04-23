# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2013
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################


all:
	@ cd vx_grid              ; $(MAKE) $(PRINT_OPTS)
	@ cd vx_nc_util           ; $(MAKE) $(PRINT_OPTS)
	@ cd vx_data2d            ; $(MAKE) $(PRINT_OPTS)
	@ cd vx_data2d_grib       ; $(MAKE) $(PRINT_OPTS)
ifeq ($(WITH_GRIB2), 1)
	@ cd vx_data2d_grib2      ; $(MAKE) $(PRINT_OPTS)
endif
	@ cd vx_data2d_nc_met     ; $(MAKE) $(PRINT_OPTS)
	@ cd vx_data2d_nccf      ; $(MAKE) $(PRINT_OPTS)
	@ cd vx_data2d_nc_pinterp ; $(MAKE) $(PRINT_OPTS)
	@ cd vx_data2d_factory    ; $(MAKE) $(PRINT_OPTS)
	@ cd vx_gsl_prob          ; $(MAKE) $(PRINT_OPTS)
	@ cd vx_gnomon            ; $(MAKE) $(PRINT_OPTS)
	@ cd vx_geodesy           ; $(MAKE) $(PRINT_OPTS)
	@ cd vx_nav               ; $(MAKE) $(PRINT_OPTS)
	@ cd vx_pb_util           ; $(MAKE) $(PRINT_OPTS)
	@ cd vx_afm               ; $(MAKE) $(PRINT_OPTS)
	@ cd vx_color             ; $(MAKE) $(PRINT_OPTS)
	@ cd vx_ps                ; $(MAKE) $(PRINT_OPTS)
	@ cd vx_pxm               ; $(MAKE) $(PRINT_OPTS)
	@ cd vx_render            ; $(MAKE) $(PRINT_OPTS)
	@ cd vx_plot_util         ; $(MAKE) $(PRINT_OPTS)
	@ cd vx_shapedata         ; $(MAKE) $(PRINT_OPTS)
	@ cd vx_statistics        ; $(MAKE) $(PRINT_OPTS)
	@ cd vx_stat_out          ; $(MAKE) $(PRINT_OPTS)
	@ cd vx_analysis_util     ; $(MAKE) $(PRINT_OPTS)
	@ cd vx_tc_util           ; $(MAKE) $(PRINT_OPTS)


########################################################################


clean:
	@ cd vx_grid              ; $(MAKE) $(PRINT_OPTS) clean
	@ cd vx_nc_util           ; $(MAKE) $(PRINT_OPTS) clean
	@ cd vx_data2d            ; $(MAKE) $(PRINT_OPTS) clean
	@ cd vx_data2d_grib       ; $(MAKE) $(PRINT_OPTS) clean
ifeq ($(WITH_GRIB2), 1)
	@ cd vx_data2d_grib2      ; $(MAKE) $(PRINT_OPTS) clean
endif
	@ cd vx_data2d_nc_met     ; $(MAKE) $(PRINT_OPTS) clean
	@ cd vx_data2d_nccf      ; $(MAKE) $(PRINT_OPTS) clean
	@ cd vx_data2d_nc_pinterp ; $(MAKE) $(PRINT_OPTS) clean
	@ cd vx_data2d_factory    ; $(MAKE) $(PRINT_OPTS) clean
	@ cd vx_gsl_prob          ; $(MAKE) $(PRINT_OPTS) clean
	@ cd vx_gnomon            ; $(MAKE) $(PRINT_OPTS) clean
	@ cd vx_geodesy           ; $(MAKE) $(PRINT_OPTS) clean
	@ cd vx_nav               ; $(MAKE) $(PRINT_OPTS) clean
	@ cd vx_pb_util           ; $(MAKE) $(PRINT_OPTS) clean
	@ cd vx_afm               ; $(MAKE) $(PRINT_OPTS) clean
	@ cd vx_ps                ; $(MAKE) $(PRINT_OPTS) clean
	@ cd vx_color             ; $(MAKE) $(PRINT_OPTS) clean
	@ cd vx_pxm               ; $(MAKE) $(PRINT_OPTS) clean
	@ cd vx_plot_util         ; $(MAKE) $(PRINT_OPTS) clean
	@ cd vx_render            ; $(MAKE) $(PRINT_OPTS) clean
	@ cd vx_shapedata         ; $(MAKE) $(PRINT_OPTS) clean
	@ cd vx_statistics        ; $(MAKE) $(PRINT_OPTS) clean
	@ cd vx_stat_out          ; $(MAKE) $(PRINT_OPTS) clean
	@ cd vx_analysis_util     ; $(MAKE) $(PRINT_OPTS) clean
	@ cd vx_tc_util           ; $(MAKE) $(PRINT_OPTS) clean


########################################################################


gen_sources:
	@ cd vx_grid              ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd vx_nc_util           ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd vx_data2d            ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd vx_data2d_grib       ; $(MAKE) $(PRINT_OPTS) gen_sources
ifeq ($(WITH_GRIB2), 1)
	@ cd vx_data2d_grib2      ; $(MAKE) $(PRINT_OPTS) gen_sources
endif
	@ cd vx_data2d_nc_met     ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd vx_data2d_nccf      ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd vx_data2d_nc_pinterp ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd vx_data2d_factory    ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd vx_gsl_prob          ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd vx_gnomon            ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd vx_geodesy           ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd vx_nav               ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd vx_pb_util           ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd vx_afm               ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd vx_ps                ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd vx_color             ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd vx_pxm               ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd vx_plot_util         ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd vx_render            ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd vx_shapedata         ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd vx_statistics        ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd vx_stat_out          ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd vx_analysis_util     ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd vx_tc_util           ; $(MAKE) $(PRINT_OPTS) gen_sources


########################################################################
