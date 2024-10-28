import os
import sys

from met.point_nc import nc_point_obs

# Description: Reads a point observation NetCDF file created by MET and passes
# the data to another MET tool via Python Embedding. This script can be copied
# to perform modifications to the data before it is passed to MET.
# Example: plot_point_obs "PYTHON_NUMPY=pyembed_met_point_nc.py in.nc" out.ps
# Contact: George McCabe <mccabe@ucar.edu>

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

print(f"Python Script:\t{sys.argv[0]}")

if len(sys.argv) != 2:
    print("ERROR: pyembed_met_point_nc.py -> Specify only 1 input file")
    sys.exit(1)

# Read the input file as the first argument
input_file = os.path.expandvars(sys.argv[1])
print("Input File:\t" + repr(input_file))

# Read MET point observation NetCDF file
try:
    point_obs = nc_point_obs(input_file)
except TypeError:
    print(f"ERROR: Could not read MET point data file {input_file}")
    sys.exit(1)

# convert point observation data to a pandas DataFrame
df = point_obs.to_pandas()

##################################################
# perform any modifications to the data here     #
##################################################

# convert pandas DataFrame to list format that is expected by MET
point_data = df.values.tolist()
print(f"     point_data: Data Length:\t{len(point_data)}")
print(f"     point_data: Data Type:\t{type(point_data)}")
