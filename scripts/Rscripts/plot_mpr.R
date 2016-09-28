########################################################################
##
##  Name: plot_mpr.R
##
##  Description:
##    This script should be called via an Rscript command to plot
##    matched pair data (MPR line type) from the MET Point-Stat
##    tool.  This script can read the following types of ASCII files:
##      - MPR files from Point-Stat (*_mpr.txt)
##      - STAT files from Point-Stat (*.stat)
##      - The output of a Stat-Analysis "filter" job
##
##    Note: This script will create a separate plot for each case found.
##    A case is a combination of the model name, forecast variable,
##    forecast level, observation type, masking region, and
##    interpolation method.  Values such as the lead time and valid
##    time are NOT included in the case information.
##
##  Usage:
##    Rscript plot_mpr.R
##      file_list
##      [-wind_rose]
##      [-out name]
##      [-save]
##
##  Arguments:
##    "file_list" is one or more files containing MPR lines.
##    "-out name" specifies an output PDF file name.
##    "-save"     calls save.image() before exiting R.
##
##  Details:
##    Updated for MET version 6.0.
##
##  Examples:
##    Rscript plot_mpr.R \
##      met-6.0/out/point_stat/*_mpr.txt
##
##   Author:
##      John Halley Gotway (johnhg@ucar.edu), NCAR-RAL/DTC
##      02/09/2011
##
########################################################################

library(stats)

########################################################################
#
# Wind Rose Plotting function.
#
########################################################################

plot_wind_rose <- function(u, v, title) {
  library(openair)
  ws = sqrt(u^2 + v^2);
  wd = atan2(u/ws, v/ws) * 180/pi;
  pollutionRose(data.frame(ws=ws, wd=wd), pollutant="ws", main=title)
}

########################################################################
#
# Constants.
#
########################################################################

# Header for the MPR line type (MET version 6.0)
mpr_header <- c("VERSION", "MODEL", "DESC",
                "FCST_LEAD", "FCST_VALID_BEG", "FCST_VALID_END",
                "OBS_LEAD", "OBS_VALID_BEG", "OBS_VALID_END",
                "FCST_VAR", "FCST_LEV",
                "OBS_VAR", "OBS_LEV",
                "OBTYPE", "VX_MASK",
                "INTERP_MTHD", "INTERP_PNTS",
                "FCST_THRESH", "OBS_THRESH", "COV_THRESH",
                "ALPHA", "LINE_TYPE",
                "TOTAL", "INDEX", "OBS_SID", "OBS_LAT", "OBS_LON",
                "OBS_LVL", "OBS_ELV", "FCST", "OBS", "CLIMO")

# Temporary input file name
tmp_file <- "mpr_input.tmp"

# Default output file name
out_file = "mpr_plots.pdf"

########################################################################
#
# Handle the arguments.
#
########################################################################

# Retreive the arguments
args = commandArgs(TRUE)

# Check the number of arguments
if(length(args) < 1) {
   cat("Usage: plot_mpr.R\n")
   cat("         mpr_file_list\n")
   cat("         [-wind_rose]\n")
   cat("         [-out name]\n")
   cat("         [-save]\n")
   cat("         where \"file_list\"  is one or more files containing MPR lines.\n")
   cat("               \"-wind_rose\" enables plotting of vector winds.\n")
   cat("               \"-out name\"  specifies an output PDF file name.\n")
   cat("               \"-save\"      calls save.image() before exiting R.\n\n")
   quit()
}

# Initialize
save = FALSE
wind_rose = FALSE
file_list = c()

# Parse the arguments
i=1
while(i <= length(args)) {

  # Check optional arguments
  if(args[i] == "-save") {
    save = TRUE
  } else if(args[i] == "-wind_rose") {
    wind_rose = TRUE
  } else if(args[i] == "-out") {

    # Set the output file name
    out_file = args[i+1]
    i = i+1

  } else {

    # Add input file to the file list
    file_list = c(file_list, args[i])
  }

  # Increment i
  i = i+1
}

########################################################################
#
# Read the input files.
#
########################################################################

# Initialize
data <- c()

for(i in 1:length(file_list)) {
  print(paste("Reading:", file_list[i]))

  # Select MPR lines out of the input file and write it to a temp file
  cmd <- paste("grep \" MPR \"", file_list[i], ">", tmp_file)
  system(cmd)

  # Try to read the input file
  status <- try(mpr_file <- read.table(tmp_file, header=FALSE))
  if(class(status) != "try-error") {
    n_lines <- dim(mpr_file)[1]
    data <- rbind(data, mpr_file)
  } else {
    n_lines <- 0
  }
  print(paste("Found:", n_lines, "MPR lines"))

  # Remove the temp file
  cmd <- paste("rm", tmp_file)
  system(cmd)
}

