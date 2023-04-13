'''
Created on Nov 10, 2021

@author: hsoh

This script reads the MET point observation NetCDF file like MET tools do.

Usage:

  python3 read_met_point_obs.py <MET_point_observation_NetCDF_name>
  python3 read_met_point_obs.py <space_separated_text_file>
      <space_separated_text_file>: 11 columns
          'typ', 'sid', 'vld', 'lat', 'lon', 'elv', 'var', 'lvl', 'hgt', 'qc', 'obs'
           string columns: 'typ', 'sid', 'vld', 'var', , 'qc'
          numeric columns: 'lat', 'lon', 'elv', 'lvl', 'hgt', 'qc', 'obs'

'''

import os
import sys
from datetime import datetime

from met.point import met_point_tools
from pyembed.python_embedding import pyembed_tools

ARG_PRINT_DATA = 'show_data'
DO_PRINT_DATA = ARG_PRINT_DATA == sys.argv[-1]

start_time = datetime.now()

point_obs_data = None
input_name = sys.argv[1]
prompt = met_point_tools.get_prompt()
if len(sys.argv) == 1 or ARG_PRINT_DATA == input_name:
   # This is an example of creating a sample data
   point_obs_data = met_point_tools.get_sample_met_point_obs()
   point_obs_data.read_data([])
elif met_point_tools.is_python_prefix(input_name):
   # This is an example of calling a python script for ascii2nc
   point_obs_data = pyembed_tools.call_python(sys.argv)
else:
   # This is an example of reading MET's point observation NetCDF file
   # from ascii2nc, madis2nc, and pb2nc
   netcdf_filename = os.path.expandvars(input_name)
   args = [ netcdf_filename ]
   #args = { 'nc_name': netcdf_filename }
   point_obs_data = met_point_tools.get_nc_point_obs()
   point_obs_data.read_data(point_obs_data.get_nc_filename(args))

if point_obs_data is not None:
   met_point_data = point_obs_data.get_point_data() 
   met_point_data['met_point_data'] = point_obs_data
   print("met_point_data: ", met_point_data)
   print(met_point_data)

   if DO_PRINT_DATA:
      point_obs_data.dump()

run_time = datetime.now() - start_time

print('{p} Done python script {s} took {t}'.format(p=prompt, s=sys.argv[0], t=run_time))
