from __future__ import print_function

import numpy as np
import os
import sys

###########################################

   ##
   ##  input file specified on the command line
   ##  load the data into the numpy array
   ##

if len(sys.argv) == 3:
    # Read the input file as the first argument
    input_file = os.path.expandvars(sys.argv[1])
    data_name  = sys.argv[2]
    try:
        # Print some output to verify that this script ran
        print("Input File: " + repr(input_file))
        print("Data Name : " + repr(data_name))
        met_data = np.loadtxt(input_file)
        print("Data Shape: " + repr(met_data.shape))
        print("Data Type:  " + repr(met_data.dtype))
    except NameError:
        print("Can't find the input file")
else:
    print("Must specify exactly one input file and a name for the data.")
    sys.exit(1)

###########################################

   ##
   ##  create the metadata dictionary
   ##

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

print("Attributes: " + repr(attrs))
