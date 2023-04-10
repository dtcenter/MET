
########################################################################
#
#   Common APIs for python wrappers by Howard Soh (from scripts by
#   George McCabe and Randy Bullock).
#
#   This is called when an user specifies python executable (MET_PYTHON_EXE).
#   The target python object is saved as a temporary file by user defined python.
#   And the python binaries (complied with MET) reads the temporary file and
#   builds the python object for MET.
#   The temporary file can be any form with matching write/read scripts.
#   - NetCDF for gridded data and point observation data.
#   - text file (ASCII data) (MPR, point observation).
#
#   NOTE: sys.argv is changed by calling call_embedded_python
#
########################################################################

import os
import sys
import importlib.util

class pyembed_tools():

    debug = False
    class_name = "pyembed_tools"

    @staticmethod
    def add_python_path(called_file):   # called_file = __file__
        method_name = f"{pyembed_tools.class_name}.add_python_path()"
        script_dir = os.path.abspath(os.path.dirname(called_file))
        if os.path.exists(script_dir) and script_dir != os.curdir:
            if pyembed_tools.debug:
                print(f"{method_name} added python path {script_dir}")
            sys.path.append(os.path.abspath(script_dir))

        # testing purpose (to switch the python path by using MET_BASE)
        met_base_dir = os.environ.get('MET_BASE', None)
        if met_base_dir is not None:
            met_python_path = os.path.join(met_base_dir, 'python')
            if pyembed_tools.debug:
                print(f"{method_name} added python path {os.path.abspath(met_python_path)} from MET_BASE")
            sys.path.append(os.path.abspath(met_python_path))

        # add share/met/python directory to system path
        met_python_path = os.path.join(script_dir, os.pardir, 'python')
        if not os.path.exists(met_python_path):
            met_python_path = os.path.join(script_dir, os.pardir, os.pardir, 'python')
        if os.path.exists(met_python_path) and met_python_path != met_base_dir:
            if pyembed_tools.debug:
                print(f"{method_name} added python path {os.path.abspath(met_python_path)}")
            sys.path.append(os.path.abspath(met_python_path))
        else:
            print("    - {d} does not exist".format(d=met_python_path))

    @staticmethod
    def call_python(argv):
        print("Python Script:\t"  + repr(argv[0]))
        print("User Command:\t"   + repr(' '.join(argv[2:])))
        print("Temporary File:\t" + repr(argv[1]))

        # argv[0] is the python wrapper script (caller)
        # argv[1] contains the temporary filename
        # argv[2] contains the user defined python script
        pyembed_module_name = argv[2]
        sys.argv = argv[2:]

        # add share/met/python directory to system path to find met_point_obs
        pyembed_tools.add_python_path(pyembed_module_name)

        # append user script dir to system path
        pyembed_dir, _ = os.path.split(pyembed_module_name)
        if pyembed_dir:
            sys.path.insert(0, pyembed_dir)

        if not pyembed_module_name.endswith('.py'):
            pyembed_module_name += '.py'

        user_base = os.path.basename(pyembed_module_name).replace('.py','')

        spec = importlib.util.spec_from_file_location(user_base, pyembed_module_name)
        met_in = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(met_in)
        return met_in

    @staticmethod
    def read_tmp_ascii(filename):
        """
        Arguments:
            filename (string): temporary file created by write_tmp_point.py or write_tmp_mpr.py

        Returns:
            (list of lists): point or mpr data
        """
        f = open(filename, 'r')
        lines = f.readlines()
        f.close()

        ascii_data = [eval(line.strip('\n')) for line in lines]

        return ascii_data

    @staticmethod
    def write_tmp_ascii(filename, met_data):
        with open(filename, 'w') as f:
            for line in met_data:
                f.write(str(line) + '\n')


if __name__ == '__main__':
    argv_org = sys.argv[:]      # save original sys.argv
    met_in = pyembed_tools.call_python(os.path.dirname(__file__), sys.argv)
    sys.argv[:] = argv_org[:]   # restore
