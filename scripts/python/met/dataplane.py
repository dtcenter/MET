import numpy as np
import os

###########################################

def load_txt(input_file, data_name)
    try:
        print("Input File:\t" + repr(input_file))
        print("Data Name:\t" + repr(data_name))
        met_data = np.loadtxt(input_file)
        print("Data Shape:\t" + repr(met_data.shape))
        print("Data Type:\t" + repr(met_data.dtype))
    except NameError:
        met_data = None
        print("Can't find the input file")
    return met_data

##
##  create the metadata dictionary
##

def get_grid_metadata(data_name):
    attrs = {

       'valid': '20050807_120000',
       'init':  '20050807_000000',
       'lead':  '120000',
       'accum': '120000',

       'name':      data_name,
       'long_name': data_name + '_word',
       'level':     'Surface',
       'units':     'None',

       'grid': {
          'type': 'Lambert Conformal',
          'hemisphere': 'N',

          'name': 'FooGrid',

          'scale_lat_1': 25.0,
          'scale_lat_2': 25.0,

          'lat_pin': 12.19,
          'lon_pin': -135.459,

          'x_pin': 0.0,
          'y_pin': 0.0,

          'lon_orient': -95.0,

          'd_km': 40.635,
          'r_km': 6371.2,

          'nx': 185,
          'ny': 129,
       }

    }
    return attrs

##
##  create the metadata dictionary from the environment variable PYTHON_GRID
##

def get_grid_metadata_from_env(data_name, grid_env_name='PYTHON_GRID'):
    attrs = {

       'valid': '20050807_120000',
       'init':  '20050807_000000',
       'lead':  '120000',
       'accum': '120000',

       'name':      data_name,
       'long_name': data_name + '_word',
       'level':     'Surface',
       'units':     'None',
       'grid':      os.path.expandvars(os.getenv(grid_env_name))
    }
    return attrs
