########################################################################
##
##   Name: mode_summary.R
##
##   Description:
##      This R script summarizes the contents of one or more MODE
##      object statistics file.
##
##   Usage:
##      Rscript mode_summary.R mode_obj_file_list
##
##   Arguments:
##      "mode_obj_file_list" is a space-seperated list of the MODE
##      object statistics file names to be used.
##
##   Details:
##      This R script summarizes the contents of one or more MODE
##      object statistics files.  It reads in the MODE file(s) passed
##      on the command line, counts the numbers/areas of the
##      matched/unmatched forecast/observation objects and prints them
##      to the screen.  It also computes the maximum interest value for
##      each object and prints out the median of them for the forecast
##      objects, observation objects, and both.
##
##      This R script is meant as an example.  Users are welcome and
##      encouraged to adapt it to perform the type of analysis they
##      need.
##
##   Examples:
##      Rscript mode_summary.R mode_120000L_20050807_120000V_120000A_obj.txt
##
##   Author:
##      John Halley Gotway, MET Development Team, 10/22/2008
##      johnhg@ucar.edu
##
########################################################################

########################################################################
#
# Handle the arguments.
#
########################################################################

# Retreive the arguments
args <- commandArgs(TRUE)

# Check the number of arguments
if(length(args) < 1) {
   cat("Usage: mode_summary.R mode_obj_file_list\n")
   cat("   where mode_obj_file_list is a space-separated list of MODE object statistic files.\n")
   quit()
}

# Constants
i_start<-1

# Variables to store the object counts
sum_n_fcst<-0
sum_n_fcst_match<-0
sum_n_fcst_unmatch<-0
sum_n_obs<-0
sum_n_obs_match<-0
sum_n_obs_unmatch<-0
sum_n_fcst_clus<-0
sum_n_obs_clus<-0

# Variables to store the object areas
sum_a_fcst<-0
sum_a_fcst_match<-0
sum_a_fcst_unmatch<-0
sum_a_obs<-0
sum_a_obs_match<-0
sum_a_obs_unmatch<-0
sum_a_fcst_clus<-0
sum_a_obs_clus<-0

# List of maximum interest values by forecast and observation objects
list_max_interest_fcst <-c()
list_max_interest_obs  <-c()

# List of cluster pair centroid distances and x,y offsets
list_centroid_distance <-c()
list_centroid_x_offset <-c()
list_centroid_y_offset <-c()

