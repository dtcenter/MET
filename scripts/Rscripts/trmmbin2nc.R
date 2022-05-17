# Daily TRMM binary files:
#   ftp://disc2.nascom.nasa.gov/data/TRMM/Gridded/Derived_Products/3B42_V7/Daily/YYYY/3B42_daily.YYYY.MM.DD.7.bin
# 3-Hourly TRMM binary files:
#   ftp://disc2.nascom.nasa.gov/data/TRMM/Gridded/3B42_V7/YYYYMM/3B42.YYMMDD.HHz.7.precipitation.bin

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

hdr_len    = 5           # Each pcp file begins with 5 header lines.
hdr_file   = "trmm.hdr"  # Temporary header file.
missing    = -9999       # Missing pcp value to be used in MET.
save       = FALSE       # If set to TRUE, call save.image()
sec_per_hr = 60*60

# Native domain specification
in_res     = 0.25        # Resolution in degrees
in_lat_ll  = -50.00      # Latitude of lower-left corner
in_lon_ll  =   0.00      # Longitude of lower-left corner
in_lat_ur  =  50.00      # Latitude of upper-right corner
in_lon_ur  = 359.75      # Longitude of upper-right corner

# Output domain specification
out_res    = 0.25        # Resolution in degrees
out_lat_ll =  -25.00     # Latitude of lower-left corner
out_lon_ll = -150.00     # Longitude of lower-left corner
out_lat_ur =   60.00     # Latitude of upper-right corner
out_lon_ur =   10.00     # Longitude of upper-right corner

rescale_lon <- function (x) {
  while(x < -180) x = x + 360;
  while(x >  180) x = x - 360;
  return(x)
}

########################################################################
#
# Handle the arguments.
#
########################################################################

# Retreive the arguments
args = commandArgs(TRUE)

