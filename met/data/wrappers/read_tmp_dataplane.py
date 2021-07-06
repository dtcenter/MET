########################################################################
#
#    Reads temporary file into memory.
#
#    usage:  /path/to/python read_tmp_dataplane.py dataplane.tmp
#
########################################################################

import sys
import numpy as np
import netCDF4 as nc

met_info = {}
netcdf_filename = sys.argv[1]

# read NetCDF file
ds = nc.Dataset(netcdf_filename, 'r')
met_data = ds['met_data'][:]
met_attrs = {}

# grid is defined as a dictionary or string
grid = {}
for attr, attr_val in ds.__dict__.items():
    if 'grid.' in attr:
        grid_attr = attr.split('.')[1]
        grid[grid_attr] = attr_val
    else:
        met_attrs[attr] = attr_val

if grid:
    met_attrs['grid'] = grid

met_attrs['name'] = met_attrs['name_str']
del met_attrs['name_str']
met_info['met_data'] = met_data
met_info['attrs'] = met_attrs

