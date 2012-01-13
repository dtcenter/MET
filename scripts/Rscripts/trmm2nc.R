########################################################################
##
##   Name: trmm2nc.R
##
##   Description:
##      This script should be called via an Rscript command to reformat
##      ASCII TRMM precipitation pcp into a NetCDF format that MET
##      can read.  The input pcp for this script can be attained via:
##      24-hour accumulations:
##        http://disc2.nascom.nasa.gov/Giovanni/tovas/realtime.3B42RT_daily.shtml
##      03-hour accumulations:
##        http://disc2.nascom.nasa.gov/Giovanni/tovas/realtime.3B42RT.shtml
##      GES DISC FAQ:
##        http://disc.sci.gsfc.nasa.gov/additional/faq/precipitation_faq.shtml
##
##   Usage:
##      Rscript trmm2nc.R
##         trmm_file
##         nc_file
##         [-save]
##
##   Arguments:
##      "trmm_file" is an ASCII file containing TRMM accumulated precipitation pcp.
##      "nc_file"   is the out NetCDF file to be written.
##      "-save"     to call save.image().
##
##   Details:
##
##   Examples:
##      Rscript trmm2nc.R \
##        trmm_sample.txt \
##        trmm_sample.nc
##
##   Author:
##      John Halley Gotway (johnhg@ucar.edu), NCAR-RAL-DTC
##      09/02/2011
##
##   Change Log:
##      Mod#   Date        Name           Description
##      ----   ----        ----           -----------
##      000    2011-09-02  Halley Gotway  New
##      001    2011-12-02  Halley Gotway  Fix parsing of the init and
##                         valid times from the header rather than
##                         assuming accumulations of 03 or 24-hours.
##
########################################################################

########################################################################
#
# Required libraries.
#
########################################################################

library(ncdf)

########################################################################
#
# Constants and command line arguments
#
########################################################################

hdr_len    = 5           # Each pcp file begins with 5 header lines.
hdr_file   = "trmm.hdr"  # Temporary header file.
missing    = -9999       # Missing pcp value to be used in MET.
save       = FALSE       # If set to TRUE, call save.image()
sec_per_hr = 60*60

########################################################################
#
# Handle the arguments.
#
########################################################################

# Retreive the arguments
args = commandArgs(TRUE)

# Check the number of arguments
if(length(args) < 2) {
   cat("Usage: trmm2nc.R\n")
   cat("        trmm_file\n")
   cat("        nc_file\n")
   cat("        [-save]\n")
   cat("        where \"trmm_file\" is an ASCII file containing TRMM accumulated precipitation pcp.\n")
   cat("              \"nc_file\"   is the out NetCDF file to be written.\n")
   cat("              \"-save\"     to call save.image().\n\n")
   quit()
}

# Store the input file names
trmm_file = args[1]
nc_file   = args[2]

# Parse optional arguments
for(i in 1:length(args)) {
   if(args[i] == "-save") {
      save = TRUE
   }
}

########################################################################
#
# Process the header pcp.
#
########################################################################

# Read the header pcp
sys_cmd = paste("head -", hdr_len, " ", trmm_file, " > ", hdr_file, sep="")
system(sys_cmd)
hdr = read.table(hdr_file, fill=TRUE)
sys_cmd = paste("rm -f", hdr_file)
system(sys_cmd)

# Determine the accumlation interval
if(as.character(hdr[1,3]) == "TRMM") {
  acc_type = 24
} else if(as.character(hdr[1,3]) == "3-hourly") {
  acc_type = 3
} else {
  cat("\n\nERROR: Can\'t figure out the accumulation interval!\n\n")
  exit(1)
}

# Parse the init and valid times
#   http://disc.sci.gsfc.nasa.gov/additional/faq/precipitation_faq.shtml#convert
if(acc_type == 03) {

  # For 03-hour accumulations, the time stamp gives the middle of the
  # accumulation interval.
  # Subtract 1.5 hours from the begin timestamp to get the init time.
  # Add 1.5 hours to the end timestamp to get the valid time.

  init  = as.POSIXct(strptime(unlist(strsplit(as.character(hdr[3,4]), '-'))[1],
                              format="(%HZ%d%b%Y", tz="GMT") - 1.5*sec_per_hr)
  valid = as.POSIXct(strptime(unlist(strsplit(as.character(hdr[3,4]), '-'))[2],
                              format="%HZ%d%b%Y)", tz="GMT") + 1.5*sec_per_hr)

} else {

  # The 24-hour accumulations are actually a sum of 03-hour accumulations:
  #   00Z, 03Z, 06Z, 12Z, 15Z, 18Z, and 21Z
  # Subtract 1.5 hours from the begin date to get the init time.
  # Add 22.5 hours to the end date to get the valid time.

  init  = as.POSIXct(strptime(unlist(strsplit(as.character(hdr[3,4]), '-'))[1],
                              format="(%d%b%Y", tz="GMT") - 1.5*sec_per_hr)
  valid = as.POSIXct(strptime(unlist(strsplit(as.character(hdr[3,4]), '-'))[2],
                              format="%d%b%Y)", tz="GMT") + 22.5*sec_per_hr)
}

