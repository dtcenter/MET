########################################################################
##
##   Name: plot_tcmpr.R
##
##   Description:
##      This script should be called via an Rscript command to plot
##      tropical cyclone matched pair lines (TCMPR line type) from the
##      MET tc_pairs tool.  This script can read the output of tc_pairs
##      with a ".tcst" suffix.
##
##   Usage:
##      Rscript plot_tcmpr.R
##         -lookin tcst_file_list
##         [-config path]
##         [-outdir path]
##         [-prefix string]
##         [-filter options]
##         [-title string]
##         [-subtitle string]
##         [-ylab string]
##         [-ylim min,max]
##         [-dep list]
##         [-series string [list]]
##         [-lead list]
##         [-plot list]
##         [-rp_thresh string]
##         [-no_ee]
##         [-save_data path]
##         [-save]
##
##   Arguments:
##      "-lookin"    is a list of files with TCMPR lines to be used.
##      "-config"    is a plotting configuration file.
##      "-outdir"    is the output directory.
##      "-prefix"    is the output file name prefix.
##      "-title"     overrides the default plot title.
##      "-subtitle"  overrides the default plot subtitle.
##      "-ylab"      overrides the default plot y-axis label.
##      "-ylim"      is the bounds for plotting the Y-axis.
##      "-filter"    is a list of filtering options for tc_stat.
##      "-dep"       is a comma-separated list of dependent variable
##                   columns to plot.
##      "-series"    is the column whose unique values define the
##                   series on the plot, optionally followed by a
##                   comma-separated list of values including:
##                   ALL, OTHER, and colon-separated groups.
##      "-lead"      is a list of lead times (h) to be plotted.
##      "-plot"      is a comma-separated list of plot types to create:
##                   BOXPLOT, SCATTER, MEAN, MEDIAN, RELPERF, RANK
##      "-rp_thresh" is a comma-separated list of thresholds to specify
##                   meaningful differences for the relative performance
##                   plot.
##      "-no_ee"     to disable event equalization.
##      "-no_ci"     to disable confidence intervals.
##      "-save_data" to save the filtered track data to a file instead
##                   of deleting it.
##      "-save"      to call save.image().
##
##   Details:
##
##   Examples:
##
##   Author:
##      John Halley Gotway (johnhg@ucar.edu), NCAR-RAL-DTC
##      09/01/2012
##
########################################################################

library(boot)

# Check that the MET_BASE environment variable is set
MET_BASE = Sys.getenv("MET_BASE", unset=NA);
if(is.na(MET_BASE)) {
  cat("ERROR: The \"MET_BASE\" environment variable must be set.\n")
  quit(status=1)
}

source(paste(MET_BASE, "/scripts/Rscripts/include/plot_tcmpr_util.R", sep=''))
source(paste(MET_BASE, "/scripts/Rscripts/include/Compute_STDerr.R", sep=''))
source(paste(MET_BASE, "/scripts/Rscripts/include/plot_tcmpr_config_default.R", sep=''));

# Read the TCMPR column information from a data file.
column_info = read.table(
  paste(MET_BASE, "/scripts/Rscripts/include/plot_tcmpr_hdr.dat", sep=''),
  header=TRUE, row.names=1)

# TO DO LIST:
# - In what cases should we be using the Compute_STDerr functions -
#   does it have to be a time series (Tressa)?
# - Need to clarify the zval bonferroni adjusement for multiple models.
# - What about percentiles for the rp_thresh argument?
# - The random assignment of ranks could lead to bad plotting limits
#   from get_yrange for the rank plot.
# - When computing a difference, should we make sure that all of the
#   lines being differenced have exactly the same header data?
# - Figure out what information should be written to a log file.
# - Add -v verbosity for logging?
# - Move legend below X-axis?
# - New plot types from Eric: windrose and minimum spanning tree

########################################################################
#
# Usage statement.
#
########################################################################

