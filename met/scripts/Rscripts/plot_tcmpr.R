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
##         [-tcst path]
##         [-title string]
##         [-subtitle string]
##         [-xlab string]
##         [-ylab string]
##         [-xlim min,max]
##         [-ylim min,max]
##         [-dep list]
##         [-scatter_x list]
##         [-scatter_y list]
##         [-skill_ref string ]
##         [-series string [list]]
##         [-series_ci list]
##         [-legend list]
##         [-lead list]
##         [-plot list]
##         [-rp_diff string]
##         [-demo_yr year]
##         [-hfip_bsln string]
##         [-footnote_flag]
##         [-no_ee]
##         [-no_log]
##         [-plot_config path]
##         [-save_data path]
##         [-save]
##
##   Arguments:
##      "-lookin"        is a list of files with TCMPR lines to be used.
##      "-config"        is a plotting configuration file.
##      "-outdir"        is the output directory.
##      "-prefix"        is the output file name prefix.
##      "-title"         overrides the default plot title.
##      "-subtitle"      overrides the default plot subtitle.
##      "-xlab"          overrides the default plot x-axis label.
##      "-ylab"          overrides the default plot y-axis label.
##      "-xlim"          is the bounds for plotting the X-axis.
##      "-ylim"          is the bounds for plotting the Y-axis.
##      "-filter"        is a list of filtering options for tc_stat.
##      "-tcst"          is a tcst data file to be used instead of running
##                       the tc_stat tool.
##      "-dep"           is a comma-separated list of dependent variable
##                       columns to plot. (y-axis for time series plots)
##      "-scatter_x"     is a comma-separated list of data on the x-axis
##      "-scatter_y"     is a comma-separated list of data on the y-axis
##      "-skill_ref"     is the identifier for the skill score reference
##      "-series"        is the column whose unique values define the
##                       series on the plot, optionally followed by a
##                       comma-separated list of values including:
##                       ALL, OTHER, and colon-separated groups.
##      "-series_ci"     is a list of true/false for confidence intervals
##      "-legend"        is a comma-separated list of strings that should
##                       be used for legend.
##      "-lead"          is a list of lead times (h) to be plotted.
##      "-plot"          is a comma-separated list of plot types to create:
##                       BOXPLOT, POINT, MEAN, MEDIAN, RELPERF, RANK, SCATTER
##                       SKILL_MN, SKILL_MD
##      "-rp_diff"       is a comma-separated list of thresholds to specify
##                       meaningful differences for the relative performance
##                       plot.
##      "-demo_yr"       is the demo year.
##      "-hfip_bsln"     is a string indicating whether to add the HFIP baseline 
##                       and which version (0, 5, 10 year goal).
##      "-footnote_flag" to disable footnote (date).
##      "-no_ee"         to disable event equalization.
##      "-no_log"        to disable log file generation.
##      "-plot_config"   to read model-specific plotting options from a
##                       configuration file.
##      "-save_data"     to save the filtered track data to a file instead
##                       of deleting it.
##      "-save"          to call save.image().
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

library(boot);

# Check that the MET_INSTALL_DIR environment variable is set
MET_INSTALL_DIR = Sys.getenv("MET_INSTALL_DIR", unset=NA);
if(is.na(MET_INSTALL_DIR)) {
  cat("ERROR: The \"MET_INSTALL_DIR\" environment variable must be set.\n");
  quit(status=1);
}

# Expand environment variables
MET_INSTALL_DIR = system(paste("echo", MET_INSTALL_DIR), intern=TRUE);

# Source utilities
RSCRIPT_INC_DIR = paste(MET_INSTALL_DIR, "/share/met/Rscripts/include", sep='');
source(paste(RSCRIPT_INC_DIR, "/plot_tcmpr_util.R", sep=''));
source(paste(RSCRIPT_INC_DIR, "/plot_tcmpr_config_default.R", sep=''));
source(paste(RSCRIPT_INC_DIR, "/Compute_STDerr.R", sep=''));

