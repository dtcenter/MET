


########################################################################


TEST_OUT_DIR = ../out


TOOLS_DIR    = ../src/tools

CORE_DIR     = $(TOOLS_DIR)/core

OTHER_DIR    = $(TOOLS_DIR)/other

MK_DIR       = ./mk


########################################################################

   ##
   ##  some of the config files need this
   ##

export TEST_OUT_DIR

########################################################################


   ##
   ##  Read the config.h file (genereated by autoconf) to see which tools
   ##
   ##    were built
   ##


TESTS := $(shell grep "define ENABLE_" ../config.h | cut -d" " -f2 | cut -b8- | tr A-Z a-z)


########################################################################


all: ${TESTS}
	@ echo
	@ echo "   *** Finished Testing the Model Evaluation Tools Project ***"
	@ echo


########################################################################


include $(MK_DIR)/ascii2nc.mk
include $(MK_DIR)/ensemble_stat.mk
include $(MK_DIR)/gen_vx_mask.mk
include $(MK_DIR)/grib2.mk
include $(MK_DIR)/grid_stat.mk
include $(MK_DIR)/gsi_tools.mk
include $(MK_DIR)/madis2nc.mk
include $(MK_DIR)/mode.mk
include $(MK_DIR)/mode_analysis.mk
include $(MK_DIR)/mode_graphics.mk
include $(MK_DIR)/mode_time_domain.mk
include $(MK_DIR)/modis.mk
include $(MK_DIR)/pb2nc.mk
include $(MK_DIR)/pcp_combine.mk
include $(MK_DIR)/plot_data_plane.mk
include $(MK_DIR)/plot_point_obs.mk
include $(MK_DIR)/point_stat.mk
include $(MK_DIR)/regrid_data_plane.mk
include $(MK_DIR)/series_analysis.mk
include $(MK_DIR)/shift_data_plane.mk
include $(MK_DIR)/stat_analysis.mk
include $(MK_DIR)/tc_utils.mk
include $(MK_DIR)/wavelet_stat.mk
include $(MK_DIR)/wwmca.mk
include $(MK_DIR)/lidar2nc.mk


########################################################################
########################################################################


which:
	@ echo ${TESTS}


########################################################################


clean:
	@ echo
	rm -f ${TESTS}
	rm -f ${TEST_OUT_DIR}/ascii2nc/*
	rm -f ${TEST_OUT_DIR}/ensemble_stat/*
	rm -f ${TEST_OUT_DIR}/gen_vx_mask/*
	rm -f ${TEST_OUT_DIR}/grid_stat/*
	rm -f ${TEST_OUT_DIR}/madis2nc/*
	rm -f ${TEST_OUT_DIR}/mode/*
	rm -f ${TEST_OUT_DIR}/mode_analysis/*
	rm -f ${TEST_OUT_DIR}/pb2nc/*
	rm -f ${TEST_OUT_DIR}/pcp_combine/*
	rm -f ${TEST_OUT_DIR}/plot_data_plane/*
	rm -f ${TEST_OUT_DIR}/plot_point_obs/*
	rm -f ${TEST_OUT_DIR}/point_stat/*
	rm -f ${TEST_OUT_DIR}/stat_analysis/*
	rm -f ${TEST_OUT_DIR}/wavelet_stat/*
	rm -f ${TEST_OUT_DIR}/lidar2nc/*


########################################################################
## This section below was added so the Rscripts subdirectory
## is installed 

SUBDIRS = \
        Rscripts

# This was added to run 'make install' in each of the defined SUBDIRS
# The $@ resolves to the value of the target being called, ie. "install"

.PHONY: install
install:
	for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir $@; \
	done


# The distdir target was added to do nothing when 'make distdir' is called.
# This distdir target is called from the top-level when running 'make dist'
# If this target is not defined than 'make dist' fails.
# Since the scripts directory and scripts/<subdirectories> are distributed
# via the top-level Makefile.am EXTRA_DIST variable, we don't really
# need this target to do anything, other than, not fail.
# This is all a bit of a hack which would go away if this Makefile is ported 
# to the world of autoconf.
#
# More info than you probably want to know.
# When 'make dist' is called, it is actually wanting to run these two commands
#       cd Rscripts && make  top_distdir=../../met-6.0 \
#       distdir=../../met-6.0/scripts/Rscripts \
#       am__remove_distdir=: am__skip_length_check=: am__skip_mode_fix=: distdir;
#       cd Rscripts/include && make  top_distdir=../../../met-6.0 \
#       distdir=../../../met-6.0/scripts/Rscripts/include \
#       am__remove_distdir=: am__skip_length_check=: am__skip_mode_fix=: distdir; 

.PHONY: distdir
distdir:

