
'''
Created on Nov 10, 2021

@author: hsoh

- This is the base class and the customized script should extend the met_point_obs.
- The customized script (for example "custom_reader") must implement "def read_data(self, args)"
  which fills the array (python list or numpy array) variables at __init__().
- The args can be 1) single string argument, 2) the list of arguments, and 3) the dictionary of arguments.
- The variable "met_point_data" must be set for MET tools
- The customized script is expected to include following codes:

    # prepare arguments for the customized script
    args = {'input', sys.argv[1]}   # or args = []
    point_obs_data = custom_reader()
    point_obs_data.read_data()
    met_point_data = point_obs_data.get_point_data()
'''

import os
from abc import ABC, abstractmethod
import numpy as np

COUNT_SHOW = 30

MET_PYTHON_OBS_ARGS = "MET_POINT_PYTHON_ARGS"


class base_met_point_obs(object):
    '''
    classdocs
    '''
    ERROR_P = " ==ERROR_PYTHON=="
    INFO_P  = " ==INFO_PYTHON=="

    python_prefix = 'PYTHON_POINT_RAW'

    def __init__(self, use_var_id=True):
        '''
        Constructor
        '''
        self.input_name = None
        self.ignore_input_file = False
        self.use_var_id = use_var_id    # True if variable index, False if GRIB code
        self.error_msg  = ""
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

        # Optional variables for PREPBUFR, not supported yet
        self.hdr_prpt_typ  = [] # optional
        self.hdr_irpt_typ  = [] # optional
        self.hdr_inst_typ  = [] # optional

    def add_error_msg(self, error_msg):
        self.has_error  = True
        self.log_error_msg(error_msg)
        if 0 == len(self.error_msg):
            self.error_msg = error_msg
        else:
            self.error_msg = "{m1}\n{m2}".format(m1=self.error_msg, m2=error_msg)

    def add_error_msgs(self, error_msgs):
        self.has_error  = True
        for error_msg in error_msgs:
            self.add_error_msg(error_msg)

    def check_data_member_float(self, local_var, var_name):
        if 0 == len(local_var):
            self.add_error_msg("{v} is empty (float)".format(v=var_name))
        elif isinstance(local_var, list):
            if isinstance(local_var[0], str) and not self.is_number(local_var[0]):
                self.add_error_msg("Not supported data type ({v} string) for {n}[0] (int or float only)".format(
                        n=var_name, v=local_var[0]))
            elif 0 <= str(type(local_var[0])).find('numpy'):
                self.log_info("Recommend using numpy instead of python list for {v} ({t}) to avoid side effect".format(
                        v=var_name, t=type(local_var[0])))
            elif not isinstance(local_var[0], (int, float)):
                # self.add_error_msg("Not supported data type ({t}) for {v}[0] (int or float only)".format(
                #         v=var_name, t=local_var[0].type))
                self.add_error_msg("Not supported data type ({t}) for {v}[0] (int or float only)".format(
                        v=var_name, t=type(local_var[0])))
        elif not isinstance(local_var, np.ndarray):
            self.add_error_msg("Not supported data type ({t}) for {v} (list and numpy.ndarray)".format(
                    v=var_name, t=type(local_var)))

    def check_data_member_int(self, local_var, var_name):
        if 0 == len(local_var):
            self.add_error_msg("{v} is empty (int)".format(v=var_name))
        elif isinstance(local_var, list):
            if isinstance(local_var[0], str) and not self.is_number(local_var[0]):
                self.add_error_msg("Not supported data type ({v} string) for {n}[0] (int or float only)".format(
                        n=var_name, v=local_var[0]))
            elif 0 <= str(type(local_var[0])).find('numpy'):
                self.log_info("Recommend using numpy instead of python list for {v} ({t}) to avoid side effect".format(
                        v=var_name, t=type(local_var[0])))
            elif not isinstance(local_var[0], int):
                self.add_error_msg("Not supported data type ({t}) for {v}[0] (int only)".format(
                        v=var_name, t=type(local_var[0])))
        elif not isinstance(local_var, np.ndarray):
            self.add_error_msg("Not supported data type ({t}) for {v} (list and numpy.ndarray)".format(
                    v=var_name, t=type(local_var)))

    def check_data_member_string(self, local_var, var_name):
        if 0 == len(local_var):
            self.add_error_msg("{v} is empty (string)".format(v=var_name))
        elif not isinstance(local_var, (list)):
            self.add_error_msg("Not supported data type ({t}) for {v} (list)".format(
                    v=var_name, t=type(local_var)))

    def check_point_data(self):
        if not self.ignore_input_file and self.input_name is not None and not os.path.exists(self.input_name):
            self.add_error_msg('The netcdf input {f} does not exist'.format(f=self.input_name))
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
        return self.__dict__

    def get_type(self, value):
        return 'string' if isinstance('str') else type(value)

    def is_number(self, num_str):
        return num_str.replace('-','1').replace('+','2').replace('.','3').isdigit()

    def log_error_msg(self, err_msg):
        print('{p} {m}'.format(p=self.ERROR_P, m=err_msg))

    def log_error(self, err_msgs):
        print(self.ERROR_P)
        for err_line in err_msgs.split('\n'):
            print('{p} {m}'.format(p=self.ERROR_P, m=err_line))
        print(self.ERROR_P)

    def log_info(self, info_msg):
        print('{p} {m}'.format(p=self.INFO_P, m=info_msg))

    @staticmethod
    def is_python_script(arg_value):
        return arg_value.startswith(met_point_obs.python_prefix)

    @staticmethod
    def get_python_script(arg_value):
        return arg_value[len(met_point_obs.python_prefix)+1:]

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
        print(' === MET point data by python embedding ===')
        if print_subset:
            met_point_obs.print_data('nhdr',met_point_data['nhdr'])
            met_point_obs.print_data('nobs',met_point_data['nobs'])
            met_point_obs.print_data('use_var_id',met_point_data['use_var_id'])
            met_point_obs.print_data('hdr_typ',met_point_data['hdr_typ'])
            met_point_obs.print_data('hdr_typ_table',met_point_data['hdr_typ_table'])
            met_point_obs.print_data('hdr_sid',met_point_data['hdr_sid'])
            met_point_obs.print_data('hdr_sid_table',met_point_data['hdr_sid_table'])
            met_point_obs.print_data('hdr_vld',met_point_data['hdr_vld'])
            met_point_obs.print_data('hdr_vld_table',met_point_data['hdr_vld_table'])
            met_point_obs.print_data('hdr_lat',met_point_data['hdr_lat'])
            met_point_obs.print_data('hdr_lon',met_point_data['hdr_lon'])
            met_point_obs.print_data('hdr_elv',met_point_data['hdr_elv'])
            met_point_obs.print_data('obs_hid',met_point_data['obs_hid'])
            met_point_obs.print_data('obs_vid',met_point_data['obs_vid'])
            met_point_obs.print_data('obs_var_table',met_point_data['obs_var_table'])
            met_point_obs.print_data('obs_qty',met_point_data['obs_qty'])
            met_point_obs.print_data('obs_qty_table',met_point_data['obs_qty_table'])
            met_point_obs.print_data('obs_lvl',met_point_data['obs_lvl'])
            met_point_obs.print_data('obs_hgt',met_point_data['obs_hgt'])
            met_point_obs.print_data('obs_val',met_point_data['obs_val'])
        else:
            print('All',met_point_data)
            print("         nhdr: ",met_point_data['nhdr'])
            print("         nobs: ",met_point_data['nobs'])
            print('   use_var_id: ',met_point_data['use_var_id'])
            print('      hdr_typ: ',met_point_data['hdr_typ'])
            print('hdr_typ_table: ',met_point_data['hdr_typ_table'])
            print('      hdr_sid: ',met_point_data['hdr_sid'])
            print('hdr_sid_table: ',met_point_data['hdr_sid_table'])
            print('      hdr_vld: ',met_point_data['hdr_vld'])
            print('hdr_vld_table: ',met_point_data['hdr_vld_table'])
            print('      hdr_lat: ',met_point_data['hdr_lat'])
            print('      hdr_lon: ',met_point_data['hdr_lon'])
            print('      hdr_elv: ',met_point_data['hdr_elv'])
            print('      obs_hid: ',met_point_data['obs_hid'])
            print('      obs_vid: ',met_point_data['obs_vid'])
            print('obs_var_table: ',met_point_data['obs_var_table'])
            print('      obs_qty: ',met_point_data['obs_qty'])
            print('obs_qty_table: ',met_point_data['obs_qty_table'])
            print('      obs_lvl: ',met_point_data['obs_lvl'])
            print('      obs_hgt: ',met_point_data['obs_hgt'])
            print('      obs_val: ',met_point_data['obs_val'])

        print(' === MET point data by python embedding ===')


