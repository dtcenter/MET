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
##         [-outdir path]
##         [-prefix string]
##         [-filter options]
##         [-title string]
##         [-subtitle string]
##         [-ylab string]
##         [-dep list]
##         [-series string [list]]
##         [-lead list]
##         [-plot list]
##         [-baseline model]
##         [-ylim min,max]
##         [-no_ee]
##         [-save]
##
##   Arguments:
##      "-lookin"   is a comma-separated list of files with TCMPR lines to be used.
##      "-outdir"   is the output directory.
##      "-prefix"   is the output file name prefix.
##      "-title"    overrides the default plot title.
##      "-subtitle" overrides the default plot subtitle.
##      "-ylab"     overrides the default plot y-axis label.
##      "-filter"   is a list of filtering options for tc_stat.
##      "-dep"      is a comma-separated list of dependent variable columns to plot.
##      "-series"   is the column whose unique values define the series on the plot,
##                  optionally followed by a comma-separated list of values including
##                  ALL, OTHER, and colon-separated groups.
##      "-lead"     is a list of lead times (h) to be plotted.
##      "-plot"     is a comma-separated list of plot types to create, including
##                  BOXPLOT, SCATTER, MEAN, MEDIAN, SKILL, RANK
##      "-baseline" is the baseline model for skill scores.
##      "-ylim"     is the bounds for plotting the Y-axis.
##      "-no_ee"    to disable event equalization.
##      "-save"     to call save.image().
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

source("plot_tcmpr_util.R");

# Read the TCMPR column information from a data file.
column_info = read.table("plot_tcmpr_hdr.dat", header=TRUE, row.names=1)

# JHG - TO DO LIST:
# move customizable settings into a "config" file sort of thing
# add -v verbosity for logging
# New plot types from Eric: windrose and minimum spanning tree
# Do mean and median plots
# Do skill plots
# Do rank plots

# JHG - DONE
# add title, subtitle, ylab options to override defaults
# why does -series level not work?
# check that MET_BASE is set
# Write out ylim plotting values that were used
# plot pairwise difference of series entries (e.g. -series AMODEL "GFSI-BCLP,GFSI-OFCL")
#    if differene, make sure that event_equal is true, otherwise error.
# Do boxplots
# Do scatter plots

  # Create time series of median and mean for raw values w/o CIs.
  # Create time series of median and mean for raw values w CIs.
  # Create time series of boxplots for absolute values.
  # Create time series of median and mean for absolute values w/o CIs.
  # Create time series of median and mean for absolute values w CIs.
  # Create time series of median and mean skill scores for abs values.
  # Create time series of percent cases over set threshold.
  # Create time series of boxplots for the difference (mod1-mod2) of the absolute values.
  # Create time series of median and mean for the difference (mod1-mod2) of the absolute values without CIs.
  # Create time series of median and mean for the difference (mod1-mod2) of the absolute values with CIs.

  # Compute ranks


########################################################################
#
# Usage statement.
#
########################################################################

