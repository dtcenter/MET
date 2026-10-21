"""
Validation of the command line arguments of the UGRID check scripts.

The input files must be existing regular files under the unit test
directories, MET_TEST_INPUT or MET_TEST_OUTPUT (the current directory if
neither is set). The paths are resolved with os.path.realpath() before the
check, so ".." components and symbolic links can not point outside of them.
"""

import argparse
import os
import re

ALLOWED_ROOT_ENVS = ('MET_TEST_INPUT', 'MET_TEST_OUTPUT')

# NetCDF variable names: letters, digits, '_', '.', '-' (no path separators)
VAR_NAME_PATTERN = re.compile(r'^[A-Za-z_][A-Za-z0-9_.\-]*$')


def allowed_roots():
    """Return the resolved directories which may contain the input files."""
    roots = [os.environ.get(name) for name in ALLOWED_ROOT_ENVS]
    roots = [os.path.realpath(r) for r in roots if r]
    return roots or [os.path.realpath(os.getcwd())]


def is_under(path, root):
    return os.path.commonpath([path, root]) == root


def input_file(suffixes):
    """Return an argparse type which validates an input file path."""

    def check(path):
        real = os.path.realpath(path)
        roots = allowed_roots()
        if not any(is_under(real, root) for root in roots):
            raise argparse.ArgumentTypeError(
                f"{path} is not under the allowed directories "
                f"({', '.join(roots)}). Set {' or '.join(ALLOWED_ROOT_ENVS)}.")
        if not os.path.isfile(real):
            raise argparse.ArgumentTypeError(f"{path} is not an existing file")
        if not real.endswith(suffixes):
            raise argparse.ArgumentTypeError(
                f"{path} must end with {' or '.join(suffixes)}")
        return real

    return check


def var_name(name):
    """argparse type which validates a NetCDF variable name."""
    if not VAR_NAME_PATTERN.match(name):
        raise argparse.ArgumentTypeError(f"invalid variable name: {name}")
    return name
