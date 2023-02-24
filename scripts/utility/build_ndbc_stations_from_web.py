#!/usr/bin/env python3

'''
Created on February 17, 2023

@author: davealbo

The script reads NDBC station information from two NOAA websites and merges the contents into one local list.
The list contains latitude, longitude and elevation data for all known stations.
The local list can be read by ascii2nc for processing of NDBC data.
Algorithm:
   Pull down active station xml file from web and create a list of active information objects.
   Write the list to an active stations text file.
   Pull down complete index list from web.
   for each file refered to in the complete index list contents:
      pull down that stations web page data and append to list of complete station information objects.
   Write the list of complete station info objects to a text file.
   Save all the individual web page data that was pulled down into a subdirectory.

   Warn about stations that are both active and complete but have different values, priority going to TBD.
   Count up number of stations that are active but not complete, complete but not active and report.

   Create a master list of all the unique stations from both lists, defer to TBD for any conflicts.
   Write the master list.

   Compare the complete list to the previous list of stations in the MET repo, and report on any stations
   that have disappeared from the web, or have changed locations.

'''

from optparse import OptionParser
import os
import shutil
import shlex
import errno
from subprocess import Popen, PIPE

# this needs to change!
# hardwired location of current default stations file
DEFAULT_STATIONS_FILE = "../../data/table_files/ndbc_stations.xml"

# hardwired NOAA top webpage
TOP_WEBSITE = "https://www.ndbc.noaa.gov"

# hardwired website with active station xml
ACTIVE_WEBSITE = "https://www.ndbc.noaa.gov/activestations.xml"

# hardwired data subdirectory
DATA_SUBDIR = "/data"

#hardwired complete stations subdirecdtory
STATIONS_SUBDIR = "/data/stations"

# hardwired result of a wget of ACTIVE_WEBSITE
ACTIVE_STATIONS_XML = "./data/activestations.xml"
  
# hardwired website with index to a complete list of stations
COMPLETE_INDEX_WEBSITE = "https://www.ndbc.noaa.gov/to_station.shtml"

# hardwired result of a wget of COMPLETE_INDEX_WEBSITE
COMPLETE_STATIONS_INDEX_INFO = "./data/to_station.shtml"
  
# hardwired name of optionally saved active stations
ACTIVE_TEXT_FILE = "./data/active.txt"

# hardwired name of optionally saved complete stations
COMPLETE_TEXT_FILE = "./data/complete.txt"

# default output file name
DEFAULT_OUTPUT_FILE = "merged.txt"

MISSING = -99.9

def usage():
    print(f'Usage: BuildNdbcStationsFromWeb.py , <--diagnostic> <--out=out_filename>')
    print(f'          -d/--diagnostic: special mode to rerun using already downloaded files, skips all downloading if True')
    print(f'          -o/--out=out_filename: save final text into the named file (default: file name is {DEFAULT_OUTPUT_FILE})"')
    print(f'       Note: <> indicates optional arguments')

#----------------------------------------------
def create_parser_options(parser):
    parser.add_option("-d", "--diagnostic", dest="diagnostic", action="store_true", default=False, help="Rerun using downlaoded files, skipping download step (optional, default: False)")
    parser.add_option("-o", "--out", dest="out_file",
            default=DEFAULT_OUTPUT_FILE, help=" Save the text into the named file (default: " + DEFAULT_OUTPUT_FILE +" )")
    parser.add_option("-H", "--Help", dest="full_usage", action="store_true", default=False, help = " show more usage information (optional, default = False)")
    return parser.parse_args()

#----------------------------------------------
class Station:
  def __init__(self, name = "", idvalue="", lat=MISSING, lon=MISSING, elev=MISSING):
    self._name = name
    self._id = idvalue
    self._lat = lat
    self._lon = lon
    self._elev = elev

  def empty(self):
    return self._id == ""

  def textForLookups(self):
    txt = '<station id="{a}" lat="{b}" lon="{c}" elev="{d}"/>'.format(a=self._id,b=self._lat,c=self._lon,d=self._elev)
    return txt
  
  def location_match(self, other):
    if self.empty() or other.empty():
      # this method is used to print mismatches, so don't print mismatches to empty stations
      return True
    return self._lat == other._lat and self._lon == other._lon and self._elev == other._elev

  def location_string(self):
    txt = '{a}({b},{c},{d})'.format(a=self._name,b=self._lat,c=self._lon,d=self._elev)
    return txt
  
  def equals(self, other):
    return self._id == other._id and self._lat == other._lat and self._lon == other._lon and self._elev == other._elev
  
