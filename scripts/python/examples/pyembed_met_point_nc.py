import os
import sys

from met.point_nc import nc_point_obs

print(f"Python Script:\t{sys.argv[0]}")

arg_cnt = len(sys.argv)
if len(sys.argv) != 2:
    print("ERROR: read_met_point.py -> Specify only 1 input file")
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

    # Read 11 column text input data by using pandas package
    point_obs = nc_point_obs()
    if not point_obs.read_data(input_file):
        print(f"ERROR: Could not read MET point data file {input_file}")
        sys.exit(1)

    df = point_obs.to_pandas()
    point_data = df.values.tolist()
    print(f"     point_data: Data Length:\t{len(point_data)}")
    print(f"     point_data: Data Type:\t{type(point_data)}")
except FileNotFoundError:
    print(f"The input file {input_file} does not exist")
    sys.exit(1)
