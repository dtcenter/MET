 #!/usr/bin/env python3

from optparse import OptionParser
import urllib.request
import os
import shutil
import shlex
import errno
from subprocess import Popen, PIPE

#----------------------------------------------------------------------------
def makeOrScrub(path, debug=False):
  if (debug):
    print("Recreating path " + path)
  if (os.path.exists(path)):
    try:
      shutil.rmtree(path)
      os.makedirs(path)
    except:
      print('WARNING: ' + path + ' not completely cleaned out.')
  else:
    os.makedirs(path)
   

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
        self._bpIndex = lookFor('Lon', tokens, filename, False)
        self._tsIndex = lookFor('Lon', tokens, filename, False)
        self._taIndex = lookFor('Lon', tokens, filename, False)
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
        self._hour = pointToInt(stationHeader._hourIndex, tokens, filename)
        if self._hour < 0:
            self._ok = False
        self._minute = pointToInt(stationHeader._minuteIndex, tokens, filename)
        if self._minute < 0:
            self._ok = False
        self._doy = pointToFloat(stationHeader._doyIndex, tokens, filename)
        if self._doy < 0:
            self._ok = False
        self._posdoy = pointToFloat(stationHeader._posdoyIndex, tokens, filename)
        if self._posdoy < 0:
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
def nextStation(data_all, index_all):

    index_all = data_all.find('.dat', index_all)
    data = ""
    if index_all == -1:
        return -1, data
  
    #is this the weird (bad) .dat.dat situtation?
    teststr = data_all[index_all:index_all+8]
    if teststr == ".dat.dat":
        indexend = data_all.find('.dat.dat<', index_all+1)
        if indexend == -1:
            print("Unexpected lack of .dat.dat<")
            return -1, data
        data = data_all[index_all+10:indexend+8]
    else:
        indexend = data_all.find('.dat<', index_all+1)
        if indexend == -1:
            print("UNexpected lack of .dat<")
            return -1, data
        data = data_all[index_all+6:indexend+4]
    return indexend+10, data

#----------------------------------------------
def getStation(sfile):
    cmd = "wget https://iabp.apl.uw.edu/WebData/" + sfile
    print(cmd)
    doCmd(cmd, True)

    # parse contents (not used for anything just yet)
    with open(sfile, 'r') as file:
        data_all = file.read()
        file.close()
        lines = data_all.splitlines()

        # first line is a header, remaining lines are a time series
        sh = StationHeader(lines[0], sfile)
        if sh._ok:
            lines = lines[1:]
            st = StationTimeSeries(sh)
            for l in lines:
                st.add(l, sfile)

            
#----------------------------------------------
def run(output_path):

    cwd = os.getcwd()

    if (output_path[0:2] != "./" and output_path[0] != '/'):
        outpath = "./" + output_path
    else:
        outpath = output_path
    makeOrScrub(outpath, True)
    os.chdir(outpath)

    cmd = "wget https://iabp.apl.uw.edu/WebData"
    print(cmd)
    s = doCmd(cmd, True)
    if not s:
      status = False

    stationfiles = []
    with open("WebData", 'r') as file:
        data_all = file.read().replace('\n', '')
    file.close()
    index_all = 0
    while index_all < len(data_all):
        index_all, data = nextStation(data_all, index_all)
        if (index_all == -1):
          break;
        stationfiles.append(data)

    print("Parsed out ", len(stationfiles), " individual station files")

    # pull down all the station files
    for i in range(len(stationfiles)):
        getStation(stationfiles[i])
        
    print("created ", len(stationfiles), " station files in ", outpath)
    os.chdir(cwd)

#----------------------------------------------
def create_parser_options(parser):
    parser.add_option("-o", "--output_path", dest="output_path",
            default="./iabp_files", help=" create an output path or clear out what is there and put output files to that path (default: ./iabp_files)")
    #parser.add_option("-H", "--Help", dest="options", action="store_true", default=False, help = " show usage information (optional, default = False)")
    return parser.parse_args()

#----------------------------------------------
if __name__ == "__main__":

  usage_str = "%prog [options]"
  parser = OptionParser(usage = usage_str)
  options, args = create_parser_options(parser)
  run(options.output_path)
  exit(0)
