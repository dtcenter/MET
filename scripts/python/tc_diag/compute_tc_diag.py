import os
import sys
import numpy as np
import argparse
import pathlib
from tc_diag_driver import post_resample_driver

###########################################

# Constants

# Expected STORM DATA diagnostic names
STORM_DATA_NAMES = [
    "LAT", "LON", "MAXWIND", "RMW", "MIN_SLP",
    "SHR_MAG", "SHR_HDG", "STM_SPD", "STM_HDG",
    "SST", "OHC", "TPW", "LAND",
    "850TANG", "850VORT", "200DVG"
]

# Expected SOUNDING DATA diagnostic names
SOUNDING_DATA_NAMES = [
    "T_SURF", "R_SURF", "P_SURF", "U_SURF", "V_SURF",
    "T", "R", "Z", "U", "V"
]

# Diagnostic names to be skipped
SKIP_DATA_NAMES = [
    "850RADIAL", "850DVG", "200VORT"
]

# Bad data integer value
BAD_DATA_INT = 9999

# Diagnostics name descriptions
LONG_NAME = {
    "LAT"     : "Latitude",
    "LON"     : "Longitude",
    "MAXWIND" : "Maximum Wind Speed",
    "RMW"     : "Radius of Maximum Winds",
    "MIN_SLP" : "Minimum Sea Level Pressure",
    "SHR_MAG" : "Shear Magnitude",
    "SHR_HDG" : "Shear Heading",
    "STM_SPD" : "Storm Speed",
    "STM_HDG" : "Storm Heading",
    "SST"     : "Sea Surface Temperature",
    "OHC"     : "Ocean Heat Content",
    "TPW"     : "Total Precipitable Water",
    "LAND"    : "Distance to Land",
    "850TANG" : "850 MB Tangential Wind",
    "850VORT" : "850 MB Vorticity",
    "200DVRG" : "200 MB Divergence",
    "T_SURF"  : "Surface Temperature",
    "R_SURF"  : "Surface Humidity",
    "P_SURF"  : "Surface Pressure",
    "U_SURF"  : "Surface U-Component of Wind",
    "V_SURF"  : "Surface V-Component of Wind",
    "T"       : "Temperature",
    "R"       : "Humidity",
    "Z"       : "Height",
    "U"       : "U-Component of Wind",
    "V"       : "V-Component of Wind",
    "TGRD"    : "Temperature Gradient"
}

###########################################

# Define global output dictionaries
storm_data    = {}
sounding_data = {}
custom_data   = {}
units         = {}
long_name     = LONG_NAME

###########################################

def main():

    # Handle arguments
    args = _get_args()

    config_filename = os.path.expandvars(args.config_file)
    land_filename   = os.path.expandvars(args.land_file)
    data_filename   = os.path.expandvars(args.data_file)

    # Print verbose arguments
    if args.verbose:
        print("Python Script:\t" + os.path.expandvars(sys.argv[0]))
        print("Config File:\t"   + config_filename)
        print("Land File:\t"     + land_filename)
        print("Data File:\t"     + data_filename)

    # Read config file and return a DriverConfig object
    config = post_resample_driver.config_from_file(
                 config_filename,
                 post_resample_driver.DriverConfig)

    # Store global configuration comments
    global comments
    comments = config.comment

    # Call drive to calculate the diagnostics
    try:
        results = post_resample_driver.diag_calcs(
                      config, data_filename, suppress_exceptions=True,
                      land_lut_override=land_filename)
    except:
        print("Error computing diagnostics with command (", ' '.join(sys.argv), ")", sep="")
        sys.exit(1)

    # Process storm data diagnostics in the expected order
    for d in STORM_DATA_NAMES + list(results.pressure_independent.keys()):

        # Skip diagnostics:
        # - That were not computed.
        # - Are in the skip list.
        # - Have already been processed.
        if ((d not in results.pressure_independent.keys()) or
            (d in SKIP_DATA_NAMES + list(storm_data.keys()) +
                  list(sounding_data.keys()) + list(custom_data.keys()))):
            continue

        # Store units as upper-case
        if 'units' in results.pressure_independent[d].attrs:
            units[d] = results.pressure_independent[d].attrs["units"].upper()

        # Check for bad data
        val = results.pressure_independent[d].values[0]
        if np.isnan(val):
            val = BAD_DATA_INT

        # Store value
        if d in STORM_DATA_NAMES:
            storm_data[d] = val
        elif d in SOUNDING_DATA_NAMES:
            sounding_data[d] = val
        else:
            custom_data[d] = val

    # Add units for sounding diagnostics
    for d in results.soundings.keys():

        # Store pressure levels
        levels = results.soundings[d].coords['level_hPa']

        # Store units as upper-case
        if 'units' in results.soundings[d].attrs:
            units[d] = results.soundings[d].attrs["units"].upper()

    # Loop over the sounding pressure levels
    for idx, prs in enumerate(levels):

        # Process sounding diagnostics in the expected order
        for d in SOUNDING_DATA_NAMES + list(results.soundings.keys()):

            # Left-pad pressure value to 4 digits
            d_prs = d + '_' + f'{prs.data:04}'

            # Skip diagnostics:
            # - That were not computed.
            # - Are in the skip list.
            # - Have already been processed.
            if ((d not in results.soundings.keys()) or
                (d_prs in SKIP_DATA_NAMES + list(storm_data.keys()) +
                          list(sounding_data.keys()) + list(custom_data.keys()))):
                continue

            # Check for bad data
            val = results.soundings[d].values[0,idx]
            if np.isnan(val):
                val = BAD_DATA_INT

            # Store value
            if d in SOUNDING_DATA_NAMES:
                sounding_data[d_prs] = val
            else:
                custom_data[d_prs] = val

    # Print verbose dictionary contents
    if args.verbose:
        print(f"\nSTORM DATA ({len(storm_data)}) =", storm_data)
        print(f"\nSOUNDING DATA ({len(sounding_data)}) =", sounding_data)
        print(f"\nCUSTOM DATA ({len(custom_data)}) =", custom_data)
        print(f"\nUNITS ({len(units)}) =", units)
        print(f"\nLONG_NAME ({len(long_name)}) =", long_name)
        print(f"\nCOMMENTS:\n", comments, "\n")

def _get_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Compute tropical cyclone diagnostics for a single storm location."
    )
    parser.add_argument(
        "config_file",
        type=pathlib.Path,
        help="YAML config file specifying how to process diag vars.",
    )
    parser.add_argument(
        "land_file",
        type=pathlib.Path,
        help="ASCII distance to land file.",
    )
    parser.add_argument(
        "data_file",
        type=pathlib.Path,
        help="NetCDF file containing model data resampled to cylindrical grid.",
    )
    parser.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        default=False,
        help="Print verbose log messages.",
    )

    return parser.parse_args()

# Always run the main function
main()

