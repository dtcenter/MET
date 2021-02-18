########################################################################
##
##  Name: plot_cnt.R
##
##  Description:
##    This script should be called via an Rscript command to plot
##    continuous statistics (CNT line type) from the MET Grid-Stat or
##    Point-Stat tools.  This script can read the following types of
##    ASCII files:
##      - CNT files from Grid-Stat and Point-Stat (*_cnt.txt)
##      - STAT files from Grid-Stat and Point-Stat (*.stat)
##      - The output of a Stat-Analysis "filter" job
##
##    Note: This script will create a separate plot for each case found.
##    A case is a combination of the model name, forecast variable,
##    forecast level, observation type, masking region, and
##    interpolation method.  Values such as the lead time and valid
##    time are NOT included in the case information.
##
##  Usage:
##    Rscript plot_cnt.R
##      file_list
##      [-column name]
##      [-out name]
##      [-met_base path]
##      [-save]
##
##  Arguments:
##    "file_list"      is one or more files containing CNT lines.
##    "-column name"   specifies a CNT statistic to be plotted (multiple).
##    "-out name"      specifies an output PDF file name.
##    "-met_base path" is MET_INSTALL_DIR/share/met for the headers.
##    "-save"          calls save.image() before exiting R.
##
##  Details:
##    Updated on 02/03/2021 to parse version-specific headers.
##
##  Examples:
##    Rscript plot_cnt.R \
##      out/point_stat/*_cnt.txt
##
##   Author:
##      John Halley Gotway (johnhg@ucar.edu), NCAR-RAL/DTC
##      02/11/2011
##
########################################################################

library(stats)

########################################################################
#
# Constants.
#
########################################################################

# Temporary input file name
tmp_file = "cnt_input.tmp"

# Default output file name
out_file = "cnt_plots.pdf"

# Default statistics to be plotted
default_stat_list <- c("RMSE")

########################################################################
#
# Handle the arguments.
#
########################################################################

# Retreive the arguments
args = commandArgs(TRUE)

# Check the number of arguments
if(length(args) < 1) {
  cat("Usage: plot_cnt.R\n")
  cat("         file_list\n")
  cat("         [-column name]\n")
  cat("         [-out name]\n")
  cat("         [-met_base path]\n")
  cat("         [-save]\n")
  cat("         where \"file_list\"      is one or more files containing CNT lines.\n")
  cat("               \"-column name\"   specifies a CNT statistic to be plotted (multiple).\n")
  cat("               \"-out name\"      specifies an output PDF file name.\n")
  cat("               \"-met_base path\" is MET_INSTALL_DIR/share/met for the headers.\n")
  cat("               \"-save\"          calls save.image() before exiting R.\n\n")
  quit()
}

# Initialize
file_list = c()
stat_list = c()
met_base  = ''
save      = FALSE

# Parse the arguments
i=1
while(i <= length(args)) {

  # Check optional arguments
  if(args[i] == "-save") {
    save = TRUE
  } else if(args[i] == "-out") {

    # Set the output file name
    out_file = args[i+1]
    i = i+1

  } else if(args[i] == "-met_base") {

    # Set MET_BASE variable
    met_base = args[i+1]
    i = i+1

  } else if(args[i] == "-column") {

    # Add column name to the stat list
    stat_list = c(stat_list, args[i+1])
    i = i+1

  } else {

    # Add input file to the file list
    file_list = c(file_list, args[i])

  }

  # Increment i
  i = i+1
}

if(length(stat_list) <= 0) stat_list <- default_stat_list

########################################################################
#
# Read the input files.
#
########################################################################

# Check for input files
if(is.null(file_list)) {
  cat("ERROR: No input files specified!\n")
  quit()
}

# Initialize
data <- c()

for(i in 1:length(file_list)) {
  print(paste("Reading:", file_list[i]))

  # Select CNT lines out of the input file and write it to a temp file
  cmd <- paste("grep \" CNT \"", file_list[i], ">", tmp_file)
  system(cmd)

  # Try to read the input file
  status <- try(cnt_file <- read.table(tmp_file, header=FALSE))
  if(class(status) != "try-error") {
    n_lines <- dim(cnt_file)[1]
    data <- rbind(data, cnt_file)
  } else {
    n_lines <- 0
  }
  print(paste("Found:", n_lines, "CNT lines"))

  # Remove the temp file
  cmd <- paste("rm", tmp_file)
  system(cmd)
}

