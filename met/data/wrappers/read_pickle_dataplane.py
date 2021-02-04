########################################################################
#
#    Reads temporary pickle file into memory.
#
#    usage:  /path/to/python read_pickle_dataplane.py pickle.tmp
#
########################################################################

import sys
import numpy as np
import pickle
import netCDF4 as nc

print('Python Script:\t', sys.argv[0])
print('Load Pickle:\t', sys.argv[1])
# met_info = pickle.load(open(sys.argv[1], "rb"))
met_info = {}

netcdf_filename = sys.argv[1] + '.nc4'
print('Read NetCDF:\t',  netcdf_filename)

# read NetCDF file
ds = nc.Dataset(netcdf_filename, 'r')
met_data = ds['met_data'][:]
met_attrs = {}
grid = {}
for attr, attr_val in ds.__dict__.items():
    if 'grid' in attr:
        grid_attr = attr.split('.')[1]
        grid[grid_attr] = attr_val
    else:
        met_attrs[attr] = attr_val
grid['nx'], grid['ny'] = int(grid['nx']), int(grid['ny'])
met_attrs['grid'] = grid
met_info['met_data'] = met_data
met_info['attrs'] = met_attrs
print(met_info)
