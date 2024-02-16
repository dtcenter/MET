########################################################################
#
#    Read tropical cyclone diagnostics from a temporary file.
#
#    usage:  /path/to/python read_tmp_diag.py \
#            tmp_output_filename
#
########################################################################

import sys

try:
   from python_embedding import pyembed_tools
except:
   from pyembed.python_embedding import pyembed_tools

diag_data = pyembed_tools.read_tmp_diag(sys.argv[1])