# Read the TCMPR column information from a data file.
column_info = read.table(
  paste(RSCRIPT_INC_DIR, "/plot_tcmpr_hdr.dat", sep=''),
  header=TRUE, row.names=1);
  
# Read the HFIP baseline information from a data file.
baseline = read.table(paste(RSCRIPT_INC_DIR, "/hfip_baseline.dat", sep=''), header=TRUE)

########################################################################
#
# Usage statement.
#
########################################################################

usage = function() {
  cat("\nUsage: plot_tcmpr.R\n");
  cat("        -lookin tcst_file_list\n");
  cat("        [-config path]\n");
  cat("        [-outdir path]\n");
  cat("        [-prefix string]\n");
  cat("        [-title string]\n");
  cat("        [-subtitle string]\n");
  cat("        [-xlab string]\n");
  cat("        [-ylab string]\n");
  cat("        [-xlim min,max]\n");
  cat("        [-ylim min,max]\n");
  cat("        [-filter options]\n");
  cat("        [-tcst path]\n");
  cat("        [-dep list]\n");
  cat("        [-scatter_x list]\n");
  cat("        [-scatter_y list]\n");
  cat("        [-skill_ref string]\n");
  cat("        [-series string [list]]\n");
  cat("        [-series_ci list]\n");
  cat("        [-legend list]\n");  
  cat("        [-lead list]\n");
  cat("        [-plot list]\n");
  cat("        [-rp_diff string]\n");
  cat("        [-demo_yr year]\n");
  cat("        [-hfip_bsln string]\n");
  cat("        [-footnote_flag]\n");
  cat("        [-no_ee]\n");
  cat("        [-no_log]\n");
  cat("        [-plot_config path]\n");
  cat("        [-save_data path]\n");
  cat("        [-save]\n");
  cat("        where \"-lookin\"        is a list of files with TCMPR lines to be used.\n");
  cat("              \"-config\"        is a plotting configuration file.\n");
  cat("              \"-outdir\"        is the output directory.\n");
  cat("              \"-prefix\"        is the output file name prefix.\n");
  cat("              \"-title\"         overrides the default plot title.\n");
  cat("              \"-subtitle\"      overrides the default plot subtitle.\n");
  cat("              \"-xlab\"          overrides the default plot x-axis label.\n");
  cat("              \"-ylab\"          overrides the default plot y-axis label.\n");
  cat("              \"-xlim\"          is the min,max bounds for plotting the X-axis.\n");
  cat("              \"-ylim\"          is the min,max bounds for plotting the Y-axis.\n");
  cat("              \"-filter\"        is a list of filtering options for the tc_stat tool.\n");
  cat("              \"-tcst\"          is a tcst data file to be used instead of running the tc_stat tool.\n");
  cat("              \"-dep\"           is a comma-separated list of dependent variable columns to plot.\n");
  cat("              \"-scatter_x\"     is a comma-separated list of x-axis variable columns to plot.\n");
  cat("              \"-scatter_y\"     is a comma-separated list of y-axis variable columns to plot.\n");
  cat("              \"-skill_ref\"     is the identifier for the skill score reference.\n");
  cat("              \"-series\"        is the column whose unique values define the series on the plot,\n");
  cat("                                 optionally followed by a comma-separated list of values, including:\n");
  cat("                                 ALL, OTHER, and colon-separated groups.\n");
  cat("              \"-series_ci\"     is a list of true/false for confidence intervals.\n");
  cat("                                 optionally followed by a comma-separated list of values, including:\n");
  cat("                                 ALL, OTHER, and colon-separated groups.\n");
  cat("              \"-legend\"        is a comma-separted list of strings to be used in the legend.\n");
  cat("              \"-lead\"          is a comma-separted list of lead times (h) to be plotted.\n");
  cat("              \"-plot\"          is a comma-separated list of plot types to create:\n");
  cat("                                 BOXPLOT, POINT, MEAN, MEDIAN, RELPERF, RANK, SKILL_MN, SKILL_MD\n");
  cat("              \"-rp_diff\"       is a comma-separated list of thresholds to specify\n");
  cat("                                 meaningful differences for the relative performance plot.\n");
  cat("              \"-demo_yr\"       is the demo year\n");
  cat("              \"-hfip_bsln\"     is a string indicating whether to add the HFIP baseline\n");
  cat("                                 and which version (no, 0, 5, 10 year goal)\n");
  cat("              \"-footnote_flag\" to disable footnote (date).\n");
  cat("              \"-no_ee\"         to disable event equalization.\n");
  cat("              \"-no_log\"        to disable log file generation.\n");
  cat("              \"-plot_config\"   to read model-specific plotting options from a configuration file.\n");
  cat("              \"-save_data\"     to save the filtered track data to a file instead of deleting it.\n");
  cat("              \"-save\"          to call save.image().\n\n");
}

