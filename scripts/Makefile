


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


TESTS := $(shell grep "define ENABLE_" ../config.h | grep -v "ENABLE_PYTHON" | grep -v "ENABLE_UGRID" | cut -d" " -f2 | cut -b8- | tr A-Z a-z)


########################################################################


all: ${TESTS}
	@ echo
	@ echo "   *** Finished Testing the Model Evaluation Tools Project ***"
	@ echo


########################################################################


include $(MK_DIR)/ascii2nc.mk
include $(MK_DIR)/ensemble_stat.mk
include $(MK_DIR)/gen_ens_prod.mk
include $(MK_DIR)/gen_vx_mask.mk
include $(MK_DIR)/grib2.mk
include $(MK_DIR)/grid_stat.mk
include $(MK_DIR)/gis_utils.mk
include $(MK_DIR)/gsi_tools.mk
include $(MK_DIR)/ioda2nc.mk
include $(MK_DIR)/lidar2nc.mk
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
include $(MK_DIR)/point2grid.mk
include $(MK_DIR)/point_stat.mk
include $(MK_DIR)/regrid_data_plane.mk
include $(MK_DIR)/series_analysis.mk
include $(MK_DIR)/shift_data_plane.mk
include $(MK_DIR)/stat_analysis.mk
include $(MK_DIR)/tc_utils.mk
include $(MK_DIR)/wavelet_stat.mk
include $(MK_DIR)/wwmca.mk


########################################################################


which:
	@ echo ${TESTS}


########################################################################


clean:
	@ echo
	rm -f ${TESTS}
	rm -f ${TEST_OUT_DIR}/ascii2nc/*
	rm -f ${TEST_OUT_DIR}/ensemble_stat/*
	rm -f ${TEST_OUT_DIR}/gen_ens_prod/*
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
