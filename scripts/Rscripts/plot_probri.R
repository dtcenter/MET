#######################################################################
##
##   Name: plot_probri.R
##
##   Description:
##
##   Usage:
##      Rscript plot_probri.R
##
##   Arguments:
##
##   Details:
##
##   Examples:
##
##   Author:
##
########################################################################

# Make sure that tc_stat can be found
#tc_stat = Sys.which("tc_stat");
tc_stat = "/d1/CODE/MET/MET_development/met/bin/tc_stat";

if(nchar(tc_stat) == 0) {
  cat("ERROR: Cannot find the tc_stat tool in your path.\n");
  quit(status=1);
}

########################################################################
#
# Usage statement.
#
########################################################################

usage = function() {
  cat("\nUsage: plot_probri.R\n");
  cat("        -lookin tcst_file_list\n");
  cat("        [-outdir path]\n");
  cat("        [-basin string]\n");
  cat("        [-amodel string]\n");
  cat("        [-by string]\n");
  cat("        [-lead n]\n");
  cat("        [-ri_window n]\n");
  cat("        [-ri_thresh n]\n");
  cat("        [-bdelta_max]\n");
  cat("        [-save]\n");
  cat("\n");
}

########################################################################
#
# read.grep in_file pattern
#
########################################################################

read.grep = function(in_file, pattern) {
  tmp_file = "/tmp/plot_probri_read_grep";
  run_cmd = paste("grep", pattern, in_file, ">", tmp_file);
  system(run_cmd);
  d = read.table(tmp_file, header=TRUE);
  system(paste("rm -f", tmp_file));
  return(d);
}

########################################################################
#
# Constants.
#
########################################################################

outdir = ".";
tcst_dump_file = paste("/tmp/plot_probri_tcst_", Sys.getpid(), ".tcst", sep='');
tcst_out_file  = paste("/tmp/plot_probri_tcst_", Sys.getpid(), ".out",  sep='');
color_list     = c("black", "red", "green", "blue", "purple", "orange", "black", "red", "green", "blue", "purple", "orange", "black", "red", "green", "blue", "purple", "orange");
lty_list       = rep(c(1, 2, 4, 1, 2, 4, 2, 4, 1, 2, 4, 1, 4, 2, 1, 4, 2, 1), 100);

# Defaults
file_list = c();
basin     = "";
amodel    = "";
by        = "";
lead      = 24;
ri_window = 24;
ri_thresh = 30;
ri_exact  = TRUE;
save      = FALSE;

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

# Parse optional arguments
i=1;
while(i <= length(args)) {

  if(args[i] == "-lookin") {
    while(i+1 <= length(args) & substring(args[i+1], 1, 1) != '-') {
       file_list = c(file_list, args[i+1]);
       i=i+1;
    }
  } else if(args[i] == "-outdir") {
    outdir = args[i+1];
    i=i+1;
  } else if(args[i] == "-basin") {
    basin = args[i+1];
    i=i+1;
  } else if(args[i] == "-amodel") {
    amodel = args[i+1];
    i=i+1;
  } else if(args[i] == "-by") {
    by = args[i+1];
    i=i+1;
  } else if(args[i] == "-lead") {
    lead = args[i+1];
    i=i+1;
  } else if(args[i] == "-ri_window") {
    ri_window = args[i+1];
    i=i+1;
  } else if(args[i] == "-ri_thresh") {
    ri_thresh = args[i+1];
    i=i+1;
  } else if(args[i] == "-bdelta_max") {
    ri_exact = FALSE;
  } else if(args[i] == "-save") {
    save = TRUE;
  } else {
    cat("ERROR: Unrecognized command line argument:", args[i], "\n");
    usage();
    quit();
  }

  # Increment count
  i=i+1;

} # end while

# Check for empty input file list
if(length(file_list) == 0) {
  cat("ERROR: Must specify input data files using the -lookin option.\n");
  quit(status=1);
}

# Expand any wildcards in the input file list
file_list = system(paste("ls -1", paste(file_list, collapse=" ")),
                   intern=TRUE);

########################################################################
#
# Run a tc_stat filter job to subset the data.
#
########################################################################

