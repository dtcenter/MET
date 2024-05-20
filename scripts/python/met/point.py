'''
Created on Nov 10, 2021

@author: hsoh

- This is the base class and the customized script should extend the met_point_obs.
- The customized script (for example "custom_reader") must implement
  "def read_data(self, args)" which fills the array variables at __init__().
- The args can be 1) single string argument, 2) the list of arguments,
  or 3) the dictionary of arguments.
- Either "point_data" or "met_point_data" python object (variable) must set:
  + "point_data" is from 11 column text input
  + "met_point_data" is array of neaders and observation data.
  + "point_obs_data" is an optional to use custom python EXE.
     It's a python instance which processes the point observation data
- The customized script is expected to include following codes:

    + Note: csv_point_obs is an example of met_point_data, not point_data

    + Example of "point_data": see met_point_tools.read_text_point_obs()

def read_custom_data(data_filename):
   # Implemente here
   return the array of 11 column data

# prepare arguments for the customized script
data_filename = sys.arg[1]
point_data = read_custom_data(data_filename)


    + Example of "met_point_data": see csv_point_obs

from met.point import met_point_obs

class custom_reader(met_point_obs):

    def read_data(data_filename):
        # Implemente here

# prepare arguments for the customized script
data_filename = sys.argv[1]
point_obs_data = custom_reader()
point_obs_data.read_data(data_filename)
met_point_data = point_obs_data.get_point_data()

'''

import os
import json
from abc import ABC, abstractmethod

import numpy as np
import pandas as pd

from met.logger import met_base, met_base_tools


COUNT_SHOW = 30


