#######################################################################
##
##   Name: mode_quilt_plot.R
##
##   Description:
##      This R script reads several MODE object statistics files which
##      contain data from the same case run using several choices of
##      convolution radius and threshold.  It then creates quilt plots
##      summarizing the MODE behavior as a function of scale.
##
##   Usage:
##      Rscript mode_quilt_plot.R prefix mode_obj_file_list
##
##   Arguments:
##      "prefix" is prepended to the output file names and included
##       on the output plots.
##      "mode_obj_file_list" is a space-seperated list of the MODE
##      object statistics file names to be used.
##
##   Details:
##      Users of MODE may be interested in anlayzing forecast
##      performance as a function of scale.  One way to do so is to
##      run the same case through MODE using a variety of convolution
##      radii and thresholds.  Once that's been done, this R script
##      may be run on the output MODE object files to produce summary
##      plots of performance as a function of scale.
##
##      This R script is meant as an example.  Users are welcome and
##      encouraged to adapt it to perform the type of analysis they
##      need.
##
##   Examples:
##
##   Author:
##      John Halley Gotway, MET Development Team, 10/28/2008
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

if(length(args) < 2) {
   cat("Usage: mode_quilt_plot.R file_prefix mode_obj_file_list\n")
   cat("   where file_prefix is the string to be prepended to the output file names.\n")
   cat("         mode_obj_file_list is a space-separated list of MODE object statistic files.\n")
   quit()
}
# Constants
i_start<-2

# Store the prefix to be used
prefix<-args[1]

# run interactively
prefix<-"BIO"
flist <- as.character(read.table("filelist", header=FALSE, sep=" ")$V1)

# Load libraries.
library(fields)
library(gdata)
library(gtools)
library(RColorBrewer)
cols <- brewer.pal(9, "YlGnBu")
########################################################################
#
# Read the input MODE files.
#
########################################################################

# Read each of the MODE files and combine them into one big data frame
#for(i in i_start:length(args)) {
i_start <- 1

for(i in 1:length(flist)) { # for interactive running

   # Store the current MODE file
   mode_file<-flist[i] # for interactive running
#   mode_file<-args[i]

   # Read the MODE file passed as an argument
    mode_tmp<-read.table(mode_file, header=TRUE)
   
   cat(paste("Read", dim(mode_tmp)[1], "lines from MODE file:", mode_file, "\n"))

   # Check for an empty file containing only the header row
   if(dim(mode_tmp)[1] > 0) {

      # Replace instances of -9999 with NA
      mode_tmp[mode_tmp==-9999]=NA

      # Add the contents to the end of the big data frame
      if(!exists("mode_data")) mode_data<-mode_tmp
      else                     mode_data<-rbind(mode_data, mode_tmp)
   } # end if statment
} # end for loop

# Dump out the total number of MODE lines read
cat(paste("Read", dim(mode_data)[1] , "lines from", length(args)-i_start+1, "MODE files.\n"))

# replace the strings with numerics
mode_data[,13] <- as.numeric(as.vector(gsub("=|<|>", "", mode_data[,13])))
mode_data[,15] <- as.numeric(as.vector(gsub("=|<|>", "", mode_data[,15])))

#test <- mode_data[order(as.numeric(gsub("=|<|>", "", mode_data[,13])), mode_data[,12], decreasing=FALSE),]
test <- mode_data[order(mode_data[,13], mode_data[,12], decreasing=FALSE),]

mode_data <- test

########################################################################
#
# Check that all of the data read has the same valid time, accumulation
# time, and lead time.
#
########################################################################

n_valid<-length(unique(mode_data$FCST_VALID))
if(n_valid > 1) {
   cat("*** WARNING *** Expected 1 valid time in data, but found ", n_valid, ":\n", sep="")
   cat(unique(mode_data$FCST_VALID), "\n")
}