# Build tc_stat command
# Toss out BDECK Land Points
run_cmd = paste(tc_stat,
                paste(" -lookin", file_list, collapse=' '),
                " -job probri",
                " -out_line_type PCT,PSTD,PRC,PJC",
                " -column_thresh BDLAND gt0",
                " -column_thresh RI_WINDOW ==", ri_window,
                " -lead ", lead,
                " -probri_bdelta_thresh ge", ri_thresh,
                " -probri_exact ", ri_exact,
                " -dump_row ", tcst_dump_file,
                " -out ", tcst_out_file,
                " -v 3",
                sep='');

# Basin option
if(nchar(basin) > 0) {
  run_cmd = paste(run_cmd, "-basin", basin);
  basin_str = basin;
} else {
  basin_str = "ALL";
}

# AModel option
if(nchar(amodel) > 0) {
  run_cmd = paste(run_cmd, "-amodel", amodel);
  amodel_str = amodel;
} else {
  amodel_str = "ALL";
}

# By option
if(nchar(by) > 0) {
  run_cmd = paste(run_cmd, "-by", by);
}

# Run the tc_stat command and check the return status
cat("CALLING:", run_cmd, "\n");
status = system(run_cmd);
if(status != 0) {
  cat("ERROR: Bad return value ", status , "\n");
  quit(status=status);
}

# Read the data
cat("Reading PROBRI data:", tcst_dump_file, "\n");
tcst = read.table(tcst_dump_file, header=TRUE);

cat("Reading PROBRI output:", tcst_out_file, "\n");
pct  = read.grep(tcst_out_file, "PROBRI_PCT");
pstd = read.grep(tcst_out_file, "PROBRI_PSTD");
prc  = read.grep(tcst_out_file, "PROBRI_PRC");
pjc  = read.grep(tcst_out_file, "PROBRI_PJC");

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

# Handle BDELTA_MAX
if(ri_exact) {
  BDelta_str = "BDELTA";
} else {
  tcst$BDELTA = tcst$BDELTA_MAX;
  BDelta_str = "BDELTA_MAX";
}

subtitle = paste(BDelta_str, " >= ", ri_thresh, ", ",
                 length(tcst$PROB_1), " probs (", sum(tcst$PROB_1>0),
                 " non-zero), ", "Basin: ", basin_str,", Model: ",
                 amodel_str, ", Lead: ", lead, sep='');

out_base = paste(outdir, "/PROBRI_", basin_str, "_", amodel_str, "_F",
                 lead, "_RI", ri_window, "_thresh", ri_thresh, sep='');
out_base = gsub(",", "_", out_base);

library(verification);
library(graphics);

########################################################################
#
# Create histogram of probability values
#
########################################################################

# Create output file name
out_file = paste(out_base, "_hist.png", sep='');

brate = sum(tcst$BDELTA >= as.integer(ri_thresh))/length(tcst$BDELTA);

# Open the output device
cat(paste("Creating image file:", out_file, "\n"));
bitmap(out_file, type="png256", height=8.5, width=11, res=72);

  hist(tcst$PROB_1, breaks=seq(0,100,1), freq=FALSE,
       main=paste("Histogram of", ri_window, "hr RI Probabilities\n",
                  BDelta_str, ">=", ri_thresh, "Rate =", round(brate, 4)),
       xlab="Probability (%)", sub=subtitle);

dev.off();

########################################################################
#
# Create discrimination plot
#
########################################################################

# Create output file name
out_file = paste(out_base, "_discrim.png", sep='');

# Open the output device
cat(paste("Creating image file:", out_file, "\n"));
bitmap(out_file, type="png256", height=8.5, width=11, res=72);

  tcst$RI = NA;
  tcst$RI[tcst$BDELTA >= ri_thresh] = "Y"
  tcst$RI[tcst$BDELTA <  ri_thresh] = "N"

  discrimination.plot(tcst$RI, tcst$PROB_1,
    main=paste(ri_window, "hr RI Probability Discrimination\n",
    subtitle, sep=''), xlab="Probability (%)");

dev.off();