usage = function() {
  cat("\nUsage: plot_tcmpr.R\n")
  cat("        -lookin tcst_file_list\n")
  cat("        [-config path]\n")
  cat("        [-outdir path]\n")
  cat("        [-prefix string]\n")
  cat("        [-title string]\n")
  cat("        [-subtitle string]\n")
  cat("        [-ylab string]\n")
  cat("        [-ylim min,max]\n")
  cat("        [-filter options]\n")
  cat("        [-dep list]\n")
  cat("        [-series string [list]]\n")
  cat("        [-lead list]\n")
  cat("        [-plot list]\n")
  cat("        [-rp_thresh string]\n")
  cat("        [-no_ee]\n")
  cat("        [-no_ci]\n")
  cat("        [-save_data]\n")
  cat("        [-save]\n")
  cat("        where \"-lookin\"    is a list of files with TCMPR lines to be used.\n")
  cat("              \"-config\"    is a plotting configuration file.\n")
  cat("              \"-outdir\"    is the output directory.\n")
  cat("              \"-prefix\"    is the output file name prefix.\n")
  cat("              \"-title\"     overrides the default plot title.\n")
  cat("              \"-subtitle\"  overrides the default plot subtitle.\n")
  cat("              \"-ylab\"      overrides the default plot y-axis label.\n")
  cat("              \"-ylim\"      is the min,max bounds for plotting the Y-axis.\n")
  cat("              \"-filter\"    is a list of filtering options for the tc_stat tool.\n")
  cat("              \"-dep\"       is a comma-separated list of dependent variable columns to plot.\n")
  cat("              \"-series\"    is the column whose unique values define the series on the plot,\n")
  cat("                           optionally followed by a comma-separated list of values, including:\n")
  cat("                           ALL, OTHER, and colon-separated groups.\n")
  cat("              \"-lead\"      is a comma-separted list of lead times (h) to be plotted.\n")
  cat("              \"-plot\"      is a comma-separated list of plot types to create:\n")
  cat("                           BOXPLOT, SCATTER, MEAN, MEDIAN, RELPERF, RANK\n")
  cat("              \"-rp_thresh\" is a comma-separated list of thresholds to specify\n")
  cat("                           meaningful differences for the relative performance plot.\n")
  cat("              \"-no_ee\"     to disable event equalization.\n")
  cat("              \"-no_ci\"     to disable confidence intervals.\n")
  cat("              \"-save_data\" to save the filtered track data to a file instead of deleting it.\n")
  cat("              \"-save\"      to call save.image().\n\n")
}

########################################################################
#
# Constants.
#
########################################################################

# Path to the tc_stat tool
tc_stat = "${MET_BASE}/bin/tc_stat"

# Strings used to select the plots to be created
boxplot_str = "BOXPLOT"
scatter_str = "SCATTER"
mean_str    = "MEAN"
median_str  = "MEDIAN"
relperf_str = "RELPERF"
rank_str    = "RANK"

########################################################################
#
# Handle the arguments.
#
########################################################################

# Retreive the arguments
args = commandArgs(TRUE)

# Check the number of arguments
if(length(args) < 2) {
  usage()
  quit()
}

# Process the -config option first
for(i in 1:length(args)) {
  if(args[i] == "-config") {
    cat("Reading plot configuration file:", args[i+1], "\n");
    source(args[i+1]);
  }
} # end for i

# Parse optional arguments
i=1
while(i <= length(args)) {

  if(args[i] == "-lookin") {
    while(i+1 <= length(args) & substring(args[i+1], 1, 1) != '-') {
       file_list = c(file_list, args[i+1])
       i=i+1
    }
  } else if(args[i] == "-config") {
    i=i+1
  } else if(args[i] == "-outdir") {
    outdir = args[i+1]
    i=i+1
  } else if(args[i] == "-prefix") {
    prefix = args[i+1]
    i=i+1
  } else if(args[i] == "-title") {
    title_str = args[i+1]
    i=i+1
  } else if(args[i] == "-subtitle") {
    subtitle_str = args[i+1]
    i=i+1
  } else if(args[i] == "-ylab") {
    ylab_str = args[i+1]
    i=i+1
  } else if(args[i] == "-ylim") {
    ymin = as.numeric(unlist(strsplit(args[i+1], ','))[1])
    ymax = as.numeric(unlist(strsplit(args[i+1], ','))[2])
    i=i+1
  } else if(args[i] == "-filter") {
    filter_opts = args[i+1]
    i=i+1
  } else if(args[i] == "-dep") {
    dep_list = unlist(strsplit(args[i+1], ','))
    i=i+1
  } else if(args[i] == "-series") {
    series = args[i+1]
    i=i+1
    
    # Check for optional list of series values
    if(i+1 <= length(args) &
       substring(args[i+1], 1, 1) != '-') {
      series_list = unlist(strsplit(args[i+1], ','))
      i=i+1
    }

  } else if(args[i] == "-lead") {
    lead_list = as.numeric(unlist(strsplit(args[i+1], ',')))
    i=i+1
  } else if(args[i] == "-plot") {
    plot_list = unlist(strsplit(args[i+1], ','))
    plot_list = toupper(plot_list)
    i=i+1
  } else if(args[i] == "-rp_thresh") {
    rp_list = unlist(strsplit(args[i+1], ','))
    i=i+1
  } else if(args[i] == "-no_ee") {
    event_equal = FALSE
  } else if(args[i] == "-no_ci") {
    ci_flag = FALSE
  } else if(args[i] == "-save_data") {
    save_data = args[i+1]
    i=i+1
  } else if(args[i] == "-save") {
    save = TRUE
  } else {
    cat("ERROR: Unrecognized command line argument:", args[i], "\n")
    usage()
    quit()
  }

  # Increment count
  i=i+1

} # end while

