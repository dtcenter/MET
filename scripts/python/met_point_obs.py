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

class met_point_obs(ABC):
    '''
    classdocs
    '''
    ERROR_P = " ==ERROR_PYTHON=="
    INFO_P = " ==INFO_PYTHON=="

    python_prefix = 'PYTHON_POINT_RAW'

    def __init__(self, use_var_id=True):
        '''
        Constructor
        '''
        self.input_name = None
        self.use_var_id = use_var_id    # True if variable index, False if GRIB code

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

    def check_data_member_float(self, local_var, var_name):
        if 0 == len(local_var):
            self.log_error("{v} is empty".format(v=var_name))
        elif isinstance(local_var, list):
            if 0 <= str(type(local_var[0])).find('numpy'):
                self.log_info("Recommend using numpy instead of python list for {v} ({t}) to avoid side effect".format(
                        v=var_name, t=type(local_var[0])))
            elif not isinstance(local_var[0], (int, float)):
                self.log_error("Not supported data type ({t}) for {v}[0] (int or float only)".format(
                        v=var_name, t=local_var[0].type))
        elif not isinstance(local_var, np.ndarray):
            self.log_error("Not supported data type ({t}) for {v} (list and numpy.ndarray)".format(
                    v=var_name, t=type(local_var)))

    def check_data_member_int(self, local_var, var_name):
        if 0 == len(local_var):
            self.log_error("{v} is empty".format(v=var_name))
        elif isinstance(local_var, list):
            if 0 <= str(type(local_var[0])).find('numpy'):
                self.log_info("Recommend using numpy instead of python list for {v} ({t}) to avoid side effect".format(
                        v=var_name, t=type(local_var[0])))
            elif not isinstance(local_var[0], int):
                self.log_error("Not supported data type ({t}) for {v}[0] (int only)".format(
                        v=var_name, t=type(local_var[0])))
        elif not isinstance(local_var, np.ndarray):
            self.log_error("Not supported data type ({t}) for {v} (list and numpy.ndarray)".format(
                    v=var_name, t=type(local_var)))

    def check_data_member_string(self, local_var, var_name):
        if 0 == len(local_var):
            self.log_error("{v} is empty".format(v=var_name))
        elif not isinstance(local_var, (list)):
            self.log_error("Not supported data type ({t}) for {v} (list)".format(
                    v=var_name, t=type(local_var)))

    def check_point_data(self):
        if self.input_name is not None and not os.path.exists(self.input_name):
            self.log_error('The netcdf input {f} does not exist'.format(f=self.input_name))
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
            self.check_data_member_string(self.obs_qty_table,'bs_qty_table')
            self.check_data_member_string(self.obs_var_table,'bs_var_table')

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
            print("      nhdr: ",met_point_data['nhdr'])
            print("      nobs: ",met_point_data['nobs'])
            print('use_var_id: ',met_point_data['use_var_id'])
            print('hdr_typ: ',met_point_data['hdr_typ'])
            print('hdr_typ_table: ',met_point_data['hdr_typ_table'])
            print('hdr_sid: ',met_point_data['hdr_sid'])
            print('hdr_sid_table: ',met_point_data['hdr_sid_table'])
            print('hdr_vld: ',met_point_data['hdr_vld'])
            print('hdr_vld_table: ',met_point_data['hdr_vld_table'])
            print('hdr_lat: ',met_point_data['hdr_lat'])
            print('hdr_lon: ',met_point_data['hdr_lon'])
            print('hdr_elv: ',met_point_data['hdr_elv'])
            print('obs_hid: ',met_point_data['obs_hid'])
            print('obs_vid: ',met_point_data['obs_vid'])
            print('obs_var_table: ',met_point_data['obs_var_table'])
            print('obs_qty: ',met_point_data['obs_qty'])
            print('obs_qty_table: ',met_point_data['obs_qty_table'])
            print('obs_lvl: ',met_point_data['obs_lvl'])
            print('obs_hgt: ',met_point_data['obs_hgt'])
            print('obs_val: ',met_point_data['obs_val'])

        print(' === MET point data by python embedding ===')


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


def main():
    args = {}   # or args = []
    point_obs_data = sample_met_point_obs()
    point_obs_data.read_data(args)
    met_point_data = point_obs_data.get_point_data()

    point_obs_data.print_point_data(met_point_data, print_subset=False)

if __name__ == '__main__':
    main()
    print('Done python scripot')