usage = function() {
  cat("\nUsage: plot_tc_mpr.R\n")
  cat("        -lookin tcst_file_list\n")
  cat("        [-outdir path]\n")
  cat("        [-prefix string]\n")
  cat("        [-title string]\n")
  cat("        [-subtitle string]\n")
  cat("        [-ylab string]\n")
  cat("        [-filter options]\n")
  cat("        [-dep list]\n")
  cat("        [-series string [list]]\n")
  cat("        [-lead list]\n")
  cat("        [-plot list]\n")
  cat("        [-baseline model]\n")
  cat("        [-ylim min,max]\n")
  cat("        [-no_ee]\n")
  cat("        [-save]\n")
  cat("        where \"-lookin\"   is a comma-separated list of files with TCMPR lines to be used.\n")
  cat("              \"-outdir\"   is the output directory.\n")
  cat("              \"-prefix\"   is the output file name prefix.\n")
  cat("              \"-title\"    overrides the default plot title.\n")
  cat("              \"-subtitle\" overrides the default plot subtitle.\n")
  cat("              \"-ylab\"     overrides the default plot y-axis label.\n")
  cat("              \"-filter\"   is a list of filtering options for the tc_stat tool.\n")
  cat("              \"-dep\"      is a comma-separated list of dependent variable columns to plot.\n")
  cat("              \"-series\"   is the column whose unique values define the series on the plot,\n")
  cat("                          optionally followed by a comma-separated list of values including\n")
  cat("                          ALL, OTHER, and colon-separated groups.\n")
  cat("              \"-lead\"     is a comma-separted list of lead times (h) to be plotted.\n")
  cat("              \"-plot\"     is a comma-separated list of plot types to create, including\n")
  cat("                          BOXPLOT, SCATTER, MEAN, MEDIAN, SKILL, RANK\n")
  cat("              \"-baseline\" is the baseline model for skill scores.\n")
  cat("              \"-ylim\"     is the min,max bounds for plotting the Y-axis.\n")
  cat("              \"-no_ee\"    to disable event equalization.\n")
  cat("              \"-save\"     to call save.image().\n\n")
}

########################################################################
#
# Constants.
#
########################################################################

# Path to the tc_stat tool
tc_stat = "${MET_BASE}/bin/tc_stat"
  
# Build unique tc_stat output file name
tcst_tmp_file = paste("/tmp/plot_tcmpr_", Sys.getpid(), ".tcst", sep='')

# Output image file format
img_fmt  = "png256"
img_ext  = "png"
img_hgt  = 8.5
img_wdth = 11.0
img_res  = 300

# Colors
color_list = c("red", "green", "blue", "purple", "orange")

# Strings used to select the plots to be created
boxplot_str = "BOXPLOT"
scatter_str = "SCATTER"
mean_str    = "MEAN"
median_str  = "MEDIAN"
skill_str   = "SKILL"
rank_str    = "RANK"

# Minimum number of values for boxplots.  Otherwise, do a scatter plot.
n_min=11

# Plotting offset value
horz_offset = 2.00
vert_offset = 1.25
ci_offset   = 0.10

# Alpha and z-value
alpha           = 0.05
zval            = qnorm(1 - (alpha/2));
zval_bonferroni = (zval + zval/sqrt(2))/2;

# Number of boostrap replicates to use
n_boot_rep = 500

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

# Initialize options for command-line arguments
file_list    = ""
outdir       = "."
prefix       = ""
title_str    = c()
subtitle_str = c()
ylab_str     = c()
filter_opts  = ""
dep_list     = c("TK_ERR")
series       = "AMODEL"
series_list  = c()
lead_list    = c( 0,  6, 12, 18, 24, 30, 36, 42, 48, 54, 60,
                 66, 72, 78, 84, 90, 96, 102, 108, 114, 120)
plot_list    = c(boxplot_str)
baseline     = NA
ymin = ymax  = NA
event_equal  = TRUE
save         = FALSE