class met_base_point(met_base):
   '''
   classdocs
   '''

   COMPONENT_NAME = "met_point"

   def __init__(self, use_var_id=True):
      '''
      Constructor
      '''
      self.count_info = ""
      self.input_name = None
      self.ignore_input_file = False
      self.use_var_id = use_var_id    # True if variable index, False if GRIB code
      self.error_msgs  = ""
      self.has_error  = False

      # Header
      self.nhdr = 0
      self.npbhdr = 0
      self.nhdr_typ = 0   # type table
      self.nhdr_sid = 0   # station_id table
      self.nhdr_vld = 0   # valid time strings
      self.hdr_typ = []   # (nhdr) integer
      self.hdr_sid = []   # (nhdr) integer
      self.hdr_vld = []   # (nhdr) integer
      self.hdr_lat = []   # (nhdr) float
      self.hdr_lon = []   # (nhdr) float
      self.hdr_elv = []   # (nhdr) float
      self.hdr_typ_table = [] # (nhdr_typ, mxstr2) string
      self.hdr_sid_table = [] # (nhdr_sid, mxstr2) string
      self.hdr_vld_table = [] # (nhdr_vld, mxstr) string

      #Observation data
      self.nobs = 0
      self.nobs_qty = 0
      self.nobs_var = 0
      self.obs_qty = []  # (nobs_qty) integer, index of self.obs_qty_table
      self.obs_hid = []  # (nobs) integer
      self.obs_vid = []  # (nobs) integer, veriable index from self.obs_var_table or GRIB code
      self.obs_lvl = []  # (nobs) float
      self.obs_hgt = []  # (nobs) float
      self.obs_val = []  # (nobs) float
      self.obs_qty_table = []  # (nobs_qty, mxstr) string
      self.obs_var_table = []  # (nobs_var, mxstr2) string, required if self.use_var_id is True
      self.obs_var_unit  = []  # (nobs_var, mxstr2) string, optional if self.use_var_id is True
      self.obs_var_desc  = []  # (nobs_var, mxstr3) string, optional if self.use_var_id is True

      # Optional variables for PREPBUFR, not supported yet
      self.hdr_prpt_typ  = [] # optional
      self.hdr_irpt_typ  = [] # optional
      self.hdr_inst_typ  = [] # optional

   def add_error_msg(self, error_msg):
      self.has_error  = True
      self.error_msg(error_msg)
      if 0 == len(self.error_msgs):
          self.error_msgs = error_msg
      else:
          self.error_msgs = "{m1}\n{m2}".format(m1=self.error_msgs, m2=error_msg)

   def add_error_msgs(self, error_msgs):
      self.has_error  = True
      for error_msg in error_msgs:
         self.add_error_msg(error_msg)

   def check_data_member_float(self, local_var, var_name):
      method_name = f"{self.__class__.__name__}.check_data_member_float()"
      if 0 == len(local_var):
         self.add_error_msg(f"{method_name} {var_name} is empty")
      elif isinstance(local_var, list):
         if isinstance(local_var[0], str) and not self.is_number(local_var[0]):
            self.add_error_msg(f"{method_name} Not supported data type: {type(local_var[0])} for {var_name}[0], string type, not a number (int or float only)")
         elif 0 > str(type(local_var[0])).find('numpy') and not isinstance(local_var[0], (int, float)):
            self.add_error_msg(f"{method_name} Not supported data type: {type(local_var[0])} for {var_name}[0] (int or float only)")
      elif not self.is_numpy_array(local_var):
         self.add_error_msg(f"{method_name} Not supported data type ({type(local_var)}) for {var_name} (list and numpy.ndarray)")

   def check_data_member_int(self, local_var, var_name):
      method_name = f"{self.__class__.__name__}.check_data_member_int()"
      if 0 == len(local_var):
         self.add_error_msg(f"{method_name} {var_name} is empty")
      elif isinstance(local_var, list):
         if isinstance(local_var[0], str) and not self.is_number(local_var[0]):
            self.add_error_msg(f"{method_name} Not supported data type: {type(local_var[0])} for {var_name}[0], string type, not a number (int only)")
         elif 0 > str(type(local_var[0])).find('numpy') and not isinstance(local_var[0], int):
            self.add_error_msg(f"{method_name} Not supported data type: {type(local_var[0])} for {var_name}[0] (int only)")
      elif not self.is_numpy_array(local_var):
         self.add_error_msg(f"{method_name} Not supported data type ({type(local_var)}) for {var_name} (list and numpy.ndarray)")

   def check_data_member_string(self, local_var, var_name):
      method_name = f"{self.__class__.__name__}.check_data_member_string()"
      if 0 == len(local_var):
         self.add_error_msg(f"{method_name} {var_name} is empty")
      elif not isinstance(local_var, (list)):
         self.add_error_msg(f"{method_name} Not supported data type ({type(local_var)}) for {var_name} (list)")

   def check_point_data(self):
      method_name = f"{self.__class__.__name__}.check_point_data()"
      if not self.ignore_input_file and self.input_name is not None and not os.path.exists(self.input_name):
         self.add_error_msg(f'{method_name} The input {self.input_name} does not exist')
      else:
         self.check_data_member_int(self.hdr_typ,'hdr_typ')
         self.check_data_member_int(self.hdr_sid,'hdr_sid')
         self.check_data_member_int(self.hdr_vld,'hdr_vld')
         self.check_data_member_float(self.hdr_lat,'hdr_lat')
         self.check_data_member_float(self.hdr_lon,'hdr_lon')
         self.check_data_member_float(self.hdr_elv,'hdr_elv')
         self.check_data_member_string(self.hdr_typ_table,'hdr_typ_table')
         self.check_data_member_string(self.hdr_sid_table,'hdr_sid_table')
         self.check_data_member_string(self.hdr_vld_table,'hdr_vld_table')

         self.check_data_member_int(self.obs_qty,'obs_qty')
         self.check_data_member_int(self.obs_hid,'obs_hid')
         self.check_data_member_int(self.obs_vid,'obs_vid')
         self.check_data_member_float(self.obs_lvl,'obs_lvl')
         self.check_data_member_float(self.obs_hgt,'obs_hgt')
         self.check_data_member_float(self.obs_val,'obs_val')
         self.check_data_member_string(self.obs_qty_table,'obs_qty_table')
         if self.use_var_id:
            self.check_data_member_string(self.obs_var_table,'obs_var_table')

   #def convert_to_numpy(self, value_list):
   #   return met_point_tools.convert_to_ndarray(value_list)

   def dump(self):
      met_point_tools.print_point_data(self.get_point_data())

   def get_count_string(self):
      return f' nobs={self.nobs} nhdr={self.nhdr} ntyp={self.nhdr_typ} nsid={self.nhdr_sid} nvld={self.nhdr_vld} nqty={self.nobs_qty} nvar={self.nobs_var}'

   def get_point_data(self):
      if self.nhdr <= 0:
         self.nhdr = len(self.hdr_lat)
      if self.nobs <= 0:
         self.nobs = len(self.obs_val)
      if self.nhdr_typ <= 0:
         self.nhdr_typ = len(self.hdr_typ_table)
      if self.nhdr_sid <= 0:
         self.nhdr_sid = len(self.hdr_sid_table)
      if self.nhdr_vld <= 0:
         self.nhdr_vld = len(self.hdr_vld_table)
      if self.npbhdr <= 0:
         self.npbhdr = len(self.hdr_prpt_typ)
      if self.nobs_qty <= 0:
         self.nobs_qty = len(self.obs_qty_table)
      if self.nobs_var <= 0:
         self.nobs_var = len(self.obs_var_table)
      self.check_point_data()

      if not self.is_numpy_array(self.hdr_typ):
         self.hdr_typ = self.convert_to_ndarray(self.hdr_typ)
      if not self.is_numpy_array(self.hdr_sid):
         self.hdr_sid = self.convert_to_ndarray(self.hdr_sid)
      if not self.is_numpy_array(self.hdr_vld):
         self.hdr_vld = self.convert_to_ndarray(self.hdr_vld)
      if not self.is_numpy_array(self.hdr_lat):
         self.hdr_lat = self.convert_to_ndarray(self.hdr_lat)
      if not self.is_numpy_array(self.hdr_lon):
         self.hdr_lon = self.convert_to_ndarray(self.hdr_lon)
      if not self.is_numpy_array(self.hdr_elv):
         self.hdr_elv = self.convert_to_ndarray(self.hdr_elv)

      if not self.is_numpy_array(self.obs_qty):
         self.obs_qty = self.convert_to_ndarray(self.obs_qty)
      if not self.is_numpy_array(self.obs_hid):
         self.obs_hid = self.convert_to_ndarray(self.obs_hid)
      if not self.is_numpy_array(self.obs_vid):
         self.obs_vid = self.convert_to_ndarray(self.obs_vid)
      if not self.is_numpy_array(self.obs_lvl):
         self.obs_lvl = self.convert_to_ndarray(self.obs_lvl)
      if not self.is_numpy_array(self.obs_hgt):
         self.obs_hgt = self.convert_to_ndarray(self.obs_hgt)
      if not self.is_numpy_array(self.obs_val):
         self.obs_val = self.convert_to_ndarray(self.obs_val)

      self.count_info = self.get_count_string()
      self.met_point_data = self
      return self.__dict__

