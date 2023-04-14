import os
import sys
from met.dataplane import dataplane

###########################################

print("Python Script:\t" + repr(sys.argv[0]))

   ##
   ##  input file specified on the command line
   ##  load the data into the numpy array
   ##

if len(sys.argv) != 3:
   print("ERROR: read_ascii_numpy.py -> Must specify exactly one input file and a name for the data.")
   sys.exit(1)

# Read the input file as the first argument
input_file = os.path.expandvars(sys.argv[1])
data_name  = sys.argv[2]

try:
   # Print some output to verify that this script ran
   print("Input File:\t" + repr(input_file))
   print("Data Name:\t" + repr(data_name))
   # read_2d_text_input() reads n by m text data and returns 2D numpy array
   met_data = dataplane.read_2d_text_input(input_file)
   print("Data Shape:\t" + repr(met_data.shape))
   print("Data Type:\t" + repr(met_data.dtype))
except NameError:
   print("Can't find the input file")

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

## create the metadata dictionary from the environment variable,
grid_info = os.path.expandvars(os.getenv('PYTHON_GRID'))

long_name = data_name + "_word"
attrs = dataplane.set_dataplane_attrs(data_name, valid_time, init_time,
                                      lead_time, accum_time, v_level, units,
                                      grid_info, long_name)

print("Attributes:\t" + repr(attrs))
