import os
import sys
import numpy as np
import datetime as dt
from netCDF4 import Dataset,chartostring

###########################################

   ##
   ##  input file specified on the command line
   ##  load the data into the numpy array
   ##

if len(sys.argv) != 4:
    print("Must specify exactly one input file, variable name, and summary axis (lat, lon, latlon).")
    sys.exit(1)

# Read the input file as the first argument
input_file = os.path.expandvars(sys.argv[1])
var_name   = sys.argv[2]
axis       = sys.argv[3]

try:
    # Print some output to verify that this script ran
    print("Input File: " + repr(input_file))
    print("Variable: "   + repr(var_name))
    print("Axis: "       + repr(axis))

    # Read input file
    f = Dataset(input_file, 'r')

    data = np.float64(f.variables[var_name][0,:,:,:])
    data[data > 1.0e30] = np.nan

    pvals = list(np.float64(f.variables["pressure"][:]))

    if axis == "lon":
       met_data = np.nanmean(data[::-1], axis=1).copy()
    elif axis == "lat":
       met_data = np.nanmean(data[::-1], axis=2).transpose().copy()
    elif axis == "latlon":
       met_data = np.nanmean(data[::-1], axis=1).copy()
    else:
       print("ERROR: Unsupported axis type: " + axis)
       sys.exit(1)

    print("Data Shape: " + repr(met_data.shape))
    print("Data Type:  " + repr(met_data.dtype))
except NameError:
    print("Trouble reading data from input file")

###########################################

   ##
   ##  create the metadata dictionary
   ##

init      = dt.datetime.strptime(getattr(f, "START_DATE"), "%Y-%m-%d_%H:%M:%S")
valid_ref = dt.datetime.strptime(getattr(f.variables["Time"], "units"), "hours since %Y-%m-%d %H:%M:%S")
add_hours = float(f.variables["Time"][:])
valid     = valid_ref + dt.timedelta(hours=add_hours)
lead, rem = divmod((valid-init).total_seconds(), 3600)
accum     = "00"

   # Use the first column of lats

if axis == "lon":
   lats   = list()
   lons   = list(np.float64(f.variables["XLONG"][0,0,:]))
elif axis == "lat":
   lats   = list(np.float64(f.variables["XLAT"][0,:,0]))
   lons   = list()
elif axis == "latlon":
   lats   = list(np.float64(f.variables["XLONG"][0,0,:]))
   lons   = list(np.float64(f.variables["XLAT"][0,0,:]))

levels = list(pvals)
times  = list()

attrs = {
   'valid': valid.strftime("%Y%m%d_%H%M%S"),
   'init':   init.strftime("%Y%m%d_%H%M%S"),
   'lead':   str(int(lead)),
   'accum':  accum,

   'name':      var_name,
   'long_name': str(getattr(f.variables[var_name], "description")),
   'level':     axis + "_mean",
   'units':     str(getattr(f.variables[var_name], "units")),

   'grid': {
     'type'   : "SemiLatLon",
     'name'   : axis + "_mean",
     'lats'   : lats,
     'lons'   : lons,
     'levels' : levels,
     'times'  : times
   }
}

print("Attributes: " + repr(attrs))