#   def get_prompt(self):
#      return met_point_tools.get_prompt()

   def is_number(self, num_str):
      return num_str.replace('-','1').replace('+','2').replace('.','3').isdigit()

   def put_data(self, point_obs_dict):
      self.use_var_id = point_obs_dict['use_var_id']
      self.hdr_typ = point_obs_dict['hdr_typ']
      self.hdr_sid = point_obs_dict['hdr_sid']
      self.hdr_vld = point_obs_dict['hdr_vld']
      self.hdr_lat = point_obs_dict['hdr_lat']
      self.hdr_lon = point_obs_dict['hdr_lon']
      self.hdr_elv = point_obs_dict['hdr_elv']
      self.hdr_typ_table = point_obs_dict['hdr_typ_table']
      self.hdr_sid_table = point_obs_dict['hdr_sid_table']
      self.hdr_vld_table = point_obs_dict['hdr_vld_table']

      #Observation data
      self.obs_qty = point_obs_dict['obs_qty']
      self.obs_hid = point_obs_dict['obs_hid']
      self.obs_lvl = point_obs_dict['obs_lvl']
      self.obs_hgt = point_obs_dict['obs_hgt']
      self.obs_val = point_obs_dict['obs_val']
      self.obs_vid = point_obs_dict['obs_vid']
      self.obs_var_table = point_obs_dict['obs_var_table']
      self.obs_qty_table = point_obs_dict['obs_qty_table']
      po_array = point_obs_dict.get('obs_unit', None)
      if po_array is not None:
         self.obs_var_unit = po_array
      po_array = point_obs_dict.get('obs_desc', None)
      if po_array is not None:
         self.obs_var_desc = po_array

      po_array = point_obs_dict.get('hdr_prpt_typ', None)
      if po_array is not None:
         self.hdr_prpt_typ = po_array
      po_array = point_obs_dict.get('hdr_irpt_typ', None)
      if po_array is not None:
         self.hdr_irpt_typ = po_array
      po_array = point_obs_dict.get('hdr_inst_typ', None)
      if po_array is not None:
         self.hdr_inst_typ = po_array

   def read_point_data(self, tmp_filename):
      method_name = f"{self.__class__.__name__}.read_point_data()"
      if met_base_tools.use_netcdf_format():
         from met.point_nc import nc_point_obs

         met_point_data = nc_point_obs()
         met_point_data.read_data(tmp_filename)
         self.put_data(met_point_data.get_point_data())
         if met_base_tools.is_debug_enabled("point"):
            met_base.log_message(f"{method_name} Read from a temporary NetCDF file (point)")
      else:
         self.read_point_data_json_numpy(tmp_filename)

   def read_point_data_json_numpy(self, tmp_filename):
      method_name = f"{self.__class__.__name__}.read_point_data_json_numpy()"
      if met_base_tools.is_debug_enabled("point"):
         met_base.log_message(f"{method_name} Read from a temporary JSON file and a temporary numpy output (point)")

      with open(tmp_filename) as json_fh:
         json_dict = json.load(json_fh)
   
         self.use_var_id = json_dict['use_var_id']
         self.nhdr = json_dict['nhdr']
         self.nobs = json_dict['nobs']
         self.hdr_typ_table = json_dict['hdr_typ_table']
         self.hdr_sid_table = json_dict['hdr_sid_table']
         self.hdr_vld_table = json_dict['hdr_vld_table']
         self.obs_var_table = json_dict['obs_var_table']
         self.obs_qty_table = json_dict['obs_qty_table']
         self.obs_var_unit = json_dict['obs_var_unit']
         self.obs_var_desc = json_dict['obs_var_desc']
         self.hdr_prpt_typ = json_dict['hdr_prpt_typ']
         self.hdr_irpt_typ = json_dict['hdr_irpt_typ']
         self.hdr_inst_typ = json_dict['hdr_inst_typ']

      # read 2D numeric data
      numpy_dump_name = met_base_tools.get_numpy_filename(tmp_filename)
      point_array_list = np.load(numpy_dump_name)

      # Header data
      self.hdr_typ = point_array_list[0,:self.nhdr]
      self.hdr_sid = point_array_list[1,:self.nhdr]
      self.hdr_vld = point_array_list[2,:self.nhdr]
      self.hdr_lat = point_array_list[3,:self.nhdr]
      self.hdr_lon = point_array_list[4,:self.nhdr]
      self.hdr_elv = point_array_list[5,:self.nhdr]
      # Observation data
      self.obs_hid = point_array_list[6]
      self.obs_lvl = point_array_list[7]
      self.obs_hgt = point_array_list[8]
      self.obs_val = point_array_list[9]
      self.obs_vid = point_array_list[10]
      self.obs_qty = point_array_list[11]
      
      if numpy_dump_name != tmp_filename:
         met_base_tools.remove_temp_file(numpy_dump_name)

   def write_point_data(self, tmp_filename):
      if met_base_tools.use_netcdf_format():
         from met.point_nc import nc_point_obs

         nc_point_obs.write_nc_file(tmp_filename, self)
         if met_base_tools.is_debug_enabled("point"):
            met_base.log_message(f"Save to a temporary NetCDF file (point)")
      else:
         self.write_point_data_json_numpy(tmp_filename)

   def write_point_data_json_numpy(self, tmp_filename):
      method_name = f"{self.__class__.__name__}.write_point_data_json_numpy()"
      if met_base_tools.is_debug_enabled("dataplane"):
         met_base.log_message(f"{method_name} Save to a temporary JSON file and a temporary numpy output (point)")

      self.nhdr = len(self.hdr_sid)
      self.nobs = len(self.obs_hid)

      json_dict = {}
      json_dict['use_var_id'] = self.use_var_id
      json_dict['nhdr'] = self.nhdr
      json_dict['nobs'] = self.nobs
      json_dict['hdr_typ_table'] = self.convert_to_array(self.hdr_typ_table)
      json_dict['hdr_sid_table'] = self.convert_to_array(self.hdr_sid_table)
      json_dict['hdr_vld_table'] = self.convert_to_array(self.hdr_vld_table)
      json_dict['obs_var_table'] = self.convert_to_array(self.obs_var_table)
      json_dict['obs_qty_table'] = self.convert_to_array(self.obs_qty_table)
      json_dict['obs_var_unit'] = self.convert_to_array(self.obs_var_unit)
      json_dict['obs_var_desc'] = self.convert_to_array(self.obs_var_desc)
      json_dict['hdr_prpt_typ'] = self.convert_to_array(self.hdr_prpt_typ)
      json_dict['hdr_irpt_typ'] = self.convert_to_array(self.hdr_irpt_typ)
      json_dict['hdr_inst_typ'] = self.convert_to_array(self.hdr_inst_typ)

      point_array_list = np.empty([12, self.nobs])

      # Header data
      point_array_list[0,:self.nhdr] = self.convert_to_ndarray(self.hdr_typ)
      point_array_list[1,:self.nhdr] = self.convert_to_ndarray(self.hdr_sid)
      point_array_list[2,:self.nhdr] = self.convert_to_ndarray(self.hdr_vld)
      point_array_list[3,:self.nhdr] = self.convert_to_ndarray(self.hdr_lat)
      point_array_list[4,:self.nhdr] = self.convert_to_ndarray(self.hdr_lon)
      point_array_list[5,:self.nhdr] = self.convert_to_ndarray(self.hdr_elv)
      # Observation data
      point_array_list[6]  = self.convert_to_ndarray(self.obs_hid)
      point_array_list[7]  = self.convert_to_ndarray(self.obs_lvl)
      point_array_list[8]  = self.convert_to_ndarray(self.obs_hgt)
      point_array_list[9]  = self.convert_to_ndarray(self.obs_val)
      point_array_list[10] = self.convert_to_ndarray(self.obs_vid)
      point_array_list[11] = self.convert_to_ndarray(self.obs_qty)

      with open(tmp_filename,'w') as json_fh:
         json.dump(json_dict, json_fh)

      numpy_dump_name = met_base_tools.get_numpy_filename(tmp_filename)
      #np.save(numpy_dump_name, self.convert_to_ndarray(point_array_list))
      np.save(numpy_dump_name, point_array_list)


