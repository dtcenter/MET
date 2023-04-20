########################################################################
#
#    Adapted from a script provided by George McCabe
#    Adapted by Randy Bullock
#
#    usage:  /path/to/python write_tmp_dataplane.py \
#            tmp_output_filename <user_python_script>.py <args>
#
########################################################################

import sys

try:
   from python_embedding import pyembed_tools
   pyembed_tools.add_python_path(__file__)
except:
   from pyembed.python_embedding import pyembed_tools

from met.dataplane import dataplane

#def write_dataplane(met_in, netcdf_filename):
#    dataplane.write_dataplane(met_in, netcdf_filename)

if __name__ == '__main__':
   netcdf_filename = sys.argv[1]
   met_in = pyembed_tools.call_python(sys.argv)
   dataplane.write_dataplane(met_in, netcdf_filename)
