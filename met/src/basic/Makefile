

########################################################################


all: $(OBJECTS) $(OBJECTS) $(EXECUTABLES)
	@ echo
	@ cd vx_cal         && make $(PRINT_OPTS)
	@ cd enum_to_string && make $(PRINT_OPTS)
	@ cd vx_math        && make $(PRINT_OPTS)
	@ cd vx_util        && make $(PRINT_OPTS)
	@ cd vx_econfig     && make $(PRINT_OPTS)
	@ cd vx_codegen     && make $(PRINT_OPTS)


########################################################################


clean:
	@ echo
	@ cd vx_cal         && make $(PRINT_OPTS) clean
	@ cd enum_to_string && make $(PRINT_OPTS) clean
	@ cd vx_math        && make $(PRINT_OPTS) clean
	@ cd vx_util        && make $(PRINT_OPTS) clean
	@ cd vx_econfig     && make $(PRINT_OPTS) clean
	@ cd vx_codegen     && make $(PRINT_OPTS) clean


########################################################################


