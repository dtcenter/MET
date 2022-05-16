########################################################################
##
##  Name: plot_ensemble.R
##
##  Description:
##    This script should be called via an Rscript command to plot
##    ensemble statistic line types from the MET Ensemble-Stat tool.
##    This script can read the following types of ASCII files:
##      - TXT files from Ensemble-Stat
##      - STAT files from Ensemble-Stat (*.stat)
##      - The output of a Stat-Analysis "filter" job
##
##    Note: This script will create a separate plot for each case found.
##    A case is a combination of the model name, description, forecast
##    variable, forecast level, observation type, masking region, and
##    interpolation method.  Values such as the lead time and valid
##    time are NOT included in the case information.
##
##  Usage:
##    Rscript plot_ensemble.R
##      file_list
##      [-line_type name]
##      [-out name]
##      [-met_base path]
##      [-save]
##
##  Arguments:
##    "file_list"       is one or more files containing CNT lines.
##    "-line_type name" specifies a line type to process (multiple).
##    "-out name"       specifies an output PDF file name.
##    "-met_base path"  is MET_INSTALL_DIR/share/met for the headers.
##    "-save"           calls save.image() before exiting R.
##
##  Details:
##    Updated on 02/03/2021 to parse version-specific headers.
##
##  Examples:
##    Rscript plot_ensemble.R \
##      out/ensemble_stat/*.stat
##
##   Author:
##      John Halley Gotway (johnhg@ucar.edu), NCAR-RAL/DTC
##      06/22/2018
##
########################################################################

library(stats)

########################################################################
#
# Constants.
#
########################################################################

# Temporary input file name
tmp_file <- "ensemble_input.tmp"

# Default output file name
out_file = "ensemble_plots.pdf"

# Line types
default_line_types = c("ECNT");

########################################################################
#
# Handle the arguments.
#
########################################################################

# Retreive the arguments
args = commandArgs(TRUE)

# Check the number of arguments
if(length(args) < 1) {
  cat("Usage: plot_ensemble.R\n")
  cat("         file_list\n")
  cat("         [-line_type name]\n")
  cat("         [-out name]\n")
  cat("         [-met_base path]\n")
  cat("         [-save]\n")
  cat("         where \"file_list\"       is one or more files containing ensemble output.\n")
  cat("               \"-line_type name\" specifies a line type to process (multiple).\n")
  cat("               \"-out name\"       specifies an output PDF file name.\n")
  cat("               \"-met_base path\"  is MET_INSTALL_DIR/share/met for the headers.\n")
  cat("               \"-save\"           calls save.image() before exiting R.\n\n")
  quit()
}

# Initialize
file_list  = c()
line_types = c()
met_base   = ''
save       = FALSE

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

   } else if(args[i] == "-line_type") {

    # Add column name to the line type list
    line_types = c(line_types, args[i+1])
    i = i+1

   } else if(args[i] == "-met_base") {

     # Set MET_BASE variable
     met_base = args[i+1]
     i = i+1

  } else {

    # Add input file to the file list
    file_list = c(file_list, args[i])

  }

  # Increment i
  i = i+1
}

if(length(line_types) <= 0) line_types <- default_line_types

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

read_line_type = function(line_type) {

  # Initialize
  d <- c()

  cat("Processing:", line_type, "line type.\n")

  for(i in 1:length(file_list)) {
    cat("Reading:", file_list[i], "\n")

    # Select lines out of the input file and write it to a temp file
    cmd <- paste("grep \"", line_type, "\"", file_list[i], ">", tmp_file)
    system(cmd)

    # Try to read the input file
    status <- try(cur_file <- read.table(tmp_file, header=FALSE))
    if(class(status) != "try-error") {
      n_lines <- dim(cur_file)[1]
      d <- rbind(d, cur_file)
    } else {
      n_lines <- 0
    }
    cat("Found:", n_lines, line_type, "lines\n")

    # Remove the temp file
    cmd <- paste("rm", tmp_file)
    system(cmd)
  }

  # Check for no data
  if(is.null(d)) {
    return(d)
  }

  # Store version from the data
  version = unlist(strsplit(as.character(d[1,1]), '\\.'))
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

  # Get that header columns
  header_file = paste(met_base, "/table_files/met_header_columns_", vXY, ".txt", sep='')
  print(paste("Reading Header File:", header_file))
  lty_str  = paste(" : ", line_type, " ", sep='')
  hdr_line = grep(lty_str, readLines(header_file), value=TRUE)
  hdr_line = gsub('[\\(,\\)]', '', hdr_line) # Strip parens from header line
  hdr_cols = trimws(unlist(strsplit(hdr_line, ':'))[4])
  hdr_lty  = unlist(strsplit(hdr_cols, ' '))

  # Do not check that the number of header and data columns match
  # since RHIST and RELP line types have variable length.

  colnames(d) <- hdr_lty

  return(d)
}

########################################################################
#
# Plot the Ranked Histogram.
#
########################################################################

plot_rhist = function(data, main_info, case_info) {

  cat("Plotting RHIST ...", main_info, case_info, "\n");

  title <- paste("Rank Histogram of", dim(data)[1], "cases",
                 main_info, "\n", case_info)

  n_rank <- max(data$N_RANK)

  i_n_rank <- grep("N_RANK", colnames(data))

  counts <- colSums(data[,(i_n_rank+1):(i_n_rank+n_rank)])
  names(counts) <- as.character(seq(1,n_rank))

  barplot(counts, main=title, xlab="Ranks");

  return
}