# Check the relative performance threshold setting
if(length(rp_list) == 1) rp_list = rep(rp_list, length(lead_list));
if(length(rp_list) != length(lead_list)) {
  cat("ERROR: The number of relative performance thresholds specified",
      "must either be 1 or match the number of lead times.\n");
  quit(status=1);
}

########################################################################
#
# Run a tc_stat filter job to subset the data.
#
########################################################################

# Add the event equalization option
if(event_equal) {
  filter_opts = paste(filter_opts, "-event_equal true")
}

# Build tc_stat command
run_cmd = paste(tc_stat,
                paste("-lookin", unlist(strsplit(file_list, ',')), collapse=" "),
                "-job filter -dump_row", tcst_tmp_file, filter_opts,
                "-v 3")

# Run the tc_stat command and check the return status
cat("CALLING: ", run_cmd, "\n")
status = system(run_cmd)
if(status != 0) {
  cat("ERROR: Bad return value ", status , "\n")
  quit(status=status)
}

# Read the data
tcst = read.table(tcst_tmp_file, header=TRUE)

# Sort the data by INIT and VALID columns
tcst = tcst[with(tcst, order(INIT,VALID)),]
  
# Dispose of the temporary file by either saving or deleting it
if(nchar(save_data) > 0) {
   run_cmd = paste("mv -f", tcst_tmp_file, save_data)
} else {
   run_cmd = paste("rm -f", tcst_tmp_file)
}
cat("CALLING: ", run_cmd, "\n")
status = system(run_cmd)

########################################################################
#
# Convert times in the data.
#
########################################################################

tcst$INIT_TIME  = as.POSIXct(strptime(tcst$INIT,
                                      format="%Y%m%d_%H%M%s"))
tcst$VALID_TIME = as.POSIXct(strptime(tcst$VALID,
                                      format="%Y%m%d_%H%M%s"))
tcst$LEAD_HR    = tcst$LEAD/10000

########################################################################
#
# Print information about the dataset.
#
########################################################################

info_list = c("AMODEL",     "BMODEL", "BASIN", "CYCLONE",
              "STORM_NAME", "LEAD",   "LEVEL", "WATCH_WARN")

for(i in 1:length(info_list)) {

  uniq_list = unique(tcst[,info_list[i]])

  cat("Found ", length(uniq_list), " unique entries for ", info_list[i], ": ",
      paste(uniq_list, collapse=", "), "\n", sep='')

  # Check for a single BDECK model
  if(info_list[i] == "BMODEL" & length(uniq_list) != 1) {
    cat("ERROR: Must have exactly 1 BDECK model name.  ",
        "Try setting \"-bmodel name\" in the \"-filter\" option.\n")
    quit(status=1)
  }
}

########################################################################
#
# Process the series information.
#
########################################################################

# Get the unique series entries from the data
series_uniq = unique(as.character(tcst[,series]))

# List unique series entries
cat("Found", length(series_uniq), "unique value(s) for the", series,
    "series:", paste(series_uniq, collapse=", "), "\n")

# Store the series list if not specified on the command line
if(length(series_list) == 0) {
  series_list = series_uniq
}

# Store the number of series to be plotted
n_series = length(series_list)

# Check for special series values "ALL" and "OTHER" and construct a
# list of values that correspond to each series entry
series_plot = list()
diff_flag = rep(FALSE, n_series)
for(i in 1:n_series) {

  # Handle NA
  if(is.na(series_list[i])) {
     series_plot[[i]] = NA;
  }
  # Handle ALL: all unique entries
  else if(series_list[i] == "ALL") {
     series_plot[[i]] = series_uniq
  }
  # Handle OTHER: all unique entries not requested in the list
  else if(series_list[i] == "OTHER") {
     series_plot[[i]] = series_uniq[!(series_uniq%in%series_list)]
  }
  # Handle differences
  else if(length(unlist(strsplit(series_list[i], "-"))) > 1) {

    # Check that event equalization is turned on
    if(!event_equal) {
      cat("ERROR: Event equalization must be on for differences.\n")
      quit(status=1)
    }

    # Store the entries for the difference
    series_plot[[i]] = unlist(strsplit(series_list[i], "-"))

    # Enable the difference flag
    diff_flag[i] = TRUE;
  }
  # Handle the rest, assuming it's a colon separated list
  else {
     series_plot[[i]] = unlist(strsplit(series_list[i], ':'))
  }
}

# Store the number of series to plot
cat("Plotting", n_series, "value(s) for the", series, "series:",
    paste(series_plot, collapse=", "), "\n")

########################################################################
#
# Loop over the list of columns and do plots.
#
########################################################################

