#! /bin/bash

################################################################################
# Script: setup_met_next_release_data.sh
# Author: George McCabe <mccabe@ucar.edu>
# Description: Run this script to begin development towards a vX.Y release,
#  typically after the main branch has been created for a release. For example,
#  if main_v12.0 was just created for v12.0.0-rc1, run this script with v12.1
#  as the argument.
#  This script assumes:
#   * /home/met_test/MET_unit_test is a symbolic link to a directory
#     available on the web server with the same name
#   * The develop directory exists under MET_unit_test which contains a tarfile
#     or symbolic link to a tarfile that includes the content of the unit_test
#     directory, and a volume_mount_directories file or symbolic link to it.
#   * The vX.Y directory does not yet exist under MET_unit_test
################################################################################

# argument should be vX.Y version of the next release
# that we are starting development towards

if [[ "$1" =~ ^v[0-9]+\.[0-9]+$ ]]; then
    NEXT_RELEASE=$1
else
    echo ERROR: Version argument must match vX.Y format
    exit 1
fi

# directory containing unit test data
# this is typically a symbolic link from the met_test home directory
# to the actual location, e.g. /d2/www/dtcenter/dfiles/code/METplus/MET/MET_unit_test
UNIT_TEST_DATA_DIR=/home/met_test/MET_unit_test

# name of tarfile containing MET unit test data
INPUT_DATA_TARFILE_NAME="unit_test-all.tgz"

# name of volume mount file
VOLUME_MOUNT_FILE_NAME="volume_mount_directories"

DEVELOP_DIR=${UNIT_TEST_DATA_DIR}/develop
NEXT_RELEASE_DIR=${UNIT_TEST_DATA_DIR}/${NEXT_RELEASE}


# exit if next release directory (e.g. v12.1) already exists

if [ -d "${NEXT_RELEASE_DIR}" ]; then
    echo "Next release directory already exists: ${NEXT_RELEASE_DIR}"
    echo "Exiting..."
    exit 0
fi

echo "Creating directory: ${NEXT_RELEASE_DIR}"
mkdir -p "${NEXT_RELEASE_DIR}"


# check if develop/unit_test-all.tgz and volume mount files exist and exit if not

INPUT_DATA_TARFILE_PATH=${DEVELOP_DIR}/${INPUT_DATA_TARFILE_NAME}
volume_mount_path=${DEVELOP_DIR}/${VOLUME_MOUNT_FILE_NAME}
if [ ! -f ${INPUT_DATA_TARFILE_PATH} ]; then
    echo "ERROR: Tarfile must exist in develop dir: ${INPUT_DATA_TARFILE_PATH}"
    exit 1
fi
if [ ! -f ${volume_mount_path} ]; then
    echo "ERROR: ${VOLUME_MOUNT_FILE_NAME} must exist in develop dir: ${volume_mount_path}"
    exit 1
fi


# copy tarfile into next version directory and extract its contents

echo "Copying tarfile ${INPUT_DATA_TARFILE_PATH} into ${NEXT_RELEASE_DIR}"
cp ${INPUT_DATA_TARFILE_PATH} "${NEXT_RELEASE_DIR}"/

echo "Extracting tarfile into ${NEXT_RELEASE_DIR}"
tar zxf "${NEXT_RELEASE_DIR}/${INPUT_DATA_TARFILE_NAME}" -C "${NEXT_RELEASE_DIR}"

# if develop tarfile is a symbolic link, unlink it, otherwise save a copy
if [ -h ${INPUT_DATA_TARFILE_PATH} ]; then
    echo Unlinking ${INPUT_DATA_TARFILE_PATH}
    unlink ${INPUT_DATA_TARFILE_PATH}
else
    save_path=${INPUT_DATA_TARFILE_PATH}.$(date +%Y%m%d)
    echo "Saving a copy of ${INPUT_DATA_TARFILE_PATH} to ${save_path}"
    mv ${INPUT_DATA_TARFILE_PATH} "${save_path}"
fi

# create symbolic link from develop files to next release version
tarfile_realpath=$(realpath "${NEXT_RELEASE_DIR}/${INPUT_DATA_TARFILE_NAME}")
echo "Creating sym link from ${INPUT_DATA_TARFILE_PATH} to ${tarfile_realpath}"
ln -s "${tarfile_realpath}" ${INPUT_DATA_TARFILE_PATH}


# do the same for the volume_mount_directories file

echo "Copying volume mount file ${volume_mount_path} into ${NEXT_RELEASE_DIR}"
cp ${volume_mount_path} "${NEXT_RELEASE_DIR}"/

if [ -h ${volume_mount_path} ]; then
    echo Unlinking ${volume_mount_path}
    unlink ${volume_mount_path}
else
    save_path=${volume_mount_path}.$(date +%Y%m%d)
    echo "Saving a copy of ${volume_mount_path} to ${save_path}"
    mv ${volume_mount_path} "${save_path}"
fi

# create symbolic link from develop files to next release version
vmd_realpath=$(realpath "${NEXT_RELEASE_DIR}/${VOLUME_MOUNT_FILE_NAME}")
echo "Creating sym link from ${volume_mount_path} to ${vmd_realpath}"
ln -s "${vmd_realpath}" ${volume_mount_path}