# Parse optional arguments
i=1
while(i <= length(args)) {

  if(args[i] == "-lookin") {
    file_list = unlist(strsplit(args[i+1], ','))
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
    i=i+1
  } else if(args[i] == "-baseline") {
    baseline = args[i+1]
    i=i+1
  } else if(args[i] == "-ylim") {
    ymin = as.numeric(unlist(strsplit(args[i+1], ','))[1])
    ymax = as.numeric(unlist(strsplit(args[i+1], ','))[2])
    i=i+1
  } else if(args[i] == "-no_ee") {
    event_equal = FALSE
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

########################################################################
#
# Run a tc_stat filter job to subset the data.
#
########################################################################

# Check that the MET_BASE environment variable is set
if(is.na(Sys.getenv("MET_BASE", unset=NA))) {
  cat("ERROR: The \"MET_BASE\" environment variable must be set.\n")
  quit(status=1)
}

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

# Delete the temporary file
run_cmd = paste("rm -f", tcst_tmp_file)
cat("CALLING: ", run_cmd, "\n")
status = system(run_cmd)

########################################################################
#
# Build strings based on the filtering options chosen.
#
########################################################################

# JHG, these strings aren't used.  Do I really need them?

# Land vs water
if(length(grep("-water_only true", filter_opts, ignore.case=TRUE)) > 0) {
  abbr_verification_type="WATERONLY"
  title_verification_type="(Water Only)"
} else {
  abbr_verification_type="LANDANDWATER"
  title_verification_type="(Land and Water)"
}
cat("Selected Land/Water:", abbr_verification_type, "\n")

# Basin
if(length(grep("-basin al", filter_opts, ignore.case=TRUE)) > 0) {
  abbr_basin="ALbasin"
} else if(length(grep("-basin ep", filter_opts, ignore.case=TRUE)) > 0) {
  abbr_basin="EPbasin"
} else {
  abbr_basin="ALLbasins"
}
cat("Selected Basin:", abbr_basin, "\n")

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

info_list = c("AMODEL", "BMODEL", "BASIN", "CYCLONE", "STORM_NAME", "LEAD", "LEVEL", "WATCH_WARN")

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
  
  # Initialize plotting strings
  plot_title = title_str
  plot_sub   = subtitle_str
  plot_ylab  = ylab_str

  # Set plotting strings
  if(length(subtitle_str) == 0) {
    plot_sub = paste("FILTER:" , filter_opts)
  }
  if(length(ylab_str) == 0) {
    plot_ylab = paste(dep_list[i], " (", col$units,")", sep='')
  }

  # PLOT: Create time series of boxplots.
  if(boxplot_str%in%plot_list == TRUE) {

    # Set plotting strings
    if(length(title_str) == 0) {
      plot_title = paste("Boxplots of", col$desc, "by",
                         column_info[series, "DESCRIPTION"])
    }
    else {
      plot_title = title_str
    }
  
    # Build output file name
    out_file = paste(outdir, "/", prefix, dep_list[i],
                     "_bplot.", img_ext, sep="")

    # Do the boxplot
    plot_time_series(dep_list[i], boxplot_str,
                     plot_title, plot_sub, plot_ylab)
  }
  
  # PLOT: Create time series of scatter plots.
  if(scatter_str%in%plot_list == TRUE) {

    # Set plotting strings
    if(length(title_str) == 0) {
      plot_title = paste("Scatter plots of", col$desc, "by",
                         column_info[series, "DESCRIPTION"])
    }
    else {
      plot_title = title_str
    }

    # Build output file name
    out_file = paste(outdir, "/", prefix, dep_list[i],
                     "_scatter.", img_ext, sep="")

    # Do the scatter plot
    plot_time_series(dep_list[i], scatter_str,
                     plot_title, plot_sub, plot_ylab)
  }

  # PLOT: Create time series of means.
  if(mean_str%in%plot_list == TRUE) {

    # Set plotting strings
    if(length(title_str) == 0) {
      plot_title = paste("Mean of", col$desc, "by",
                         column_info[series, "DESCRIPTION"])
    }
    else {
      plot_title = title_str
    }

    # Build output file name
    out_file = paste(outdir, "/", prefix, dep_list[i],
                     "_mean.", img_ext, sep="")

    # Do the mean plot
    plot_time_series(dep_list[i], mean_str,
                     plot_title, plot_sub, plot_ylab)
  }

  # PLOT: Create time series of medians.
  if(median_str%in%plot_list == TRUE) {

    # Set plotting strings
    if(length(title_str) == 0) {
      plot_title = paste("Median of", col$desc, "by",
                         column_info[series, "DESCRIPTION"])
    }
    else {
      plot_title = title_str
    }

    # Build output file name
    out_file = paste(outdir, "/", prefix, dep_list[i],
                     "_median.", img_ext, sep="")

    # Do the median plot
    plot_time_series(dep_list[i], median_str,
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