#----------------------------------------------
def matchingId(id, stations):
  for station in stations:
    if station._id == id:
      return station
  return Station()

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
def main(diagnostic, out_file):
  status = True

  cwd = os.getcwd()

  if not diagnostic:
    dataDir = cwd + DATA_SUBDIR
    print("cleanining out ", dataDir)
    makeOrScrub(dataDir)

    os.chdir(dataDir)

    # pull the active stations xml from the web
    cmd = "wget " + ACTIVE_WEBSITE
    print(cmd)
    s = doCmd(cmd, True)
    if not s:
      status = False
    # pull the complete stations html from the web
    cmd = "wget " + COMPLETE_INDEX_WEBSITE
    print(cmd)
    s = doCmd(cmd, True)
    if not s:
      status = False
    if not status:
      print("ERROR reading web content")
      os.exit(1)

    # move back to top directory
    os.chdir(cwd)

  # prepare to compare to the default stations file to see what has changed
  default_stations = parse("Default", DEFAULT_STATIONS_FILE)
  #[ids_default, lats_default, lons_default, elevs_default] = parse(DEFAULT_STATIONS_FILE)
  print("PARSED DEFAUILT STATIONS FILE NUM=", len(default_stations))
  
  # parse the active stations XML to create a list, which will become the final list
  if diagnostic:
    active_stations = parse("Active", ACTIVE_TEXT_FILE)
    print("PARSED ACTIVE STATION FILES: num=", len(active_stations))
  else:
    active_stations = processActive("Active")
    print("BUILT ACTIVE STATION FILES: num=", len(active_stations))

  # read the complete stations html, find all the individual stations web links,
  # pull each stations data, parse that downloaded station content to create a list
  if diagnostic:
    complete_stations = parse("Complete", COMPLETE_TEXT_FILE)
    #[ids_complete, lats_complete, lons_complete, elevs_complete] = parse(COMPLETE_TEXT_FILE)
    print("PARSED COMPLETE STATIONS FILES: num=", len(complete_stations))
  else:
    complete_stations = processComplete("Complete")
    #[ids_complete, lats_complete, lons_complete, elevs_complete] = processComplete()
    print("BUILT COMPLETE STATIONS FILES: num=", len(complete_stations))

  # see which ids are not in complete from active,  and which have different lat/lons
  # note the one used if that happens is always the active one at this point
  numConflict = 0
  numActiveNotComplete = 0
  numCompleteNotActive = 0
  numCompleteAndActive = 0
  
  for active in active_stations:
    id = active._id
    complete = matchingId(id, complete_stations)
    if complete.empty():
      numActiveNotComplete = numActiveNotComplete+1
    else:
      numCompleteAndActive = numCompleteAndActive+1
      if (not active.location_match(complete)):
        numConflict = numConflict + 1
        print("latlonelev disagree for ", id, ":", active.location_string(), ",", complete.location_string())

  for complete in complete_stations:
    id = complete._id
    active = matchingId(id, active_stations)
  if active.empty():
    numCompleteNotActive = numCompleteNotActive + 1

  # see which id's have vanished from the current default list 
  # and which have conflicts with active and/or complete lists
  numVanished = 0
  print("Comparing current default stations to final list")
  for default in default_stations:
    id = default._id
    active = matchingId(id, active_stations)
    complete = matchingId(id, complete_stations)
    if active.empty() and complete.empty():
      print("Station in the local table file but no longer on the webpages:", id)
      numVanished = numVanished+1
    else:
      if (not active.location_match(default)):
        numConflict = numConflict + 1
        print("latlonelev disagree for ", id, ":", active.location_string(), ",", default.location_string())
      if not active.equals(complete):
        if (not complete.location_match(default)):
          numConflict = numConflict + 1
          print("latlonelev disagree for ", id, ":", complete.location_string(), ",", default.location_string())

  # see which ids are not in active but are in complete, make a list of those as ones to merge
  # Note might add in something about the default lists as well
  toMerge = []
  for complete in complete_stations:
    id = complete._id
    active = matchingId(id, active_stations)
    if active.empty():
      toMerge.append(complete)
  print("Merging ", len(toMerge), " items from complete into active to make final list")
  final_stations = active_stations
  for m in toMerge:
    final_stations.append(m)

  numNew = 0
  for f in final_stations:
    id = f._id
    default = matchingId(id, default_stations)
    if default.empty():
      print("New station on web not in local table file:", id)
      numNew = numNew+1

  #now write out the full meal deal by creating a string list
  nout = 0
  txtAll = []
  for f in final_stations:
    txt = f.textForLookups()
    txtAll.append(txt)
    nout = nout + 1

  # sort for ease of use
  txtAll.sort()
  fout = open(out_file, "w")
  for txt in txtAll:
    fout.write(txt+"\n")
  fout.close()

  print("Done, wrote out ", nout, " total items to ", out_file)
  print("Number of stations that vanished (are in default ndbc_stations.xml and are not now online): ", numVanished)
  print("Number of stations that appeared (not in default ndbc_stations.xml and are now online): ", numNew)
  print("Number of stations for which there is a conflict from the various sources:", numConflict)
  print("Number of stations for which there is both and active and a complete entry:", numCompleteAndActive)
  print("Number of stations for which there is an active but no complete entry:", numActiveNotComplete)
  print("Number of stations for which there is a complete but no active entry:", numCompleteNotActive)
  
  return 0  
    