########################################################################
#
# Constants.
#
########################################################################

# Path to the tc_stat tool
tc_stat = paste(MET_INSTALL_DIR, "/bin/tc_stat", sep='');

# Strings used to select the plots to be created
boxplot_str = "BOXPLOT";
point_str   = "POINT";
mean_str    = "MEAN";
median_str  = "MEDIAN";
relperf_str = "RELPERF";
rank_str    = "RANK";
scatter_str = "SCATTER";
skillmn_str = "SKILL_MN";
skillmd_str = "SKILL_MD";

########################################################################
#
# Handle the arguments.
#
########################################################################

# Retreive the arguments
args = commandArgs(TRUE);

# Check the number of arguments
if(length(args) < 2) {
  usage();
  quit();
}

# Process the -config option first
for(i in 1:length(args)) {
  if(args[i] == "-config") {
    cat("Reading plot configuration file:", args[i+1], "\n");
    source(args[i+1]);
  }
} # end for i

# Parse optional arguments
i=1;
while(i <= length(args)) {

  if(args[i] == "-lookin") {
    while(i+1 <= length(args) & substring(args[i+1], 1, 1) != '-') {
       file_list = c(file_list, args[i+1]);
       i=i+1;
    }
  } else if(args[i] == "-config") {
    i=i+1;
  } else if(args[i] == "-outdir") {
    outdir = args[i+1];
    i=i+1;
  } else if(args[i] == "-prefix") {
    prefix = args[i+1];
    i=i+1;
  } else if(args[i] == "-title") {
    title_str = eval(parse(text=paste("'", args[i+1], "'", sep='')));
    i=i+1;
  } else if(args[i] == "-subtitle") {
    subtitle_str = eval(parse(text=paste("'", args[i+1], "'", sep='')));
    i=i+1;
  } else if(args[i] == "-xlab") {
    xlab_str = eval(parse(text=paste("'", args[i+1], "'", sep='')));
    i=i+1;
  } else if(args[i] == "-ylab") {
    ylab_str = eval(parse(text=paste("'", args[i+1], "'", sep='')));
    i=i+1;
  } else if(args[i] == "-xlim") {
    xmin = as.numeric(unlist(strsplit(args[i+1], ','))[1]);
    xmax = as.numeric(unlist(strsplit(args[i+1], ','))[2]);
    i=i+1;
  } else if(args[i] == "-ylim") {
    ymin = as.numeric(unlist(strsplit(args[i+1], ','))[1]);
    ymax = as.numeric(unlist(strsplit(args[i+1], ','))[2]);
    i=i+1;
  } else if(args[i] == "-filter") {
    filter_opts = args[i+1];
    i=i+1;
  } else if(args[i] == "-tcst") {
    tcst_file = args[i+1];
    i=i+1;
  } else if(args[i] == "-dep") {
    dep_list = unlist(strsplit(args[i+1], ','));
    i=i+1;
  } else if(args[i] == "-scatter_x") {
    scatter_x_list = unlist(strsplit(args[i+1], ','));
    i=i+1;
  } else if(args[i] == "-scatter_y") {
    scatter_y_list = unlist(strsplit(args[i+1], ','));
    i=i+1;
  } else if(args[i] == "-skill_ref") {
    skill_ref = args[i+1];
    i=i+1;
  } else if(args[i] == "-series") {
    series = args[i+1];
    i=i+1;
    
    # Check for optional list of series values
    if(i+1 <= length(args) &
       substring(args[i+1], 1, 1) != '-') {
      series_list = unlist(strsplit(args[i+1], ','));
      i=i+1;
    }
  } else if(args[i] == "-series_ci") {
    series_ci = unlist(strsplit(args[i+1], ','));
    i=i+1;
  } else if(args[i] == "-legend") {
    legend_list = unlist(strsplit(args[i+1], ','));
    i=i+1;
  } else if(args[i] == "-lead") {
    lead_list = as.numeric(unlist(strsplit(args[i+1], ',')));
    i=i+1;
  } else if(args[i] == "-plot") {
    plot_list = unlist(strsplit(args[i+1], ','));
    plot_list = toupper(plot_list);
    i=i+1;
  } else if(args[i] == "-rp_diff") {
    rp_diff_list = unlist(strsplit(args[i+1], ','));
    i=i+1;
  } else if(args[i] == "-demo_yr") {
    demo_yr = args[i+1];
    i=i+1;
  } else if(args[i] == "-hfip_bsln") {
    hfip_bsln = unlist(strsplit(args[i+1], ','));
    i=i+1;
  } else if(args[i] == "-footnote_flag") {
    footnote_flag = FALSE;
  } else if(args[i] == "-no_ee") {
    event_equal = FALSE;
  } else if(args[i] == "-no_log") {
    log_flag = FALSE;
  } else if(args[i] == "-save_data") {
    save_data = args[i+1];
    i=i+1;
  } else if(args[i] == "-plot_config") {
    plot_config = args[i+1];
    i=i+1;
  } else if(args[i] == "-save") {
    save = TRUE;
  } else {
    cat("ERROR: Unrecognized command line argument:", args[i], "\n");
    usage();
    quit(status=1);
  }

  # Increment count
  i=i+1;

} # end while

