########################################################################
##
##   Name: mode_nc_inventory.R
##
##   Description:
##      This R script inventories objects contained in an output MODE
##      NetCDF file.  It writes out a handful of object attributes.
##      This script is meant for testing purposes to compare the data in
##      the NetCDF file to the object attributes written directly from
##      MODE.
##
##   Usage:
##      Rscript mode_nc_inventory.R mode_nc_file
##
##   Arguments:
##      A single output MODE NetCDF file
##
##   Details:
##
##   Examples:
##      Rscript mode_nc_inventory.R mode_120000L_20050807_120000V_120000A_obj.nc
##
##   Author:
##      John Halley Gotway, MET Development Team, 3/27/2013
##      johnhg@ucar.edu
##
########################################################################

library(ncdf4)

########################################################################
#
# Handle the arguments.
#
########################################################################

# Retreive the arguments
args <- commandArgs(TRUE)

# Check the number of arguments
if(length(args) < 1) {
   cat("Usage: mode_nc_inventory.R mode_nc_file\n")
   cat("   where mode_nc_file is the NetCDF output from MODE.\n")
   quit()
}

########################################################################

# Open the input NetCDF file
nc = nc_open(args[1])

# Retrieve the data
fcst_raw     = ncvar_get(nc, "fcst_raw")
obs_raw      = ncvar_get(nc, "obs_raw")
fcst_obj_id  = ncvar_get(nc, "fcst_obj_id")
obs_obj_id   = ncvar_get(nc, "obs_obj_id")
fcst_clus_id = ncvar_get(nc, "fcst_clus_id")
obs_clus_id  = ncvar_get(nc, "obs_clus_id")

# Forecast simple object areas
for(i in 1:max(fcst_obj_id, na.rm=T)) {
   cat(sprintf("Single Simple  Fcst %2i: Area = %7i, Intensity50 = %11.5f, IntensitySum = %11.5f\n",
               i,
               sum(fcst_obj_id==i, na.rm=T),
               median(fcst_raw[fcst_obj_id==i], na.rm=T),
               sum(fcst_raw[fcst_obj_id==i], na.rm=T)))
}

# Observation simple object areas
for(i in 1:max(obs_obj_id, na.rm=T)) {
   cat(sprintf("Single Simple  Obs  %2i: Area = %7i, Intensity50 = %11.5f, IntensitySum = %11.5f\n",
               i,
               sum(obs_obj_id==i, na.rm=T),
               median(obs_raw[obs_obj_id==i], na.rm=T),
               sum(obs_raw[obs_obj_id==i], na.rm=T)))
}

# Forecast cluster object areas
for(i in 1:max(fcst_clus_id, na.rm=T)) {
   cat(sprintf("Single Cluster Fcst %2i: Area = %7i, Intensity50 = %11.5f, IntensitySum = %11.5f\n",
               i,
               sum(fcst_clus_id==i, na.rm=T),
               median(fcst_raw[fcst_clus_id==i], na.rm=T),
               sum(fcst_raw[fcst_clus_id==i], na.rm=T)))
}

# Observation cluster object areas
for(i in 1:max(obs_clus_id, na.rm=T)) {
   cat(sprintf("Single Cluster Obs  %2i: Area = %7i, Intensity50 = %11.5f, IntensitySum = %11.5f\n",
               i,
               sum(obs_clus_id==i, na.rm=T),
               median(obs_raw[obs_clus_id==i], na.rm=T),
               sum(obs_raw[obs_clus_id==i], na.rm=T)))
}

# Intersection areas
for(i in 1:max(fcst_obj_id, na.rm=T)) {
   for(j in 1:max(obs_obj_id, na.rm=T)) {
      inter   = sum(fcst_obj_id==i & obs_obj_id==j, na.rm=T)
      union   = sum(fcst_obj_id==i | obs_obj_id==j, na.rm=T)
      symdiff = union - inter
      if(inter > 0) {
         fsa = sum(fcst_obj_id==i, na.rm=T)
         osa = sum(obs_obj_id==j, na.rm=T)
         cat(sprintf("Pair Simple  (Fcst %2i, Obs %2i): AreaRatio = %7.5f, Intersection = %7i, Union = %7i, SymmetricDiff = %7i\n",
                     i, j, min(fsa/osa, osa/fsa),
                     inter, union, symdiff))
      }
   }
}

# Intersection areas
for(i in 1:max(fcst_clus_id, na.rm=T)) {
   inter   = sum(fcst_clus_id==i & obs_clus_id==i, na.rm=T)
   union   = sum(fcst_clus_id==i | obs_clus_id==i, na.rm=T)
   symdiff = union - inter
   fsa = sum(fcst_clus_id==i, na.rm=T)
   osa = sum(obs_clus_id==i, na.rm=T)
   cat(sprintf("Pair Cluster (Fcst %2i, Obs %2i): AreaRatio = %7.5f, Intersection = %7i, Union = %7i, SymmetricDiff = %7i\n",
               i, i, min(fsa/osa, osa/fsa),
               inter, union, symdiff))
}

# Close
nc_close(nc)
