import os
import sys
import numpy
from met.dataplane import dataplane

###########################################

def log(msg):
   dataplane.log_message(msg)

def set_dataplane_attrs():
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
   return dataplane.set_dataplane_attrs(data_name, valid_time, init_time,
                                      lead_time, accum_time, v_level, units,
                                      grid_lambert_conformal, long_name)

log("Python Script:\t" + repr(sys.argv[0]))

##
##  input file specified on the command line
##  load the data into the numpy array
##

SCRIPT_NAME = "read_ascii_numpy.py ->"
if len(sys.argv) < 3:
   dataplane.quit(f"{SCRIPT_NAME} Must specify exactly one input file and a name for the data.")
elif len(sys.argv) > 4:
   dataplane.quit(f"{SCRIPT_NAME} Have not supported arguments [{sys.argv[4:]}]")

# Read the input file as the first argument
input_file = os.path.expandvars(sys.argv[1])
data_name  = sys.argv[2]

try:
   user_fill_value = None
   try:
      if len(sys.argv) > 3:
         user_fill_value = float(sys.argv[3])
   except:
      log(f"{SCRIPT_NAME} Ignored argument {sys.argv[3]}")

   log(f"{SCRIPT_NAME} Input File:\t{repr(input_file)}")
   log(f"{SCRIPT_NAME} Data Name:\t{repr(data_name)}")
   if os.path.exists(input_file):
      # read_2d_text_input() reads n by m text data and returns 2D numpy array
      met_data = dataplane.read_2d_text_input(input_file)
      if met_data is None:
         dataplane.quit(f"{SCRIPT_NAME} Fail to build met_data from {input_file}")
      else:
         log(f"{SCRIPT_NAME} Data Shape:\t{repr(met_data.shape)}")
         log(f"{SCRIPT_NAME} Data Type:\t{repr(met_data.dtype)}")
         if user_fill_value is not None:
            met_data = numpy.ma.masked_values(met_data, user_fill_value)
         log(f"{SCRIPT_NAME} Python Type:\t{type(met_data)}")
   else:
      dataplane.quit(f"{SCRIPT_NAME} input {input_file} does exist!!!")
except:
   import traceback
   traceback.print_exc()
   dataplane.quit(f"{SCRIPT_NAME} Unknown error with {sys.argv[0]}: ")

attrs = set_dataplane_attrs()
log(f"{SCRIPT_NAME} Attributes:\t{repr(attrs)}")

# Sets fill_value if it exists at the dataplane
#attrs['fill_value'] = 255  # for letter.txt