# Check for no data
if(is.null(data)) {
  cat("ERROR: No CNT data found!\n")
  quit()
}

# Store version from the data
version = unlist(strsplit(as.character(data[1,1]), '\\.'))
vXY = paste(version[1], version[2], sep='.')

# Check met_base
if(nchar(met_base) == 0) {
  met_base = Sys.getenv("MET_BASE")
}
if(nchar(met_base) == 0) {
  cat("ERROR: The -met_base command line option or MET_BASE environment variable must be set!\n",
      "ERROR: Define it as {MET INSTALLATION DIRECTORY}/share/met.\n", sep='')
  quit()
}

# Get the header columns
header_file = paste(met_base, "/table_files/met_header_columns_", vXY, ".txt", sep='')
print(paste("Reading Header File:", header_file))
lty_str  = paste(" : CNT ", sep='')
hdr_line = grep(lty_str, readLines(header_file), value=TRUE)
hdr_cols = trimws(unlist(strsplit(hdr_line, ':'))[4])
hdr_lty  = unlist(strsplit(hdr_cols, ' '))

# Check that header and data columns match
if(length(hdr_lty) != dim(data)[2]) {
  cat("ERROR: The number of data (", dim(data)[2],
      ") and header (", length(hdr_lty),
      ") columns do not match!\n", sep='')
  quit()
}

# After constructing the input data, attach column names
colnames(data) <- hdr_lty

# Convert date/time columns to date/time objects
data$FCST_VALID_BEG <- as.POSIXct(strptime(data$FCST_VALID_BEG,
                                  format="%Y%m%d_%H%M%S"))
data$FCST_VALID_END <- as.POSIXct(strptime(data$FCST_VALID_END,
                                  format="%Y%m%d_%H%M%S"))
data$OBS_VALID_BEG  <- as.POSIXct(strptime(data$OBS_VALID_BEG,
                                  format="%Y%m%d_%H%M%S"))
data$OBS_VALID_END  <- as.POSIXct(strptime(data$OBS_VALID_END,
                                  format="%Y%m%d_%H%M%S"))

########################################################################
#
# Create output images for each case.
#
########################################################################

# Construct an index
data$index <- paste(data$MODEL,
                    data$FCST_VAR, data$FCST_LEV,
                    data$OBS_VAR,  data$OBS_LEV,
                    data$OBTYPE, data$VX_MASK,
                    data$INTERP_MTHD, data$INTERP_PNTS,
                    data$ALPHA,
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
                     "(", data$INTERP_PNTS[ind][1], ")",
                     ",", data$ALPHA[ind][1], sep='')

  # Loop through each of the statistics to be plotted
  for(j in 1:length(stat_list)) {

    # Create a boxplot of stats versus lead time
    title <- paste(stat_list[j], "Boxplots for", sum(ind),
                   "statistics versus lead time.\n",
                   main_info, "\n", case_info)
    boxplot(data[[stat_list[j]]][ind] ~ data$FCST_LEAD[ind],
            main=title, ylab=stat_list[j], xlab="Lead Time")
    abline(h=0, lwd=2, lty=2)

    # Create a time series plot for each lead time.
    lead_list <- unique(data$FCST_LEAD[ind])
    for(k in 1:length(lead_list)) {

      # Get indicator for this set of lead times
      ind2 <- data$FCST_LEAD[ind] == lead_list[k]

      title <- paste(stat_list[j], "Time Series of", sum(ind2),
                     "statistics for", lead_list[k], "lead time.\n",
                     main_info, "\n", case_info)
      plot(data$FCST_VALID_BEG[ind][ind2],
           data[[stat_list[j]]][ind][ind2], type='b',
           main=title, ylab=stat_list[j], xlab="Valid Time")
      abline(h=0, lwd=2, lty=2)

    } # end for k
  } # end for j
} # end for i

# Finished with the plots
dev.off()

# List the completed output file
print(paste("Finished:", out_file))

# Save the R working environment
if(save == TRUE) {
  print("Saving image...")
  save.image()
}