class csv_point_obs(ABC, base_met_point_obs):

    def __init__(self, point_data):
        super(csv_point_obs, self).__init__()

        hdr_cnt = obs_cnt = len(point_data)
        self.point_data = point_data
        self.nhdr = self.nobs = obs_cnt
        self.nhdr_typ = self.nhdr_sid = self.nhdr_vld = hdr_cnt
        self.nobs_qty = self.nobs_var = obs_cnt
        self.input_file = None
        self.ignore_input_file = True

        self.obs_qty = [ 0 for _ in range(0, obs_cnt) ]  # (nobs_qty) integer, index of self.obs_qty_table
        self.obs_hid = [ 0 for _ in range(0, obs_cnt) ]  # (nobs) integer
        self.obs_vid = [ 0 for _ in range(0, obs_cnt) ]  # (nobs) integer, veriable index from self.obs_var_table or GRIB code
        self.obs_lvl = [ nc_tools.FILL_VALUE for _ in range(0, obs_cnt) ]  # (nobs) float
        self.obs_hgt = [ nc_tools.FILL_VALUE for _ in range(0, obs_cnt) ]  # (nobs) float
        self.obs_val = [ nc_tools.FILL_VALUE for _ in range(0, obs_cnt) ]  # (nobs) float

    def check_csv_record(self, csv_point_data, index):
        error_msgs = []
        # names=['typ', 'sid', 'vld', 'lat', 'lon', 'elv', 'var', 'lvl', 'hgt', 'qc', 'obs']
        # dtype={'typ':'str', 'sid':'str', 'vld':'str', 'var':'str', 'qc':'str'}
        if 11 > len(csv_point_data):
            error_msgs.append("{i}-th data: missing columns. should be 11 columns, not {c} columns".format(
                    i=index, c=len(csv_point_data)))
        elif 11 < len(csv_point_data):
            print("{i}-th data: ignore after 11-th columns out of {c} columns".format(
                    i=index, c=len(csv_point_data)))
        if not isinstance(csv_point_data[0], str):
            error_msgs.append("{i}-th data: message_type is not string".format(i=index))
        if not isinstance(csv_point_data[1], str):
            error_msgs.append("{i}-th data: station_id is not string".format(i=index))
        if not isinstance(csv_point_data[2], str):
            error_msgs.append("{i}-th data: valid_time is not string".format(i=index))
        if isinstance(csv_point_data[3], str):
            error_msgs.append("{i}-th data: latitude can not be a string".format(i=index))
        elif csv_point_data[3] < -90.0 or csv_point_data[3] > 90.0:
            error_msgs.append("{i}-th data: latitude ({l}) is out of range".format(i=index, l=csv_point_data[3]))
        if isinstance(csv_point_data[4], str):
            error_msgs.append("{i}-th data: longitude can not be a string".format(i=index))
        elif csv_point_data[4] < -180.0 or csv_point_data[4] > 360.0:
            error_msgs.append("{i}-th data: longitude ({l}) is out of range".format(i=index, l=csv_point_data[4]))
        if not isinstance(csv_point_data[6], str):
            error_msgs.append("{i}-th data: grib_code/var_name is not string".format(i=index))
        if not isinstance(csv_point_data[9], str):
            error_msgs.append("{i}-th data: quality_mark is not string".format(i=index))
        is_string, is_num = self.is_num_string(csv_point_data[5])
        if is_string and not is_num:
            error_msgs.append("{i}-th data: elevation: only NA is accepted as string".format(i=index))
        is_string, is_num = self.is_num_string(csv_point_data[7])
        if is_string and not is_num:
            error_msgs.append("{i}-th data: obs_level: only NA is accepted as string".format(i=index))
        is_string, is_num = self.is_num_string(csv_point_data[8])
        if is_string and not is_num:
            error_msgs.append("{i}-th data: obs_height: only NA is accepted as string".format(i=index))
        is_string, is_num = self.is_num_string(csv_point_data[10])
        if is_string and not is_num:
            error_msgs.append("{i}-th data: obs_value: only NA is accepted as string".format(i=index))
        return error_msgs

    def check_csv_point_data(self, all_records=False):
        if 0 == len(self.point_data):
            self.add_error_msg("No data!")
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

        index = 0
        # Build map
        # names=['typ', 'sid', 'vld', 'lat', 'lon', 'elv', 'var', 'lvl', 'hgt', 'qc', 'obs']
        for csv_point_data in self.point_data:
            hdr_typ_str = csv_point_data[0]
            hdr_typ_idx = hdr_typ_map.get(hdr_typ_str,-1)
            if hdr_typ_idx < 0:
                hdr_typ_idx = hdr_typ_cnt
                hdr_typ_map[hdr_typ_str] = hdr_typ_idx
                hdr_typ_cnt += 1

            hdr_sid_str = csv_point_data[1]
            hdr_sid_idx = hdr_sid_map.get(hdr_sid_str,-1)
            if hdr_sid_idx < 0:
                hdr_sid_idx = hdr_sid_cnt
                hdr_sid_map[hdr_sid_str] = hdr_sid_idx
                hdr_sid_cnt += 1

            hdr_vld_str = csv_point_data[2]
            hdr_vld_idx = hdr_vld_map.get(hdr_vld_str,-1)
            if hdr_vld_idx < 0:
                hdr_vld_idx = hdr_vld_cnt
                hdr_vld_map[hdr_vld_str] = hdr_vld_idx
                hdr_vld_cnt += 1

            lat = csv_point_data[3]
            lon = csv_point_data[4]
            elv = self.get_num_value(csv_point_data[5] )
            hdr_key = (hdr_typ_idx,hdr_sid_idx,hdr_vld_idx,lat,lon,elv)
            hdr_idx = hdr_map.get(hdr_key,-1)
            if hdr_idx < 0:
                hdr_idx = hdr_cnt
                hdr_map[hdr_key] = hdr_idx
                hdr_cnt += 1
            
            var_id_str = csv_point_data[6] 
            if self.use_var_id:
                var_id = obs_var_map.get(var_id_str,-1)
                if var_id < 0:
                    var_id = var_name_cnt
                    obs_var_map[var_id_str] = var_id
                    var_name_cnt += 1
            else:
                var_id = int(var_id_str)

            qc_str = csv_point_data[9] 
            qc_id = obs_qty_map.get(qc_str,-1)
            if qc_id < 0:
                qc_id = qc_cnt
                obs_qty_map[qc_str] = qc_id
                qc_cnt += 1

            # names=['typ', 'sid', 'vld', 'lat', 'lon', 'elv', 'var', 'lvl', 'hgt', 'qc', 'obs']
            self.obs_vid[index] = var_id
            self.obs_hid[index] = hdr_idx
            self.obs_lvl[index] = self.get_num_value(csv_point_data[7])
            self.obs_hgt[index] = self.get_num_value(csv_point_data[8])
            self.obs_val[index] = self.get_num_value(csv_point_data[10])
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
        self.hdr_lat = [ nc_tools.FILL_VALUE for _ in range(0, hdr_cnt) ]
        self.hdr_lon = [ nc_tools.FILL_VALUE for _ in range(0, hdr_cnt) ]
        self.hdr_elv = [ nc_tools.FILL_VALUE for _ in range(0, hdr_cnt) ]
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

        return self.get_point_data()

    def get_num_value(self, column_value):
        num_value = column_value
        if isinstance(column_value, str):
            if column_value.lower() == 'na':
                num_value = nc_tools.FILL_VALUE
            elif self.is_number(column_value):
                num_value = float(column_value)
            num_value = nc_tools.FILL_VALUE
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
            is_num = True if self.is_number(column_value) or column_value.lower() == 'na' else False
        else:
            is_num = True
        return is_string, is_num


