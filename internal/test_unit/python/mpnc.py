#!/usr/bin/env python3
"""Standalone Python equivalent of mpnc.R.

Usage:
    python mpnc.py [-v] <nc_file>
"""

from __future__ import annotations

import os
import sys

import numpy as np
from netCDF4 import Dataset


HEADER_VARS = {"hdr_typ", "hdr_sid", "hdr_vld", "obs_qty"}
ARRAY_VARS = {"hdr_arr", "obs_arr"}
REQUIRED_1D_VARS = {"hdr_lat", "hdr_lon", "obs_hid", "obs_val"}
OPTIONAL_1D_VARS = {"hdr_elv", "obs_vid", "obs_gc", "obs_lvl", "obs_hgt"}


def usage() -> None:
    print(
        "usage: python mpnc.py [-v] {nc_file}\n"
        "  where -v indicates verbose mode, default off\n"
    )


def check_size_and_return(var):
    do_return = False
    size = total_size(var)
    if size <= zero_count_non_na(var):
        if verb:
            print("all zeroes")
        do_return = True
    if size <= na_count(var):
        if verb:
            print("all NAs")
        do_return = True
    if verb:
        print("OK")
    return do_return


def parse_args(argv: list[str]) -> tuple[bool, str]:
    verb = False

    if len(argv) == 2:
        if argv[0] != "-v":
            usage()
            raise SystemExit(1)
        verb = True
        nc_file = argv[1]
    elif len(argv) == 1:
        nc_file = argv[0]
    else:
        usage()
        raise SystemExit(1)

    return verb, nc_file


def to_masked_array(values) -> np.ma.MaskedArray:
    return np.ma.array(values)


def total_size(arr: np.ma.MaskedArray) -> int:
    return int(arr.size)


def na_count(arr: np.ma.MaskedArray) -> int:
    mask = np.ma.getmaskarray(arr)
    data = np.ma.getdata(arr)
    if np.issubdtype(np.asarray(data).dtype, np.floating):
        return int(np.count_nonzero(mask | np.isnan(data)))
    return int(np.count_nonzero(mask))


def zero_count_non_na(arr: np.ma.MaskedArray) -> int:
    compressed = np.ma.compressed(arr)
    if compressed.size == 0:
        return 0
    return int(np.count_nonzero(compressed == 0))


def main(argv: list[str]) -> int:
    verb, nc_path = parse_args(argv)

    if not os.path.isfile(nc_path):
        if verb:
            print(f"File not found: {nc_path}")
        return 1

    if verb:
        print(f"Opening {nc_path}")

    int_num_hdr_var = 0
    int_num_arr_var = 0
    int_num_1d_var = 0

    with Dataset(nc_path, "r") as ncfile:
        for var_name, variable in ncfile.variables.items():
            var = to_masked_array(variable[:])

            if verb:
                print(f"Checking {var_name} ... ", end="")

            if var_name in HEADER_VARS:
                if total_size(var) < 1:
                    if verb:
                        print(f"{var_name} empty")
                    return 1
                if verb:
                    print("OK")
                int_num_hdr_var += 1

            elif var_name in ARRAY_VARS:
                if check_size_and_return(var):
                    return 1
                int_num_arr_var += 1

            elif var_name in REQUIRED_1D_VARS:
                if check_size_and_return(var):
                    return 1
                int_num_1d_var += 1

            elif var_name in OPTIONAL_1D_VARS:
                check_size_and_return(var):
                int_num_1d_var += 1

            elif verb:
                print("ignored")

    if int_num_hdr_var not in {3, 4}:
        if verb:
            print(f"Unexpected number of header variables ({int_num_hdr_var})")
        return 1

    if int_num_arr_var != 2 and int_num_1d_var != 8:
        if verb:
            print(
                f"Unexpected number of array variables ({int_num_arr_var}) "
                f"or 1D variables ({int_num_1d_var})"
            )
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
