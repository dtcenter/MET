"""
Import standard modules
"""
import os
import sys
import argparse
import logging
import math
from glob import glob
import numpy as np

"""
Parse command line arguments
"""
parser = argparse.ArgumentParser()
parser.add_argument('--datadir', type=str,
    default=os.getenv('DATA_DIR'),
    help='top-level data directory (default $DATA_DIR)')
parser.add_argument('--filename', type=str,
    required=True,
    help='netcdf file name')
parser.add_argument('--logfile', type=str, 
    default=sys.stdout,
    help='log file (default stdout)')
parser.add_argument('--debug', action='store_true',
    help='set logging level to debug')
args = parser.parse_args()

"""
Setup logging
"""
logging_level = logging.DEBUG if args.debug else logging.INFO
logging.basicConfig(stream=args.logfile, level=logging_level)
logging.info(args.datadir)
logging.info(args.filename)
