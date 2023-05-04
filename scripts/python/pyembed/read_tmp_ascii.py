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

try:
   from python_embedding import pyembed_tools
except:
   from pyembed.python_embedding import pyembed_tools

def read_tmp_ascii(filename):
   global ascii_data   # defined at python_handler.cc (tmp_list_name)
   ascii_data = pyembed_tools.read_tmp_ascii(filename)
   return ascii_data

if __name__ == '__main__':
   """
   Parse command line arguments
   """
   parser = argparse.ArgumentParser()
   parser.add_argument('--filename', type=str)
   args = parser.parse_args()

   data = read_tmp_ascii(args.filename)
