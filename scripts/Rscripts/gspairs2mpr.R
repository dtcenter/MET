########################################################################
##
##  Name: gspairs2mpr.R
##
##  Description:
##    This script may be run to convert a NetCDF matched pairs file
##    produced by the MET Grid-Stat tool into a ASCII file containing
##    a matched pair (MPR) line for each valid forecast and observation
##    pair.
##
##  Usage:
##    Rscript gspairs2mpr.R
##      nc_file
##      [-var name]
##      [-out path]
##      [-save]
##
##  Arguments:
##    "nc_file"   is the Grid-Stat NetCDF pairs file.
##    "-var name" specifies the variable to process (multiple).
##    "-out path" specifies the output file name.
##    "-save"     calls save.image() before exiting R.
##
##  Details:
##    Updated for MET version 6.0.
##
##  Examples:
##    Rscript gspairs2mpr.R \
##      grid_stat_APCP_24_240000L_20050808_000000V_pairs.nc
##
##   Author:
##      John Halley Gotway (johnhg@ucar.edu), NCAR-RAL/DTC
##      08/18/2015
##
########################################################################

library(ncdf4)

########################################################################
#
# Constants.
#
########################################################################

# Precision
prec = 4;

########################################################################
#
# Handle the arguments.
#
########################################################################

# Retreive the arguments
args = commandArgs(TRUE);

# Check the number of arguments
if(length(args) < 1) {
   cat("Usage: gspairs2mpr.R\n")
   cat("         nc_file\n")
   cat("         [-var name]\n")
   cat("         [-out path]\n")
   cat("         [-save]\n")
   cat("         where \"nc_file\"   is the Grid-Stat NetCDF pairs file.\n")
   cat("               \"-var name\" specifies the variable to process (multiple).\n")
   cat("               \"-out path\" specifies the output file name.\n")
   cat("               \"-save\"     calls save.image() before exiting R.\n\n")
   quit()
}

# Initialize
nc_file  = c();
var_list = c();
out_file = "mpr.txt"
save     = FALSE;

# Parse the arguments
i=1
while(i <= length(args)) {
  if(args[i] == "-save") {
    save = TRUE
  } else if(args[i] == "-var") {
    var_list = c(var_list, args[i+1])
    i = i+1
  } else if(args[i] == "-out") {
    out_file = args[i+1]
    i = i+1
  } else {
    nc_file = args[i]
  }
  i = i+1
}

########################################################################
#
# Read the input NetCDF file.
#
########################################################################

print(paste("Reading:", nc_file));
nc = nc_open(nc_file, write=F);

# Get lat/lon values
lat = round(nc$dim[["lat"]]$vals, prec);
lon = round(nc$dim[["lon"]]$vals, prec);

# Get the list of cases to process, if not specified.
if(length(var_list) == 0) {
  ind = grepl("FCST_|OBS", names(nc$var));
  var_list = unique(gsub("FCST_|OBS_", "", names(nc$var)[ind]));
}

# Loop through the variables to process
for(i in 1:length(var_list)) {
  print(paste("Processing:", var_list[i]));
  fstr  = paste("FCST_", var_list[i], sep="");
  ostr  = paste("OBS_",  var_list[i], sep="");
  fvar  = round(ncvar_get(nc, fstr), prec);
  ovar  = round(ncvar_get(nc, ostr), prec);
  ind   = !is.na(fvar) & !is.na(ovar);
  flead = as.numeric(ncatt_get(nc, fstr, "valid_time_ut")$value) -
          as.numeric(ncatt_get(nc, fstr, "init_time_ut")$value);
  olead = as.numeric(ncatt_get(nc, ostr, "valid_time_ut")$value) -
          as.numeric(ncatt_get(nc, ostr, "init_time_ut")$value);
  mpr   = data.frame(
            VERSION=ncatt_get(nc, 0, "MET_version")$value,
            MODEL=ncatt_get(nc, 0, "model")$value,
            DESC=ncatt_get(nc, fstr, "desc")$value,
            FCST_LEAD=sprintf("%02d%02d%02d", round(flead/3600, 0), round(flead%%3600/60, 0), flead%%60),
            FCST_VALID_BEG=ncatt_get(nc, fstr, "valid_time")$value,
            FCST_VALID_END=ncatt_get(nc, fstr, "valid_time")$value,
            OBS_LEAD=sprintf("%02d%02d%02d", round(olead/3600, 0), round(olead%%3600/60, 0), olead%%60),
            OBS_VALID_BEG=ncatt_get(nc, ostr, "valid_time")$value,
            OBS_VALID_END=ncatt_get(nc, ostr, "valid_time")$value,
            FCST_VAR=ncatt_get(nc, fstr, "name")$value,
            FCST_LEV=ncatt_get(nc, fstr, "level")$value,
            OBS_VAR=ncatt_get(nc, ostr, "name")$value,
            OBS_LEV=ncatt_get(nc, ostr, "level")$value,
            OBTYPE=ncatt_get(nc, 0, "obtype")$value,
            VX_MASK=ncatt_get(nc, fstr, "masking_region")$value,
            INTERP_MTHD=ncatt_get(nc, fstr, "smoothing_method")$value,
            INTERP_PNTS=ncatt_get(nc, fstr, "smoothing_neighborhood")$value,
            FCST_THRESH=NA, OBS_THRESH=NA, COV_THRESH=NA, ALPHA=NA,
            LINE_TYPE="MPR",
            TOTAL=sum(ind), INDEX=seq(1, sum(ind)), OBS_SID=NA,
            OBS_LAT=lat[ind], OBS_LON=lon[ind],
            OBS_LVL=NA, OBS_ELV=NA,
            FCST=fvar[ind], OBS=ovar[ind],
            CLIMO=NA, OBS_QC=NA);
  write.table(mpr, file=out_file, append=(i>1), col.names=(i==1),
              quote=FALSE, row.names=FALSE);
}

# List the completed output file
print(paste("Writing:", out_file))

# Save the R working environment
if(save == TRUE) {
  print("Saving image...")
  save.image()
}