# Check the relative performance threshold settings
if(length(rp_diff_list) == 1) rp_diff_list = rep(rp_diff_list, length(lead_list));
if(length(rp_diff_list) != length(lead_list)) {
  cat("ERROR: The number of relative performance thresholds specified",
      "must either be 1 or match the number of lead times.\n");
  quit(status=1);
}

# Check the relative scatter settings
if(length(scatter_x_list) != length(scatter_y_list)) {
  cat("ERROR: The number of scatter_x and scatter_y variables specified",
      "must match each other.\n");
  quit(status=1);
}

# Check for empty input file list
if(length(file_list) == 0) {
  cat("ERROR: Must specify input track data files using the",
      "-lookin option.\n");
  quit(status=1);
}

# Expand any wildcards in the input file list
file_list = system(paste("ls -1", paste(file_list, collapse=" ")),
                   intern=TRUE);

# Read the plotting configuration file, if specified
if(nchar(plot_config) > 0) {
  cat("Reading plotting configuration file: ", plot_config, "\n");  
  plot_config_data = read.table(plot_config, header=TRUE, comment.char='');
} else {
  cat("No plotting configuration file specified.\n");
}

########################################################################
#
# Run a tc_stat filter job to subset the data.
#
########################################################################

# Only run tc_stat is a tcst data has not been specified
if(nchar(tcst_file) == 0) {

  # Add the event equalization option
  if(event_equal) {
    filter_opts = paste(filter_opts, "-event_equal true");
  }

  # Build tc_stat command
  run_cmd = paste(tc_stat,
                  paste("-lookin", file_list, collapse=' '),
                  "-job filter -dump_row", tcst_tmp_file, filter_opts,
                  "-v 3");

  # Run the tc_stat command and check the return status
  cat("CALLING: ", run_cmd, "\n");
  status = system(run_cmd);
  if(status != 0) {
    cat("ERROR: Bad return value ", status , "\n");
    quit(status=status);
  }

  # Read the data
  cat("Reading track data:", tcst_tmp_file, "\n");
  tcst = read.table(tcst_tmp_file, header=TRUE);

  # Dispose of the temporary file by either saving or deleting it
  if(nchar(save_data) > 0) {
     run_cmd = paste("mv -f", tcst_tmp_file, save_data);
  } else {
     run_cmd = paste("rm -f", tcst_tmp_file);
  }
  cat("CALLING: ", run_cmd, "\n");
  status = system(run_cmd);

# Otherwise, read the tcst data file directly
} else {
  cat("Reading track data:", tcst_file, "\n");
  tcst = read.table(tcst_file, header=TRUE);
}
  
