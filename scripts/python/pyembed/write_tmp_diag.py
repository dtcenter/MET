########################################################################
#
#    Runs python diagnostics script and writes a temporary file.
#
#    usage:  /path/to/python write_tmp_diag.py \
#            tmp_output_filename <user_python_script>.py <args>
#
########################################################################

import sys

try:
   from python_embedding import pyembed_tools
except:
   from pyembed.python_embedding import pyembed_tools

if __name__ == '__main__':
   tmp_filename = sys.argv[1]
   met_in = pyembed_tools.call_python(sys.argv)
   pyembed_tools.write_tmp_diag(tmp_filename, met_in.diag_data)
