import os
import sys
import numpy as np
import argparse
import pathlib
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

# Diagnostic names to be skipped
SKIP_DATA_NAMES = (
    "850RADIAL", "850DVRG", "200VORT"
)

# Bad data integer value
BAD_DATA_INT = 9999

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


    # JHG, need to catch and handle exceptions here
    # Robert will update the driver code to make sure that sufficient
    # radii have been handed to the code and produce an exception if not

    # Call drive to calculate the diagnostics
    try:
        results = post_resample_driver.diag_calcs(
                      config, data_filename, suppress_exceptions=True,
                      land_lut_override=land_filename)
    except:
        print("Error computing diagnostics with command (", ' '.join(sys.argv), ")", sep="")
        sys.exit(1)

    # Store results in STORM, SOUNDING, and CUSTOM dictionaries
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
        if name in SKIP_DATA_NAMES:
            continue
        elif name in STORM_DATA_NAMES:
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

    # Print verbose dictionary contents
    if args.verbose:
        print(f"\nSTORM DATA ({len(storm_data)}) =", storm_data)
        print(f"\nSOUNDING DATA ({len(sounding_data)}) =", sounding_data)
        print(f"\nCUSTOM DATA ({len(custom_data)}) =", custom_data)
        print(f"\nUNITS ({len(units)}) =", units, "\n")

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

if __name__ == "__main__":
    main()