for(i in 1:length(dep_list)) {

  cat("Processing column:", dep_list[i], "\n")

  # Get the data to be plotted
  col       = get_dep_column(dep_list[i])
  tcst$PLOT = col$val

  # Remove special characters from output file name
  out_file_dep = sub('[)]', '', sub('[(]', '_', dep_list[i]))

  # Set plotting strings
  plot_sub  = ifelse(length(subtitle_str) == 0,
                     paste("FILTER:" , filter_opts),
                     subtitle_str);
  plot_ylab = ifelse(length(ylab_str) == 0,
                     paste(dep_list[i], " (", col$units,")", sep=''),
                     ylab_str);

  # PLOT: Create time series of boxplots.
  if(boxplot_str%in%plot_list == TRUE) {

    # Set plotting strings
    plot_title = ifelse(length(title_str) == 0,
                        paste("Boxplots of\n", col$desc, "\nby",
                              column_info[series, "DESCRIPTION"]),
                        title_str);
  
    # Build output file name
    out_file = paste(outdir, "/", prefix, out_file_dep,
                     "_bplot.", img_ext, sep="")

    # Do the boxplot
    plot_time_series(dep_list[i], boxplot_str,
                     plot_title, plot_sub, plot_ylab)
  }
  
  # PLOT: Create time series of scatter plots.
  if(scatter_str%in%plot_list == TRUE) {

    # Set plotting strings
    plot_title = ifelse(length(title_str) == 0,
                        paste("Scatter Plots of\n", col$desc, "\nby",
                              column_info[series, "DESCRIPTION"]),
                        title_str);

    # Build output file name
    out_file = paste(outdir, "/", prefix, out_file_dep,
                     "_scatter.", img_ext, sep="")

    # Do the scatter plot
    plot_time_series(dep_list[i], scatter_str,
                     plot_title, plot_sub, plot_ylab)
  }

  # PLOT: Create time series of means.
  if(mean_str%in%plot_list == TRUE) {

    # Set plotting strings
    plot_title = ifelse(length(title_str) == 0,
                        paste("Mean of\n", col$desc, "\nby",
                              column_info[series, "DESCRIPTION"]),
                        title_str);

    # Build output file name
    out_file = paste(outdir, "/", prefix, out_file_dep,
                     "_mean.", img_ext, sep="")

    # Do the mean plot
    plot_time_series(dep_list[i], mean_str,
                     plot_title, plot_sub, plot_ylab)
  }

  # PLOT: Create time series of medians.
  if(median_str%in%plot_list == TRUE) {

    # Set plotting strings
    plot_title = ifelse(length(title_str) == 0,
                        paste("Median of\n", col$desc, "\nby",
                              column_info[series, "DESCRIPTION"]),
                        title_str);

    # Build output file name
    out_file = paste(outdir, "/", prefix, out_file_dep,
                     "_median.", img_ext, sep="")

    # Do the median plot
    plot_time_series(dep_list[i], median_str,
                     plot_title, plot_sub, plot_ylab)
  }

  # PLOT: Create time series of relative performance.
  if(relperf_str%in%plot_list == TRUE) {

    # Set plotting strings
    if(length(title_str) > 0) {
      plot_title = title_str;
    }
    else if(length(unique(rp_list)) == 1) {
      plot_title = paste("Relative Performance of\n", col$desc,
                         "\nDifference", rp_list[1], col$units, "by",
                         column_info[series, "DESCRIPTION"]);
    }
    else {
      plot_title = paste("Relative Performance of\n", col$desc,
                         "\nDifference by",
                         column_info[series, "DESCRIPTION"]);
    }

    plot_ylab  = ifelse(length(ylab_str) == 0,
                        "Percent of Cases",
                        ylab_str);

    # Build output file name
    out_file = paste(outdir, "/", prefix, out_file_dep,
                     "_rel_perf.", img_ext, sep="")

    # Do the relative performance plot
    plot_time_series(dep_list[i], relperf_str,
                     plot_title, plot_sub, plot_ylab)
  }

  # PLOT: Create time series of ranks for the first model.
  if(rank_str%in%plot_list == TRUE) {

    # Set plotting strings
    plot_title = ifelse(length(title_str) == 0,
                        paste(series_list[1],
                              column_info[series, "DESCRIPTION"], "\n",
                              col$desc, "\nRank Frequency"),
                        title_str);
    plot_ylab  = ifelse(length(ylab_str) == 0,
                        "Percent of Cases",
                        ylab_str);

    # Build output file name
    out_file = paste(outdir, "/", prefix, out_file_dep,
                     "_rank.", img_ext, sep="")

    # Do the relative performance plot
    plot_time_series(dep_list[i], rank_str,
                     plot_title, plot_sub, plot_ylab)
  }
  
} # end for i

########################################################################
#
# Clean up.
#
########################################################################

# Optionally, save all of the data to an .RData file
if(save == TRUE) save.image()