class csv_point_obs(met_base_point):

   def __init__(self, point_data):
      self.point_data = point_data
      super(csv_point_obs, self).__init__()

      self.obs_cnt = obs_cnt = len(point_data)
      self.obs_qty = [ 0 for _ in range(0, obs_cnt) ]  # (nobs_qty) integer, index of self.obs_qty_table
      self.obs_hid = [ 0 for _ in range(0, obs_cnt) ]  # (nobs) integer
      self.obs_vid = [ 0 for _ in range(0, obs_cnt) ]  # (nobs) integer, veriable index from self.obs_var_table or GRIB code
      self.obs_lvl = [ self.FILL_VALUE for _ in range(0, obs_cnt) ]  # (nobs) float
      self.obs_hgt = [ self.FILL_VALUE for _ in range(0, obs_cnt) ]  # (nobs) float
      self.obs_val = [ self.FILL_VALUE for _ in range(0, obs_cnt) ]  # (nobs) float

      self.convert_point_data()

   def check_csv_record(self, csv_point_data, index):
      method_name = f"{self.__class__.__name__}.check_csv_record()"
      error_msgs = []
      # names=['typ', 'sid', 'vld', 'lat', 'lon', 'elv', 'var', 'lvl', 'hgt', 'qc', 'obs']
      # dtype={'typ':'str', 'sid':'str', 'vld':'str', 'var':'str', 'qc':'str'}
      if 11 > len(csv_point_data):
         error_msgs.append(f"{method_name} {index}-th data: missing columns. should be 11 columns, not {len(csv_point_data)} columns")
      elif 11 < len(csv_point_data):
         print("{i}-th data: ignore after 11-th columns out of {len(csv_point_data)} columns")
      if not isinstance(csv_point_data[0], str):
         error_msgs.append(f"{method_name} {index}-th data: message_type is not string")
      if not isinstance(csv_point_data[1], str):
         error_msgs.append(f"{method_name} {index}-th data: station_id is not string")
      if not isinstance(csv_point_data[2], str):
         error_msgs.append(f"{method_name} {index}-th data: valid_time is not string")
      if isinstance(csv_point_data[3], str):
         error_msgs.append(f"{method_name} {index}-th data: latitude can not be a string")
      elif csv_point_data[3] < -90.0 or csv_point_data[3] > 90.0:
         error_msgs.append(f"{method_name} {index}-th data: latitude ({csv_point_data[3]}) is out of range")
      if isinstance(csv_point_data[4], str):
         error_msgs.append(f"{method_name} {index}-th data: longitude can not be a string")
      elif csv_point_data[4] < -180.0 or csv_point_data[4] > 360.0:
         error_msgs.append(f"{method_name} {index}-th data: longitude ({csv_point_data[4]}) is out of range")
      if not isinstance(csv_point_data[6], str):
         error_msgs.append(f"{method_name} {index}-th data: grib_code/var_name is not string")
      if not isinstance(csv_point_data[9], str):
         error_msgs.append(f"{method_name} {index}-th data: quality_mark is not string")
      is_string, is_num = self.is_num_string(csv_point_data[5])
      if is_string and not is_num:
         error_msgs.append(f"{method_name} {index}-th data: elevation: only NA is accepted as string")
      is_string, is_num = self.is_num_string(csv_point_data[7])
      if is_string and not is_num:
         error_msgs.append(f"{method_name} {index}-th data: obs_level: only NA is accepted as string")
      is_string, is_num = self.is_num_string(csv_point_data[8])
      if is_string and not is_num:
         error_msgs.append(f"{method_name} {index}-th data: obs_height: only NA is accepted as string")
      is_string, is_num = self.is_num_string(csv_point_data[10])
      if is_string and not is_num:
         error_msgs.append(f"{method_name} {index}-th data: obs_value: only NA is accepted as string")
      return error_msgs

   def check_csv_point_data(self, all_records=False):
      method_name = f"{self.__class__.__name__}.check_csv_point_data()"
      if 0 == len(self.point_data):
         self.add_error_msg(f"{method_name} No data!")
      elif all_records:
         data_idx = 0
         for csv_point_data in self.point_data:
            data_idx += 1
            error_messages = self.check_csv_record(csv_point_data, data_idx)
            if len(error_messages) > 0:
               self.add_error_msgs(error_messages)
      else:
         error_messages = self.check_csv_record(self.point_data[0], index=1)
         if len(error_messages) > 0:
            self.add_error_msgs(error_messages)
         if 1 < len(self.point_data):
            error_messages = self.check_csv_record(self.point_data[-1], index=len(self.point_data))
            if len(error_messages) > 0:
               self.add_error_msgs(error_messages)

   def convert_point_data(self):
      hdr_cnt = hdr_typ_cnt = hdr_sid_cnt = hdr_vld_cnt = 0
      var_name_cnt = qc_cnt = 0

      hdr_map = {}
      hdr_typ_map = {}
      hdr_sid_map = {}
      hdr_vld_map = {}
      obs_var_map = {}
      obs_qty_map = {}
      self.use_var_id = not self.is_grib_code()

      index = 0
      #names=['typ', 'sid', 'vld', 'lat', 'lon', 'elv', 'var', 'lvl', 'hgt', 'qc', 'obs']
      for csv_point_record in self.point_data:
         # Build header map.
         hdr_typ_str = csv_point_record[0]
         hdr_typ_idx = hdr_typ_map.get(hdr_typ_str,-1)
         if hdr_typ_idx < 0:
            hdr_typ_idx = hdr_typ_cnt
            hdr_typ_map[hdr_typ_str] = hdr_typ_idx
            hdr_typ_cnt += 1

         hdr_sid_str = csv_point_record[1]
         hdr_sid_idx = hdr_sid_map.get(hdr_sid_str,-1)
         if hdr_sid_idx < 0:
            hdr_sid_idx = hdr_sid_cnt
            hdr_sid_map[hdr_sid_str] = hdr_sid_idx
            hdr_sid_cnt += 1

         hdr_vld_str = csv_point_record[2]
         hdr_vld_idx = hdr_vld_map.get(hdr_vld_str,-1)
         if hdr_vld_idx < 0:
            hdr_vld_idx = hdr_vld_cnt
            hdr_vld_map[hdr_vld_str] = hdr_vld_idx
            hdr_vld_cnt += 1

         lat = csv_point_record[3]
         lon = csv_point_record[4]
         elv = self.get_num_value(csv_point_record[5] )
         hdr_key = (hdr_typ_idx,hdr_sid_idx,hdr_vld_idx,lat,lon,elv)
         hdr_idx = hdr_map.get(hdr_key,-1)
         if hdr_idx < 0:
            hdr_idx = hdr_cnt
            hdr_map[hdr_key] = hdr_idx
            hdr_cnt += 1
         
         var_id_str = csv_point_record[6]
         if self.use_var_id:
            var_id = obs_var_map.get(var_id_str,-1)
            if var_id < 0:
               var_id = var_name_cnt
               obs_var_map[var_id_str] = var_id
               var_name_cnt += 1
         else:
            var_id = int(var_id_str)

         qc_str = csv_point_record[9]
         qc_id = obs_qty_map.get(qc_str,-1)
         if qc_id < 0:
            qc_id = qc_cnt
            obs_qty_map[qc_str] = qc_id
            qc_cnt += 1

         # names=['typ', 'sid', 'vld', 'lat', 'lon', 'elv', 'var', 'lvl', 'hgt', 'qc', 'obs']
         self.obs_vid[index] = var_id
         self.obs_hid[index] = hdr_idx
         self.obs_lvl[index] = self.get_num_value(csv_point_record[7])
         self.obs_hgt[index] = self.get_num_value(csv_point_record[8])
         self.obs_val[index] = self.get_num_value(csv_point_record[10])
         self.obs_qty[index]  = qc_id

         index += 1

      self.nhdr = hdr_cnt
      self.nhdr_typ = hdr_typ_cnt
      self.nhdr_sid = hdr_sid_cnt
      self.nhdr_vld = hdr_vld_cnt
      self.nobs_var = var_name_cnt
      self.nobs_qty = qc_cnt

      # Fill header array and table array based on the map
      self.hdr_typ = [ 0 for _ in range(0, hdr_cnt) ]
      self.hdr_sid = [ 0 for _ in range(0, hdr_cnt) ]
      self.hdr_vld = [ 0 for _ in range(0, hdr_cnt) ]
      self.hdr_lat = [ self.FILL_VALUE for _ in range(0, hdr_cnt) ]
      self.hdr_lon = [ self.FILL_VALUE for _ in range(0, hdr_cnt) ]
      self.hdr_elv = [ self.FILL_VALUE for _ in range(0, hdr_cnt) ]
      for key, idx in hdr_map.items():
         self.hdr_typ[idx] = key[0]
         self.hdr_sid[idx] = key[1]
         self.hdr_vld[idx] = key[2]
         self.hdr_lat[idx] = key[3]
         self.hdr_lon[idx] = key[4]
         self.hdr_elv[idx] = key[5]

      self.hdr_typ_table = [ "" for _ in range(0, hdr_typ_cnt) ]
      self.hdr_sid_table = [ "" for _ in range(0, hdr_sid_cnt) ]
      self.hdr_vld_table = [ "" for _ in range(0, hdr_vld_cnt) ]
      self.obs_qty_table = [ "" for _ in range(0, qc_cnt) ]
      self.obs_var_table = [ "" for _ in range(0, var_name_cnt) ]
      for key, idx in hdr_typ_map.items():
         self.hdr_typ_table[idx] = key
      for key, idx in hdr_sid_map.items():
         self.hdr_sid_table[idx] = key
      for key, idx in hdr_vld_map.items():
         self.hdr_vld_table[idx] = key
      for key, idx in obs_qty_map.items():
         self.obs_qty_table[idx] = key
      for key, idx in obs_var_map.items():
         self.obs_var_table[idx] = key

   def get_num_value(self, column_value):
      method_name = f"{self.__class__.__name__}.get_num_value()"
      num_value = column_value
      if isinstance(column_value, str):
         if self.is_number(column_value):
            num_value = float(column_value)
         else:
            num_value = self.FILL_VALUE
            if column_value.lower() != 'na' and column_value.lower() != 'n/a':
               self.info_msg(f'{method_name} {column_value} is not a number, converted to the missing value')
      return num_value

   def is_grib_code(self):
      grib_code = True
      for _point_data in self.point_data:
         if isinstance(_point_data[6], int):
            continue
         elif isinstance(_point_data[6], str) and not _point_data[6].isdecimal():
            grib_code = False
            break;
      return grib_code

   def is_num_string(self, column_value):
      is_string = isinstance(column_value, str)
      if is_string:
         is_num = True if self.is_number(column_value) or column_value.lower() == 'na' or column_value.lower() == 'n/a' else False
      else:
         is_num = True
      return is_string, is_num