# Handle each of the MODE files one at a time
for(i in i_start:length(args)) {

   # Store the current MODE file
   mode_file<-args[i]

   # Read the MODE file passed as an argument
   mode<-read.table(mode_file, header=TRUE)

   cat(paste("Read", dim(mode)[1], "lines from MODE file:", mode_file, "\n"))

   # Check for an empty file containing only the header row
   if(dim(mode)[1] > 0) {

      # Replace instances of -9999 with NA
      mode[mode==-9999]=NA

      ########################################################################
      #
      # Compute indicators for each of the line types.
      #
      ########################################################################

      ind_simp_fcst <-regexpr("^F[0-9][0-9][0-9]$", mode$OBJECT_ID) == 1
      ind_simp_obs  <-regexpr("^O[0-9][0-9][0-9]$", mode$OBJECT_ID) == 1
      ind_simp_pair <-regexpr("^F[0-9][0-9][0-9]_O[0-9][0-9][0-9]$", mode$OBJECT_ID) == 1

      ind_clus_fcst <-regexpr("^CF[0-9][0-9][0-9]$", mode$OBJECT_ID) == 1
      ind_clus_obs  <-regexpr("^CO[0-9][0-9][0-9]$", mode$OBJECT_ID) == 1
      ind_clus_pair <-regexpr("^CF[0-9][0-9][0-9]_CO[0-9][0-9][0-9]$", mode$OBJECT_ID) == 1

      ind_match     <-(regexpr("^C[F,O]000$", mode$OBJECT_CAT) == -1) *
                      (regexpr("^C[F,O][0-9][0-9][0-9]$", mode$OBJECT_CAT) == 1)
      ind_unmatch   <-regexpr("^C[F,O]000$", mode$OBJECT_CAT) == 1

      ########################################################################
      #
      # Keep a running sum of object counts and areas.
      #
      ########################################################################

      # Keep a running sum of the object counts
      sum_n_fcst         <-sum_n_fcst+sum(ind_simp_fcst)
      sum_n_fcst_match   <-sum_n_fcst_match+sum(ind_simp_fcst*ind_match > 0)
      sum_n_fcst_unmatch <-sum_n_fcst_unmatch+sum(ind_simp_fcst*ind_unmatch > 0)

      sum_n_obs          <-sum_n_obs+sum(ind_simp_obs)
      sum_n_obs_match    <-sum_n_obs_match+sum(ind_simp_obs*ind_match > 0)
      sum_n_obs_unmatch  <-sum_n_obs_unmatch+sum(ind_simp_obs*ind_unmatch > 0)

      sum_n_fcst_clus    <-sum_n_fcst_clus+sum(ind_clus_fcst)
      sum_n_obs_clus     <-sum_n_obs_clus+sum(ind_clus_obs)

      # Keep a running sum of the object areas
      sum_a_fcst         <-sum_a_fcst+sum(mode$AREA[ind_simp_fcst])
      sum_a_fcst_match   <-sum_a_fcst_match+sum(mode$AREA[ind_simp_fcst*ind_match > 0])
      sum_a_fcst_unmatch <-sum_a_fcst_unmatch+sum(mode$AREA[ind_simp_fcst*ind_unmatch > 0])

      sum_a_obs          <-sum_a_obs+sum(mode$AREA[ind_simp_obs])
      sum_a_obs_match    <-sum_a_obs_match+sum(mode$AREA[ind_simp_obs*ind_match > 0])
      sum_a_obs_unmatch  <-sum_a_obs_unmatch+sum(mode$AREA[ind_simp_obs*ind_unmatch > 0])

      sum_a_fcst_clus    <-sum_a_fcst_clus+sum(mode$AREA[ind_clus_fcst])
      sum_a_obs_clus     <-sum_a_obs_clus+sum(mode$AREA[ind_clus_obs])

      # Check for zero forecast or zero observation objects
      if(sum(ind_simp_fcst) == 0 || sum(ind_simp_obs) == 0) next

      ########################################################################
      #
      # Keep track of maximum interest values for each object.
      #
      ########################################################################

      # Use only the simple pair lines.
      ind<-ind_simp_pair

      if(sum(ind) > 0) {

         # Find the maximum interest for each forecast object.
         max_interest_fcst <-aggregate(mode$INTEREST[ind], by=list(fcst=substr(mode$OBJECT_ID[ind], 1, 4)), FUN=max)

         # Find the maximum interest for each observation object.
         max_interest_obs <-aggregate(mode$INTEREST[ind], by=list(obs=substr(mode$OBJECT_ID[ind], 6, 9)), FUN=max)

         # Append maximum interest for each object to the previous list
         for(j in 1:sum(ind_simp_fcst)) {
            list_max_interest_fcst <- c(list_max_interest_fcst,
               max(max_interest_fcst$x[max_interest_fcst$fcst == sprintf("F%.3i", j)], 0.0))
         }

         for(j in 1:sum(ind_simp_obs)) {
            list_max_interest_obs <- c(list_max_interest_obs,
               max(max_interest_obs$x[max_interest_obs$obs == sprintf("O%.3i", j)], 0.0))
         }
      }

      ########################################################################
      #
      # Keep track of centroid X,Y offsets.
      #
      ########################################################################

      # For each cluster pair, compute the centroid X/Y offset.
      for(str in mode$OBJECT_ID[ind_clus_pair]) {

         # Pull out the fcst/obs object id's
         fcst = substr(str, 1, 5)
         obs  = substr(str, 7, 11)

         # Compute the X/Y offsets.
         distance = mode$CENTROID_DIST[mode$OBJECT_ID==str]
         x_offset = mode$CENTROID_X[mode$OBJECT_ID==fcst] - mode$CENTROID_X[mode$OBJECT_ID==obs]
         y_offset = mode$CENTROID_Y[mode$OBJECT_ID==fcst] - mode$CENTROID_Y[mode$OBJECT_ID==obs]

         # Append to the list of distances and X/Y offsets.
         list_centroid_distance <-c(list_centroid_distance, distance)
         list_centroid_x_offset <-c(list_centroid_x_offset, x_offset)
         list_centroid_y_offset <-c(list_centroid_y_offset, y_offset)

      } # end for loop

   } # end if statement

} # end for loop