#----------------------------------------------------
def processComplete(name):
  '''
  read the complete stations html, find all the individual stations web links,
  pull each stations data, parse that downloaded station content to create a list
  '''

  # initialize return to empty
  stations = []
  
  # create the output location, which should be ./data/stations
  cwd = os.getcwd()
  outLoc = cwd + STATIONS_SUBDIR
  if not makeDirIfNeeded(outLoc):
    print("ERROR creating storage for individual station files ", outLoc)
    return stations


  # Open the file with the list of php pages online (or local files pulled down)
  with open(COMPLETE_STATIONS_INDEX_INFO, 'r') as file:
    data = file.read().replace('\n', '')
  file.close()

  # start at the beginning
  index = 0
  txtAll = []
  while index < len(data):
    # pull down another stations info if you can, and parse it
    [index, station] = createNextStationInfo(name, data, index)
    if index == -1:
      break
    if not station.empty():
      # form a string and append that plus all the individual stuff to lists
      txt = station.textForLookups()
      txtAll.append(txt)
      stations.append(station)

  # keep the subdirectory of individual stations information
  # sort the list for ease of use, then write it
  txtAll.sort()
  fout = open(COMPLETE_TEXT_FILE, "w")
  for txt in txtAll:
    fout.write(txt+"\n")
  fout.close()  
  return stations

#----------------------------------------------
def createNextStationInfo(name, data, i):

  #lat = MISSING
  #lon = MISSING
  #elev = MISSING
  #station = ""

  s = Station()

  #data has entries like this:  <a href="station_page.php?station=45001">45001</a>
  #on entry i points to the starting location within data to start looking
  index = data.find('href="station_page.php?', i)
  if index == -1:
    return [-1, s]

  # the stuff beyond 'href="' is the file name that you get via wget, followed by another  '"'
  index2 = index + 6   # point to 'station_page'
  index3 = data.find('">', index2)  # point to " at end (which is followed by >)

  index = index3 + 3  # set index for return to beyond this

  # what to go for online:
  ref = TOP_WEBSITE + '/' + data[index2:index3]

  # name of returned file
  filename = data[index2:index3]
  
  # temporarily change to the correct subdirectory
  cwd = os.getcwd()
  os.chdir(cwd + STATIONS_SUBDIR)
  # go get it
  cmd = 'wget "' + ref + '"'
  print(cmd)
  s = doCmd(cmd, True)
  # move back
  os.chdir(cwd)
  if not s:
    # note try to keep going forward as index has been updated
    print("ERROR data not online: ", ref)
    return [index, s]

  # parse the file and return the information, including the next index
  return parseStationInfo(name, cwd + STATIONS_SUBDIR + "/" + filename, index)

#----------------------------------------------------------------------------
def makeDirIfNeeded(path, debug=False):
  if (debug):
    print("Making directory if needed " + path)

  try:
    os.makedirs(path)
    return True
  except OSError as exception:
    if exception.errno != errno.EEXIST:
      print("ERROR creating", path)
      return False
    else:
      return True

