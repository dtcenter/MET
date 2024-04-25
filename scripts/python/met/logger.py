
###########################################

import os
import sys
import re

import numpy as np

class logger():

   PROMPT = " PYTHON:"
   ERROR_P = " ==PYTHON_ERROR=="
   INFO_P  = " ==PYTHON_INFO=="

   @staticmethod
   def append_error_prompt(msg):
      return f'{logger.ERROR_P}: {msg}'

   @staticmethod
   def error_message(msg):
      msgs = msg if isinstance(msg, list) else [msg]
      msgs.insert(0, '')
      msgs.append('')
      for a_msg in msgs:
         logger.log_message(logger.append_error_prompt(a_msg))

   #@staticmethod
   #def get_met_fill_value():
   #   return logger.MET_FILL_VALUE

   @staticmethod
   def info_message(msg):
      print(f'{logger.PROMPT} {logger.INFO_P} {msg}')

   @staticmethod
   def log_message(msg):
      print(f'{logger.PROMPT} {msg}')

   @staticmethod
   def quit(msg, do_quit=True):
      logger.quit_msg(msg)
      if do_quit:
          sys.exit(1)

   @staticmethod
   def quit_msg(msg):
      logger.error_message([msg, "Quit..."])


class met_base(logger):

   MET_FILL_VALUE = -9999.

   def convert_to_array(self, ndarray_data):
      return met_base_tools.convert_to_array(ndarray_data)

   def convert_to_ndarray(self, array_data):
      return met_base_tools.convert_to_ndarray(array_data)

   def get_met_fill_value(self):
      return met_base.MET_FILL_VALUE

   def error_msg(self, msg):
      logger.error_message(msg)

   def get_prompt(self):
      return met_base_tools.get_prompt()

   def info_msg(self, msg):
      logger.info_message(msg)

   def is_numpy_array(self, var_data):
      return isinstance(var_data, np.ndarray)

   def log_msg(self, msg):
      logger.log_message(msg)

   @staticmethod
   def get_numpy_filename(tmp_filename):
      return logger.replace_extension(tmp_filename, "json", "npy") if tmp_filename.endswith(".json") else \
             logger.replace_extension(tmp_filename, "nc", "npy") if tmp_filename.endswith(".nc") else f'{tmp_filename}.npy'

   def is_debug_enabled(self, component_name=""):
      return met_base_tools.is_debug_enabled(component_name)

   def replace_extension(self, file_name, from_ext, to_ext):
      return met_base_tools.replace_extension(file_name, from_ext, to_ext)

   def remove_file(self, file_name):
       os.remove(file_name)

   def use_netcdf_format(self):
      return met_base_tools.use_netcdf_format()

class met_base_tools(object):

   ENV_MET_KEEP_TEMP_FILE = "MET_KEEP_TEMP_FILE"
   ENV_MET_PYTHON_DEBUG = "MET_PYTHON_DEBUG"
   ENV_MET_PYTHON_TMP_FORMAT = "MET_PYTHON_TMP_FORMAT"

   @staticmethod
   def convert_to_array(ndarray_data):
      is_byte_type = False
      if 0 < len(ndarray_data):
         is_byte_type = isinstance(ndarray_data[0], (bytes, np.bytes_))
         if isinstance(ndarray_data[0], np.ndarray):
            if 0 < len(ndarray_data[0]):
               is_byte_type = isinstance(ndarray_data[0][0], (bytes, np.bytes_))
      if is_byte_type:
         array_data = []
         if isinstance(ndarray_data[0], (np.ma.MaskedArray, np.ma.core.MaskedArray)):
            for byte_data in ndarray_data:
               array_data.append(byte_data.tobytes(fill_value=' ').decode('utf-8').rstrip())
         else:
            for byte_data in ndarray_data:
               array_data.append(byte_data.decode("utf-8").rstrip())
      elif isinstance(ndarray_data, (np.ma.MaskedArray, np.ma.core.MaskedArray)):
         array_data = ndarray_data.filled(fill_value=-9999).tolist()
      elif isinstance(ndarray_data, np.ndarray):
         array_data = ndarray_data.tolist()
      else:
         array_data = ndarray_data
      return array_data

   @staticmethod
   def convert_to_ndarray(array_data):
      if isinstance(array_data, (np.ma.MaskedArray, np.ma.core.MaskedArray)):
         ndarray_data = array_data.filled(fill_value=-9999)
      elif isinstance(array_data, np.ndarray):
         ndarray_data = array_data
      else:
         ndarray_data = np.array(array_data)
      return ndarray_data

   @staticmethod
   def get_numpy_filename(tmp_filename):
      return logger.replace_extension(tmp_filename, "json", "npy") if tmp_filename.endswith(".json") else \
             logger.replace_extension(tmp_filename, "txt", "npy") if tmp_filename.endswith(".txt") else \
             logger.replace_extension(tmp_filename, "nc", "npy") if tmp_filename.endswith(".nc") else f'{tmp_filename}.npy'

   @staticmethod
   def get_prompt():
      return logger.PROMPT

   @staticmethod
   def is_debug_enabled(component_name=""):
      env_value = os.getenv(met_base_tools.ENV_MET_PYTHON_DEBUG, "").lower()
      return env_value == "all" or env_value == component_name.lower()

   @staticmethod
   def keep_temp_file():
      env_value = os.getenv(met_base_tools.ENV_MET_KEEP_TEMP_FILE, "")
      return env_value.lower() == "true" or env_value.lower() == "yes"

   @staticmethod
   def replace_extension(file_name, from_ext, to_ext):
      return re.sub(f".{from_ext}$", f".{to_ext}", file_name)

   @staticmethod
   def remove_file(file_name):
      if os.path.exists(file_name):
         os.remove(file_name)

   @staticmethod
   def remove_temp_file(file_name):
      if not met_base_tools.keep_temp_file():
         met_base_tools.remove_file(file_name)

   @staticmethod
   def use_netcdf_format():
      env_value = os.getenv(met_base_tools.ENV_MET_PYTHON_TMP_FORMAT, "")
      return env_value.lower() == "netcdf"
