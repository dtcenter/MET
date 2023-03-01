########################################################################
#
#    Reads temporary point obs. file into memory.
#
#    usage:  /path/to/python read_tmp_point_nc.py tmp_output_filename
#
########################################################################

import os
import sys

# add share/met/python directory to system path to find met_point_obs
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__),
                                             os.pardir, 'python')))
from met_point_obs import met_point_obs
from met_point_obs_nc import nc_point_obs

netcdf_filename = sys.argv[1]

# read NetCDF file
print('{p}  reading{f}'.format(p=met_point_obs.get_prompt(), f=netcdf_filename))
point_obs_data = nc_point_obs()
point_obs_data.read_data(netcdf_filename)

met_point_data = point_obs_data.get_point_data() 
met_point_data['met_point_data'] = point_obs_data