n_accum<-length(unique(mode_data$FCST_ACCUM))
if(length(unique(mode_data$FCST_ACCUM)) > 1) {
   cat("*** WARNING *** Expected 1 accumulation time in data, but found ", n_accum, ":\n", sep="")
   cat(unique(mode_data$FCST_ACCUM), "\n")
}

n_lead<-length(unique(mode_data$FCST_LEAD))
if(length(unique(mode_data$FCST_LEAD)) > 1) {
   cat("*** WARNING *** Expected 1 lead time in data, but found ", n_lead, ":\n", sep="")
   cat(unique(mode_data$FCST_LEAD), "\n")
}

########################################################################
#
# Extract the forecast radii and thresholds used.
#
########################################################################

#radius<-sort(unique(c(mode_data$FCST_RAD, mode_data$OBS_RAD)))
radius<-unique(c(mode_data$FCST_RAD, mode_data$OBS_RAD))

thresh<-unique(c(as.character(mode_data$FCST_THR), as.character(mode_data$OBS_THR)))
#thresh<-mixedsort(unique(c(as.character(mode_data$FCST_THR), as.character(mode_data$OBS_THR))))
print(thresh)
thresh0 <- as.numeric(as.vector(gsub("=|<|>", "", thresh)))
print(thresh0)
len <- length(thresh0)


cat(paste(length(radius), "Convolution Radii Found:\n"), radius, "\n")
cat(paste(length(thresh), "Convolution Thresholds Found:\n"), thresh, "\n")

########################################################################
#
# Compute indicators for each of the line types.
#
########################################################################

ind_simp_fcst <-regexpr("^F[0-9][0-9][0-9]$", mode_data$OBJECT_ID) == 1
ind_simp_obs  <-regexpr("^O[0-9][0-9][0-9]$", mode_data$OBJECT_ID) == 1
ind_simp_pair <-regexpr("^F[0-9][0-9][0-9]_O[0-9][0-9][0-9]$", mode_data$OBJECT_ID) == 1

ind_comp_fcst <-regexpr("^CF[0-9][0-9][0-9]$", mode_data$OBJECT_ID) == 1
ind_comp_obs  <-regexpr("^CO[0-9][0-9][0-9]$", mode_data$OBJECT_ID) == 1
ind_comp_pair <-regexpr("^CF[0-9][0-9][0-9]_CO[0-9][0-9][0-9]$", mode_data$OBJECT_ID) == 1

ind_match     <-(regexpr("^C[F,O]000$", mode_data$OBJECT_CAT) == -1) *
                (regexpr("^C[F,O][0-9][0-9][0-9]$", mode_data$OBJECT_CAT) == 1)
ind_unmatch   <-regexpr("^C[F,O]000$", mode_data$OBJECT_CAT) == 1

########################################################################
#
# Aggregate counts of objects by combination of radius and threshold.
#
########################################################################

