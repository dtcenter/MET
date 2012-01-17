# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2012
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################


all:
ifeq ($(ENABLE_ENSEMBLE_STAT), 1)
	@ cd ensemble_stat  ; $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(ENABLE_GRID_STAT), 1)
	@ cd grid_stat      ; $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(ENABLE_MODE), 1)
	@ cd mode           ; $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(ENABLE_MODE_ANALYSIS), 1)
	@ cd mode_analysis  ; $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(ENABLE_PCP_COMBINE), 1)
	@ cd pcp_combine    ; $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(ENABLE_POINT_STAT), 1)
	@ cd point_stat     ; $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(ENABLE_STAT_ANALYSIS), 1)
	@ cd stat_analysis  ; $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(ENABLE_WAVELET_STAT), 1)
	@ cd wavelet_stat   ; $(MAKE) $(PRINT_OPTS)
endif


########################################################################


clean:
	@ cd ensemble_stat  ; $(MAKE) $(PRINT_OPTS) clean
	@ cd grid_stat      ; $(MAKE) $(PRINT_OPTS) clean
	@ cd mode           ; $(MAKE) $(PRINT_OPTS) clean
	@ cd mode_analysis  ; $(MAKE) $(PRINT_OPTS) clean
	@ cd pcp_combine    ; $(MAKE) $(PRINT_OPTS) clean
	@ cd point_stat     ; $(MAKE) $(PRINT_OPTS) clean
	@ cd stat_analysis  ; $(MAKE) $(PRINT_OPTS) clean
	@ cd wavelet_stat   ; $(MAKE) $(PRINT_OPTS) clean


########################################################################


gen_sources:
	@ cd ensemble_stat  ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd grid_stat      ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd mode           ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd mode_analysis  ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd pcp_combine    ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd point_stat     ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd stat_analysis  ; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd wavelet_stat   ; $(MAKE) $(PRINT_OPTS) gen_sources


########################################################################
