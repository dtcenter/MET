

########################################################################


   ##
   ##  MET definitions
   ##


########################################################################


MET_BASE_DIR        = $(PWD)

MET_LIB_DIR         = $(MET_BASE_DIR)/lib

MET_INC_DIR         = $(MET_BASE_DIR)/include

MET_SRC_DIR         = $(MET_BASE_DIR)/src

MET_BIN_DIR         = $(MET_BASE_DIR)/bin

MET_OUTPUT_DIR      = $(MET_BASE_DIR)/out


########################################################################


ENUM_TO_STRING      = $(MET_SRC_DIR)/basic/enum_to_string/enum_to_string

ECONFIG_CODEGEN     = $(MET_SRC_DIR)/basic/vx_econfig/econfig_codegen

CONFIG_TEMPLATE_DIR = $(MET_BASE_DIR)/data/config


########################################################################


   ##
   ##  GRIB2 support
   ##


ifeq ($(WITH_GRIB2), 1)
	ARCH_FLAGS := $(ARCH_FLAGS) -DWITH_GRIB2
	G2SUP_LIBS = -lgrib2c -ljasper -lpng -lz
	GRIB2_LIBS = -lvx_data2d_grib2 $(G2SUP_LIBS)
else
	G2SUP_LIBS =
	GRIB2_LIBS =
endif


########################################################################


CXX_FLAGS := $(CXX_FLAGS) $(ARCH_FLAGS)

