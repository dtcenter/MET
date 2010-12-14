

########################################################################


all:
ifneq ($(DISABLE_ASCII2NC), 0)
	@ cd ascii2nc       ; $(MAKE) $(PRINT_OPTS)
endif
ifneq ($(DISABLE_ENSEMBLE_STAT), 0)
	@ cd ensemble_stat  ; $(MAKE) $(PRINT_OPTS)
endif
ifneq ($(DISABLE_GEN_POLY_MASK), 0)
	@ cd gen_poly_mask  ; $(MAKE) $(PRINT_OPTS)
endif
ifneq ($(DISABLE_GRID_STAT), 0)
	@ cd grid_stat      ; $(MAKE) $(PRINT_OPTS)
endif
ifneq ($(DISABLE_MODE), 0)
	@ cd mode           ; $(MAKE) $(PRINT_OPTS)
endif
ifneq ($(DISABLE_MODE_ANALYSIS), 0)
	@ cd mode_analysis  ; $(MAKE) $(PRINT_OPTS)
endif
ifneq ($(DISABLE_PB2NC), 0)
	@ cd pb2nc          ; $(MAKE) $(PRINT_OPTS)
endif
ifneq ($(DISABLE_PCP_COMBINE), 0)
	@ cd pcp_combine    ; $(MAKE) $(PRINT_OPTS)
endif
ifneq ($(DISABLE_POINT_STAT), 0)
	@ cd point_stat     ; $(MAKE) $(PRINT_OPTS)
endif
ifneq ($(DISABLE_STAT_ANALYSIS), 0)
	@ cd stat_analysis  ; $(MAKE) $(PRINT_OPTS)
endif
ifneq ($(DISABLE_WAVELET_STAT), 0)
	@ cd wavelet_stat   ; $(MAKE) $(PRINT_OPTS)
endif
ifneq ($(DISABLE_PLOT_POINT_OBS), 0)
	@ cd plot_point_obs ; $(MAKE) $(PRINT_OPTS)
endif
ifneq ($(DISABLE_WWMCA), 0)
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
	@ cd pcp_combine    ; $(MAKE) $(PRINT_OPTS) clean
	@ cd point_stat     ; $(MAKE) $(PRINT_OPTS) clean
	@ cd stat_analysis  ; $(MAKE) $(PRINT_OPTS) clean
	@ cd wavelet_stat   ; $(MAKE) $(PRINT_OPTS) clean
	@ cd plot_point_obs ; $(MAKE) $(PRINT_OPTS) clean
	@ cd wwmca_tool     ; $(MAKE) $(PRINT_OPTS) clean


########################################################################


