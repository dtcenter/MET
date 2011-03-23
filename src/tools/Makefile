

########################################################################


all:
ifeq ($(ENABLE_ASCII2NC), 1)
	@ cd ascii2nc       ; $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(ENABLE_ENSEMBLE_STAT), 1)
	@ cd ensemble_stat  ; $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(ENABLE_GEN_POLY_MASK), 1)
	@ cd gen_poly_mask  ; $(MAKE) $(PRINT_OPTS)
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
ifeq ($(ENABLE_PB2NC), 1)
	@ cd pb2nc          ; $(MAKE) $(PRINT_OPTS)
endif
	@ cd pbtime         ; $(MAKE) $(PRINT_OPTS)
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
ifeq ($(ENABLE_PLOT_POINT_OBS), 1)
	@ cd plot_point_obs ; $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(ENABLE_WWMCA), 1)
	@ cd wwmca_tool     ; $(MAKE) $(PRINT_OPTS)
endif


########################################################################


clean:
	@ cd ascii2nc       ; $(MAKE) $(PRINT_OPTS) clean
	@ cd ensemble_stat  ; $(MAKE) $(PRINT_OPTS) clean
	@ cd gen_poly_mask  ; $(MAKE) $(PRINT_OPTS) clean
	@ cd grid_stat      ; $(MAKE) $(PRINT_OPTS) clean
	@ cd mode           ; $(MAKE) $(PRINT_OPTS) clean
	@ cd mode_analysis  ; $(MAKE) $(PRINT_OPTS) clean
	@ cd pb2nc          ; $(MAKE) $(PRINT_OPTS) clean
	@ cd pbtime         ; $(MAKE) $(PRINT_OPTS) clean
	@ cd pcp_combine    ; $(MAKE) $(PRINT_OPTS) clean
	@ cd point_stat     ; $(MAKE) $(PRINT_OPTS) clean
	@ cd stat_analysis  ; $(MAKE) $(PRINT_OPTS) clean
	@ cd wavelet_stat   ; $(MAKE) $(PRINT_OPTS) clean
	@ cd plot_point_obs ; $(MAKE) $(PRINT_OPTS) clean
	@ cd wwmca_tool     ; $(MAKE) $(PRINT_OPTS) clean


########################################################################