class met_point_obs(ABC, met_base_point):

   @abstractmethod
   def read_data(self, args):
      # args can be input_file_name, list, or dictionary
      # - The variables at __init__ should be filled as python list or numpy array
      # - set self.input_name
      #
      # Here is a template
      '''
      if isinstance(args, dict):
         in_filename = args.get('in_name',None)
      elif isinstance(args, list):
         in_filename = args[0]
      else:
         in_filename = args
      self.input_name = in_filename
      '''
      pass


class dummy_point_obs(met_point_obs):

   def read_data(self, args):
      pass


class met_point_tools(met_base_tools):

   python_prefix = 'PYTHON_POINT_USER'

   @staticmethod
   def convert_point_data(point_data, check_all_records=False, input_type='csv'):
      tmp_point_data = {}
      if 'csv' == input_type:
         csv_point_data = csv_point_obs(point_data)
         csv_point_data.check_csv_point_data(check_all_records)
         tmp_point_data = csv_point_data.get_point_data()
      else:
         met_base_point.error_message(f'met_point_tools.convert_point_data() Not supported input type: {input_type}')
      return tmp_point_data

   @staticmethod
   def get_sample_point_obs():
      return sample_met_point_obs()

   @staticmethod
   def get_python_script(arg_value):
      return arg_value[len(met_point_tools.python_prefix)+1:]

   @staticmethod
   def is_python_prefix(user_cmd):
      return user_cmd.startswith(met_point_tools.python_prefix)

   @staticmethod
   def print_data(key, data_array, show_count=COUNT_SHOW):
      if isinstance(data_array, list):
          data_len = len(data_array)
          if show_count >= data_len:
              print("      {k:10s}: {v}".format(k=key, v= data_array))
          else:
              end_offset = int(show_count/2)
              print("      {k:10s}: count={v}".format(k=key, v=data_len))
              print("      {k:10s}[0:{o}] {v}".format(k=key, v=data_array[:end_offset], o=end_offset))
              print("      {k:10s}[{s}:{e}]: {v}".format(k=key, v='...', s=end_offset+1, e=data_len-end_offset-1))
              print("      {k:10s}[{s}:{e}]: {v}".format(k=key, v= data_array[-end_offset:], s=(data_len-end_offset), e=(data_len-1)))
      else:
          print("      {k:10s}: {v}".format(k=key, v= data_array))

   @staticmethod
   def print_point_data(met_point_data, print_subset=True):
      method_name = f"met_point_tools.print_point_data()"
      print(' === MET point data by python embedding ===')
      if print_subset:
          met_point_tools.print_data('nhdr',met_point_data['nhdr'])
          met_point_tools.print_data('nobs',met_point_data['nobs'])
          met_point_tools.print_data('use_var_id',met_point_data['use_var_id'])
          met_point_tools.print_data('hdr_typ',met_point_data['hdr_typ'])
          met_point_tools.print_data('hdr_typ_table',met_point_data['hdr_typ_table'])
          met_point_tools.print_data('hdr_sid',met_point_data['hdr_sid'])
          met_point_tools.print_data('hdr_sid_table',met_point_data['hdr_sid_table'])
          met_point_tools.print_data('hdr_vld',met_point_data['hdr_vld'])
          met_point_tools.print_data('hdr_vld_table',met_point_data['hdr_vld_table'])
          met_point_tools.print_data('hdr_lat',met_point_data['hdr_lat'])
          met_point_tools.print_data('hdr_lon',met_point_data['hdr_lon'])
          met_point_tools.print_data('hdr_elv',met_point_data['hdr_elv'])
          met_point_tools.print_data('obs_hid',met_point_data['obs_hid'])
          met_point_tools.print_data('obs_vid',met_point_data['obs_vid'])
          met_point_tools.print_data('obs_var_table',met_point_data['obs_var_table'])
          met_point_tools.print_data('obs_qty',met_point_data['obs_qty'])
          met_point_tools.print_data('obs_qty_table',met_point_data['obs_qty_table'])
          met_point_tools.print_data('obs_lvl',met_point_data['obs_lvl'])
          met_point_tools.print_data('obs_hgt',met_point_data['obs_hgt'])
          met_point_tools.print_data('obs_val',met_point_data['obs_val'])
      else:
          print(f'{method_name} All',met_point_data)
          print(f"         nhdr: met_point_data['nhdr']")
          print(f"         nobs: met_point_data['nobs']")
          print(f"   use_var_id: met_point_data['use_var_id']")
          print(f"      hdr_typ: met_point_data['hdr_typ']")
          print(f"hdr_typ_table: met_point_data['hdr_typ_table']")
          print(f"      hdr_sid: met_point_data['hdr_sid']")
          print(f"hdr_sid_table: met_point_data['hdr_sid_table']")
          print(f"      hdr_vld: met_point_data['hdr_vld']")
          print(f"hdr_vld_table: met_point_data['hdr_vld_table']")
          print(f"      hdr_lat: met_point_data['hdr_lat']")
          print(f"      hdr_lon: met_point_data['hdr_lon']")
          print(f"      hdr_elv: met_point_data['hdr_elv']")
          print(f"      obs_hid: met_point_data['obs_hid']")
          print(f"      obs_vid: met_point_data['obs_vid']")
          print(f"obs_var_table: met_point_data['obs_var_table']")
          print(f"      obs_qty: met_point_data['obs_qty']")
          print(f"obs_qty_table: met_point_data['obs_qty_table']")
          print(f"      obs_lvl: met_point_data['obs_lvl']")
          print(f"      obs_hgt: met_point_data['obs_hgt']")
          print(f"      obs_val: met_point_data['obs_val']")

      print(' === MET point data by python embedding ===')

   @staticmethod
   # Read the input file which is 11 column text file as the first argument
   def read_text_point_obs(input_file, header=None,
                           delim_whitespace=True, keep_default_na=False):
      # Read and format the input 11-column observations:
      #   (1)  string:  Message_Type
      #   (2)  string:  Station_ID
      #   (3)  string:  Valid_Time(YYYYMMDD_HHMMSS)
      #   (4)  numeric: Lat(Deg North)
      #   (5)  numeric: Lon(Deg East)
      #   (6)  numeric: Elevation(msl)
      #   (7)  string:  Var_Name(or GRIB_Code)
      #   (8)  numeric: Level
      #   (9)  numeric: Height(msl or agl)
      #   (10) string:  QC_String
      #   (11) numeric: Observation_Value
      ascii_point_data = pd.read_csv(
            input_file, header=header,
            delim_whitespace=delim_whitespace,
            keep_default_na=keep_default_na,
            names=['typ', 'sid', 'vld', 'lat', 'lon', 'elv', 'var', 'lvl', 'hgt', 'qc', 'obs'],
            dtype={'typ':'string', 'sid':'string', 'vld':'string', 'var':'string', 'qc':'string'}).values.tolist()
      return ascii_point_data