########################################################################
#
# Preprocess the tcst data
#
########################################################################

# Format time strings
tcst$INIT_TIME  = as.POSIXct(strptime(tcst$INIT,
                                      format="%Y%m%d_%H%M%s"));
tcst$VALID_TIME = as.POSIXct(strptime(tcst$VALID,
                                      format="%Y%m%d_%H%M%s"));
tcst$LEAD_HR    = tcst$LEAD/10000;

# Define a case column
tcst$CASE = paste(tcst$BMODEL,  tcst$STORM_ID, tcst$INIT,
                  tcst$LEAD_HR, tcst$VALID, sep=':');

# Sort the data by the CASE column
tcst = tcst[with(tcst, order(CASE)),];

# Define a demo and retro column
if(!is.na(demo_yr)) {
  tcst$TYPE[tcst$VALID_TIME >= as.POSIXct(strptime(paste(demo_yr,"0101"), format="%Y%m%d"))] = "DEMO"
  tcst$TYPE[tcst$VALID_TIME <  as.POSIXct(strptime(paste(demo_yr,"0101"), format="%Y%m%d"))] = "RETRO"
}

########################################################################
#
# Print information about the dataset.
#
########################################################################

info_list = c("AMODEL",     "BMODEL", "BASIN", "CYCLONE",
              "STORM_NAME", "LEAD",   "LEVEL", "WATCH_WARN");

for(i in 1:length(info_list)) {

  uniq_list = unique(tcst[,info_list[i]]);

  cat("Found ", length(uniq_list), " unique entries for ", info_list[i], ": ",
      paste(uniq_list, collapse=", "), "\n", sep='');

  # Check for a single BDECK model
  if(info_list[i] == "BMODEL" & length(uniq_list) != 1) {
    cat("ERROR: Must have exactly 1 BDECK model name.  ",
        "Try setting \"-bmodel name\" in the \"-filter\" option.\n");
    quit(status=1);
  }
}

########################################################################
#
# Process the series information.
#
########################################################################

# Get the unique series entries from the data
series_uniq = unique(as.character(tcst[,series]));

# List unique series entries
cat("Found", length(series_uniq), "unique value(s) for the", series,
    "series:", paste(series_uniq, collapse=", "), "\n");

# Store the series list if not specified on the command line
if(length(series_list) == 0) {
  series_list = series_uniq
}

# Store the number of series to be plotted
n_series = length(series_list);

# If the legend was not specified, default to series_list
# Otherwise, check that the length of legend_list and series_list match
if(length(legend_list) == 0) {
  legend_list = series_list
} else if(length(legend_list) != n_series) {
  cat(paste("WARNING: The number of series (", n_series,
            ") does not match the number of entries in the legend (",
            length(legend_list), ")", sep=''))
}

