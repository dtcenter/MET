import os
import sys
import json
import re
import numpy as np
import xarray as xr

from importlib import util as import_util
from met.logger import logger

###########################################

class dataplane(logger):

   KEEP_XARRAY = True
   class_name = "dataplane"

   MET_FILL_VALUE = -9999.
   ATTR_USER_FILL_VALUE = 'user_fill_value'

   @staticmethod
   def call_python(argv):
       # argv[0] is the python wrapper script (caller)
       logger.log_msg(f"Module:\t{repr(argv[0])}")
       if 1 == len(argv):
          logger.quit_msg(f"User python command is missing")
          sys.exit(1)

       logger.log_msg(f"User python command:\t{repr(' '.join(argv[1:]))}")

       # argv[1] contains the user defined python script
       pyembed_module_name = argv[1]
       # append user script dir to system path
       pyembed_dir, pyembed_name = os.path.split(pyembed_module_name)
       if pyembed_dir:
          sys.path.insert(0, pyembed_dir)

       if not pyembed_module_name.endswith('.py'):
          pyembed_module_name += '.py'

       user_base = pyembed_name.replace('.py','')

       argv_org = sys.argv  # save sys.argv
       sys.argv = argv[1:]
       spec = import_util.spec_from_file_location(user_base, pyembed_module_name)
       met_in = import_util.module_from_spec(spec)
       spec.loader.exec_module(met_in)
       sys.argv = argv_org  # restore sys.argv
       return met_in

   @staticmethod
   def get_tmp_numpy_filename(tmp_filename):
      return re.sub(".json$", ".npy", tmp_filename) if tmp_filename.endswith(".json") else f'{tmp_filename}.npy'

   @staticmethod
   def is_integer(a_data):
      return isinstance(a_data, int)

   @staticmethod
   def is_xarray_dataarray(a_data):
      return isinstance(a_data, xr.DataArray)

   ##
   ##  create the metadata dictionary
   ##

   # Python dictionary items:
   #      'name': data name
   # 'long_name': descriptive name
   #     'valid': valid time (format = 'yyyymmdd_hhmmss')
   #      'init': init time (format = 'yyyymmdd_hhmmss')
   #      'lead': lead time (format = 'hhmmss')
   #     'accum': accumulation time (format = 'hhmmss')
   #     'level': vertilcal level
   #     'units': units of the data
   #      'grid': contains the grid information
   #              - a grid name (G212)
   #              - a gridded data file name
   #              - MET specific grid string, "lambert 185 129 12.19 -133.459 -95 40.635 6371.2 25 25 N"
   #              - a dictionary for the grid information
   @staticmethod
   def set_dataplane_attrs(data_name, valid_time, init_time, lead_time,
                           accum_time, v_level, units, grid_info, long_name=None):
      hdr_attrs = {

         'valid': valid_time,
         'init':  init_time,
         'lead':  lead_time,
         'accum': accum_time,

         'name':      data_name,
         'long_name': long_name if long_name is not None and long_name != "" else data_name + '_long',
         'level':     v_level,
         'units':     units,

         'grid': grid_info

      }
      return hdr_attrs

   @staticmethod
   def read_2d_text_input(input_file):
      if os.path.exists(input_file):
          met_data = np.loadtxt(input_file)
      else:
          met_data = None
      return met_data

   @staticmethod
   def read_dataplane(tmp_filename):
      met_info = {}
      with open(tmp_filename) as json_h:
          met_info['attrs'] = json.load(json_h)
      # read 2D numeric data
      numpy_dump_name = dataplane.get_tmp_numpy_filename(tmp_filename)
      met_dp_data = np.load(numpy_dump_name)
      met_info['met_data'] = met_dp_data
      return met_info

   @staticmethod
   def write_dataplane(met_in, tmp_filename):
      if hasattr(met_in.met_data, 'attrs') and met_in.met_data.attrs:
         attrs = met_in.met_data.attrs
      else:
         attrs = met_in.attrs
      with open(tmp_filename,'w') as json_h:
          json.dump(attrs, json_h)

      if isinstance(met_in.met_data, (np.ma.MaskedArray, np.ma.core.MaskedArray)):
         met_dp_data = np.ma.getdata(met_in.met_data, subok=False)
      elif isinstance(met_in.met_data, np.ndarray):
         met_dp_data = met_in.met_data
      else:
         met_dp_data = np.array(met_in.met_data)
      numpy_dump_name = dataplane.get_tmp_numpy_filename(tmp_filename)
      np.save(numpy_dump_name, met_dp_data)

   @staticmethod
   def validate_met_data(met_data, fill_value=None):
      method_name = f"{dataplane.class_name}.validate()"
      #logger.log_msg(f"{method_name} type(met_data)= {type(met_data)}")
      attrs = None
      from_xarray = False
      from_ndarray = False
      if met_data is None:
         logger.quit(f"{method_name} The met_data is None")
         sys.exit(1)

      nx, ny = met_data.shape

      met_fill_value = dataplane.MET_FILL_VALUE
      if dataplane.is_xarray_dataarray(met_data):
         from_xarray = True
         attrs = met_data.attrs
         met_data = met_data.data
         modified_met_data = True
      if isinstance(met_data, np.ndarray):
         from_ndarray = True
         met_data = np.ma.array(met_data)

      if isinstance(met_data, np.ma.MaskedArray):
         is_int_data = dataplane.is_integer(met_data[0,0]) or dataplane.is_integer(met_data[int(nx/2),int(ny/2)])
         met_data = np.ma.masked_equal(met_data, float('nan'))
         met_data = np.ma.masked_equal(met_data, float('inf'))
         if fill_value is not None:
            met_data = np.ma.masked_equal(met_data, fill_value)
         met_data = met_data.filled(int(met_fill_value) if is_int_data else met_fill_value)
      else:
         logger.log_msg(f"{method_name} unknown datatype {type(met_data)}")

      if dataplane.KEEP_XARRAY:
         return xr.DataArray(met_data,attrs=attrs) if from_xarray else met_data
      else:
         return met_data


def main(argv):
   global attrs, met_data, met_info

   met_in = dataplane.call_python(sys.argv)

   user_fill_value = None
   try:
      met_info = met_in.met_info
      attrs = met_info['attrs']
      init_met_data = met_info['met_data']
   except:
      met_info = {}
      init_met_data = met_in.met_data
      try:     # numpy and attrs
         attrs = met_in.attrs
      except:  # xarray
         attrs = init_met_data.attrs
      met_info['attrs'] = attrs
      if hasattr(met_in, 'user_fill_value'):
         fill_value = met_in.user_fill_value

   fill_value = attrs.get('fill_value', None)
   dataplane.log_msg('validating the dataplane array...')
   met_data = dataplane.validate_met_data(init_met_data, fill_value)
   met_info['met_data'] = met_data

   if os.environ.get('MET_PYTHON_DEBUG', None) is not None:
      dataplane.log_msg('--- met_data after validating ---')
      dataplane.log_msg(met_data)

if __name__ == '__main__' or __name__ == sys.argv[0]:
   main(sys.argv)
   dataplane.log_msg(f'{__name__} complete')
