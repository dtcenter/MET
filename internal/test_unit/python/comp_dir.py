#! /usr/bin/env python3

import os
import shutil
import sys

metplus_util_path = os.getenv('METPLUS_UTIL')
sys.path.append(metplus_util_path)

from diff_util import compare_dir

# note: this is how compare_dir is imported in run_diff_docker.py in METplus...
# GITHUB_WORKSPACE = os.environ.get('GITHUB_WORKSPACE')
# # add util directory to sys path to get diff utility
# diff_util_dir = os.path.join(GITHUB_WORKSPACE,
#                              'metplus',
#                              'util')
# sys.path.insert(0, diff_util_dir)
# from diff_util import compare_dir

# Assume the command run is ./comp_dir.py MET-${1}/test_output MET-${2}/test_output

def comp_dir(truth_dir, output_dir, diff_dir='', debug=True, save_diff=True):
    """
    Compare test output between two MET versions.
    
    Essentially a wrapper for compare_dir function from METplus diff_util.
    Runs compare_dir on test and ref directories. If differences are found,
    copies relevant files into a new diff directory.

    Parameters
    -----------
    truth_dir : path-like
        Directory containing test output from reference version
    output_dir : path-like
        Directory containing test output from test version
    diff_dir : path-like, default=''
        Directory where to save files containing differences between two versions
        If not provided, will be set to output_dir/diff
    debug : bool, default=True
        More verbose option (based on settings of compare_dir function)
    save_diff : bool, default=True
        Option to save diff files (based on settings of compare_dir function)
    
    Returns
    -------
    None

    """

    print('******************************')
    print("Comparing output to truth data")
    diff_files = compare_dir(truth_dir, output_dir,
                             debug=True,
                             save_diff=True)

    # copy difference files into directory
    # so it can be easily downloaded and compared
    if diff_files:
        if not diff_dir:
            diff_dir = os.path.join(output_dir, 'diff')

        for truth_file, out_file, _, diff_file in diff_files:
            if truth_file:
                copy_to_diff_dir(
                    file_path=truth_file, 
                    file_label='_truth', 
                    data_dir=truth_dir, 
                    diff_dir=diff_dir)
            if out_file:
                copy_to_diff_dir(
                    file_path=out_file, 
                    file_label='_output', 
                    data_dir=output_dir, 
                    diff_dir=diff_dir)
            if diff_file:
                copy_to_diff_dir(
                    file_path=diff_file, 
                    file_label='', 
                    data_dir=output_dir, 
                    diff_dir=diff_dir)

    
def copy_to_diff_dir(file_path, file_label, data_dir, diff_dir):
    """
    Copy file to new directory, with label appended to file name.

    Parameters
    ----------
    file_path : path-like
        Current path of file to copy
    file_label : str
        Label to append to file name (e.g. '_truth', '_output', or '')
    data_dir : path-like
        Current directory of file to copy
    diff_dir : path-like
        Target directory for file copied

    Returns
    -------
    None
    """
    
    # replace data dir with diff directory
    # add data type identifier to filename before extension (blank for diff output)
    diff_out = file_path.replace(data_dir.removesuffix('/'), diff_dir.removesuffix('/'))
    output_path, extension = os.path.splitext(diff_out)
    output_path = f'{output_path}{file_label}{extension}'

    # create output directory if it doesn't exist
    output_dir = os.path.dirname(output_path)
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    try:
        shutil.copyfile(file_path, output_path)
    except OSError as err:
        print(f'Could not copy file {file_path} to {output_path}. {err}')                            


if __name__ == "__main__":
    import argparse

    # Assume the command run is ./comp_dir.py MET-${1}/test_output MET-${2}/test_output
    parser = argparse.ArgumentParser(description="Compare output files between two directories.")
    parser.add_argument('dir_1',
                        help='"truth" or "ref" version test_output directory')
    parser.add_argument('dir_2',
                        help='"test" version test_output directory')
    #note: the args below are not currently supported (legacy options from comp_dir.R)
    parser.add_argument('-v', default=1, choices=[0,1,2,3],
                        help='indicates verbosity level (0-3), default 1 (not currently supported)')
    parser.add_argument('-hist', default=0, choices=[0,1],
                        help='1 to produce histogram error plots for each file (not currently supported)')
    parser.add_argument('-nc_var', action='store_true',
                        help='if present, compare NetCDF variables (not currently supported)')
    parser.add_argument('-strict', action='store_true',
                        help='applies strict equality when comparing numerical values (not currently supported)')
    args = parser.parse_args()

    comp_dir(truth_dir=args.dir_1, output_dir=args.dir_2)

