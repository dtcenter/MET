########################################################################
#
#    Adapted from a script provided by George McCabe
#    Adapted by Howard Soh
#
#    usage:  /path/to/python write_tmp_point_nc.py \
#            tmp_output_filename <user_python_script>.py <args>
#
########################################################################

import os
import sys

try:
    from python_embedding import pyembed_tools
except:
    from pyembed.python_embedding import pyembed_tools

pyembed_tools.add_python_path(__file__)
from met.point import met_point_tools

if __name__ == '__main__':
    argv_org = sys.argv[:]
    tmp_filename = sys.argv[1]
    met_in = pyembed_tools.call_python(sys.argv)

    if hasattr(met_in, 'point_data'):
        pyembed_tools.write_tmp_ascii(tmp_filename, met_in.point_data)
    elif hasattr(met_in, 'point_obs_data'):
        met_in.point_obs_data.save_ncfile(tmp_filename)
    else:
        if hasattr(met_in.met_point_data, 'point_obs_data'):
            met_in.met_point_data['point_obs_data'].save_ncfile(tmp_filename)
        else:
            tmp_point_obs = met_point_tools.get_nc_point_obs()
            tmp_point_obs.put_data(met_in.met_point_data)
            tmp_point_obs.save_ncfile(tmp_filename)