class met_point_obs(ABC, base_met_point_obs):

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


# Note: caller should import netCDF4
# the argements nc_group(dataset) and nc_var should not be None
class nc_tools():

    FILL_VALUE = -9999.
    met_missing = -99999999.

    @staticmethod
    def get_num_array(nc_group, var_name):
        nc_var = nc_group.variables.get(var_name, None)
        #return [] if nc_var is None else nc_var[:].filled(nc_var._FillValue if '_FillValue' in nc_var.ncattrs() else nc_tools.met_missing) 
        return [] if nc_var is None else nc_var[:]

    @staticmethod
    def get_ncbyte_array_to_str(nc_var):
        nc_str_data = nc_var[:]
        if nc_var.datatype.name == 'bytes8':
            nc_str_data = [ str(s.compressed(),"utf-8") for s in nc_var[:] ]
        return nc_str_data

    @staticmethod
    def get_string_array(nc_group, var_name):
        nc_var = nc_group.variables.get(var_name, None)
        return [] if nc_var is None else nc_tools.get_ncbyte_array_to_str(nc_var)


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

def convert_point_data(point_data, check_all_records=False):
    _csv_point_data = csv_point_obs(point_data)
    if _csv_point_data.is_grib_code():
        _csv_point_data.use_var_id = False
    _csv_point_data.check_csv_point_data(check_all_records)
    return _csv_point_data.convert_point_data()

def main():
    args = {}   # or args = []
    point_obs_data = sample_met_point_obs()
    point_obs_data.read_data(args)
    met_point_data = point_obs_data.get_point_data()

    point_obs_data.print_point_data(met_point_data, print_subset=False)

if __name__ == '__main__':
    main()
    print('Done python scripot')