# After constructing the input data, attach column names
colnames(data) <- mpr_header

########################################################################
#
# Create output images for each case.
#
########################################################################

# Construct an idex
data$index <- paste(data$MODEL,
                    data$FCST_VAR, data$FCST_LEV,
                    data$OBS_VAR,  data$OBS_LEV,
                    data$OBTYPE, data$VX_MASK,
                    data$INTERP_MTHD, data$INTERP_PNTS,
                    sep='_')

# Build a list of cases
case_list <- unique(data$index)

# Open up the output device
print(paste("Writing:", out_file))
pdf(out_file, height=8.5, width=11, useDingbats=FALSE)

# Loop through each of the cases and create plots
for(i in 1:length(case_list)) {

  # Print status message
  print(paste("Processing case:", case_list[i]))

  # Get the subset for this case
  ind       <- data$index == case_list[i]
  main_info <- paste(data$MODEL[ind][1], ": ", data$FCST_VAR[ind][1],
                     " at ", data$FCST_LEV[ind][1], sep='')
  case_info <- paste(data$OBTYPE[ind][1],
                     ", ", data$VX_MASK[ind][1],
                     ", ", data$INTERP_MTHD[ind][1],
                     "(", data$INTERP_PNTS[ind][1], ")", sep='')

  # Setup a 2x2 plotting page
  par(mfrow=c(2,2))

  # Create histograms of the forecsat and observation values
  chist <- hist(c(data$FCST[ind], data$OBS[ind]), plot=FALSE)
  brks  <- chist$breaks
  fhist <- hist(data$FCST[ind], breaks=brks, plot=FALSE)
  ohist <- hist(data$OBS[ind],  breaks=brks, plot=FALSE)
  ymax  <- max(fhist$counts, ohist$counts)

  title <- paste("Forecast Histogram of", sum(ind), " points\n", main_info, "\n", case_info)
  hist(data$FCST[ind], main=title, ylab="Frequency", xlab="Forecast",
       breaks=brks, ylim=c(0, ymax))
  title <- paste("Observation Histogram of", sum(ind), " points\n", main_info, "\n", case_info)
  hist(data$OBS[ind], main=title, ylab="Frequency", xlab="Observation",
       breaks=brks, ylim=c(0, ymax))

  # Colors for plotting points
  colors <- rgb(0, 0, 1, 0.25)

  # Create a scatter plot
  title <- paste("Scatter Plot of", sum(ind), " points\n", main_info, "\n", case_info)
  plot(data$FCST[ind], data$OBS[ind], main=title, ylab="Observation", xlab="Forecast",
       col=colors, pch=19)
  abline(a=0, b=1, lwd=2, lty=2)

  # Create a Q-Q plot
  title <- paste("Q-Q Plot of", sum(ind), " points\n", main_info, "\n", case_info)
  qqplot(data$FCST[ind], data$OBS[ind], main=title, ylab="Observation", xlab="Forecast",
         col=colors, pch=19)
  abline(a=0, b=1, lwd=2, lty=2)

  # Check for UGRD/VGRD vector pairs and plot wind rose
  if(wind_rose &&
     data$FCST_VAR[ind][1] == "UGRD" &&
     data$OBS_VAR[ind][1]  == "UGRD") {

    # Store UGRD/VGRD indices
    uind <- ind;
    vgrd_case = gsub("UGRD", "VGRD", case_list[i]);
    vind <- data$index == vgrd_case;

    # Make sure they are the same lenght and the station id's match
    if(sum(uind) != sum(vind) ||
       sum(data$OBS_SID[uind] == data$OBS_SID[vind]) != sum(uind)) {
       print(paste("WARNING: UGRD/VGRD vectors do not exactly match"))
       next;
    }

    # Print status message
    print(paste("Processing vector:", case_list[i], "and", vgrd_case))

    # Title string
    title_str = paste(sum(ind),  "points\n",
                      gsub("UGRD", "Winds", main_info),
                      case_info);

    # Create wind rose plots
    plot_wind_rose(data$FCST[uind], data$FCST[vind],
      paste("Forecast Winds for", title_str));
    plot_wind_rose( data$OBS[uind],  data$OBS[vind],
      paste("Observed Winds for", title_str));
    plot_wind_rose(data$FCST[uind] - data$OBS[uind],
                   data$FCST[vind] - data$OBS[vind],
      paste("Wind Errors for", title_str));
  }
}

# Finished with the plots
dev.off()

# List the completed output file
print(paste("Finished:", out_file))

# Save the R working environment
if(save == TRUE) {
  print("Saving image...")
  save.image()
}
