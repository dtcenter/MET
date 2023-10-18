import os
import sys
import numpy as np
from tc_diag_driver import post_resample_driver

###########################################

# Constants

# Expected STORM DATA diagnostic names
STORM_DATA_NAMES = (
    "LAT", "LON", "MAXWIND", "RMW", "MIN_SLP",
    "SHR_MAG", "SHR_HDG", "STM_SPD", "STM_HDG",
    "SST", "OHC", "TPW", "LAND",
    "850TANG", "850VORT", "200DVG"
)

# Expected SOUNDING DATA diagnostic names
SOUNDING_DATA_NAMES = (
    "T_SURF", "R_SURF", "P_SURF", "U_SURF", "V_SURF",
    "T", "R", "Z", "U", "V"
)

# Bad data integer value
BAD_DATA_INT = 9999

###########################################

print("Python Script:\t" + repr(sys.argv[0]))

   ##
   ## Expect exactly 3 arguments
   ##

if len(sys.argv) != 4:
    print("ERROR: compute_tc_diag.py -> Must specify a configuration file, distance to land file, and NetCDF cylindrical coordinates file.")
    sys.exit(1)

# Configuration file
config_filename = os.path.expandvars(sys.argv[1])
try:
   print("Config File:\t" + repr(config_filename))
except NameError:
   print("Can't find the configuration file")
   sys.exit(1)

# Distance to land file
land_filename = os.path.expandvars(sys.argv[2])
try:
   print("Land File:\t" + repr(land_filename))
except NameError:
   print("Can't find the distance to land file")
   sys.exit(1)

# Input NetCDF file
data_filename = os.path.expandvars(sys.argv[3])

try:
   print("Data File:\t" + repr(data_filename))
except NameError:
   print("Can't find the input data file")
   sys.exit(1)

# Read config file and return a DriverConfig object
config = post_resample_driver.config_from_file(
    config_filename, post_resample_driver.DriverConfig)

# JHG, need to catch and handle exceptions here
# Robert will update the driver code to make sure that sufficient
# radii have been handed to the code and produce an exception if not

# JHG, note that VMAX reported here should be renamed to MAXWIND for the diag output
#    # Change VMAX to MAXWIND
#    if name == "VMAX":
#        name = "MAXWIND"

results = post_resample_driver.diag_calcs(
    config, data_filename, suppress_exceptions=True,
    land_lut_override=land_filename)

# Store results in STORM, SOUNDING, and CUSTOM data dictionaries
# based on their output file location.
storm_data    = {}
sounding_data = {}
custom_data   = {}
units         = {}

# Process pressure independent diagnostics
for d in results.pressure_independent.keys():

    # Diagonstic names are reported as upper-case
    name = d.upper()

    # Store units
    if 'units' in results.pressure_independent[d].attrs:
        units[name] = results.pressure_independent[d].attrs["units"].upper()

    # Check for bad data
    val = results.pressure_independent[d].values[0]
    if np.isnan(val):
        val = BAD_DATA_INT

    # Store value
    if name in STORM_DATA_NAMES:
        storm_data[name] = val
    elif name in SOUNDING_DATA_NAMES:
        sounding_data[name] = val
    else:
        custom_data[name] = val

# Add units for sounding diagnostics
for d in results.soundings.keys():

    # Store pressure levels
    levels = results.soundings[d].coords['level_hPa']

    # Store units for diagnostic names, all upper-case
    if 'units' in results.soundings[d].attrs:
        units[d.upper()] = results.soundings[d].attrs["units"].upper()

# Loop over the sounding pressure levels
for idx, prs in enumerate(levels):

    # Loop over the sounding diagnostics
    for d in results.soundings.keys():

        # Left-pad pressure value to 4 digits
        name_prs = d.upper() + '_' + f'{prs.data:04}'

        # Check for bad data
        val = results.soundings[d].values[0,idx]
        if np.isnan(val):
            val = BAD_DATA_INT

        # Store value
        if d.upper() in SOUNDING_DATA_NAMES:
            sounding_data[name_prs] = val
        else:
            custom_data[name_prs] = val

# Print dictionaries
print(f"\nSTORM DATA ({len(storm_data)}):\n", storm_data)
print(f"\nSOUNDING DATA ({len(sounding_data)}):\n", sounding_data)
print(f"\nCUSTOM DATA ({len(custom_data)}):\n", custom_data)
print(f"\nUNITS ({len(units)}):\n", units, "\n")