# Check for special series values "ALL" and "OTHER" and construct a
# list of values that correspond to each series entry
series_plot = list();
diff_flag = rep(FALSE, n_series);
series_ci = rep(series_ci, n_series);
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
      cat("ERROR: Event equalization must be on for differences.\n");
      quit(status=1);
    }

    # Store the entries for the difference
    series_plot[[i]] = unlist(strsplit(series_list[i], "-"));

    # Enable the difference flag
    diff_flag[i] = TRUE;
  }
  # Handle the rest, assuming it's a colon separated list
  else {
     series_plot[[i]] = unlist(strsplit(series_list[i], ':'));
  }
}

# Store the number of series to plot
cat("Plotting", n_series, "value(s) for the", series, "series:",
    paste(series_plot, collapse=", "), "\n");

########################################################################
#
# Loop over the list of columns and do plots.
#
########################################################################

for(i in 1:length(dep_list)) {

  cat("Processing column:", dep_list[i], "\n");
  
  # Build requested HFIP baseline
  if(hfip_bsln == "no" | diff_flag[i] == "TRUE") {
    cur_baseline = "no"
    cur_baseline_data = NA
#     cat("Plot HFIP Baseline: ",cur_baseline, "\n");
  } else if(dep_list[i] %in% baseline$VARIABLE) {
    cur_baseline_data = subset(baseline,
         baseline$BASIN %in% tcst$BASIN & 
         baseline$VARIABLE %in% dep_list[i] & 
         baseline$LEAD %in% tcst$LEAD)
    if(hfip_bsln == "0") {
      cur_baseline = "HFIP Baseline" 
#       cat(cur_baseline, "\n");
#       cat("HFIP Baseline subset:\n");
#       write.table(cur_baseline_data,row.names = FALSE);
    } else if(hfip_bsln == "5") {
      cur_baseline = "Error Target for 20% HFIP Goal"
      cur_baseline_data$VALUE = round(cur_baseline_data$VALUE*0.8,1)
#       cat("Target for 20% HFIP Goal\n");
#       cat("HFIP Baseline subset:\n");
#       write.table(cur_baseline_data,row.names = FALSE);
    } else if(hfip_bsln == "10") {
      cur_baseline = "Error Target for 50% HFIP Goal"
      cur_baseline_data$VALUE = round(cur_baseline_data$VALUE*0.5,1)
#       cat("Target for 20% HFIP Goal\n");
#       cat("HFIP Baseline subset:\n");
#       write.table(cur_baseline_data,row.names = FALSE);
    } else {
      cat("ERROR: Unexpected HFIP baseline value:", hfip_bsln, "\n");
      quit(status=1);
    }
  } else {
    cur_baseline = "no"
    cur_baseline_data = NA
  }    

  # Get the data to be plotted
  col       = get_dep_column(dep_list[i]);
  tcst$PLOT = col$val;

  # Remove special characters from output file name
  out_file_dep = sub('[)]', '', sub('[(]', '_', dep_list[i]));

  # Set plotting strings
  plot_sub  = ifelse(length(subtitle_str) == 0,
                     paste("FILTER:" , filter_opts),
                     subtitle_str);
  plot_ylab = ifelse(length(ylab_str) == 0,
                     paste(dep_list[i], " (", col$units,")", sep=''),
                     ylab_str);

  # Loop over the list of plots
  for(j in 1:length(plot_list)) {
  
    # Build output image and log file names
    if(nchar(prefix) > 0) {
      out_file = paste(outdir, "/", prefix, ".", img_ext, sep='');
      log_file = paste(outdir, "/", prefix, ".log", sep='');
    }
    else {
      out_file = paste(outdir, "/", out_file_dep, "_", 
                       tolower(plot_list[j]), ".", img_ext, sep='');
      log_file = paste(outdir, "/", out_file_dep, "_",
                       tolower(plot_list[j]), ".log", sep='');
    }

    # PLOT: Create time series of boxplots.
    if(plot_list[j] == boxplot_str) {

      # Set plotting strings
      plot_title = ifelse(length(title_str) == 0,
                          paste("Boxplots of\n", col$desc, "\nby",
                                column_info[series, "DESCRIPTION"]),
                          title_str);

      # Do the boxplot
      plot_time_series(dep_list[i], plot_list[j],
                       plot_title, plot_sub, plot_ylab, 
                       cur_baseline, cur_baseline_data,
                       skill_ref, footnote_flag);
    }
  
    # PLOT: Create time series of point plots.
    else if(plot_list[j] == point_str) {

      # Set plotting strings
      plot_title = ifelse(length(title_str) == 0,
                          paste("Point Plots of\n", col$desc, "\nby",
                                column_info[series, "DESCRIPTION"]),
                          title_str);

      # Do the point plot
      plot_time_series(dep_list[i], plot_list[j],
                       plot_title, plot_sub, plot_ylab, 
                       cur_baseline, cur_baseline_data,
                       skill_ref, footnote_flag);
    }

    # PLOT: Create time series of means.
    else if(plot_list[j] == mean_str) {

      # Set plotting strings
      plot_title = ifelse(length(title_str) == 0,
                          paste("Mean of\n", col$desc, "\nby",
                                column_info[series, "DESCRIPTION"]),
                          title_str);

      # Do the mean plot
      plot_time_series(dep_list[i], plot_list[j],
                       plot_title, plot_sub, plot_ylab, 
                       cur_baseline, cur_baseline_data,
                       skill_ref, footnote_flag);
    }

    # PLOT: Create time series of medians.
    else if(plot_list[j] == median_str) {

      # Set plotting strings
      plot_title = ifelse(length(title_str) == 0,
                          paste("Median of\n", col$desc, "\nby",
                                column_info[series, "DESCRIPTION"]),
                          title_str);

      # Do the median plot
      plot_time_series(dep_list[i], plot_list[j],
                       plot_title, plot_sub, plot_ylab, 
                       cur_baseline, cur_baseline_data,
                       skill_ref, footnote_flag);
    }

    # PLOT: Create time series of relative performance.
    else if(plot_list[j] == relperf_str) {

      # Set plotting strings
      if(length(title_str) > 0) {
        plot_title = title_str;
      }
      else {
        plot_title = paste("Relative Performance of \n", col$desc, "\n", sep='');
        if(length(unique(rp_diff_list)) == 1) {
          plot_title = paste(plot_title, "Difference", rp_diff_list[1], col$units, sep='');
        }
        plot_title = paste(plot_title, " by ", column_info[series, "DESCRIPTION"], sep='');
      }

      plot_ylab  = ifelse(length(ylab_str) == 0,
                          "Percent of Cases",
                          ylab_str);

      # Do the relative performance plot
      plot_time_series(dep_list[i], plot_list[j],
                       plot_title, plot_sub, plot_ylab, 
                       cur_baseline, cur_baseline_data,
                       skill_ref, footnote_flag);
    }

    # PLOT: Create time series of ranks for the first model.
    else if(plot_list[j] == rank_str) {

      # Set plotting strings
      plot_title = ifelse(length(title_str) == 0,
                          paste(series_list[1],
                                column_info[series, "DESCRIPTION"], "\n",
                                col$desc, "\nRank Frequency"),
                          title_str);
      plot_ylab  = ifelse(length(ylab_str) == 0,
                          "Percent of Cases",
                          ylab_str);

      # Do the relative performance plot
      plot_time_series(dep_list[i], plot_list[j],
                       plot_title, plot_sub, plot_ylab, 
                       cur_baseline, cur_baseline_data,
                       skill_ref, footnote_flag);
    }

    # PLOT: Create time series of mean skill scores.
    else if(plot_list[j] == skillmn_str) {

      # Set plotting strings
      plot_title = ifelse(length(title_str) == 0,
                          paste("Mean Skill Scores of\n", col$desc, "\nby",
                                column_info[series, "DESCRIPTION"]),
                          title_str);

      # Do the mean plot
      plot_time_series(dep_list[i], plot_list[j],
                       plot_title, plot_sub, plot_ylab, 
                       cur_baseline, cur_baseline_data,
                       skill_ref, footnote_flag);
    }

    # PLOT: Create time series of median skill scores.
    else if(plot_list[j] == skillmd_str) {

      # Set plotting strings
      plot_title = ifelse(length(title_str) == 0,
                          paste("Median Skill Scores of\n", col$desc, "\nby",
                                column_info[series, "DESCRIPTION"]),
                          title_str);

      # Do the median plot
      plot_time_series(dep_list[i], plot_list[j],
                       plot_title, plot_sub, plot_ylab, 
                       cur_baseline, cur_baseline_data,
                       skill_ref, footnote_flag);
    }

    # Ignore the scatter string here.
    else if(plot_list[j] == scatter_str) {
      next;
    }
    
    # Unsupported plot type
    else {
      cat("ERROR: Unsupported plot type:", plot_list[j], "\n");
      quit(status=1);
    }
    
  } # end for j
} # end for i

