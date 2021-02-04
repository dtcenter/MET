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
met_info = pickle.load(open(sys.argv[1], "rb"))

netcdf_filename = sys.argv[1] + '.nc4'
print('Read NetCDF:\t',  netcdf_filename)

# read NetCDF file
ds = nc.Dataset(netcdf_filename, 'r')
met_data = ds['met_data'][:]
grid = {}
for attr, attr_val in ds.__dict__.items():
    print(attr, attr_val)
    if 'grid' in attr:
        grid_attr = attr.split('.')[1]
        grid[grid_attr] = attr_val
print(grid)
met_info['met_data'] = met_data
