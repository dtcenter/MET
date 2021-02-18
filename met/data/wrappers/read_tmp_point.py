"""
Module Name: read_tmp_point.py

Read MET Point Observations from a text file created by write_tmp_point.py script.

    Message_Type, Station_ID, Valid_Time, Lat, Lon, Elevation,
    GRIB_Code or Variable_Name, Level, Height, QC_String, Observation_Value

Version  Date
1.0.0    2021/02/18  David Fillmore  Initial version
"""

__author__ = 'David Fillmore'
__version__ = '1.0.0'
__email__ = 'met_help@ucar.edu'

import argparse

def read_tmp_point(filename):
    """
    Arguments:
        filename (string): temporary file created by write_tmp_point.py

    Returns:
        (list of lists): point data
    """
    f = open(filename, 'r')
    lines = f.readlines()
    f.close()

    point_data = [eval(line.strip('\n')) for line in lines]

    return point_data

if __name__ == '__main__':
    """
    Parse command line arguments
    """
    parser = argparse.ArgumentParser()
    parser.add_argument('--filename', type=str)
    args = parser.parse_args()

    point_data = read_tmp_point(args.filename)
