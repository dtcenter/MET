########################################################################
#
#    Reads temporary point obs. file into memory.
#
#    usage:  /path/to/python read_tmp_point_nc.py tmp_output_filename
#
########################################################################

import sys

from met.point import met_point_tools
try:
    from python_embedding import pyembed_tools
except:
    from pyembed.python_embedding import pyembed_tools

#pyembed_tools.add_python_path(__file__)

input_filename = sys.argv[1]

# read NetCDF file
print('{p}  reading {f}'.format(p=met_point_tools.get_prompt(), f=input_filename))
try:
    point_obs_data = met_point_tools.get_nc_point_obs()
    point_obs_data.read_data(input_filename)

    met_point_data = point_obs_data.get_point_data()
    met_point_data['met_point_data'] = point_obs_data
except:
    point_data = pyembed_tools.read_tmp_ascii(input_filename)
