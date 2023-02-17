#!/usr/bin/env python3

'''
Created on February 17, 2023

@author: davealbo

The script reads NDBC station information from two NOAA websites and merges the contents into one local list.
The list contains latitude, longitude and elevation data for all known stations.
The local list can be read by ascii2nc for processing of NDBC data.
Algorithm:
   Pull down active station xml file from web and create a list of active information.
   Optionally write the list to an active stations text file.

   Pull down complete index list from web.
   for each file refered to in the complete index list contents:
      pull down that stations web page data and append to list of complete station information
   optionally write the list of complete station info to a text file.
   optionally save all the individual web page data that was pulled down into a subdirectory.

   warn about stations that are both active and complete but have different values
   create a master list of all the unique stations from both lists, defer to active for any conflicts
   write the master list
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

# hardwired result of a wget of ACTIVE_WEBSITE
ACTIVE_STATIONS_XML = "activestations.xml"
  
# hardwired website with index to a complete list of stations
COMPLETE_INDEX_WEBSITE = "https://www.ndbc.noaa.gov/to_station.shtml"

# hardwired result of a wget of COMPLETE_INDEX_WEBSITE
COMPLETE_STATIONS_INDEX_INFO = "to_station.shtml"
  
# hardwired name of optionally saved active stations
ACTIVE_TEXT_FILE = "active.txt"

# hardwired name of optionally saved complete stations
COMPLETE_TEXT_FILE = "complete.txt"

# default output file name
DEFAULT_OUTPUT_FILE = "merged.txt"

MISSING = -99.9

def usage():
    print(f'Usage: BuildNdbcStationsFromWeb.py <--save_complete_stations> <--save_intermediate_lists> <--out=out_filename>')
    print(f'          -s/--save_intermediate_lists: save intermediate active and complete text files, as well as a subdirectory with all the individual complete stations full information (optional, default: False)"')
    print(f'                                     if True files {ACTIVE_TEXT_FILE} and ${COMPLETE_TEXT_FILE} and contents of subdir ./stations are saved, otherwise deleted"')
    print(f'          -o/--out=out_filename: save final text into the named file (default: file name is {DEFAULT_OUTPUT_FILE})"')
    print(f'       Note: <> indicates optional arguments')

#----------------------------------------------
def create_parser_options(parser):
    parser.add_option("-i", "--save_intermediate_lists", dest="save_intermediate_lists",
                          action="store_true", default=False,
                          help=" save intermediate files pulled from web:" + ACTIVE_TEXT_FILE + "," + COMPLETE_TEXT_FILE + "and save individual complete station info pulled from the web into a subdirectory ./stations  (optional, default: False)")
    parser.add_option("-o", "--out", dest="out_file",
            default=DEFAULT_OUTPUT_FILE, help=" Save the text into the named file (default: " + DEFAULT_OUTPUT_FILE +" )")
    parser.add_option("-H", "--Help", dest="full_usage", action="store_true", default=False, help = " show more usage information (optional, default = False)")
    return parser.parse_args()

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
def main(save_intermediate_lists, out_file):
  status = True

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
  
  # prepare to compare to the default stations file to see what has changed
  [ids0, lats0, lons0, elevs0] = parse(DEFAULT_STATIONS_FILE)
  

  # parse the active stations XML to create a list
  [ids, lats, lons, elevs] = processActive(save_intermediate_lists)

  # read the complete stations html, find all the individual stations web links,
  # pull each stations data, parse that downloaded station content to create a list
  [ids2, lats2, lons2, elevs2] = processComplete(save_intermediate_lists)

  # see which ids are not in complete from active,  and which have different lat/lons
  # note the one used if that happens is always the active one
  for id in ids:
    i1 = ids.index(id)
    if id in ids2:
      i2 = ids2.index(id)
      if (lats[i1] != lats2[i2]) or (lons[i1] != lons2[i2]) or (elevs[i1] != elevs2[i2]):
        print("latlonelev disagree for ", id, " active:(", lats[i1], ",", lons[i1], ",", elevs[i1],
                ")  complete:(", lats2[i2], ",", lons2[i2], ",", elevs2[i2], ")")

  # see which ids are not in active but are in complete, make a list of those as ones to merge
  toMergeIndex = []
  for id in ids2:
    i2 = ids2.index(id)
    if id in ids:
      i1 = ids.index(id)
    else:
      toMergeIndex.append(i2)

  print("Merging ", len(toMergeIndex), " items from complete into active")

  for i in toMergeIndex:
    id = ids2[i]
    lat = lats2[i]
    lon = lons2[i]
    elev = elevs2[i]
    ids.append(id)
    lats.append(lat)
    lons.append(lon)
    elevs.append(elev)

  #now write out the full meal deal by creating a string list
  nout = 0
  txtAll = []
  for id in ids:
    i = ids.index(id)
    station = id
    lat = lats[i]
    lon = lons[i]
    elev = elevs[i]
    txt = '<station id="{a}" lat="{b}" lon="{c}" elev="{d}"/>'.format(a=station,b=lat,c=lon,d=elev)
    txtAll.append(txt)
    nout = nout + 1

  # sort for ease of use
  txtAll.sort()
  fout = open(out_file, "w")
  for txt in txtAll:
    fout.write(txt+"\n")
  fout.close()
  print("Done, wrote out ", nout, " total items to ", out_file)


  print("Comparing to current default stations")
  for id in ids0:
    if id not in ids:
      print("Station vanished from web:", id)
    else:
      i0 = ids0.index(id)
      station = id
      lat0 = lats0[i0]
      lon0 = lons0[i0]
      elev0 = elevs0[i0]
      i = ids.index(id)
      lat = lats[i]
      lon = lons[i]
      elev = elevs[i]
      if (lat0 != lat or lon0 != lon or elev0 != elev):
        print("latlonelev disagree for ", id, " web:(", lat, ",", lon, ",", elev,
                ")  default:(", lat0, ",", lon0, ",", elev0, ")")
  for id in ids:
    if id not in ids0:
      print("New station on web not in defaults:", id)

  return 0  
    
#----------------------------------------------------
def processComplete(save_intermediate):
  '''
  read the complete stations html, find all the individual stations web links,
  pull each stations data, parse that downloaded station content to create a list
  '''

  # initialize these lists to empty
  ids = []
  lats = []
  lons = []
  elevs = []

  # Open the file with the list of php pages online (or local files pulled down)
  with open(COMPLETE_STATIONS_INDEX_INFO, 'r') as file:
    data = file.read().replace('\n', '')
  file.close()

  # start at the beginning
  index = 0
  txtAll = []
  while index < len(data):
    # pull down another stations info if you can, and parse it
    [index, lat, lon, elev, station] = createNextStationInfo(data, index)
    if index == -1:
      break

    if station:
      # form a string and append that plus all the individual stuff to lists
      txt = '<station id="{a}" lat="{b}" lon="{c}" elev="{d}"/>'.format(a=station,b=lat,c=lon,d=elev)
      txtAll.append(txt)
      ids.append(station)
      lats.append(lat)
      lons.append(lon)
      elevs.append(elev)

  if save_intermediate:
    # keep the subdirectory of individual stations information

    # sort the list for ease of use, then write it
    txtAll.sort()
    fout = open(COMPLETE_TEXT_FILE, "w")
    for txt in txtAll:
      fout.write(txt+"\n")
    fout.close()  


  else:
    # delete the subdirectory stations
    path = os.getcwd() + "/stations"
    if (os.path.exists(path)):
      try:
        shutil.rmtree(path)
      except:
        print('WARNING: ' + path + ' not completely cleaned out.')

  return [ids, lats, lons, elevs]

#----------------------------------------------
def createNextStationInfo(data, i):

  lat = MISSING
  lon = MISSING
  elev = MISSING
  station = ""

  #data has entries like this:  <a href="station_page.php?station=45001">45001</a>
  #on entry i points to the starting location within data to start looking
  index = data.find('href="station_page.php?', i)
  if index == -1:
    return [-1, lat, lon, elev, station]

  # the stuff beyond 'href="' is the file name that you get via wget, followed by another  '"'
  index2 = index + 6   # point to 'station_page'
  index3 = data.find('">', index2)  # point to " at end (which is followed by >)

  index = index3 + 3  # set index for return to beyond this

  # what to go for online:
  ref = TOP_WEBSITE + '/' + data[index2:index3]

  # name of returned file
  filename = data[index2:index3]
  
  # go get it
  cmd = 'wget "' + ref + '"'
  print(cmd)
  s = doCmd(cmd, True)
  if not s:
    # note try to keep going forward as index has been updated
    print("ERROR data not online: ", ref)
    return [index, lat, lon, elev, station]

  # make a stations subdirectory if needed
  cwd = os.getcwd()
  if not makeDirIfNeeded(cwd + "/stations"):
    print("ERROR cannot create needed subdirectory 'stations'")
    return [index, lat, lon, elev, station]
  
  # move this file to the stations subdirectory
  shutil.move(cwd + "/" + filename, cwd + "/stations/" + filename)
    
  # parse the file and return the information, including the next index
  return parseStationInfo(filename, index)

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
def parseStationInfo(filename, index):

  lat = MISSING
  lon = MISSING
  elev = MISSING
  station = ""

  # the file is assumed already downloaded,  in subdirectory stations
  fname = "./stations/" + filename

  # initialize station values
  station = setStationId(fname)
  if not station:
    return [index, lat, lon, elev, station]
  elev = setElev(fname)
  lat = setLat(fname)
  lon = setLon(fname)

  return [index, lat, lon, elev, station]

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
def processActive(save_intermediate):
  '''
  read the active stations XML to create a list
  '''
  [ids, lats, lons, elevs] = parse(ACTIVE_STATIONS_XML)
  txtAll = []
  for id in ids:
    i = ids.index(id)
    station = ids[i]
    lat = lats[i]
    lon = lons[i]
    elev = elevs[i]
    txt = '<station id="{a}" lat="{b}" lon="{c}" elev="{d}"/>'.format(a=station,b=lat,c=lon,d=elev)
    txtAll.append(txt)
  txtAll.sort()    

  if save_intermediate:
    fout = open(ACTIVE_TEXT_FILE, "w")
    for txt in txtAll:
      fout.write(txt+"\n")
    fout.close()

  return [ids, lats, lons, elevs]

#----------------------------------------------
def parse(fname):
  ids = []
  lats = []
  lons = []
  elevs = []

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

    ids.append(stationId)
    lats.append(lat)
    lons.append(lon)
    elevs.append(elev)
    index_all = indexend+3

  return [ids, lats, lons, elevs]

#----------------------------------------------
if __name__ == "__main__":
  usage_str = "%prog [options]"
  parser = OptionParser(usage = usage_str)
  options, args = create_parser_options(parser)
  print(options)
  if options.full_usage:
    usage()
    exit(0)
  main(options.save_intermediate_lists, options.out_file)