########################################################################
#
# Plot the Relative Performance Histogram.
#
########################################################################

plot_relp = function(data, main_info, case_info) {

  cat("Plotting RELP ...", main_info, case_info, "\n");

  title <- paste("Relative Performance of", dim(data)[1], "cases",
                 main_info, "\n", case_info)

  n_ens <- max(data$N_ENS)

  i_n_ens <- grep("N_ENS", colnames(data))

  counts <- colSums(data[,(i_n_ens+1):(i_n_ens+n_ens)])
  names(counts) <- as.character(seq(1,n_ens))

  barplot(counts, main=title, xlab="Ensemble Member");

  return
}

########################################################################
#
# Plot the Continuous Ensemble Statistics.
#
########################################################################

plot_ecnt_spread_skill = function(data, main_info, case_info) {

  cat("Plotting ECNT ...", main_info, case_info, "\n");

  title <- paste("Ensemble Spread/Skill of", dim(data)[1], "cases",
                 main_info, "\n", case_info)

  # Compute lead mean statistic for each lead time
  lead_list <- unique(data$FCST_LEAD)
  rmse = spread = rmse_oerr = spread_oerr = spread_plus_oerr = c()

  for(i in 1:length(lead_list)) {
    ind    <- data$FCST_LEAD == lead_list[i]
    rmse   <- c(rmse, mean(data[ind,]$RMSE))
    spread <- c(spread, mean(data[ind,]$SPREAD))
    rmse_oerr   <- c(rmse_oerr, mean(data[ind,]$RMSE_OERR))
    spread_oerr <- c(spread_oerr, mean(data[ind,]$SPREAD_OERR))
    spread_plus_oerr <- c(spread_plus_oerr, mean(data[ind,]$SPREAD_PLUS_OERR))
  }

  lead_list = lead_list/10000
  plot(lead_list, rmse, type="b",
       ylim=range(c(rmse, spread, rmse_oerr, spread_oerr, spread_plus_oerr), na.rm=TRUE),
       main=title, ylab="Spread/Skill", xlab="Forecast Lead",
       col="red")
  lines(lead_list, spread, type="b", add=TRUE, col="blue")
  lines(lead_list, rmse, type="b", add=TRUE, col="red")
  lines(lead_list, rmse_oerr, type="b", add=TRUE, col="orange")
  lines(lead_list, spread_oerr, type="b", add=TRUE, col="black")
  lines(lead_list, spread_plus_oerr, type="b", add=TRUE, col="gray")

  legend("topleft", inset=0.02,
    legend=c("Spread", "RMSE", "RMSE_oerr", "Spread_oerr", "Spread_plus_oerr"),
    col=c("blue", "red", "orange", "black", "gray"),
    lty=1, cex=1)

  return
}

########################################################################
#
# Process the input data.
#
########################################################################

# Open up the output device
cat("Writing:", out_file, "\n")
pdf(out_file, height=8.5, width=11, useDingbats=FALSE)

# Process each line type
for(i in 1:length(line_types)) {

  data = read_line_type(line_types[i]);

  # Check for no data found
  if(length(data) == 0) {
    cat("WARNING: No input data for line type ",
        line_types[i], "!\n", sep='')
    next
  }

  # Convert date/time columns to date/time objects
  data$FCST_VALID_BEG <- as.POSIXct(strptime(data$FCST_VALID_BEG,
                                    format="%Y%m%d_%H%M%S"))
  data$FCST_VALID_END <- as.POSIXct(strptime(data$FCST_VALID_END,
                                    format="%Y%m%d_%H%M%S"))
  data$OBS_VALID_BEG  <- as.POSIXct(strptime(data$OBS_VALID_BEG,
                                    format="%Y%m%d_%H%M%S"))
  data$OBS_VALID_END  <- as.POSIXct(strptime(data$OBS_VALID_END,
                                    format="%Y%m%d_%H%M%S"))

  # Construct an index
  data$index <- paste(data$MODEL, data$DESC,
                      data$FCST_VAR, data$FCST_LEV,
                      data$OBS_VAR, data$OBS_LEV,
                      data$OBTYPE, data$VX_MASK,
                      data$INTERP_MTHD, data$INTERP_PNTS,
                      data$ALPHA,
                      sep='_')

  # Build a list of cases
  case_list <- unique(data$index)

  # Create plot for each case
  for(j in 1:length(case_list)) {

    # Get the subset for this case
    ind       <- data$index == case_list[j]

    main_info <- paste(data$MODEL[ind][1], ": ", data$FCST_VAR[ind][1],
                       " at ", data$FCST_LEV[ind][1], sep='')
    case_info <- paste(data$OBTYPE[ind][1],
                       ", ", data$VX_MASK[ind][1],
                       ", ", data$INTERP_MTHD[ind][1],
                       "(", data$INTERP_PNTS[ind][1], ")",
                       ",", data$ALPHA[ind][1], sep='')

    if(line_types[i] == "RHIST") {
      plot_rhist(data[ind,], main_info, case_info)
    }
    else if(line_types[i] == "RELP") {
      plot_relp(data[ind,], main_info, case_info)
    }
    else if(line_types[i] == "ECNT") {
      plot_ecnt_spread_skill(data[ind,], main_info, case_info)
    }
    else {
      cat("WARNING! Unsupported line type:", line_types[i], "\n")
    }

  } # end for j
} # end for i

# Finished with the plots
dev.off()

# List the completed output file
cat("Finished:", out_file, "\n")

# Save the R working environment
if(save == TRUE) {
  cat("Saving image...\n")
  save.image()
}
