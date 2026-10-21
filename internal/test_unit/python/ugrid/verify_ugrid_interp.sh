#!/bin/bash
#
# Verify the unit_ugrid.xml outputs of the N closest faces interpolation
# (Point-Stat) and smoothing (Grid-Stat) for the unstructured grid against
# an independent Python reference.
#
# Run unit_ugrid.xml first, then:
#   MET_TEST_OUTPUT=... MET_TEST_INPUT=... verify_ugrid_interp.sh
#
# Requires python3 with numpy, netCDF4 and scipy (e.g. MET_PYTHON_EXE).
#

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
PYTHON=${MET_PYTHON_EXE:-python3}

: ${MET_TEST_INPUT:?"MET_TEST_INPUT must be set"}
: ${MET_TEST_OUTPUT:?"MET_TEST_OUTPUT must be set"}

MPAS_DIR=${MET_TEST_INPUT}/ugrid_data/mpas
status=0

echo "=== point_stat_ugrid_mpas_npts"
${PYTHON} ${SCRIPT_DIR}/check_ugrid_point_stat.py \
   ${MET_TEST_OUTPUT}/point_stat_ugrid/point_stat_UGRID_MPAS_NPTS_120000L_20250518_120000V.stat \
   ${MPAS_DIR}/diag.2025-05-18_12.00.00.9506.OK.nc \
   ${MPAS_DIR}/static.2025-05-17.9506.OK_reduced.nc \
   temperature_925hPa || status=1

echo
echo "=== grid_stat_ugrid_mpas_smooth"
${PYTHON} ${SCRIPT_DIR}/check_ugrid_grid_stat_smooth.py \
   ${MET_TEST_OUTPUT}/grid_stat_ugrid/grid_stat_UGRID_MPAS_SMOOTH_120000L_20120409_120000V.stat \
   ${MPAS_DIR}/diag.2012-04-09_12.00.00_reduced.nc temperature_925hPa \
   ${MPAS_DIR}/mpasout.2012-04-09_12.00.00_reduced.nc theta \
   ${MPAS_DIR}/static.40962_reduced.nc || status=1

echo
[ ${status} -eq 0 ] && echo "verify_ugrid_interp.sh: PASS" || echo "verify_ugrid_interp.sh: FAIL"
exit ${status}
