import os
import sys
import xarray as xr
from met.dataplane import dataplane

###########################################

def log(msg):
   dataplane.log_msg(msg)

log("Python Script:\t" + repr(sys.argv[0]))

   ##
   ##  input file specified on the command line
   ##  load the data into the numpy array
   ##

if len(sys.argv) != 3:
   dataplane.quit("read_ascii_xarray.py -> Must specify exactly one input file and a name for the data.")

# Read the input file as the first argument
input_file = os.path.expandvars(sys.argv[1])
data_name  = sys.argv[2]

try:
   log("Input File:\t" + repr(input_file))
   log("Data Name:\t" + repr(data_name))
   if os.path.exists(input_file):
      # read_2d_text_input() reads n by m text data and returns 2D numpy array
      met_data = dataplane.read_2d_text_input(input_file)
      if met_data is None:
         dataplane.quit(f" Fail to build met_data from {input_file}")
      else:
         log("Data Shape:\t" + repr(met_data.shape))
         log("Data Type:\t" + repr(met_data.dtype))
   else:
      dataplane.quit(f"input {input_file} does exist!!!")
except:
   met_data = None
   import traceback
   traceback.print_exc()
   dataplane.quit(f"Unknown error with {sys.argv[0]}: ")

###########################################

   ##
   ##  create the metadata dictionary
   ##

# attrs is a dictionary which contains attributes describing the dataplane.
# attrs should have 9 items, each of data type string:
#      'name': data name
# 'long_name': descriptive name
#     'valid': valid time (format = 'yyyymmdd_hhmmss')
#      'init': init time (format = 'yyyymmdd_hhmmss')
#      'lead': lead time (format = 'hhmmss')
#     'accum': accumulation time (format = 'hhmmss')
#     'level': vertilcal level
#     'units': units of the data
#      'grid': contains the grid information
#              - a grid name (G212)
#              - a gridded data file name
#              - MET specific grid string, "lambert 185 129 12.19 -133.459 -95 40.635 6371.2 25 25 N"
#              - a dictionary for the grid information

valid_time = '20050807_120000'
init_time = '20050807_000000'
lead_time = '120000'
accum_time = '120000'
v_level = 'Surface'
units = 'None'

grid_lambert_conformal = {
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

long_name = data_name + "_word"
attrs = dataplane.set_dataplane_attrs(data_name, valid_time, init_time,
                                      lead_time, accum_time, v_level, units,
                                      grid_lambert_conformal)

print("Attributes:\t" + repr(attrs))

# Create an xarray DataArray object
da = xr.DataArray(met_data)
ds = xr.Dataset({"fcst":da})

# Add the attributes to the dataarray object
ds.attrs = attrs

# Delete the local variable attrs to mimic the real world,
# where a user will rely on da.attrs rather than construct it themselves
del attrs

# Delete the met_data variable, and reset it to be the Xarray object
del met_data

# Sets fill_value/min_value/max_value if it exists
#ds.attrs['fill_value'] = 255

# Create met_data and specify attrs because XR doesn't persist them.
met_data = xr.DataArray(ds.fcst, attrs=ds.attrs)