########################################################################
#
# Create scatter plot
#
########################################################################

# Create output file name
out_file = paste(out_base, "_scatter.png", sep='');

# Open the output device
cat(paste("Creating image file:", out_file, "\n"));

bitmap(out_file, type="png256", height=8.5, width=11, res=72);

  smoothScatter(tcst$PROB_1, tcst$BDELTA, nrpoints=0,
    main=paste(ri_window, "hr RI Probability vs BDECK Change"),
    ylab=paste(BDelta_str, "(kts)"), xlab="Probability (%)", sub=subtitle);
  abline(h=ri_thresh, col="red", lwd=2.0);

dev.off();

########################################################################
#
# Create ROC curve.
#
########################################################################

# Create output file name
out_file = paste(out_base, "_roc.png", sep='');

# Open the output device
cat(paste("Creating image file:", out_file, "\n"));

bitmap(out_file, type="png256", height=8.5, width=11, res=72);
  plot(x=c(0,1), y=c(0,1), type="n", xlab="POFD", ylab="PODY",
       main=paste(ri_window, "hr RI Probability ROC"),
       sub=subtitle);

  # Add reference lines
  abline(h=0, col="gray", lwd=2);
  abline(v=0, col="gray", lwd=2);
  abline(0, 1, col="gray", lwd=2, lty=2);

  for(i in 1:dim(prc)[1]) {
    pody = unlist(c(1, prc[i, grep("PODY", names(prc))], 0));
    pofd = unlist(c(1, prc[i, grep("POFD", names(prc))], 0));
    lines(x=pofd, y=pody, col=color_list[i], lty=lty_list[i], lwd=2);
  }

  if(nchar(by) > 0) {
    for(col in unlist(strsplit(toupper(by), ','))) {
      if(is.null(prc$CASE)) {
        prc$CASE = prc[,col];
      } else {
        prc$CASE = paste(prc$CASE, "_", prc[,col], sep='');
      }
    }
  } else {
    prc$CASE = "ALL";
  }

  par(bg="white");
  legend("bottomright",
         paste(as.character(prc$CASE),
               ": Area = ",  round(pstd$ROC_AUC, 4),
               ", Reli = ",  round(pstd$RELIABILITY, 4),
               ", Brier = ", round(pstd$BRIER, 4), sep=''),
         col=color_list, lty=lty_list, lwd=2);

dev.off();

########################################################################
#
# Create Reliability Diagram.
#
########################################################################

# Create output file name
out_file = paste(out_base, "_reli.png", sep='');

# Open the output device
cat(paste("Creating image file:", out_file, "\n"));

bitmap(out_file, type="png256", height=8.5, width=11, res=72);

  # Probability mid-points
  probs = pct[1, grep("THRESH_", names(pct))];
  y.i   = as.numeric((probs[1:(length(probs)-1)] + probs[2:length(probs)])/2);

  # Observed rate = oy / (oy + on)
  oy     = pct[, grep("OY_", names(pct))];
  on     = pct[, grep("ON_", names(pct))];
  obar.i = as.matrix(oy / (oy + on));

  # Probability forecast rate = (oy + on) / total
  total  = pct[, "TOTAL"];
  prob.y = as.matrix((oy + on) / total);

  if(nchar(by) > 0) {
    for(col in unlist(strsplit(toupper(by), ','))) {
      if(is.null(pct$CASE)) {
        pct$CASE = pct[,col];
      } else {
        pct$CASE = paste(pct$CASE, "_", pct[,col], sep='');
      }
    }
  } else {
    pct$CASE = "ALL";
  }

  reliability.plot(y.i, t(obar.i), t(prob.y),
                   titl=paste(ri_window, "hr RI Reliability Diagram\n", subtitle),
                   legend.names=as.character(pct$CASE));

dev.off();

########################################################################
#
# Clean up.
#
########################################################################

# Optionally, save all of the data to an .RData file
if(save == TRUE) save.image();

# Dispose of the temporary files
run_cmd = paste("rm -f", tcst_dump_file, tcst_out_file);
cat("CALLING: ", run_cmd, "\n");
status = system(run_cmd);
