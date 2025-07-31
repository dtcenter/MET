#! /usr/bin/env python3
import os
import sys

module_path = os.path.join(os.getenv('LOCAL_METPLUS_DIR'), '.github', 'jobs')
sys.path.append(module_path)

from docker_utils import VERSION_EXT

print(VERSION_EXT)