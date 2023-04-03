import os
import sys
import xarray as xr
from met.dataplane import load_txt, get_grid_metadata

###########################################

print("Python Script:\t" + repr(sys.argv[0]))

   ##
   ##  input file specified on the command line
   ##  load the data into the numpy array
   ##

if len(sys.argv) != 3:
    print("ERROR: read_ascii_xarray.py -> Must specify exactly one input file and a name for the data.")
    sys.exit(1)

# Read the input file as the first argument
input_file = os.path.expandvars(sys.argv[1])
data_name  = sys.argv[2]

met_data = load_txt(input_file, data_name)

###########################################

   ##
   ##  create the metadata dictionary
   ##

attrs = get_grid_metadata(data_name)

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

# Create met_data and specify attrs because XR doesn't persist them.
met_data = xr.DataArray(ds.fcst, attrs=ds.attrs)
