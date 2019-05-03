# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2013
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################


all:
ifeq ($(ENABLE_TC_UTILS), 1)
	@ cd tc_dland; $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(ENABLE_TC_UTILS), 1)
	@ cd tc_pairs; $(MAKE) $(PRINT_OPTS)
endif
ifeq ($(ENABLE_TC_UTILS), 1)
	@ cd tc_stat;  $(MAKE) $(PRINT_OPTS)
endif


########################################################################


clean:
	@ cd tc_dland; $(MAKE) $(PRINT_OPTS) clean
	@ cd tc_pairs; $(MAKE) $(PRINT_OPTS) clean
	@ cd tc_stat;  $(MAKE) $(PRINT_OPTS) clean


########################################################################


gen_sources:
	@ cd tc_dland; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd tc_pairs; $(MAKE) $(PRINT_OPTS) gen_sources
	@ cd tc_stat;  $(MAKE) $(PRINT_OPTS) gen_sources


########################################################################
