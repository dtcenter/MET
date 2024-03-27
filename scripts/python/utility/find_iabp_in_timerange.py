 #!/usr/bin/env python3

from optparse import OptionParser
import urllib.request
import datetime
from datetime import date
import os
import shutil
import shlex
import errno
from subprocess import Popen, PIPE



#----------------------------------------------
def usage():
   print("Usage: find_iabp_in_timerange.py -s yyyymmdd -e yyyymmdd [-d PATH]")

#----------------------------------------------
def is_date_in_range(input_date, start_date, end_date):
   return start_date <= input_date <= end_date

#----------------------------------------------
def lookFor(name, inlist, filename, printWarning=False):
    rval = -1
    try:
        rval = inlist.index(name)
    except:
        if printWarning:
            print(name, " not in header line, file=", filename)

    return rval

#----------------------------------------------
def pointToInt(index, tokens, filename):
    if index < 0 or index >= len(tokens):
        print("ERROR index out of range ", index)
        return -1
    return int(tokens[index])
    
#----------------------------------------------
def pointToFloat(index, tokens, filename):
    if index < 0 or index >= len(tokens):
        print("ERROR index out of range ", index)
        return -99.99
    return float(tokens[index])
    
#----------------------------------------------
class StationHeader:
    def __init__(self, headerLine, filename):
        tokens = headerLine.split()
        self._ok = True
        self._idIndex = lookFor('BuoyID', tokens, filename, True)
        self._yearIndex = lookFor('Year', tokens, filename, True)
        self._hourIndex = lookFor('Hour', tokens, filename, True)
        self._minuteIndex = lookFor('Min', tokens, filename, True)
        self._doyIndex = lookFor('DOY', tokens, filename, True)
        self._posdoyIndex = lookFor('POS_DOY', tokens, filename, True)
        self._latIndex = lookFor('Lat', tokens, filename, True)
        self._lonIndex = lookFor('Lon', tokens, filename, True)
        self._bpIndex = lookFor('BP', tokens, filename, False)
        self._tsIndex = lookFor('Ts', tokens, filename, False)
        self._taIndex = lookFor('Ta', tokens, filename, False)
        self._ok = self._idIndex != -1 and self._yearIndex != -1 and self._hourIndex != -1 \
            and self._minuteIndex != -1 and self._doyIndex != -1 and self._posdoyIndex != -1 \
                and self._latIndex != -1 and self._lonIndex != -1
        if not self._ok:
            print("ERROR badly formed header line")
        
#----------------------------------------------
class Station:
    def __init__(self, line, filename, stationHeader):
        self._ok = True
        tokens = line.split()
        self._id = pointToInt(stationHeader._idIndex, tokens, filename)
        if self._id < 0:
            self._ok = False
        self._year = pointToInt(stationHeader._yearIndex, tokens, filename)
        if self._year < 0:
            self._ok = False
        self._hour = pointToInt(stationHeader._hourIndex, tokens, filename)
        if self._hour < 0:
            self._ok = False
        self._minute = pointToInt(stationHeader._minuteIndex, tokens, filename)
        if self._minute < 0:
            self._ok = False
        self._doy = pointToFloat(stationHeader._doyIndex, tokens, filename)
        if self._doy < 0:
            self._ok = False
        if self._doy > 365:
            self._ok = False
        self._posdoy = pointToFloat(stationHeader._posdoyIndex, tokens, filename)
        if self._posdoy < 0:
            self._ok = False
        if self._posdoy > 365:
            self._ok = False
        self._lat = pointToFloat(stationHeader._latIndex, tokens, filename)
        if self._lat == -99.99:
            self._ok = False
        self._lon = pointToFloat(stationHeader._lonIndex, tokens, filename)
        if self._lon == -99.99:
            self._ok = False
        if stationHeader._bpIndex >= 0:
            self._pressure = pointToFloat(stationHeader._bpIndex, tokens, filename)
        else:
            self._pressure = -99.99
        if stationHeader._tsIndex >= 0:
            self._tempsurface = pointToFloat(stationHeader._tsIndex, tokens, filename)
        else:
            self._tempsurface = -99.99
        if stationHeader._taIndex >= 0:
            self._tempair = pointToFloat(stationHeader._taIndex, tokens, filename)
        else:
            self._tempair = -99.99

        if self._ok:
            d = datetime.datetime(self._year, 1, 1) + datetime.timedelta(self._doy - 1)
            self._month = d.month
            self._day = d.day
        else:
            self._month = -1
            self._day = -1
    def timeInRange(self, start_date, end_date):
        if self._ok:
            input_date = date(self._year, self._month, self._day)
            return is_date_in_range(input_date, start_date, end_date)
        else:
            return False
        