# All simple objects
ind <- c()
ind<-ind_simp_fcst+ind_simp_obs > 0
n_simp  <-aggregate(mode_data$OBJECT_ID[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=length)

med_area_simp <-aggregate(mode_data$AREA[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=median)
tot_area_simp <-aggregate(mode_data$AREA[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=sum)

# All matched simple objects
ind <- c()
ind<-(ind_simp_fcst+ind_simp_obs)*ind_match > 0
n_simp_match        <-aggregate(mode_data$OBJECT_ID[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=length)
med_area_simp_match <-aggregate(mode_data$AREA[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=median)
tot_area_simp_match <-aggregate(mode_data$AREA[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=sum)

# Simple forecast objects
ind <- c()
ind<-ind_simp_fcst
n_simp_fcst        <-aggregate(mode_data$OBJECT_ID[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=length)
med_area_simp_fcst <-aggregate(mode_data$AREA[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=median)
tot_area_simp_fcst <-aggregate(mode_data$AREA[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=sum)

# Matched simple forecast objects
ind <- c()
ind<-ind_simp_fcst*ind_match > 0
n_simp_fcst_match        <-aggregate(mode_data$OBJECT_ID[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=length)
med_area_simp_fcst_match <-aggregate(mode_data$AREA[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=median)
tot_area_simp_fcst_match <-aggregate(mode_data$AREA[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=sum)

# Unmatched simple forecast objects
ind <- c()
ind<-ind_simp_fcst*ind_unmatch > 0
n_simp_fcst_unmatch        <-aggregate(mode_data$OBJECT_ID[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=length)
med_area_simp_fcst_unmatch <-aggregate(mode_data$AREA[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=median)
tot_area_simp_fcst_unmatch <-aggregate(mode_data$AREA[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=sum)

# Simple observation objects
ind <- c()
ind<-ind_simp_obs
n_simp_obs        <-aggregate(mode_data$OBJECT_ID[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=length)
med_area_simp_obs <-aggregate(mode_data$AREA[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=median)
tot_area_simp_obs <-aggregate(mode_data$AREA[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=sum)

# Matched simple observation objects
ind <- c()
ind<-ind_simp_obs*ind_match > 0
n_simp_obs_match        <-aggregate(mode_data$OBJECT_ID[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=length)
med_area_simp_obs_match <-aggregate(mode_data$AREA[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=median)
tot_area_simp_obs_match <-aggregate(mode_data$AREA[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=sum)

# Unmatched simple observation objects
ind <- c()
ind<-ind_simp_obs*ind_unmatch > 0
n_simp_obs_unmatch        <-aggregate(mode_data$OBJECT_ID[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=length)
med_area_simp_obs_unmatch <-aggregate(mode_data$AREA[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=median)
tot_area_simp_obs_unmatch <-aggregate(mode_data$AREA[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=sum)

########################################################################
#
# Compute information about the interest values in the simple pair
# lines.
#
########################################################################
ind <- c()
# Use only the simple pair lines.
ind<-ind_simp_pair

# Aggregate maximum interest for each radius/threshold combination.
max_interest <-aggregate(mode_data$INTEREST[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=max)

# Aggregate median interest for each radius/threshold combination.
med_interest <-aggregate(mode_data$INTEREST[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind]), FUN=median)

# Aggregate maximum interest for each radius/threshold/fcst object combination.
max_interest_fcst <-aggregate(mode_data$INTEREST[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind], fcst=substr(mode_data$OBJECT_ID[ind], 1, 4)), FUN=max)

# Aggregate the maximum interests just computed and take the median for each radius/threshold combination.
med_of_max_interest_fcst <-aggregate(max_interest_fcst$x, by=list(rad=max_interest_fcst$rad, thresh=max_interest_fcst$thresh), FUN=median)

# Aggregate maximum interest for each radius/threshold/obs object combination.
max_interest_obs  <-aggregate(mode_data$INTEREST[ind], by=list(rad=mode_data$FCST_RAD[ind], thresh=mode_data$FCST_THR[ind], fcst=substr(mode_data$OBJECT_ID[ind], 6, 9)), FUN=max)

# Aggregate the maximum interests just computed and take the median for each radius/threshold combination.
med_of_max_interest_obs <-aggregate(max_interest_obs$x, by=list(rad=max_interest_obs$rad, thresh=max_interest_obs$thresh), FUN=median)

# Aggregate the maximum fcst and obs interests just compute and take the median for each radius/threshold combination.
tmp<-rbind(max_interest_fcst, max_interest_obs)
med_of_max_interest_fcst_obs <-aggregate(tmp$x, by=list(rad=tmp$rad, thresh=tmp$thresh), FUN=median)

########################################################################
#
# Combine results from aggregations above into one big data frame.
#
########################################################################

# Build the radius column
R<-rep(radius, length(thresh))

# Build the threshold column
T<-c()
for(i in 1:length(thresh)) {
   if(length(T) > 1) T<-c(T, rep(thresh[i], length(radius)))
   else              T<-rep(thresh[i], length(radius))
}
#T <- rep(thresh, length(radius))

# Construct a data frame with 3 columns
mode_summary<-data.frame(cbind(rad=R, thresh=T, case=paste(R, T, sep="")))
print(mode_summary)
ind <- c()

# All simple objects
ind<-mode_summary$case%in%paste(n_simp$rad, n_simp$thresh, sep="")
mode_summary$n_simp[ind]<-n_simp$x
print(mode_summary)
ind <- c()

ind<-mode_summary$case%in%paste(med_area_simp$rad, med_area_simp$thresh, sep="")
mode_summary$med_area_simp[ind]<-med_area_simp$x
ind <- c()

ind<-mode_summary$case%in%paste(tot_area_simp$rad, tot_area_simp$thresh, sep="")
mode_summary$tot_area_simp[ind]<-tot_area_simp$x
ind <- c()

# All matched simple objects
ind<-mode_summary$case%in%paste(n_simp_match$rad, n_simp_match$thresh, sep="")
mode_summary$n_simp_match[ind]<-n_simp_match$x
ind <- c()

ind<-mode_summary$case%in%paste(med_area_simp_match$rad, med_area_simp_match$thresh, sep="")
mode_summary$med_area_simp_match[ind]<-med_area_simp_match$x
ind <- c()

ind<-mode_summary$case%in%paste(tot_area_simp_match$rad, tot_area_simp_match$thresh, sep="")
mode_summary$tot_area_simp_match[ind]<-tot_area_simp_match$x
ind <- c()

# Simple forecast objects
ind<-mode_summary$case%in%paste(n_simp_fcst$rad, n_simp_fcst$thresh, sep="")
mode_summary$n_simp_fcst[ind]<-n_simp_fcst$x
ind <- c()

ind<-mode_summary$case%in%paste(med_area_simp_fcst$rad, med_area_simp_fcst$thresh, sep="")
mode_summary$med_area_simp_fcst[ind]<-med_area_simp_fcst$x
ind <- c()

ind<-mode_summary$case%in%paste(tot_area_simp_fcst$rad, tot_area_simp_fcst$thresh, sep="")
mode_summary$tot_area_simp_fcst[ind]<-tot_area_simp_fcst$x
ind <- c()

# Matched simple forecast objects
ind<-mode_summary$case%in%paste(n_simp_fcst_match$rad, n_simp_fcst_match$thresh, sep="")
mode_summary$n_simp_fcst_match[ind]<-n_simp_fcst_match$x
ind <- c()

ind<-mode_summary$case%in%paste(med_area_simp_fcst_match$rad, med_area_simp_fcst_match$thresh, sep="")
mode_summary$med_area_simp_fcst_match[ind]<-med_area_simp_fcst_match$x
ind <- c()

ind<-mode_summary$case%in%paste(tot_area_simp_fcst_match$rad, tot_area_simp_fcst_match$thresh, sep="")
mode_summary$tot_area_simp_fcst_match[ind]<-tot_area_simp_fcst_match$x
ind <- c()

# Unmatched simple forecast objects
ind<-mode_summary$case%in%paste(n_simp_fcst_unmatch$rad, n_simp_fcst_unmatch$thresh, sep="")
mode_summary$n_simp_fcst_unmatch[ind]<-n_simp_fcst_unmatch$x
ind <- c()

ind<-mode_summary$case%in%paste(med_area_simp_fcst_unmatch$rad, med_area_simp_fcst_unmatch$thresh, sep="")
mode_summary$med_area_simp_fcst_unmatch[ind]<-med_area_simp_fcst_unmatch$x
ind <- c()

ind<-mode_summary$case%in%paste(tot_area_simp_fcst_unmatch$rad, tot_area_simp_fcst_unmatch$thresh, sep="")
mode_summary$tot_area_simp_fcst_unmatch[ind]<-tot_area_simp_fcst_unmatch$x
ind <- c()

# Simple observation objects
ind<-mode_summary$case%in%paste(n_simp_obs$rad, n_simp_obs$thresh, sep="")
mode_summary$n_simp_obs[ind]<-n_simp_obs$x
ind <- c()

ind<-mode_summary$case%in%paste(med_area_simp_obs$rad, med_area_simp_obs$thresh, sep="")
mode_summary$med_area_simp_obs[ind]<-med_area_simp_obs$x
ind <- c()

ind<-mode_summary$case%in%paste(tot_area_simp_obs$rad, tot_area_simp_obs$thresh, sep="")
mode_summary$tot_area_simp_obs[ind]<-tot_area_simp_obs$x
ind <- c()

# Matched simple observation objects
ind<-mode_summary$case%in%paste(n_simp_obs_match$rad, n_simp_obs_match$thresh, sep="")
mode_summary$n_simp_obs_match[ind]<-n_simp_obs_match$x
ind <- c()

ind<-mode_summary$case%in%paste(med_area_simp_obs_match$rad, med_area_simp_obs_match$thresh, sep="")
mode_summary$med_area_simp_obs_match[ind]<-med_area_simp_obs_match$x
ind <- c()

ind<-mode_summary$case%in%paste(tot_area_simp_obs_match$rad, tot_area_simp_obs_match$thresh, sep="")
mode_summary$tot_area_simp_obs_match[ind]<-tot_area_simp_obs_match$x
ind <- c()

# Unmatch simple observation objects
ind<-mode_summary$case%in%paste(n_simp_obs_unmatch$rad, n_simp_obs_unmatch$thresh, sep="")
mode_summary$n_simp_obs_unmatch[ind]<-n_simp_obs_unmatch$x
ind <- c()

ind<-mode_summary$case%in%paste(med_area_simp_obs_unmatch$rad, med_area_simp_obs_unmatch$thresh, sep="")
mode_summary$med_area_simp_obs_unmatch[ind]<-med_area_simp_obs_unmatch$x
ind <- c()

ind<-mode_summary$case%in%paste(tot_area_simp_obs_unmatch$rad, tot_area_simp_obs_unmatch$thresh, sep="")
mode_summary$tot_area_simp_obs_unmatch[ind]<-tot_area_simp_obs_unmatch$x
ind <- c()

# Interest value measures
ind<-mode_summary$case%in%paste(max_interest$rad, max_interest$thresh, sep="")
mode_summary$max_interest[ind]<-max_interest$x
ind <- c()

ind<-mode_summary$case%in%paste(med_interest$rad, med_interest$thresh, sep="")
mode_summary$med_interest[ind]<-med_interest$x
ind <- c()

ind<-mode_summary$case%in%paste(med_of_max_interest_fcst$rad, med_of_max_interest_fcst$thresh, sep="")
mode_summary$med_of_max_interest_fcst[ind]<-med_of_max_interest_fcst$x
ind <- c()

ind<-mode_summary$case%in%paste(med_of_max_interest_obs$rad, med_of_max_interest_obs$thresh, sep="")
mode_summary$med_of_max_interest_obs[ind]<-med_of_max_interest_obs$x
ind <- c()

ind<-mode_summary$case%in%paste(med_of_max_interest_fcst_obs$rad, med_of_max_interest_fcst_obs$thresh, sep="")
mode_summary$med_of_max_interest_fcst_obs[ind]<-med_of_max_interest_fcst_obs$x
ind <- c()

# Replace any instances of NA with zero
#mode_summary[is.na(mode_summary)]=0

# Save all of the data in the current directory for use later
save.image()

###############################################################################
#
# Create output ASCII file and quilt plots.
#
###############################################################################

txt_file<-paste(prefix, "_mode_quilts.txt", sep="")
cat("Creating ASCII file:", txt_file, "\n")
write.fwf(mode_summary, file=txt_file, quote=FALSE, colnames=TRUE, rownames=FALSE, justify="right")

pdf_file<-paste(prefix, "_mode_quilts.pdf", sep="")
cat("Creating PDF file:", pdf_file, "\n")
pdf(pdf_file, height = 8.5, width = 11)

###############################################################################
#
# Quilts for all simple objects:
#    - Object counts
#    - Median area
#    - Total areas
#    - Percent objects matched
#    - Percent area matched
#
###############################################################################

# Total object counts
print(mode_summary$n_simp)

yy <- rep(1, length(radius))
for (rr in 2:length(thresh))
  yy <- c(yy, rep(rr, length(radius)))
          
quilt.plot(x=as.numeric(as.vector(mode_summary$rad)),
#           y=as.numeric(as.vector(gsub("=|<|>", "", mode_summary$thresh))),
           y=yy, yaxt="n", #col=cols, nlevel=9,
           z=mode_summary$n_simp,
           nrow=length(radius), ncol=length(thresh),
           main=paste(prefix, "\nTotal Simple Object Counts\n", "Valid:", mode_data$FCST_VALID[1]),
           xlab="Convolution Radius",
           ylab="Convolution Threshold")
axis(2, at=1:len, labels=thresh0)
box()

# Median object areas
quilt.plot(x=as.numeric(as.vector(mode_summary$rad)),
           y=yy, yaxt="n",#col=cols, nlevel=9,
           z=mode_summary$med_area_simp,
           nrow=length(radius), ncol=length(thresh),
           main=paste(prefix, "\nMedian Object Area\n", "Valid:", mode_data$FCST_VALID[1]),
           xlab="Convolution Radius",
           ylab="Convolution Threshold")
axis(2, at=1:len, labels=thresh0)
box()

# Total object areas
quilt.plot(x=as.numeric(as.vector(mode_summary$rad)),
           y=yy, yaxt="n",#col=cols, nlevel=9,
           z=mode_summary$tot_area_simp,
           nrow=length(radius), ncol=length(thresh),
           main=paste(prefix, "\nTotal Object Area\n", "Valid:", mode_data$FCST_VALID[1]),
           xlab="Convolution Radius",
           ylab="Convolution Threshold")
axis(2, at=1:len, labels=thresh0)
box()

# Percentage of objects matched
quilt.plot(x=as.numeric(as.vector(mode_summary$rad)),
           y=yy, yaxt="n",#col=cols, 
           z=mode_summary$n_simp_match/mode_summary$n_simp,
           zlim=c(0,1),
           nrow=length(radius), ncol=length(thresh),
           main=paste(prefix, "\nPercentage of Simple Objects Matched\n", "Valid:", mode_data$FCST_VALID[1]),
           xlab="Convolution Radius",
           ylab="Convolution Threshold")
axis(2, at=1:len, labels=thresh0)
box()

# Percentage of area matched
quilt.plot(x=as.numeric(as.vector(mode_summary$rad)),
           y=yy, yaxt="n",
           z=mode_summary$tot_area_simp_match/mode_summary$tot_area_simp,
           zlim=c(0,1),
           nrow=length(radius), ncol=length(thresh),
           main=paste(prefix, "\nPercentage of Area Matched\n", "Valid:", mode_data$FCST_VALID[1]),
           xlab="Convolution Radius",
           ylab="Convolution Threshold")
axis(2, at=1:len, labels=thresh0)
box()

###############################################################################
#
# Quilts for forecast simple objects:
#    - Forecast object counts
#    - Median forecast area
#    - Total forecast areas
#    - Percent forecast objects matched
#    - Percent forecast area matched
#
###############################################################################

# Forecast object counts
quilt.plot(x=as.numeric(as.vector(mode_summary$rad)),
           y=yy, yaxt="n",
           z=mode_summary$n_simp_fcst,
           nrow=length(radius), ncol=length(thresh),
           main=paste(prefix, "\nForecast Simple Object Counts\n", "Valid:", mode_data$FCST_VALID[1]),
           xlab="Convolution Radius",
           ylab="Convolution Threshold")
axis(2, at=1:len, labels=thresh0)
box()

# Median forecast object areas
quilt.plot(x=as.numeric(as.vector(mode_summary$rad)),
           y=yy, yaxt="n",
           z=mode_summary$med_area_simp_fcst,
           nrow=length(radius), ncol=length(thresh),
           main=paste(prefix, "\nMedian Forecast Object Area\n", "Valid:", mode_data$FCST_VALID[1]),
           xlab="Convolution Radius",
           ylab="Convolution Threshold")
axis(2, at=1:len, labels=thresh0)
box()

# Total forecast object areas
quilt.plot(x=as.numeric(as.vector(mode_summary$rad)),
           y=yy, yaxt="n",
           z=mode_summary$tot_area_simp_fcst,
           nrow=length(radius), ncol=length(thresh),
           main=paste(prefix, "\nTotal Forecast Object Area\n", "Valid:", mode_data$FCST_VALID[1]),
           xlab="Convolution Radius",
           ylab="Convolution Threshold")
axis(2, at=1:len, labels=thresh0)
box()

# Percentage of forecast objects matched
quilt.plot(x=as.numeric(as.vector(mode_summary$rad)),
           y=yy, yaxt="n",
           z=mode_summary$n_simp_fcst_match/mode_summary$n_simp_fcst,
           zlim=c(0,1),
           nrow=length(radius), ncol=length(thresh),
           main=paste(prefix, "\nPercentage of Simple Forecast Objects Matched\n", "Valid:", mode_data$FCST_VALID[1]),
           xlab="Convolution Radius",
           ylab="Convolution Threshold")
axis(2, at=1:len, labels=thresh0)
box()

# Percentage of forecast area matched
quilt.plot(x=as.numeric(as.vector(mode_summary$rad)),
           y=yy, yaxt="n",
           z=mode_summary$tot_area_simp_fcst_match/mode_summary$tot_area_simp_fcst,
           zlim=c(0,1),
           nrow=length(radius), ncol=length(thresh),
           main=paste(prefix, "\nPercentage of Forecast Area Matched\n", "Valid:", mode_data$FCST_VALID[1]),
           xlab="Convolution Radius",
           ylab="Convolution Threshold")
axis(2, at=1:len, labels=thresh0)
box()

###############################################################################
#
# Quilts for observation simple objects:
#    - Observation object counts
#    - Median observation area
#    - Total observation areas
#    - Percent observation objects matched
#    - Percent observation area matched
#
###############################################################################

# Observation object counts
quilt.plot(x=as.numeric(as.vector(mode_summary$rad)),
           y=yy, yaxt="n",
           z=mode_summary$n_simp_obs,
           nrow=length(radius), ncol=length(thresh),
           main=paste(prefix, "\nObservation Simple Object Counts\n", "Valid:", mode_data$FCST_VALID[1]),
           xlab="Convolution Radius",
           ylab="Convolution Threshold")
axis(2, at=1:len, labels=thresh0)
box()

# Median observation object areas
quilt.plot(x=as.numeric(as.vector(mode_summary$rad)),
           y=yy, yaxt="n",
           z=mode_summary$med_area_simp_obs,
           nrow=length(radius), ncol=length(thresh),
           main=paste(prefix, "\nMedian Observation Object Area\n", "Valid:", mode_data$FCST_VALID[1]),
           xlab="Convolution Radius",
           ylab="Convolution Threshold")
axis(2, at=1:len, labels=thresh0)
box()

# Total observation object areas
quilt.plot(x=as.numeric(as.vector(mode_summary$rad)),
           y=yy, yaxt="n",
           z=mode_summary$tot_area_simp_obs,
           nrow=length(radius), ncol=length(thresh),
           main=paste(prefix, "\nTotal Observation Object Area\n", "Valid:", mode_data$FCST_VALID[1]),
           xlab="Convolution Radius",
           ylab="Convolution Threshold")
axis(2, at=1:len, labels=thresh0)
box()

# Percentage of observation objects matched
quilt.plot(x=as.numeric(as.vector(mode_summary$rad)),
           y=yy, yaxt="n",
           z=mode_summary$n_simp_obs_match/mode_summary$n_simp_obs,
           zlim=c(0,1),
           nrow=length(radius), ncol=length(thresh),
           main=paste(prefix, "\nPercentage of Simple Observation Objects Matched\n", "Valid:", mode_data$FCST_VALID[1]),
           xlab="Convolution Radius",
           ylab="Convolution Threshold")
axis(2, at=1:len, labels=thresh0)
box()

# Percentage of observation area matched
quilt.plot(x=as.numeric(as.vector(mode_summary$rad)),
           y=yy, yaxt="n",
           z=mode_summary$tot_area_simp_obs_match/mode_summary$tot_area_simp_obs,
           zlim=c(0,1),
           nrow=length(radius), ncol=length(thresh),
           main=paste(prefix, "\nPercentage of Observation Area Matched\n", "Valid:", mode_data$FCST_VALID[1]),
           xlab="Convolution Radius",
           ylab="Convolution Threshold")
axis(2, at=1:len, labels=thresh0)
box()

###############################################################################
#
# Quilts for pairs of simple objects:
#    - Object pair max interest
#    - Object pair median interest
#    - Median of max interest by fcst
#    - Median of max interest by obs
#    - Median of max interest by fcst and obs
#
###############################################################################

# Object pair max interest
quilt.plot(x=as.numeric(as.vector(mode_summary$rad)),
           y=yy, yaxt="n",
           z=mode_summary$max_interest,
           zlim=c(0,1),
           nrow=length(radius), ncol=length(thresh),
           main=paste(prefix, "\nMaximum Interest for All Pairs\n", "Valid:", mode_data$FCST_VALID[1]),
           xlab="Convolution Radius",
           ylab="Convolution Threshold")
axis(2, at=1:len, labels=thresh0)
box()

# Object pair median interest
quilt.plot(x=as.numeric(as.vector(mode_summary$rad)),
           y=yy, yaxt="n",
           z=mode_summary$med_interest,
           zlim=c(0,1),
           nrow=length(radius), ncol=length(thresh),
           main=paste(prefix, "\nMedian Interest for All Pairs\n", "Valid:", mode_data$FCST_VALID[1]),
           xlab="Convolution Radius",
           ylab="Convolution Threshold")
axis(2, at=1:len, labels=thresh0)
box()

# Median of maximum interest by fcst
quilt.plot(x=as.numeric(as.vector(mode_summary$rad)),
           y=yy, yaxt="n",
           z=mode_summary$med_of_max_interest_fcst,
           zlim=c(0,1),
           nrow=length(radius), ncol=length(thresh),
           main=paste(prefix, "\nMedian of Maximum Interests for Forecast Objects\n", "Valid:", mode_data$FCST_VALID[1]),
           xlab="Convolution Radius",
           ylab="Convolution Threshold")
axis(2, at=1:len, labels=thresh0)
box()

# Median of maximum interest by obs
quilt.plot(x=as.numeric(as.vector(mode_summary$rad)),
           y=yy, yaxt="n",
           z=mode_summary$med_of_max_interest_obs,
           zlim=c(0,1),
           nrow=length(radius), ncol=length(thresh),
           main=paste(prefix, "\nMedian of Maximum Interests for Observation Objects\n", "Valid:", mode_data$FCST_VALID[1]),
           xlab="Convolution Radius",
           ylab="Convolution Threshold")
axis(2, at=1:len, labels=thresh0)
box()

# Median of maximum interest by fcst and obs
quilt.plot(x=as.numeric(as.vector(mode_summary$rad)),
           y=yy, yaxt="n",
           z=mode_summary$med_of_max_interest_fcst_obs,
           zlim=c(0,1),
           nrow=length(radius), ncol=length(thresh),
           main=paste(prefix, "\nMedian of Maximum Interests for Forecast and Observation Objects\n", "Valid:", mode_data$FCST_VALID[1]),
           xlab="Convolution Radius",
           ylab="Convolution Threshold")

dev.off()
