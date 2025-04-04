#! /bin/bash

################################################################################
# Script: update_met_unit_test_data.sh
# Author: George McCabe <mccabe@ucar.edu>
# Description: Run this script after updating data in the unit_test directory
#  in the vX.Y directory, where vX.Y is the version of MET that is currently
#  in development.
#  This script assumes:
#   * /home/met_test/MET_unit_test is a symbolic link to a directory
#     available on the web server with the same name
#   * The vX.Y directory exists under MET_unit_test, which contains a directory
#     named unit_test that contains the test data, a tarfile that includes the
#     content of the unit_test directory, and a volume_mount_directories file
################################################################################

if [[ "$1" =~ ^v[0-9]+\.[0-9]+$ ]]; then
    NEXT_RELEASE=$1
else
    echo ERROR: Version argument must match vX.Y format, e.g. v12.1
    exit 1
fi

# directory containing unit test data
# this is typically a symbolic link from the met_test home directory
# to the actual location, e.g. /d2/www/dtcenter/dfiles/code/METplus/MET/MET_unit_test
UNIT_TEST_DATA_DIR=/home/met_test/MET_unit_test

# name of tarfile containing MET unit test data
INPUT_DATA_TARFILE_NAME="unit_test-all.tgz"

# directory containing data and tarfiles for upcoming release
NEXT_RELEASE_DIR=${UNIT_TEST_DATA_DIR}/${NEXT_RELEASE}


# exit if next release directory (e.g. v12.1) does not exist

if [ ! -d "${NEXT_RELEASE_DIR}" ]; then
    echo "Next release directory does not exist: ${NEXT_RELEASE_DIR}"
    echo "Exiting..."
    exit 0
fi


# check if vX.Y/unit_test-all.tgz exists and exit if not

UPDATE_DATA_TARFILE_PATH=${NEXT_RELEASE_DIR}/${INPUT_DATA_TARFILE_NAME}
if [ ! -f "${UPDATE_DATA_TARFILE_PATH}" ]; then
    echo "ERROR: Tarfile must exist in ${NEXT_RELEASE} dir: ${UPDATE_DATA_TARFILE_PATH}"
    exit 1
fi


# save a copy of the tarfile in case it needs to be recovered
save_path=${UPDATE_DATA_TARFILE_PATH}.$(date +%Y%m%d)
echo "Saving a copy of ${UPDATE_DATA_TARFILE_PATH} to ${save_path}"
mv "${UPDATE_DATA_TARFILE_PATH}" "${save_path}"

echo "Creating ${INPUT_DATA_TARFILE_NAME} from unit_test directory in ${NEXT_RELEASE_DIR}"
(cd "${NEXT_RELEASE_DIR}" && tar -cvzf ${INPUT_DATA_TARFILE_NAME} unit_test)