########################################################################
#
# Print the results to the screen.
#
########################################################################

# Print out the counts and areas of the single objects
cat(paste("Total Number of Files Processed =", length(args)), "\n")
cat(paste("Total Number of Single Objects =", sum_n_fcst+sum_n_obs), "\n")

cat(paste("Number of Single Fcst Objects =", sum_n_fcst), "\n")
cat(paste("Number of Matched Single Fcst Objects =", sum_n_fcst_match), "\n")
cat(paste("Number of Unmatched Single Fcst Objects =", sum_n_fcst_unmatch), "\n")

cat(paste("Number of Single Obs Objects =", sum_n_obs), "\n")
cat(paste("Number of Matched Single Obs Objects =", sum_n_obs_match), "\n")
cat(paste("Number of Unmatched Single Obs Objects =", sum_n_obs_unmatch), "\n")

cat(paste("Total Number of Cluster Objects =", sum_n_fcst_clus+sum_n_obs_clus), "\n")
cat(paste("Total Number of Fcst Cluster Objects =", sum_n_fcst_clus), "\n")
cat(paste("Total Number of Obs Cluster Objects =", sum_n_obs_clus), "\n")

cat(paste("Total Area of Objects =", sum_a_fcst+sum_a_obs), "\n")

cat(paste("Area of Single Fcst Objects =", sum_a_fcst), "\n")
cat(paste("Area of Matched Single Fcst Objects =", sum_a_fcst_match), "\n")
cat(paste("Area of Unmatched Single Fcst Objects =", sum_a_fcst_unmatch), "\n")

cat(paste("Area of Single Obs Objects =", sum_a_obs), "\n")
cat(paste("Area of Matched Single Obs Objects =", sum_a_obs_match), "\n")
cat(paste("Area of Unmatched Single Obs Objects =", sum_a_obs_unmatch), "\n")

# Print out the median of the maximums
cat(paste("Median of Max Interest by Fcst Object =", median(list_max_interest_fcst)), "\n")
cat(paste("Median of Max Interest by Obs Object =", median(list_max_interest_obs)), "\n")
cat(paste("Median of Max Interest by Fcst+Obs Objects =", median(c(list_max_interest_fcst, list_max_interest_obs))), "\n")

#cat(paste("Dump Max Interest by Fcst Object ="), list_max_interest_fcst, "\n")
#cat(paste("Dump Max Interest by Obs Object ="), list_max_interest_obs, "\n")
#cat(paste("Dump Max Interest by Fcst+Obs Object ="), list_max_interest_fcst, list_max_interest_obs, "\n")

# Print out information about the cluster centroid distances and offsets
cat("Cluster Pair Centroid Distance:\n")
summary(list_centroid_distance)
cat("Cluster Pair Centroid X-Offsets:\n")
summary(list_centroid_x_offset)
cat("Cluster Pair Centroid Y-Offsets:\n")
summary(list_centroid_y_offset)

cat(paste("Cluster Pair Centroid X,Y Offsets and Distance [", length(list_centroid_x_offset), "]:\n", sep=""))
cat(sprintf("(%.4f, %.4f) %.4f\n", list_centroid_x_offset, list_centroid_y_offset, list_centroid_distance), sep="")

