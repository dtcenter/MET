import os
import sys

from met.point import met_point_tools as tools

########################################################################

print("Python Script:\t" + repr(sys.argv[0]))

##
##  input file specified on the command line
##  load the data into the numpy array
##

arg_cnt = len(sys.argv)
if arg_cnt < 2:
   print("ERROR: read_ascii_point.py -> Missing an input file.")
   sys.exit(1)

do_convert = False
last_index = 2
if arg_cnt > last_index:
   opt_convert = sys.argv[2]
   if opt_convert.lower() == "do_convert" or opt_convert.lower() == "convert":
      do_convert = True
      last_index += 1

if last_index < arg_cnt:
   print(" INFO: read_ascii_point.py -> Too many argument, ignored {o}.".format(
         o=' '.join(sys.argv[last_index:])))

# Read the input file as the first argument
input_file = os.path.expandvars(sys.argv[1])
try:
   print("Input File:\t" + repr(input_file))

   # Read and format the input 11-column observations:
   #   (1)  string:  Message_Type
   #   (2)  string:  Station_ID
   #   (3)  string:  Valid_Time(YYYYMMDD_HHMMSS)
   #   (4)  numeric: Lat(Deg North)
   #   (5)  numeric: Lon(Deg East)
   #   (6)  numeric: Elevation(msl)
   #   (7)  string:  Var_Name(or GRIB_Code)
   #   (8)  numeric: Level
   #   (9)  numeric: Height(msl or agl)
   #   (10) string:  QC_String
   #   (11) numeric: Observation_Value

   # Read 11 column text input data by using pandas package
   point_data = tools.read_text_point_obs(input_file)
   print("     point_data: Data Length:\t" + repr(len(point_data)))
   print("     point_data: Data Type:\t" + repr(type(point_data)))
   if do_convert:
      # Convert 11 column list to MET's internal python instance
      met_point_data = tools.convert_point_data(point_data)
      print(" met_point_data: Data Type:\t" + repr(type(met_point_data)))
except FileNotFoundError:
   print(f"The input file {input_file} does not exist")
   sys.exit(1)

########################################################################