########################################################################
#
# Loop over the list of scatter columns and do plots.
#
########################################################################

if(scatter_str %in% plot_list) {

  for(i in 1:length(scatter_x_list)) {

    cat("Processing scatter columns:", scatter_x_list[i], "versus", scatter_y_list[i], "\n");

    # Get the data to be plotted
    col_x          = get_dep_column(scatter_x_list[i]);
    tcst$SCATTER_X = col_x$val;

    # Get the data to be plotted
    col_y          = get_dep_column(scatter_y_list[i]);
    tcst$SCATTER_Y = col_y$val;

    # Remove special characters from output file name
    out_file_x = sub('[)]', '', sub('[(]', '_', scatter_x_list[i]));
    out_file_y = sub('[)]', '', sub('[(]', '_', scatter_y_list[i]));

    # Set plotting strings
    plot_sub  = ifelse(length(subtitle_str) == 0,
                       paste("FILTER:" , filter_opts),
                       subtitle_str);
    plot_xlab = ifelse(length(xlab_str) == 0,
                       paste(scatter_x_list[i], " (", col_x$units,")", sep=''),
                       xlab_str);
    plot_ylab = ifelse(length(ylab_str) == 0,
                       paste(scatter_y_list[i], " (", col_y$units,")", sep=''),
                       ylab_str);

    # Build output image and log file names
    if(nchar(prefix) > 0) {
      out_file = paste(outdir, "/", prefix, ".", img_ext, sep='');
      log_file = paste(outdir, "/", prefix, ".log", sep='');
    }
    else {
      out_file = paste(outdir, "/", out_file_x, "_vs_", out_file_y, "_scatter",
                       ".", img_ext, sep='');
      log_file = paste(outdir, "/",  out_file_x, "_vs_", out_file_y, "_scatter",
                       ".log", sep='');
    }

    # Set plotting strings
    plot_title = ifelse(length(title_str) == 0,
                        paste("Scatter plot of\n", col_x$desc, "\nversus",
                              col_y$desc),
                        title_str);

    if(all(is.na(tcst$SCATTER_X))) {
      cat("ALL entries are NA for tcst$SCATTER_X:", scatter_x_list[i],"\n");
    } else if(all(is.na(tcst$SCATTER_Y))) {
      cat("ALL entries are NA for tcst$SCATTER_Y:", scatter_y_list[i],"\n");
    } else {
      # Do the scatter plot
      plot_scatter(scatter_x_list[i], scatter_y_list[i],
                   plot_title, plot_sub, plot_xlab, plot_ylab,
                   footnote_flag);
    }
  
  } # end for i
} # end if scatter

########################################################################
#
# Clean up.
#
########################################################################

# Optionally, save all of the data to an .RData file
if(save == TRUE) save.image();
