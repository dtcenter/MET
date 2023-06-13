########################################################################
#
#    Reads temporary file into memory.
#
#    usage:  /path/to/python read_tmp_tc_diag.py tc_diag.tmp
#
########################################################################

import sys

# PYTHON path for met.tc_diag is added by write_tmp_dataplane.py
from met.tc_diag import tc_diag

# read NetCDF file
tc_diag = tc_diag.read_diag(sys.argv[1])