#----------------------------------------------
class StationTimeSeries:
    def __init__(self, stationHeader):
        self._stationHeader = stationHeader
        self._data = []
    def add(self, line, filename):
        s = Station(line, filename, self._stationHeader)
        if s._ok:
            self._data.append(s)
    def print(self):
        print("Nothing")
    def hasTimesInRange(self, start_date, end_date):
        for s in self._data:
            if (s.timeInRange(start_date, end_date)):
                return True
        return False

#----------------------------------------------
def doCmd(cmd, debug=False):
    #print(cmd)
  my_env = os.environ.copy()
  args = shlex.split(cmd)
  proc = Popen(args, stdout=PIPE, stderr=PIPE, env=my_env)
  out, err = proc.communicate()
  exitcode = proc.returncode
  if exitcode == 0:
    return str(out)
  else:
    if debug:
        print("Command failed ", cmd)
    return ""

#----------------------------------------------
def getdatafilenames(aDir):
    if (os.path.exists(aDir)):
        allFiles = [name for name in os.listdir(aDir) \
                    if not os.path.isdir(os.path.join(aDir, name))]
        return [s for s in allFiles if '.dat' in s]
    else:
        return []
        
#----------------------------------------------
def run2(data_path, start, end):

    if (data_path[0:2] != "./" and data_path[0] != '/'):
        inpath = "./" + data_path
    else:
        inpath = data_path

    print("data_path = ", inpath)

    # could put testing here to make sure strings will convert
    print("start = ", start)
    print("end = ", end)
   
    y0 = int(start[0:4])
    m0 = int(start[4:6])
    d0 = int(start[6:8])

    y1 = int(end[0:4])
    m1 = int(end[4:6])
    d1 = int(end[6:8])

    print("Looking for file with data in range ", y0, m0, d0, " to ", y1, m1, d1)

    # read each file that ends in .dat
    stationfiles = getdatafilenames(inpath)
    stationfiles.sort()
    
    print("We have ", len(stationfiles), " data files to look at")
    start_date = date(y0, m0, d0)
    end_date = date(y1, m1, d1)

    for i in range(len(stationfiles)):

        #print("Looking at ", stationfiles[i])
        with open(inpath + "/" + stationfiles[i], 'r') as file:
            data_all = file.read()
        file.close()
        lines = data_all.splitlines()

        # first line is a header, remaining lines are a time series
        sh = StationHeader(lines[0], stationfiles[i])
        if sh._ok:
            lines = lines[1:]
            st = StationTimeSeries(sh)
            for l in lines:
                st.add(l, stationfiles[i])

            if (st.hasTimesInRange(start_date, end_date)):
                print(stationfiles[i])
            
#----------------------------------------------
def create_parser_options(parser):
    parser.add_option("-d", "--data_path", dest="data_path",
            default="./iabp_files", help=" path to the station files (.dat) (default: ./iabp_files)")
    parser.add_option("-s", "--start", dest="start",
            default="notset", help=" starting yyyymmdd.  Must be set")
    parser.add_option("-e", "--end", dest="end",
            default="notset", help=" ending yyyymmdd.  Must be set")
    return parser.parse_args()
            
#----------------------------------------------
if __name__ == "__main__":
  usage_str = "%prog [options]"
  parser = OptionParser(usage = usage_str)
  options, args = create_parser_options(parser)
  if (options.start == "notset" or options.end == "notset"):
     usage()
     exit(0)
  run2(options.data_path, options.start, options.end)
  exit(0)
