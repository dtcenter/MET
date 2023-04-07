########################################################################
#
#    Reads temporary file into memory.
#
#    usage:  /path/to/python read_tmp_dataplane.py dataplane.tmp
#
########################################################################

import sys

# PYTHON path for met.dataplane is added by write_tmp_dataplane.py
from met.dataplane import dataplane

netcdf_filename = sys.argv[1]
# read NetCDF file
met_info = dataplane.read_dataplane(netcdf_filename)
