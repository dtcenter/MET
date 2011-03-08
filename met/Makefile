# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research(UCAR)
# ** National Center for Atmospheric Research(NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the Model Evaluation Tools Project
# for use with the GNU Compilers
#
###############################################################################

#include user_defs.mk
include user_defs_dev.mk
include met_defs.mk
include export.mk

###############################################################################

all:
	@ echo
	@ echo "*** Making the Model Evaluation Tools Project ***"
	@ echo
	@ cd src ; $(MAKE) $(PRINT_OPTS)
	@ echo
	@ echo "*** Finished Making the Model Evaluation Tools Project ***"
	@ echo

###############################################################################

clean:
	cd src ; $(MAKE) clean $(PRINT_OPTS)
	rm -r -f *.a *.o junk temp core log c.ps a.out
	rm -f include/*.h

########################################################################

.PHONY: all clean libs targets gen_sources

########################################################################


