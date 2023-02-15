########################################################################
#
#    Reads temporary file into memory.
#
#    usage:  /path/to/python read_tmp_dataplane.py dataplane.tmp
#
########################################################################

import os
import sys

met_base_dir = os.getenv('MET_BASE',None)
if met_base_dir is not None:
    sys.path.append(os.path.join(met_base_dir, 'python'))

from met_point_obs import met_point_obs
from met_point_obs_nc import nc_point_obs

netcdf_filename = sys.argv[1]

# read NetCDF file
print('{p}  reading{f}'.format(p=met_point_obs.get_prompt(), f=netcdf_filename))
point_obs_data = nc_point_obs()
point_obs_data.read_data(netcdf_filename)

met_point_data = point_obs_data.get_point_data() 
met_point_data['met_point_data'] = point_obs_data