# This is a sample drived class
class sample_met_point_obs(met_point_obs):

   #@abstractmethod
   def read_data(self, arg_map={}):
      self.hdr_typ = np.array([ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ])
      self.hdr_sid = np.array([ 0, 0, 0, 0, 0, 1, 2, 3, 3, 1, 2, 2, 3, 0, 0, 0, 0, 0, 1, 2, 3, 3, 1, 2, 2, 3 ])
      self.hdr_vld = np.array([ 0, 1, 2, 3, 4, 4, 3, 4, 3, 4, 5, 4, 3, 0, 1, 2, 3, 4, 4, 3, 4, 3, 4, 5, 4, 3 ])
      self.hdr_lat = np.array([ 43., 43., 43., 43., 43., 43., 43., 43., 43., 46., 46., 46., 46., 43., 43., 43., 43., 43., 43., 43., 43., 43., 46., 46., 46., 46. ])
      self.hdr_lon = np.array([ -89., -89., -89., -89., -89., -89., -89., -89., -89., -92., -92., -92., -92., -89., -89., -89., -89., -89., -89., -89., -89., -89., -92., -92., -92., -92. ])
      self.hdr_elv = np.array([ 220., 220., 220., 220., 220., 220., 220., 220., 220., 220., 220., 220., 220., 220., 220., 220., 220., 220., 220., 220., 220., 220., 220., 220., 220., 220. ])

      self.obs_hid = np.array([ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 25 ])
      self.obs_vid = np.array([ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ])
      self.obs_qty = np.array([ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ])
      self.obs_lvl = np.array([ 1000., 1000., 1000., 1000., 1000., 1000., 1000., 1000., 1000., 1000., 1000., 1000., 1000., 1000., 1000., 1000., 1000., 1000., 1000., 1000., 1000., 1000., 1000., 1000., 1000., 1000., 1000., 1000. ])
      self.obs_hgt = np.array([ 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2. ])
      self.obs_val = np.array([ 292., 292.5, 293., 293.5, 294., 294.5, 295., 295.5, 296., 292., 293.4, 293., 296., 294., 92., 92.5, 93., 93.5, 94., 94.5, 95., 95.5, 96., 92., 93.4, 93., 96., 94. ])

      self.hdr_typ_table = [ "ADPSFC" ]
      self.hdr_sid_table = [ "001", "002", "003", "004" ]
      self.hdr_vld_table = [
              "20120409_115000", "20120409_115500", "20120409_120100", "20120409_120500", "20120409_121000",
              "20120409_120000" ]
      self.obs_var_table = [ "TMP", "RH" ]
      self.obs_qty_table = [ "NA" ]


def convert_point_data(point_data, check_all_records=False, input_type='csv'):
   tmp_point_data = {}
   if 'csv' == input_type:
      csv_point_data = csv_point_obs(point_data)
      csv_point_data.check_csv_point_data(check_all_records)
      tmp_point_data = csv_point_data.get_point_data()
   else:
      met_base.error_message(f'convert_point_data(() Not supported input type: {input_type}')
   return tmp_point_data

def get_empty_point_obs():
   return dummy_point_obs()

def main():
   args = {}   # or args = []
   point_obs_data = sample_met_point_obs()
   point_obs_data.read_data(args)
   met_point_data = point_obs_data.get_point_data()

   point_obs_data.print_point_data(met_point_data, print_subset=False)

if __name__ == '__main__':
   main()
   print('Done python script')