# Compute the accumulation interval
acc_sec =  as.double(valid - init, units="secs")
acc_hr  = floor(acc_sec / 3600)
acc_str = sprintf("%.2i", acc_hr)

########################################################################
#
# Process the accumulated precipitation pcp.
#
########################################################################

# Read the TRMM file
cat(paste("Reading:", trmm_file, "\n"))
trmm = read.table(trmm_file, header=TRUE, skip=hdr_len)

# Determine lat/lon dimensions
lat  = unique(trmm$Latitude)
lon  = unique(trmm$Longitude)
dlat = lat[2]-lat[1]
dlon = lon[2]-lon[1]
nlat = length(lat)
nlon = length(lon)

# Slice the 1D array into a 2D array
pcp = array(trmm$AccRain, dim=c(nlon, nlat), dimnames=c("lon", "lat"))
pcp[pcp < missing] = missing

########################################################################
#
# Create the NetCDF output file.
#
########################################################################

# Define dimensions
dim_lat = dim.def.ncdf("lat", "degrees_north", lat,
                       create_dimvar=TRUE)
dim_lon = dim.def.ncdf("lon", "degrees_east",  lon,
                       create_dimvar=TRUE)

# Define variables
var_pcp = var.def.ncdf(paste("APCP_", acc_str, sep=''), "kg/m^2",
                       list(dim_lon, dim_lat), missing,
                       longname="Total precipitation", prec="single")

# Define file
nc = create.ncdf(nc_file, var_pcp)

# Add variable attributes
att.put.ncdf(nc, var_pcp, "name", "APCP")
att.put.ncdf(nc, var_pcp, "level", paste("A", acc_hr, sep=''))
att.put.ncdf(nc, var_pcp, "grib_code", 61)
att.put.ncdf(nc, var_pcp, "_FillValue", missing, prec="single")
att.put.ncdf(nc, var_pcp, "init_time", format(init, "%Y%m%d_%H%M%S", tz="GMT"), prec="text")
att.put.ncdf(nc, var_pcp, "init_time_ut", as.numeric(init), prec="double")
att.put.ncdf(nc, var_pcp, "valid_time", format(valid, "%Y%m%d_%H%M%S", tz="GMT"), prec="text")
att.put.ncdf(nc, var_pcp, "valid_time_ut", as.numeric(valid), prec="double")
att.put.ncdf(nc, var_pcp, "accum_time", paste(acc_str, "0000", sep=''))
att.put.ncdf(nc, var_pcp, "accum_time_sec", acc_sec)

# Add global attributes
cur_time = Sys.time()
att.put.ncdf(nc, 0, "FileOrigins", paste("File", nc_file, "generated", format(Sys.time(), "%Y%m%d_%H%M%S"),
                                         "on host", Sys.info()[4], "by the Rscript trmm2nc.R"))
att.put.ncdf(nc, 0, "MET_version", "V3.0.1")
att.put.ncdf(nc, 0, "Projection", "LatLon", prec="text")
att.put.ncdf(nc, 0, "lat_ll", paste(min(lat), "degrees_north"), prec="text")
att.put.ncdf(nc, 0, "lon_ll", paste(min(lon), "degrees_east"), prec="text")
att.put.ncdf(nc, 0, "delta_lat", paste(dlat, "degrees"), prec="text")
att.put.ncdf(nc, 0, "delta_lon", paste(dlon, "degrees"), prec="text")
att.put.ncdf(nc, 0, "Nlat", paste(nlat, "grid_points"), prec="text")
att.put.ncdf(nc, 0, "Nlon", paste(nlon, "grid_points"), prec="text")

# Add pcp to the file
put.var.ncdf(nc, var_pcp, pcp)

# Close the file
close.ncdf(nc)

cat(paste("Writing:", nc_file, "\n"))

########################################################################
#
# Finished.
#
########################################################################

# Optionally, save all of the pcp to an .RData file
if(save == TRUE) save.image()
