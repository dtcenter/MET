########################################################################
#
#    Reads temporary TC diagnostics file into memory.
#
#    usage:  /path/to/python read_tmp_diag.py diag.tmp
#
########################################################################

import sys

try:
   from python_embedding import pyembed_tools
except:
   from pyembed.python_embedding import pyembed_tools

if __name__ == '__main__':
   diag_data = pyembed_tools.read_tmp_diag(sys.argv[1])
