
import numpy as np
import os
import sys

###########################################

   ##
   ##  load the data into the numpy array
   ##

if "FCST_INPUT" in os.environ:
    # Retrieve the environment variables for the input file
    input_file = os.path.expandvars(os.environ['FCST_INPUT'])
    try:
        met_data = np.loadtxt(input_file)
        # Print some output to verify that this script ran 
        print met_data.shape
        print met_data.dtype
    except NameError:
        print("Can't find the input file")
else:
    print("FCST_INPUT environment variable not set.")
    sys.exit(1)




###########################################

   ##
   ##  create the metadata dictionary
   ##

attrs = {

   'valid': '20050807_120000', 
   'init':  '20050807_000000', 
   'lead':   120000, 
   'accum': '120000', 

   'name':      'FCST', 
   'long_name': 'Forecast_Word', 
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


print attrs