#----------------------------------------------------------------------------
def parseStationInfo(name, fname, index):

  s = Station()

  # the file is assumed already downloaded
  # initialize station values
  station = setStationId(fname)
  if not station:
    return [index, s]
  elev = setElev(fname)
  lat = setLat(fname)
  lon = setLon(fname)
  s = Station(name, station, lat, lon, elev)
  return [index, s]

#----------------------------------------------
def setStationId(fname):
  stationId = ""
  cmd = 'grep "var currentstnid" ' + fname 
  s = doCmd(cmd, True)
  if s:
    index6 = s.find("'", 0)
    index7 = s.find("'", index6+1)
    stationId = s[index6+1:index7]
  return stationId

#----------------------------------------------
def setElev(fname):
  elev = MISSING
  cmd = 'grep "Site elev" ' + fname
  #print(cmd)
  s = doCmd(cmd)
  if s:
    if "m above mean sea level" in s:
      # scan to </b>
      index6 = s.find("</b>")
      index7 = s.find("m above")
      elev = float(s[index6+4:index7])
    elif "</b> sea level<br" in s:
      elev = 0.0
    else:
      print("ERROR UNKNOWN FORMAT FOR ELEV:", s)
  #else:
    #print("No Site Elev for ", name)
  return elev

#----------------------------------------------
def setLat(fname):
  lat = MISSING
  cmd = 'grep "var currentstnlat" ' + fname
  s = doCmd(cmd, True)
  if s:
    index6 = s.find("=")
    index7 = s.find(";")
    lat = float(s[index6+1:index7])
  else:
    print("ERROR No Lat for ", fname)
  return lat

#----------------------------------------------
def setLon(fname):
  lon = MISSING
  cmd = 'grep "var currentstnlng" ' + fname
  s = doCmd(cmd, True)
  if s:
    index6 = s.find("=")
    index7 = s.find(";")
    lon = float(s[index6+1:index7])
  else:
    print("ERROR No Lon for ", fname)
  return lon

#----------------------------------------------
def processActive(name):
  '''
  read the active stations XML to create a list
  '''
  astations = parse(name, ACTIVE_STATIONS_XML)
  txtAll = []
  for s in astations:
    txt = s.textForLookups()
    txtAll.append(txt)
  txtAll.sort()    

  fout = open(ACTIVE_TEXT_FILE, "w")
  for txt in txtAll:
    fout.write(txt+"\n")
  fout.close()
  return astations

#----------------------------------------------
def parse(name, fname, debug=False):
  if debug:
    print("Parsing ", fname)
  stations = []
  #ids = []
  #lats = []
  #lons = []
  #elevs = []

  with open(fname, 'r') as file:
    data_all = file.read().replace('\n', '')
  file.close()
  index_all = 0
  while index_all < len(data_all):
    index_all = data_all.find('<station id=', index_all)
    if index_all == -1:
      break
    indexend = data_all.find('/>', index_all+1)
    if indexend == -1:
      print("UNexpected lack of />")
      break
    
    data = data_all[index_all:indexend+2]
    if debug:
      print("Parsing this: ", data)
    index = 0

    # expect to see '<station id="xxxx" lat="##" lon="##"
    index1 = data.find('"', index)
    index2 = data.find('"', index1+1)
    stationId = data[index1+1:index2]

    index3 = data.find('lat="', index2)
    index4 = data.find('"', index3+5)
    lat = float(data[index3+5:index4])

    index5 = data.find('lon="', index4)
    index6 = data.find('"', index5+5)
    lon = float(data[index5+5:index6])

    index7 = data.find('elev="', index6)
    if index7 == -1:
      #print("Station has no elev ", stationId)
      elev = MISSING
      index8 = index6
    else:
      index8 = data.find('"', index7+6)
      elev = float(data[index7+6:index8])

    stations.append(Station(name, stationId, lat, lon, elev))
    #ids.append(stationId)
    #lats.append(lat)
    #lons.append(lon)
    #elevs.append(elev)

    index_all = indexend+2

  #return [ids, lats, lons, elevs]
  return stations

#----------------------------------------------
if __name__ == "__main__":
  usage_str = "%prog [options]"
  parser = OptionParser(usage = usage_str)
  options, args = create_parser_options(parser)
  print(options)
  if options.full_usage:
    usage()
    exit(0)
  main(options.diagnostic, options.out_file)

