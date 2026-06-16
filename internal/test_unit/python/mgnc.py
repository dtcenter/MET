#!/usr/bin/env python3
"""Standalone Python equivalent of mgnc.R.

Usage:
    python mgnc.py [-v] <nc_file>
"""

from __future__ import annotations

import os
import sys
from typing import Iterable

import numpy as np
from netCDF4 import Dataset


def usage() -> None:
    print(
        "usage: python mgnc.py [-v] {nc_file}\n"
        "  where -v indicates verbose mode, default off\n"
    )


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


def all_missing(values: Iterable) -> bool:
    arr = np.ma.array(values)
    flat = np.ma.ravel(arr)

    if flat.size == 0:
        return True

    mask = np.ma.getmaskarray(flat)
    data = np.ma.getdata(flat)

    if np.issubdtype(np.asarray(data).dtype, np.floating):
        missing = mask | np.isnan(data)
    else:
        # For non-floating variables, only masked values (and Python None) are treated as missing.
        missing = mask | np.equal(data, None)  # noqa: E711

    return bool(np.all(missing))


def main(argv: list[str]) -> int:
    verb, nc_path = parse_args(argv)

    if not os.path.isfile(nc_path):
        if verb:
            print(f"File not found: {nc_path}")
        return 1

    if verb:
        print(f"Opening {nc_path}")

    with Dataset(nc_path, "r") as ncfile:
        num_var = 0

        for var_name, variable in ncfile.variables.items():
            if verb:
                print(f"Checking {var_name} ... ", end="")

            values = variable[:]
            if all_missing(values):
                if verb:
                    print("all NAs")
                return 1

            if verb:
                print("OK")
            num_var += 1

        if num_var < 1:
            if verb:
                print("No variables found")
            return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
