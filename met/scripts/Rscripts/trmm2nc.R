########################################################################
##
##   Name: trmm2nc.R
##
##   Description:
##      This script should be called via an Rscript command to reformat
##      3B42 V7 ASCII TRMM precipitation pcp into a NetCDF format that MET
##      can read.  The input pcp for this script can be attained via:
##      24-hour accumulations:
##        http://gdata1.sci.gsfc.nasa.gov/daac-bin/G3/gui.cgi?instance_id=TRMM_3B42_Daily
##      03-hour accumulations:
##        http://gdata1.sci.gsfc.nasa.gov/daac-bin/G3/gui.cgi?instance_id=TRMM_3-Hourly
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
##      002    2014-05-15  Halley Gotway  Update parsing of header for
##                         3B42 version 7.
##
########################################################################

########################################################################
#
# Required libraries.
#
########################################################################

library(ncdf4)

########################################################################
#
# Constants and command line arguments
#
########################################################################

hdr_len    = 9           # Each pcp file begins with 9 header lines.
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
hdr = readLines(hdr_file)
sys_cmd = paste("rm -f", hdr_file)
system(sys_cmd)

# Determine if it's a daily or 3-hourly file
if(grepl("daily", hdr[1], ignore.case=TRUE)) {
  acc_type  = 24
  init_fmt  = "Selected time period: %d%b%Y"
  valid_fmt = "%d%b%Y"
} else if(grepl("3-hour", hdr[1], ignore.case=TRUE)) {
  acc_type  = 3
  init_fmt  = "Selected time period: %H:%MZ%d%b%Y"
  valid_fmt = "%H:%MZ%d%b%Y"
} else {
  cat("\n\nERROR: Can\'t figure out the accumulation interval!\n\n")
  quit()
}

# Parse the init and valid times
#   http://disc.sci.gsfc.nasa.gov/additional/faq/precipitation_faq.shtml#convert
toks = unlist(strsplit(hdr[4], "-"))
init = as.POSIXct(strptime(toks[1], format=init_fmt, tz="GMT"))
if(length(toks) == 2) {
   valid = as.POSIXct(strptime(toks[2], format=valid_fmt, tz="GMT"))
} else {
   valid = as.POSIXct(init + acc_type * sec_per_hr)
}

# Shift the valid times by 1.5 hours
if(acc_type == 24) {
   init  = as.POSIXct(init - 1.5 * sec_per_hr)
   valid = as.POSIXct(valid - 1.5 * sec_per_hr)
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

names(trmm) = c("latitude", "longitude", "precipitation")

# Determine lat/lon dimensions
lat  = unique(trmm$latitude)
lon  = unique(trmm$longitude)
dlat = lat[2]-lat[1]
dlon = lon[2]-lon[1]
nlat = length(lat)
nlon = length(lon)

# Slice the 1D array into a 2D array
pcp = array(trmm$precipitation, dim=c(nlon, nlat))
pcp[pcp < missing] = missing

########################################################################
#
# Create the NetCDF output file.
#
########################################################################

# Define dimensions
dim_lat = ncdim_def("lat", "degrees_north", lat, create_dimvar=TRUE)
dim_lon = ncdim_def("lon", "degrees_east",  lon, create_dimvar=TRUE)

# Define variables
var_pcp = ncvar_def(paste("APCP_", acc_str, sep=''), "kg/m^2",
                       list(dim_lon, dim_lat), missing,
                       longname="Total precipitation", prec="single")

# Define file
nc = nc_create(nc_file, var_pcp)

# Add variable attributes
ncatt_put(nc, var_pcp, "name", "APCP")
ncatt_put(nc, var_pcp, "level", paste("A", acc_hr, sep=''))
ncatt_put(nc, var_pcp, "grib_code", 61, prec="int")
ncatt_put(nc, var_pcp, "_FillValue", missing, prec="single")
ncatt_put(nc, var_pcp, "init_time", format(init, "%Y%m%d_%H%M%S", tz="GMT"), prec="text")
ncatt_put(nc, var_pcp, "init_time_ut", as.numeric(init), prec="int")
ncatt_put(nc, var_pcp, "valid_time", format(valid, "%Y%m%d_%H%M%S", tz="GMT"), prec="text")
ncatt_put(nc, var_pcp, "valid_time_ut", as.numeric(valid), prec="int")
ncatt_put(nc, var_pcp, "accum_time", paste(acc_str, "0000", sep=''))
ncatt_put(nc, var_pcp, "accum_time_sec", acc_sec, prec="int")

# Add global attributes
cur_time = Sys.time()
ncatt_put(nc, 0, "FileOrigins", paste("File", nc_file, "generated", format(Sys.time(), "%Y%m%d_%H%M%S"),
                                         "on host", Sys.info()[4], "by the Rscript trmm2nc.R"))
ncatt_put(nc, 0, "MET_version", "V4.1")
ncatt_put(nc, 0, "Projection", "LatLon", prec="text")
ncatt_put(nc, 0, "lat_ll", paste(min(lat), "degrees_north"), prec="text")
ncatt_put(nc, 0, "lon_ll", paste(min(lon), "degrees_east"), prec="text")
ncatt_put(nc, 0, "delta_lat", paste(dlat, "degrees"), prec="text")
ncatt_put(nc, 0, "delta_lon", paste(dlon, "degrees"), prec="text")
ncatt_put(nc, 0, "Nlat", paste(nlat, "grid_points"), prec="text")
ncatt_put(nc, 0, "Nlon", paste(nlon, "grid_points"), prec="text")

# Add pcp to the file
ncvar_put(nc, var_pcp, pcp)

# Close the file
nc_close(nc)

cat(paste("Writing:", nc_file, "\n"))

########################################################################
#
# Finished.
#
########################################################################

# Optionally, save all of the pcp to an .RData file
if(save == TRUE) save.image()
