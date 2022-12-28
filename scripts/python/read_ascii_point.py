import pandas as pd
import os
import sys
from met_point_obs import convert_point_data

########################################################################

print("Python Script:\t" + repr(sys.argv[0]))

##
##  input file specified on the command line
##  load the data into the numpy array
##

if len(sys.argv) != 2:
    print("ERROR: read_ascii_point.py -> Must specify exactly one input file.")
    sys.exit(1)

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

    point_data = pd.read_csv(input_file, header=None, delim_whitespace=True, keep_default_na=False,
                             names=['typ', 'sid', 'vld', 'lat', 'lon', 'elv', 'var', 'lvl', 'hgt', 'qc', 'obs'],
                             dtype={'typ':'str', 'sid':'str', 'vld':'str', 'var':'str', 'qc':'str'}).values.tolist()
    print("     point_data: Data Length:\t" + repr(len(point_data)))
    print("     point_data: Data Type:\t" + repr(type(point_data)))
    met_point_data = convert_point_data(point_data)
    print(" met_point_data: Data Type:\t" + repr(type(met_point_data)))
except NameError:
    print("Can't find the input file")
    sys.exit(1)

########################################################################
