"""
Module Name: read_tmp_ascii.py

Read MET Point Observations from a text file created by write_tmp_point.py script
  or MET Matched Pairs from a text file created by write_tmp_mpr.py script

Point observation format:
    Message_Type, Station_ID, Valid_Time, Lat, Lon, Elevation,
    GRIB_Code or Variable_Name, Level, Height, QC_String, Observation_Value

MPR format: See documentation of the MPR line type

Version  Date
1.0.0    2021/02/18  David Fillmore  Initial version
"""

__author__ = 'David Fillmore'
__version__ = '1.0.0'


import argparse

def read_tmp_ascii(filename):
    """
    Arguments:
        filename (string): temporary file created by write_tmp_point.py or write_tmp_mpr.py

    Returns:
        (list of lists): point or mpr data
    """
    f = open(filename, 'r')
    lines = f.readlines()
    f.close()

    global ascii_data
    ascii_data = [eval(line.strip('\n')) for line in lines]
    
    return ascii_data

if __name__ == '__main__':
    """
    Parse command line arguments
    """
    parser = argparse.ArgumentParser()
    parser.add_argument('--filename', type=str)
    args = parser.parse_args()

    data = read_tmp_ascii(args.filename)
