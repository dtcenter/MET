#!/usr/bin/env python3
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
          numeric columns 'lat', 'lon', 'elv', 'lvl', 'hgt', 'qc', 'obs'
  python3 read_met_point_obs.py <space_separated_text_file>

'''

import os
import sys
from datetime import datetime

met_base_dir = os.getenv('MET_BASE',None)
if met_base_dir is not None:
    sys.path.append(os.path.join(met_base_dir, 'python'))

from met_point_obs import met_point_obs, sample_met_point_obs
from met_point_obs_nc import nc_point_obs

DO_PRINT_DATA = False
ARG_PRINT_DATA = 'show_data'

start_time = datetime.now()

prompt = met_point_obs.get_prompt()
point_obs_data = None
if len(sys.argv) == 1 or ARG_PRINT_DATA == sys.argv[1]:
    point_obs_data = sample_met_point_obs()
    point_obs_data.read_data([])
elif met_point_obs.is_python_prefix(sys.argv[1]):
    import importlib.util

    print("{p} Python Script:\t".format(p=prompt) + repr(sys.argv[0]))
    print("{p} User Command:\t".format(p=prompt) + repr(' '.join(sys.argv[2:])))

    pyembed_module_name = sys.argv[2]
    sys.argv = sys.argv[1:]

    # append user script dir to system path
    pyembed_dir, pyembed_file = os.path.split(pyembed_module_name)
    if pyembed_dir:
        sys.path.insert(0, pyembed_dir)

    if not pyembed_module_name.endswith('.py'):
        pyembed_module_name += '.py'
    os.environ[met_point_obs.MET_ENV_RUN] = "TRUE"

    user_base = os.path.basename(pyembed_module_name).replace('.py','')

    spec = importlib.util.spec_from_file_location(user_base, pyembed_module_name)
    met_in = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(met_in)

    met_point_obs = met_in.met_point_obs
    print("met_point_obs: ", met_point_obs)
    met_point_data = met_in.met_point_data
    print("met_point_data: ", met_point_data)
    #print(hasattr("met_in: ", dir(met_in)))
    #met_point_data = met_point_obs.get_point_data()
    #met_point_data = None if met_in.get('met_point_data', None) else met_in.met_point_data
    #met_data = None if met_in.get('met_data', None) else met_in.met_data
    print(met_point_data)
else:
    netcdf_filename = sys.argv[1]
    args = [ netcdf_filename ]
    #args = { 'nc_name': netcdf_filename }
    point_obs_data = nc_point_obs()
    point_obs_data.read_data(point_obs_data.get_nc_filename(args))

if point_obs_data is not None:
    met_point_data = point_obs_data.get_point_data() 
    met_point_data['met_point_data'] = point_obs_data

    if DO_PRINT_DATA or ARG_PRINT_DATA == sys.argv[-1]:
        point_obs_data.dump()

run_time = datetime.now() - start_time

print('{p} Done python script {s} took {t}'.format(p=prompt, s=sys.argv[0], t=run_time))