# Check the number of arguments
if(length(args) < 2) {
   cat("Usage: trmmbin2nc.R\n")
   cat("        trmm_file\n")
   cat("        nc_file\n")
   cat("        [-save]\n")
   cat("        where \"trmm_file\" is a binary TRMM files.\n")
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
# Parse accumulation interval and time from file name.
#
########################################################################

# Daily files contain the string "daily"
if(grepl("daily", basename(trmm_file), ignore.case=TRUE)) {
  tok   = unlist(strsplit(basename(trmm_file), '.', fixed=TRUE))
  ftime = strptime(paste(tok[2], tok[3], tok[4]), format="%Y %m %d", tz="GMT")
  init  = as.POSIXct(ftime -  0*sec_per_hr)
  valid = as.POSIXct(ftime + 24*sec_per_hr)

# 3-hourly files contain the string "[0-9][0-9]z"
} else if(grepl("[0-9][0-9]z", basename(trmm_file), ignore.case=TRUE)) {
  tok   = unlist(strsplit(basename(trmm_file), '.', fixed=TRUE))
  ftime = strptime(paste(tok[2], tok[3]), format="%y%m%d %H", tz="GMT")
  init  = as.POSIXct(ftime - 0*sec_per_hr)
  valid = as.POSIXct(ftime + 3*sec_per_hr)

# Fail otherwise
} else {
  cat("\n\nERROR: Can\'t figure out the accumulation interval!\n\n")
  quit()
}

# Compute the accumulation interval
acc_sec = as.double(valid - init, units="secs")
acc_hr  = floor(acc_sec / 3600)
acc_str = sprintf("%.2i", acc_hr)

########################################################################
#
# Read the 1/4 degree binary TRMM data.
#
########################################################################

in_lat  = seq(in_lat_ll, in_lat_ur, in_res)
in_lon  = seq(in_lon_ll, in_lon_ur, in_res)
in_nlat = length(in_lat)
in_nlon = length(in_lon)
data    = readBin(trmm_file, "numeric", n=in_nlon*in_nlat, size = 4, endian="big")
in_pcp  = array(data, dim=c(in_nlon, in_nlat), dimnames=c("lon", "lat"))

# Rescale the input longitudes from -180.0 to 180
in_lon = sapply(in_lon, rescale_lon)

########################################################################
#
# Select subset of data to be written.
#
########################################################################

out_lat  = seq(out_lat_ll, out_lat_ur, out_res)
out_lon  = seq(out_lon_ll, out_lon_ur, out_res)
out_nlat = length(out_lat)
out_nlon = length(out_lon)

# Rescale the output longitudes from -180.0 to 180
out_lon = sort(sapply(out_lon, rescale_lon))

# Extract the output data
out_pcp = matrix(nrow=out_nlon, ncol=out_nlat)
out_cnt = 0
out_vld = 0
out_sum = 0

for(cur_out_lon in 1:out_nlon) {
  for(cur_out_lat in 1:out_nlat) {
  
    cur_in_lon = which(out_lon[cur_out_lon] == in_lon)
    cur_in_lat = which(out_lat[cur_out_lat] == in_lat)

    if(length(cur_in_lon) == 1 &&
       length(cur_in_lat) == 1) {
      out_pcp[cur_out_lon, cur_out_lat] = in_pcp[cur_in_lon, cur_in_lat]
      out_vld = out_vld + 1
      out_sum = out_sum + out_pcp[cur_out_lon, cur_out_lat];
    }

    out_cnt = out_cnt + 1
  }
}

########################################################################
#
# Create the NetCDF output file.
#
########################################################################

# Define dimensions
dim_lat = ncdim_def("lat", "degrees_north", out_lat,
                    create_dimvar=TRUE)
dim_lon = ncdim_def("lon", "degrees_east",  out_lon,
                    create_dimvar=TRUE)

# Define variables
var_pcp = ncvar_def(paste("APCP_", acc_str, sep=''), "kg/m^2",
                    list(dim_lon, dim_lat), missing,
                    longname="Total precipitation", prec="single")

# Define file
nc = nc_create(nc_file, var_pcp)

# Add variable attributes
ncatt_put(nc, var_pcp, "name", "APCP")
ncatt_put(nc, var_pcp, "level", paste("A", acc_hr, sep=''))
ncatt_put(nc, var_pcp, "grib_code", 61)
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
                                         "on host", Sys.info()[4], "by the Rscript trmmbin2nc.R"))
ncatt_put(nc, 0, "MET_version", "V4.1")
ncatt_put(nc, 0, "Projection", "LatLon", prec="text")
ncatt_put(nc, 0, "lat_ll", paste(min(out_lat), "degrees_north"), prec="text")
ncatt_put(nc, 0, "lon_ll", paste(min(out_lon), "degrees_east"), prec="text")
ncatt_put(nc, 0, "delta_lat", paste(out_res, "degrees"), prec="text")
ncatt_put(nc, 0, "delta_lon", paste(out_res, "degrees"), prec="text")
ncatt_put(nc, 0, "Nlat", paste(out_nlat, "grid_points"), prec="text")
ncatt_put(nc, 0, "Nlon", paste(out_nlon, "grid_points"), prec="text")

# Add pcp to the file
ncvar_put(nc, var_pcp, out_pcp)

# Close the file
nc_close(nc)

# Print summary info

cat(paste("Output File:\t", nc_file, "\n", sep=""))
cat(paste("Output Domain:\t", out_nlat, " X ", out_nlon, " from (",
    out_lat_ll, ", ", out_lon_ll, ") to (",
    out_lat_ur, ", ", out_lon_ur, ") by ", out_res, " deg\n", sep=""))
cat(paste("Output Precip:\t", out_vld, " of ", out_cnt, " points valid, ",
    round(out_sum, 2), " mm total, ",
    round(out_sum/out_vld, 2), " mm avg\n", sep=""))

########################################################################
#
# Finished.
#
########################################################################

# Optionally, save all of the pcp to an .RData file
if(save == TRUE) save.image()
