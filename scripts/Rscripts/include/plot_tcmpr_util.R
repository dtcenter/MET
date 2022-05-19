########################################################################
#                                                           
#      Add footnote to graphics      
#                                                           
########################################################################

footnote <- paste(format(strftime(Sys.time(), "%Y-%m-%d %H:%M:%S", tz = "UTC")))

# default footnote is today's date, cex=.7 (size) and color
# is a kind of grey

makeFootnote <- function(footnoteText=
                         footnote, size= .7, color= grey(.66))
{
   require(grid)
   pushViewport(viewport())
   grid.text(label= footnoteText ,
             x = unit(1,"npc") - unit(2, "mm"),
             y= unit(2, "mm"),
             just=c("right", "bottom"),
             gp=gpar(cex= size, col=color))
   popViewport()
}

########################################################################
#
# Get a column of data to be plotted, handling absolute values and
# differences.
#
########################################################################

get_dep_column = function(dep) {

  # Check for absolute value
  abs_flag = (substring(dep, 1, 3) == "ABS");

  # Strip off the absolute value
  if(abs_flag) {
    dep = unlist(strsplit(dep, "[(|)]"))[2];
  }

  # Split based on differences
  diff_list = unlist(strsplit(dep, "[-]"));

  # Initialize output
  col       = c();
  col$val   = get_column_val(diff_list[1]);
  col$desc  = column_info[diff_list[1], "DESCRIPTION"];
  col$units = column_info[diff_list[1], "UNITS"];

  # Loop over any remaining entries
  i = 2
  while(i <= length(diff_list)) {
    col$val   = col$val - get_column_val(diff_list[i]);
    col$desc  = paste(col$desc, "-",
                      column_info[diff_list[i], "DESCRIPTION"]);
    # Only append units that differ
    if(col$units != column_info[diff_list[i], "UNITS"]) {
      col$units = paste(col$units, "-",
                        column_info[diff_list[i], "UNITS"]);
    }
    i = i+1
  }

  # Apply absolute value
  if(abs_flag) {
    col$val  = abs(col$val);
    col$desc = paste("Absolute Value of", col$desc);
  }

  return(col);
}

########################################################################
#
# Get the column values, handling wind data.
#
########################################################################

get_column_val = function(dep) {

  # Compute the average of the wind radii, if requested
  if(length(grep("AVG_WIND", dep)) > 0) {
  
    # Parse the first character and the last 2 characters
    typ = substr(dep, 1, 1);
    rad = substr(dep, nchar(dep)-1, nchar(dep));
    
    # Pull wind radii for the 4 quadrants
    ne_wind = tcst[, paste(typ, "NE_WIND_", rad, sep='')];
    se_wind = tcst[, paste(typ, "SE_WIND_", rad, sep='')];
    sw_wind = tcst[, paste(typ, "SW_WIND_", rad, sep='')];
    nw_wind = tcst[, paste(typ, "NW_WIND_", rad, sep='')];
    
    # Replace any instances of 0 with NA
    ne_wind[ne_wind == 0] = NA;
    se_wind[se_wind == 0] = NA;
    sw_wind[sw_wind == 0] = NA;
    nw_wind[nw_wind == 0] = NA;
    
    # Compute the average
    val = (ne_wind + se_wind + sw_wind + nw_wind)/4;
  }
  # Otherwise, just get the column value
  else {
    val = tcst[,dep];
  }

  # For _WIND_ columns, replace any instances of 0 with NA
  if(length(grep("_WIND_", dep)) > 0) {
    val[val == 0] = NA;
  }

  return(val);
}

########################################################################
#
# Subset the data for the current series and requested lead times.
#
########################################################################

get_series_data = function(cur, cur_plot, diff) {

  # Check for series entry differences
  if(diff == TRUE) {

    # Initialize to the first entry
    series_data = tcst[tcst[,series] == cur_plot[1],];

    # Store the first PLOT value as the percent improvement reference
    series_data$REF = series_data$PLOT;

    # Loop over the remaining series entries
    i = 2;
    while(i <= length(cur_plot)) {
      series_diff = tcst[tcst[,series] == cur_plot[i],];

      # Check that the CASE column lines up exactly
      if(sum(series_data$CASE != series_diff$CASE) > 0) {
        cat("ERROR: When computing series differences for",
            cur, "the case data does not match:",
            series_data$CASE, "!=", series_diff$CASE);
        quit(status=1);
      }

      # Compute series difference
      series_data$PLOT = series_data$PLOT - series_diff$PLOT

      i = i+1;
    } # end while
  }
  # Otherwise, just subset based on a single series entry
  else {
    series_data = tcst[tcst[,series]%in%cur_plot,];
  }

  # Subset based on requested lead times
  series_data = series_data[series_data$LEAD_HR%in%lead_list,];
  
  return(series_data);
}

########################################################################
#
# Aggregation functions for case data.
#
########################################################################

find_winner = function(x) { if(sum(is.na(x)) > 0)  { return(NA); }
                            else                   { return(series_list[which.min(x)]); }
}
find_thresh = function(x) { return(rp_diff_list[which(lead_list == x)]); }
eval_exp    = function(x) { return(eval(parse(text=x))); }
rank_random = function(x) { return(rank(x, na.last="keep", ties.method="random")[1]); }
rank_min    = function(x) { return(rank(x, na.last="keep", ties.method="min")[1]); }

########################################################################
#
# Build a table with summary information for each case.
#
########################################################################

get_case_data = function() {

  # Check that series_list equals series_plot
  for(i in 1:length(series_list)) {
    if(series_list[i] != series_plot[[i]]) {
      cat(paste("ERROR: Cannot compute case data when plotting series",
                "aggregations.\n"));
      quit(status=1);
    }
  } # end for i

  # Initialize
  series_data = c();

  # Get the data for all series to be used in order
  for(i in 1:length(series_list)) {
    series_data =
      rbind(series_data,
            get_series_data(series_list[i], series_list[i], FALSE));
  } # end for i

  # Build a set of unique cases
  case_data = unique(data.frame(CASE=series_data$CASE,
                                LEAD_HR=series_data$LEAD_HR,
                                MIN=NA, MAX=NA, WIN=NA,
                                DIFF=NA, RP_THRESH=NA, DIFF_TEST=NA, RESULT=NA,
                                PLOT=NA, RANK_RANDOM=NA, RANK_MIN=NA));

  # Check for equal numbers of entries for each case
  if(sum(aggregate(series_data$PLOT, by=list(series_data$CASE), length)$x != n_series)) {
      cat(paste("ERROR: Must have the same number of entries for each case.\n"));
      quit(status=1);
  }

  # Compute summary info for each case
  case_data$MIN         = aggregate(series_data$PLOT, by=list(series_data$CASE), FUN=min)$x;
  case_data$MAX         = aggregate(series_data$PLOT, by=list(series_data$CASE), FUN=max)$x;
  case_data$WIN         = aggregate(series_data$PLOT, by=list(series_data$CASE), FUN=find_winner)$x;
  case_data$DIFF        = case_data$MAX - case_data$MIN;
  case_data$RP_THRESH   = lapply(case_data$LEAD_HR, FUN=find_thresh);
  case_data$DIFF_TEST   = paste(case_data$DIFF, case_data$RP_THRESH, sep='');
  case_data$RESULT      = lapply(case_data$DIFF_TEST, FUN=eval_exp);
  case_data$PLOT        = ifelse(case_data$RESULT, case_data$WIN, "TIE");
  case_data$RANK_RANDOM = aggregate(series_data$PLOT, by=list(series_data$CASE), FUN=rank_random)$x;
  case_data$RANK_MIN    = aggregate(series_data$PLOT, by=list(series_data$CASE), FUN=rank_min)$x;

  return(case_data);
}

########################################################################
#
# Compute a confidence interval for a proportion.
#
########################################################################

get_prop_ci <- function(x, n) {

  # Compute the standard proportion error
  zval   = abs(qnorm(alpha/2));
  phat   = x/n;
  bound  = (zval * ((phat * (1 - phat) + (zval^2)/(4 * n))/n)^(1/2))/
           (1 + (zval^2)/n);
  midpnt = (phat + (zval^2)/(2 * n))/(1 + (zval^2)/n);

  # Compute the statistic and confidence interval
  stat     = c();
  stat$val = 100*phat;
  stat$ncl = ifelse(n < n_min, NA, 100*(midpnt - bound));
  stat$ncu = ifelse(n < n_min, NA, 100*(midpnt + bound));

  return(stat);
}

########################################################################
#
# Compute a confidence interval about the mean.
#
########################################################################

get_mean_ci = function(d) {

  # Degrees of freedom for t-distribution
  df = sum(!is.na(d)) - 1;

  # Compute the standard error
  s = Compute_STDerr_from_mean(d, "ML");
  if(length(s) > 1 && !is.na(s[2]) && s[2] == 0) {
    tval   = abs(qt(alpha/2, df));
    stderr = tval*s[1];
  }
  else {
    stderr = NA;
  }

  # Compute the statistic and confidence interval
  stat     = c();
  stat$val = mean(d, na.rm=TRUE);
  stat$ncl = ifelse(sum(!is.na(d)) < n_min || is.na(stderr), NA, stat$val - stderr);
  stat$ncu = ifelse(sum(!is.na(d)) < n_min || is.na(stderr), NA, stat$val + stderr);

  # Compute the p-value
  ss_pval   = 0.0 - abs(stat$val/s[1]);
  stat$pval = 1 - 2*pt(ss_pval, df);
  
  return(stat);
}

########################################################################
#
# Compute a confidence interval about the median.
#
########################################################################

get_median_ci = function(d) {

  # Degrees of freedom for t-distribution
  df = sum(!is.na(d)) - 1;
  
  # Compute the standard error
  s = Compute_STDerr_from_median(d, "ML");
  if(length(s) > 1 && !is.na(s[2]) && s[2] == 0) {
    tval   = abs(qt(alpha/2, df));
    stderr = tval*s[1];
  }
  else {
    stderr = NA;
  }

  # Compute the statistic and confidence interval
  stat     = c();
  stat$val = median(d, na.rm=TRUE);
  stat$ncl = ifelse(sum(!is.na(d)) < n_min || is.na(stderr), NA, stat$val - stderr);
  stat$ncu = ifelse(sum(!is.na(d)) < n_min || is.na(stderr), NA, stat$val + stderr);

  # Compute the p-value
  ss_pval   = 0.0 - abs(stat$val/s[1]);
  stat$pval = 1 - 2*pt(ss_pval, df);

  return(stat);
}

########################################################################
#
# Get the range of the data based on the plot type.
#
########################################################################

get_yrange = function(plot_type, cur_baseline_data, skill_ref) {

  # Initialize
  ylim = c(NA,NA);

  # Get the case data when necessary
  if(plot_type == relperf_str || plot_type == rank_str) {
    case_data = get_case_data();
  }

  # Get current subset of skill score reference data
  if(plot_type == skillmn_str || plot_type == skillmd_str) {
    skill_ref_data = get_series_data(skill_ref, skill_ref,
                                  diff_flag[i]);
  }

  # Loop over the series list entries
  for(i in 1:n_series) {

    # Get current subset of data
    series_data = get_series_data(series_list[i], series_plot[[i]],
                                  diff_flag[i]);
    # Skip this iteration if there's no valid data
    if(sum(!is.na(series_data$PLOT)) == 0) next;

    # Initialize
    yvals = c();

    # Get the data range based on plot type
    if(plot_type == mean_str || plot_type == median_str) {

      for(j in 1:length(lead_list)) {

        # Get data for the current lead time
        data = subset(series_data, series_data$LEAD_HR == lead_list[j] &
                      !is.na(series_data$PLOT));

        if(all(!is.na(cur_baseline_data))) {
          baseline_lead = cur_baseline_data[cur_baseline_data$LEAD/10000 == lead_list[j],]$VALUE;
        }
        else {
          baseline_lead = NA;
        }

        # Skip lead times for which no data is found
        if(dim(data)[1] == 0) next;

        # Get the values to be plotted for this lead time
        if(plot_type == mean_str) { cur =   get_mean_ci(data$PLOT); }
        else                      { cur = get_median_ci(data$PLOT); }

        # Append the current values to the list
        if(series_ci[i]) { yvals = c(yvals, cur$val, cur$ncl, cur$ncu, baseline_lead); }
        else             { yvals = c(yvals, cur$val, baseline_lead);                   }

      } # end for j
    }
    else if(plot_type == skillmn_str || plot_type == skillmd_str) {
    
      for(j in 1:length(lead_list)) {

        # Get data for the current lead time
        data     = subset(series_data, series_data$LEAD_HR == lead_list[j] &
                          !is.na(series_data$PLOT));
        data_ref = subset(skill_ref_data, skill_ref_data$LEAD_HR == lead_list[j] &
                          !is.na(skill_ref_data$PLOT));
                          
        if(all(!is.na(cur_baseline_data))) {
          baseline_lead = round(100*(cur_baseline_data[cur_baseline_data$LEAD/10000 == lead_list[j] & cur_baseline_data$TYPE == "OCD5",]$VALUE-cur_baseline_data[cur_baseline_data$LEAD/10000 == lead_list[j] & cur_baseline_data$TYPE == "CONS",]$VALUE)/cur_baseline_data[cur_baseline_data$LEAD/10000 == lead_list[j] & cur_baseline_data$TYPE == "OCD5",]$VALUE,1)
        }
        else {
          baseline_lead = NA;
        }

        # Skip lead times for which no data is found
        if(dim(data)[1] == 0) next;

        # Get the values to be plotted for this lead time
        if(plot_type == skillmn_str) {
          # Compute the statistic
          cur      = c();
          cur_mean = mean(data$PLOT, na.rm=TRUE);
          ref_mean = mean(data_ref$PLOT); 
          cur$val  = round(100*(ref_mean - cur_mean)/ref_mean,0)
        } else                    {
          # Compute the statistic
          cur      = c();
          cur_median = median(data$PLOT, na.rm=TRUE);
          ref_median = median(data_ref$PLOT); 
          cur$val  = round(100*(ref_median - cur_median)/ref_median,0)
        }
        
        # Append the current values to the list
        if(series_ci[i]) { yvals = c(yvals, cur$val, baseline_lead);                   }
        else             { yvals = c(yvals, cur$val, baseline_lead);                   }

      } # end for j
    }
    else if(plot_type == relperf_str || plot_type == rank_str) {

      for(j in 1:length(lead_list)) {

        ind = (case_data$LEAD_HR == lead_list[j]);

        # Append the plotting limits for each lead time
        if(plot_type == relperf_str) {

          # Get counts
          n_cur = sum(case_data$PLOT[ind] == series_list[i], na.rm=TRUE);
          n_tot = sum(!is.na(case_data$PLOT[ind]));

          # Compute the current relative performance and CI
          cur = get_prop_ci(n_cur, n_tot);

          # Append the current values to the list
          if(series_ci[i]) { yvals = c(yvals, cur$val, cur$ncl, cur$ncu); }
          else             { yvals = c(yvals, cur$val);                   }

          # Handle the ties
          n_cur = sum(case_data$PLOT[ind] == "TIE", na.rm=TRUE);
          n_tot = sum(!is.na(case_data$PLOT[ind]));

          # Compute the current relative performance and CI
          cur = get_prop_ci(n_cur, n_tot);

          # Append the current values to the list
          if(series_ci[i]) { yvals = c(yvals, cur$val, cur$ncl, cur$ncu); }
          else             { yvals = c(yvals, cur$val);                   }
        }
        # Handle the rank frequency
        else {

          # Get counts
          n_cur = sum(case_data$RANK_RANDOM[ind] == i, na.rm=TRUE);
          n_tot = sum(!is.na(case_data$RANK_RANDOM[ind]));

          # Compute the current rank value's frequency and CI
          cur = get_prop_ci(n_cur, n_tot);

          # Append the current values to the list
          if(series_ci[i]) { yvals = c(yvals, cur$val, cur$ncl, cur$ncu); }
          else             { yvals = c(yvals, cur$val);                   }
        }

      } # end for j
    }
    else {
      if(all(!is.na(cur_baseline_data))) { baseline_lead = cur_baseline_data$VALUE; }
      else                               { baseline_lead = NA;                      }
      yvals = range(c(series_data$PLOT, baseline_lead), na.rm=TRUE);
    }

    # Update the plotting limits
    ylim = range(c(ylim, yvals), na.rm=TRUE);
    
  } # end for i

  return(ylim);
}

########################################################################
#
# Plot time-series of data.
#
########################################################################

plot_time_series = function(dep, plot_type,
                            title_str, subtitle_str, ylab_str, 
                            cur_baseline, cur_baseline_data,
                            skill_ref, footnote_flag) {

  cat("Plotting", plot_type, "time series by", series, "\n");

  # Compute the series offsets
  hoff = ifelse(plot_type == relperf_str || plot_type == rank_str,
                relperf_rank_horz_offset, horz_offset);
  horz = (seq(1, n_series) - n_series/2 - 0.5)*hoff;
  if(event_equal == FALSE) {
    vert = seq(2.0, 2.0-(n_series*vert_offset), by=-1.0*vert_offset);
  } else {
    vert = rep(1.0,n_series);
  }

  # Determine whether adding the HFIP baseline is appropriate
  if(cur_baseline == "no") {
    cat("Plot HFIP Baseline:", cur_baseline, "\n");
  } else if(length(i <- grep("Water Only", title_str)) | (plot_type != boxplot_str 
            & plot_type != point_str & plot_type != mean_str 
            & plot_type != skillmn_str)) {
    cur_baseline = "no"
    cur_baseline_data = NA
    cat("Plot HFIP Baseline:", cur_baseline, "\n");
  } else {
    if(plot_type == "SKILL_MN") {
      cur_baseline = gsub("Error ", "Skill ", cur_baseline)
      cur_baseline = gsub("HFIP Baseline ", "HFIP Skill Baseline", cur_baseline)
    } else {
      cur_baseline_data = na.omit(subset(cur_baseline_data, cur_baseline_data$TYPE %in% "CONS"))
    }    
    cat("Plot HFIP Baseline:", gsub("Error ", "", cur_baseline), "\n");
    write.table(cur_baseline_data,row.names = FALSE);
  }

  # Set the range for the Y-axis
  if(!is.na(ymin) & !is.na(ymax)) { yrange = c(ymin, ymax);                            }
  else                            { yrange = get_yrange(plot_type, cur_baseline_data,
                                                        skill_ref);                    }

  # Do not create plot file if the y limits are not finite
  if(any(!is.finite(yrange))) {
    cat("Not plotting", plot_type, "time series by", series, "since yrange is", yrange,"\n")
  } else {

  # Open the output device
  cat(paste("Creating image file:", out_file, "\n"));
  bitmap(out_file, type=img_fmt,
         height=img_hgt, width=img_wdth, res=img_res);

  cat(paste("Range of ", dep, ":", sep=''),
      paste(yrange, collapse=", "), "\n");

  # Create an empty plot
  top_mar = ifelse(event_equal, 6, 6+floor(n_series/2));
  par(mfrow=c(1,1), mar=c(5,4,top_mar,2), cex=1.5);  
  plot(x=seq(0, max(lead_list), 6), type="n",
       xlab="Lead Time (h)",
       ylab=ylab_str,
       main=NA, sub=subtitle_str,
       xlim=c(0+min(horz), max(lead_list)+max(horz)),
       ylim=yrange,
       xaxt='n', col=0, col.axis="black");
  title(main=title_str, line=top_mar-3.5);

  # Draw the X-axis
  axis(1, at=lead_list, tick=TRUE, labels=lead_list);

  # Determine the plotting options to be used.  If defined, use the
  # plotting options for each series.
  if(exists("plot_config_data") &&
    series %in% names(plot_config_data) &&
    plot_type != relperf_str &&
    plot_type != rank_str) {
 
    col_list = rep(NA, n_series);
    pch_list = rep(NA, n_series);
    lty_list = rep(NA, n_series);
    lwd_list = rep(NA, n_series);
    
    # Loop through the series and store the plotting options
    for(i in 1:n_series) {
    
      # Get indicator for the current series
      if(length(ii <- grep("-", series_list))) {
        ind = (plot_config_data[[series]] == legend_list[i]);
      } else {
        ind = (plot_config_data[[series]] == series_list[i]);
      }

      # If there's not exactly one setting, use defaults
      if(sum(ind) != 1) {
        if(length(i <- grep("-", series_list))) {
          cat("WARNING: Problem with \"", legend_list[i],
              "\" entry for the \"", series, "\" series in file \"",
              plot_config, "\".\n", sep='');
        } else {
          cat("WARNING: Problem with \"", series_list[i],
              "\" entry for the \"", series, "\" series in file \"",
              plot_config, "\".\n", sep='');
        }
        col_list[i] = default_color;
        pch_list[i] = default_pch;
        lty_list[i] = default_lty;
        lwd_list[i] = default_lwd;
      }
      # Otherwise, retrieve the plotting configuration entries
      else {

        col_list[i] = as.character(plot_config_data$COL[ind]);
        pch_list[i] = plot_config_data$PCH[ind];
        lty_list[i] = plot_config_data$LTY[ind];
        lwd_list[i] = plot_config_data$LWD[ind];

      }
    }
  }
  # Otherwise, use color list for the current plot type
  else {
    col_list = eval(parse(text=paste(tolower(plot_type),
                                     "_color_list", sep='')));
    pch_list = rep(default_pch, n_series);
    lty_list = rep(default_lty, n_series);
    lwd_list = rep(default_lwd, n_series);
  }

  # Check for too few colors
  if(n_series > length(col_list)) {
    cat("WARNING: The number of series (", n_series,
        ") exceeds the number of colors (", length(col_list),
        ").\n", sep='');
  }

  # Store all the plotting options in one list
  plot_opts = list(col_list=col_list, pch_list=pch_list,
                   lty_list=lty_list, lwd_list=lwd_list);

  # Populate the plot based on plot type
  if(plot_type == boxplot_str || plot_type == point_str) {
    plot_box_point(dep, plot_type, horz, vert, plot_opts,
                   cur_baseline, cur_baseline_data);
  }
  else if(plot_type == mean_str || plot_type == median_str) {
    plot_mean_median(dep, plot_type, horz, vert, plot_opts,
                     cur_baseline, cur_baseline_data);
  }
  else if(plot_type == skillmn_str || plot_type == skillmd_str) {
    plot_skill_mean_median(dep, plot_type, horz, vert, plot_opts,
                           cur_baseline, cur_baseline_data, skill_ref);
  }
  else if(plot_type == relperf_str) {
    plot_relperf(dep, horz, vert, plot_opts);
  }
  else if(plot_type == rank_str) {
    plot_rank(dep, horz, vert, plot_opts);
  }

  if(footnote_flag == "TRUE") {
    makeFootnote(footnote)
  }

  # Close the output device
  dev.off();
  }
}

########################################################################
#
# Plot data on scatter plot.
#
########################################################################

plot_scatter = function(scatter_x, scatter_y,
                        title_str, subtitle_str, xlab_str, 
                        ylab_str, footnote_flag) {

  cat("Plotting scatter plot.\n");

  # CWILL: For transparent dots, must use pdf
#   pdf(out_file,
#         height=img_hgt, width=img_wdth, useDingbats=FALSE);

#   # Set the range for the X and Y-axes
#   if(!is.na(xmin) & !is.na(xmax)) { xrange = c(xmin, xmax);      }
#   else                            { xrange = range(tcst$SCATTER_X, na.rm=TRUE); }
#   if(!is.na(ymin) & !is.na(ymax)) { yrange = c(ymin, ymax);      }
#   else                            { yrange = range(tcst$SCATTER_Y, na.rm=TRUE); }
#   cur_baseline_data = na.omit(subset(cur_baseline_data, cur_baseline_data$TYPE %in% "CONS"))
#   tcst$SCATTER_X = subset(tcst$SCATTER_X, !is.na(tcst$SCATTER_Y))
#   tcst$SCATTER_Y = subset(tcst$SCATTER_Y, !is.na(tcst$SCATTER_X))
#     ne_wind[ne_wind == 0] = NA;

  # Set non-matched SCATTER_X and SCATTER_Y values to NA
  tcst$SCATTER_X[is.na(tcst$SCATTER_Y)] = NA;
  tcst$SCATTER_Y[is.na(tcst$SCATTER_X)] = NA;

  # Set the range for the X and Y-axes
  if(!is.na(xmin) & !is.na(xmax)) {
    xrange = c(xmin, xmax);
  } else if(length(i <- grep("_WIND_", scatter_x)) && length(i <- grep("_WIND_", scatter_y))) {
    xrange = range(c(tcst$SCATTER_X,tcst$SCATTER_Y), na.rm=TRUE);
  } else if(length(i <- grep("AMAX_WIND", scatter_x)) && length(i <- grep("BMAX_WIND", scatter_y))) {
    xrange = range(c(tcst$SCATTER_X,tcst$SCATTER_Y), na.rm=TRUE);
  } else if(length(i <- grep("BMAX_WIND", scatter_x)) && length(i <- grep("AMAX_WIND", scatter_y))) {
    xrange = range(c(tcst$SCATTER_X,tcst$SCATTER_Y), na.rm=TRUE);
  } else {
    xrange = range(tcst$SCATTER_X, na.rm=TRUE);
  }
  if(!is.na(ymin) & !is.na(ymax)) {
    yrange = c(ymin, ymax);
  } else if(length(i <- grep("_WIND_", scatter_x)) && length(i <- grep("_WIND_", scatter_y))) {
    yrange = range(c(tcst$SCATTER_X,tcst$SCATTER_Y), na.rm=TRUE);
  } else if(length(i <- grep("AMAX_WIND", scatter_x)) && length(i <- grep("BMAX_WIND", scatter_y))) {
    yrange = range(c(tcst$SCATTER_X,tcst$SCATTER_Y), na.rm=TRUE);
  } else if(length(i <- grep("BMAX_WIND", scatter_x)) && length(i <- grep("AMAX_WIND", scatter_y))) {
    yrange = range(c(tcst$SCATTER_X,tcst$SCATTER_Y), na.rm=TRUE);
  } else {
    yrange = range(tcst$SCATTER_Y, na.rm=TRUE);
  }
  
  # Do not create plot file if the y limits are not finite
  if(any(!is.finite(xrange)) || any(!is.finite(yrange))) {
    cat("Not plotting scatter plot since \n",
         "xrange is", xrange,"\n",
         "yrange is", yrange,"\n")
  } else if(sum(tcst$LEAD_HR[!is.na(tcst$SCATTER_X)] %in% lead_list) == 0) {
    cat("Not plotting scatter plot since \n",
         "lead times for", scatter_x,"column:", unique(tcst$LEAD_HR[!is.na(tcst$SCATTER_X)]), "\n",
         "are not in the list of lead times for this plot:", lead_list,"\n")
  } else {

  # Open the output device
  cat(paste("Creating image file:", out_file, "\n"));
  bitmap(out_file, type=img_fmt,
         height=img_hgt, width=img_wdth, res=img_res);
  
  cat(paste("x-axis range of ", scatter_x, ":", sep=''),
      paste(xrange, collapse=", "), "\n");
  cat(paste("y-axis range of ", scatter_y, ":", sep=''),
      paste(yrange, collapse=", "), "\n");
  
  # Add extra space to right of plot area; change clipping to figure
  par(mar=c(5,4,4,8), xpd=TRUE)

# Create an empty plot
#   top_mar = ifelse(event_equal, 6, 6+floor(n_series/2));
#   par(mfrow=c(1,1), mar=c(5,4,top_mar,2), cex=1.5);
#   plot(x=seq(0, max(lead_list), 6), type="n",
#        xlab=xlab_str,
#        ylab=ylab_str,
#        main=NA, sub=subtitle_str,
#        xlim=c(0+min(horz), max(lead_list)+max(horz)),
#        ylim=yrange,
#        xaxt='n', col=0, col.axis="black");
  plot(x=tcst$SCATTER_X,
       y=tcst$SCATTER_Y,
       type="n",
       xlab=xlab_str,
       ylab=ylab_str,
       main=NA, sub=subtitle_str,
       xlim=xrange,
       ylim=yrange,
       col="black", col.axis="black");
  title(main=title_str);

  par(xpd=FALSE)
  if(length(i <- grep("_WIND_", scatter_x)) && length(i <- grep("_WIND_", scatter_y))) {
    abline(0, 1, col="gray", lty=2); # Draw a 1 to 1 reference line
    cat(scatter_x," ",scatter_y," ","print1", "\n")
  } else if(length(i <- grep("AMAX_WIND", scatter_x)) && length(i <- grep("BMAX_WIND", scatter_y))) {
    abline(0, 1, col="gray", lty=2); # Draw a 1 to 1 reference line
    cat(scatter_x," ",scatter_y," ","print2", "\n")
  } else if(length(i <- grep("BMAX_WIND", scatter_x)) && length(i <- grep("AMAX_WIND", scatter_y))) {
    abline(0, 1, col="gray", lty=2); # Draw a 1 to 1 reference line
    cat(scatter_x," ",scatter_y," ","print3", "\n")
  } else {
    abline(h=0, col="gray", lty=2); # Draw a reference line at 0
    cat(scatter_x," ",scatter_y," ","print4", "\n")
  }
  par(xpd=TRUE)
  
  # Loop through the lead times in tcst$LEAD???, lookup the correct color, and call "points()" function once for each lead time

  # Get the list of colors to be used
  color_list = eval(parse(text="scatter_color_list"));

  # Check for too few colors
  if(n_series > length(color_list)) {
    cat("WARNING: The number of series (", n_series,
        ") exceeds the number of colors (", length(color_list),
        ").\n", sep='');
  }

  # Add color points for each lead time
  i = 1
  lead_list_leg <- numeric(0)
  color_list_leg <- character(0)
  pch_list_leg=""
  for(lead in lead_list) {
      
    # Create a point plot
# CWILL: for transparent dots use...
#     rgb_val = col2rgb(color_list[i])/255;
#     rgb_col = rgb(rgb_val[1], rgb_val[2], rgb_val[3], 0.25); # (or whatever transparency value)
#     points(tcst$SCATTER_X[tcst$LEAD_HR == lead], tcst$SCATTER_Y[tcst$LEAD_HR == lead], pch=1, col=rgb_col);
    points(tcst$SCATTER_X[tcst$LEAD_HR == lead], tcst$SCATTER_Y[tcst$LEAD_HR == lead],           pch=1, col=color_list[i]);
    
    if(lead %in% tcst$LEAD_HR && !all(is.na(tcst$SCATTER_Y[tcst$LEAD_HR == lead])) && !all(is.na(tcst$SCATTER_X[tcst$LEAD_HR == lead]))) {
      lead_list_leg=c(lead_list_leg,lead)
      color_list_leg=c(color_list_leg,color_list[i])
    }
    
    i = i +1
    
  } # end for lead
  # Add a legend
  legend(x="topright",
         inset=c(-0.15,0),
         legend=c(lead_list_leg),
         col=c(color_list_leg),
         pch=c(rep(1,length(lead_list_leg))),
         bty="n",
         title="Lead Times");
         
  if(footnote_flag == "TRUE") {
    makeFootnote(footnote)
  }

  # Close the output device
  dev.off();
  }
}

########################################################################
#
# Create time series of boxplots or point plots.
#
########################################################################

plot_box_point = function(dep, plot_type, horz, vert, plot_opts, cur_baseline, cur_baseline_data) {

  # Only generate a log file for boxplots
  do_log = (log_flag == TRUE && plot_type == boxplot_str);
  if(do_log) log_data = c();
  
  # Draw a reference line at 0
  abline(h=0, lty=3, lwd=2.0);

  # Loop over the series list entries
  for(i in 1:n_series) {

    # Get the current plotting information
    cur_i   = i%%length(plot_opts$col_list);
    cur_i   = ifelse(cur_i == 0, length(plot_opts$col_list), cur_i);
    cur_col = plot_opts$col_list[cur_i];
    cur_pch = plot_opts$pch_list[cur_i];
    cur_lwd = plot_opts$lwd_list[cur_i];
    cur_lty = plot_opts$lty_list[cur_i];

    # Get current subset of data
    series_data = get_series_data(series_list[i], series_plot[[i]],
                                  diff_flag[i]);

    # Add boxplots or point plots for each lead time
    for(lead in lead_list) {

      # Get data for the current lead time
      data = subset(series_data, series_data$LEAD_HR == lead &
                    !is.na(series_data$PLOT));
                    
      # Point plot
      if(plot_type == point_str || sum(!is.na(data$PLOT)) < n_min) {

        # Create a point plot
        points(rep(lead+horz[i], length(data$PLOT)), data$PLOT,
               pch=cur_pch, col=cur_col);
      }
      # Boxplot
      else if(plot_type == boxplot_str) {

        # Set the boxplot color
        bxp_col = ifelse(cur_col == "black", "white", cur_col);

        # Create a boxplot
        b = boxplot(data$PLOT,
                    add=TRUE, notch=TRUE, boxwex=1.5,
                    outpch=cur_pch, outcex=1.0,
                    at=lead+horz[i], xaxt="n", yaxt="n",
                    col=bxp_col, outcol=cur_col,
                    whiskcol=cur_col, staplecol=cur_col,
                    varwidth=TRUE);

        # Store logging information
        if(do_log) {

          # Sort the unique outlier values
          for(outlier in sort(unique(b$out), decreasing=TRUE)) {

            # Get the row(s) where this value appears
            for(row in which(data$PLOT == outlier)) {

              # Log boxplot outlier information:
              #   lead hour, model name, storm ID, initialization time,
              #   dependent variable, outlier value
              outlier_info = c(lead,
                               series_list[i],
                               as.character(data$STORM_ID)[row],
                               as.character(data$INIT)[row],
                               out_file_dep,
                               data$PLOT[row]);

              # Store the log data
              log_data = rbind(log_data, outlier_info);

            } # end for row
          } # end for outlier
        } # end if do_log
      }
      
      # Plot the mean value
      points(lead+horz[i], mean(data$PLOT, na.rm=TRUE), pch=8);

    } # end for lead
    
    # Plot the valid data counts    
    if(event_equal == FALSE || i == 1) {
      plot_valid_counts(series_data, cur_col, vert[n_series+1-i]);
    }

  } # end for i

  # Add HFIP baseline for each lead time
  for(lead in lead_list) {

    # Get data for the current lead time
    if(all(!is.na(cur_baseline_data))) {
      baseline_lead = cur_baseline_data[cur_baseline_data$LEAD/10000 == lead,]$VALUE;
    }
    else {
      baseline_lead = NA;
    }
    
    # Plot the HFIP baseline
    if(length(baseline_lead) > 0) {
#       cat(paste("baseline_lead is",baseline_lead,"for lead",lead,"\n"))
      points(lead, baseline_lead, pch=9, col="blue")
    }
    
  } # end for lead


  # Add a legend
  if(cur_baseline == "no") {
    legend(x="topleft",
           legend=c(legend_list, "Mean"),
           col=c(rep(plot_opts$col_list, n_series)[1:n_series], "black"),
           lty=c(rep(default_lty, n_series), NA),
           lwd=c(rep(default_lwd, n_series), NA),
           pch=c(rep(plot_opts$pch_list, n_series)[1:n_series], 8),
           bty="n");
  } else {
    legend(x="topleft",
         legend=c(legend_list, "Mean", cur_baseline),
         col=c(rep(plot_opts$col_list, n_series)[1:n_series], "black", "blue"),
         lty=c(rep(default_lty, n_series), NA, NA),
         lwd=c(rep(default_lwd, n_series), NA, NA),
         pch=c(rep(plot_opts$pch_list, n_series)[1:n_series], 8, 9),
         bty="n");
  }  

  # Write the log data out to the log file
  if(do_log) {
    cat(paste("Writing log file:", log_file, "\n"));

    # Check for no outliers
    if(is.null(log_data)) {
      write("LEAD_HR,AMODEL,STORM_ID,INIT,VARIABLEOUTLIER",
            file=log_file);
    }
    # Otherwise, write the data
    else {
      write.table(log_data, file=log_file, sep=',',
                  row.names=FALSE, quote=FALSE,
                  col.names=c("LEAD_HR", "AMODEL", "STORM_ID",
                              "INIT", "VARIABLE", "OUTLIER"));
    }
  }
}

########################################################################
#
# Create time series of mean or median line plots.
#
########################################################################

plot_mean_median = function(dep, plot_type, horz, vert, plot_opts, cur_baseline, cur_baseline_data) {

  # Only generate a log file for a single series of differences
  do_log = (log_flag == TRUE && sum(diff_flag) > 0);

  # Draw a reference line at 0
  abline(h=0, lty=3, lwd=2.0);

  # Loop over the series list entries
  for(i in 1:n_series) {

    # Get the current plotting information
    cur_i   = i%%length(plot_opts$col_list);
    cur_i   = ifelse(cur_i == 0, length(plot_opts$col_list), cur_i);
    cur_col = plot_opts$col_list[cur_i];
    cur_pch = plot_opts$pch_list[cur_i];
    cur_lwd = plot_opts$lwd_list[cur_i];
    cur_lty = plot_opts$lty_list[cur_i];

    # Get current subset of data
    series_data = get_series_data(series_list[i], series_plot[[i]],
                                  diff_flag[i]);

    # Compute statistics plus CI's for each lead time
    stat_val = stat_ncl = stat_ncu = rep(NA, length(lead_list));

    # Prepare the data for each lead time
    for(j in 1:length(lead_list)) {

      # Get data for the current lead time
      data = subset(series_data, series_data$LEAD_HR == lead_list[j] &
                    !is.na(series_data$PLOT));

      # Skip lead times for which no data is found
      if(dim(data)[1] == 0) next;

      # Handle the mean and median
      if(plot_type == mean_str) { s =   get_mean_ci(data$PLOT); }
      else                      { s = get_median_ci(data$PLOT); }

      # Store stats for this lead time
      stat_ncl[j] = s$ncl;
      stat_val[j] = s$val;
      stat_ncu[j] = s$ncu;

      # Store logging information
      if(do_log && diff_flag[i] && !is.na(stat_val[j])) {

        # Construct a table of logging information
        if(!exists("log_data")) {
          log_data = data.frame(row.names=c("SERIES", "LEAD_HR", "COUNT", "NONZERO",
                                            "STAT", "PC", "PVAL"));
        }

        # Get the reference statistic
        if(plot_type == mean_str) {
          stat_ref = mean(data$REF, na.rm=TRUE);
        }
        else {
          stat_ref = median(data$REF, na.rm=TRUE);
        }

        # Log the lead time, statistic, percent change, and p-value
        log_data = cbind(log_data, c(
                     series_list[i],
                     lead_list[j],
                     sum(!is.na(data$PLOT)),
                     sum(!is.na(data$PLOT) & data$PLOT != 0),
                     stat_val[j],
                     paste(100*stat_val[j]/stat_ref, "%", sep=''),
                     s$pval));
      }

    } # end for j

    # Plot the statistics
    ind = !is.na(stat_val);
    points(lead_list[ind]+horz[i], stat_val[ind],
           type='b', col=cur_col, pch=cur_pch, lty=cur_lty, lwd=cur_lwd);
           
    # Plot the confidence intervals
    if(series_ci[i]) {
      arrows(lead_list[ind]+horz[i], stat_ncl[ind],
             lead_list[ind]+horz[i], stat_ncu[ind],
             col=cur_col, lwd=cur_lwd, length=0.02, angle=90, code=3);
    
      # Indicate which differences are statistically significance
      if(diff_flag[i] == TRUE) {
        sig_pch = ifelse(cur_pch == 1, 16, cur_pch);
        sig_stat_val = stat_val[ind];
        sig_stat_val[stat_ncl[ind] <= 0 & stat_ncu[ind] >= 0] = NA;
        sig_stat_val[is.na(stat_ncl[ind]) | is.na(stat_ncu[ind])] = NA;
        points(lead_list[ind]+horz[i], sig_stat_val,
               type='p', col=cur_col, cex=1.5,
               pch=sig_pch, lty=cur_lty, lwd=cur_lwd);
      }
    }
    

    # Plot the valid data counts
    if(event_equal == FALSE || i == 1) {
      plot_valid_counts(series_data, cur_col, vert[n_series+1-i]);
    }

  } # end for i

  # Add HFIP baseline for each lead time
  for(lead in lead_list) {

    # Get data for the current lead time
    if(all(!is.na(cur_baseline_data))) {
      baseline_lead = cur_baseline_data[cur_baseline_data$LEAD/10000 == lead,]$VALUE;
    }
    else {
      baseline_lead = NA;
    }

    # Plot the HFIP baseline
    if(length(baseline_lead) > 0) {
#       cat(paste("baseline_lead is",baseline_lead,"for lead",lead,"\n"))
      points(lead, baseline_lead, pch=9, col="blue")
    }
    
  } # end for lead

  # Add a legend
  if(cur_baseline == "no") {
    legend(x="topleft",
         legend=legend_list,
         col=rep(plot_opts$col_list, n_series)[1:n_series],
         lty=rep(plot_opts$lty_list, n_series)[1:n_series],
         lwd=rep(plot_opts$lwd_list, n_series)[1:n_series],
         pch=rep(plot_opts$pch_list, n_series)[1:n_series],
         bty="n");
  } else {
    legend(x="topleft",
         legend=c(legend_list, cur_baseline),
         col=c(rep(plot_opts$col_list, n_series)[1:n_series], "blue"),
         lty=c(rep(plot_opts$lty_list, n_series)[1:n_series], NA),
         lwd=c(rep(plot_opts$lwd_list, n_series)[1:n_series], 1),
         pch=c(rep(plot_opts$pch_list, n_series)[1:n_series], 9),
         bty="n");
  }  

  # Write the log data out to the log file
  if(do_log) {
    cat(paste("Writing log file:", log_file, "\n"));
    write.table(log_data, file=log_file, sep=',',
                row.names=FALSE, col.names=FALSE, quote=FALSE);
  }
}

########################################################################
#
# Create time series of mean or median skill score line plots.
#
########################################################################

plot_skill_mean_median = function(dep, plot_type, horz, vert, plot_opts,
                                  cur_baseline, cur_baseline_data, skill_ref) {

  # Only generate a log file for a single series of differences
  do_log = (log_flag == TRUE && n_series == 1 && diff_flag[1] == TRUE);

  # Draw a reference line at 0
  abline(h=0, lty=3, lwd=2.0);

  # Loop over the series list entries
  for(i in 1:n_series) {

    # Get the current plotting information
    cur_i   = i%%length(plot_opts$col_list);
    cur_i   = ifelse(cur_i == 0, length(plot_opts$col_list), cur_i);
    cur_col = plot_opts$col_list[cur_i];
    cur_pch = plot_opts$pch_list[cur_i];
    cur_lwd = plot_opts$lwd_list[cur_i];
    cur_lty = plot_opts$lty_list[cur_i];

    # Get current subset of data
    series_data    = get_series_data(series_list[i], series_plot[[i]],
                                     diff_flag[i]);
    skill_ref_data = get_series_data(skill_ref, skill_ref,
                                     diff_flag[i]);

    # Compute statistics plus CI's for each lead time
    stat_val = stat_ncl = stat_ncu = rep(NA, length(lead_list));

    # Prepare the data for each lead time
    # Remove 0 hr skill scores as not reasonable to improve analysis by 20 or 50%    
    for(j in 2:length(lead_list)) {

      # Get data for the current lead time
      data     = subset(series_data, series_data$LEAD_HR == lead_list[j] &
                        !is.na(series_data$PLOT));

      data_ref = subset(skill_ref_data, skill_ref_data$LEAD_HR == lead_list[j] &
                        !is.na(skill_ref_data$PLOT));

      # Skip lead times for which no data is found
      if(dim(data)[1] == 0) next;

      # Handle the mean and median
      if(plot_type == skillmn_str) {
        # Compute the statistic
        s        = c();
        s_mean   = mean(data$PLOT, na.rm=TRUE);
        ref_mean = mean(data_ref$PLOT); 
        s$val    = round(100*(ref_mean - s_mean)/ref_mean,0)
      } else                    {
        # Compute the statistic
        s          = c();
        s_median   = median(data$PLOT, na.rm=TRUE);
        ref_median = median(data_ref$PLOT); 
        s$val      = round(100*(ref_median - s_median)/ref_median,0)
      }

      # Store stats for this lead time
#       stat_ncl[j] = s$ncl;
      stat_val[j] = s$val;
#       stat_ncu[j] = s$ncu;

      # Store logging information
      if(do_log && !is.na(stat_val[j])) {

        # Construct a table of logging information
        if(!exists("log_data")) {
          log_data = data.frame(row.names=c("LEAD_HR", "COUNT", "NONZERO",
                                            "STAT"));
        }

        # Log the lead time, statistic, percent change, and p-value
        log_data = cbind(log_data, c(
                     lead_list[j],
                     sum(!is.na(data$PLOT)),
                     sum(!is.na(data$PLOT) & data$PLOT != 0),
                     stat_val[j]));
      }

    } # end for j

    # Plot the statistics
    ind = !is.na(stat_val);
    points(lead_list[ind]+horz[i], stat_val[ind],
           type='b', pch=cur_pch, lty=cur_lty, lwd=cur_lwd, col=cur_col);

    # Plot the valid data counts
    if(event_equal == FALSE || i == 1) {
      plot_valid_counts(series_data, cur_col, vert[n_series+1-i]);
    }

  } # end for i

  # Add HFIP baseline for each lead time
  for(lead in lead_list) {
    if(lead != "0") {
      # Get data for the current lead time
      if(all(!is.na(cur_baseline_data))) {
        baseline_lead = round(100*(cur_baseline_data[cur_baseline_data$LEAD/10000 == lead & cur_baseline_data$TYPE == "OCD5",]$VALUE-cur_baseline_data[cur_baseline_data$LEAD/10000 == lead & cur_baseline_data$TYPE == "CONS",]$VALUE)/cur_baseline_data[cur_baseline_data$LEAD/10000 == lead & cur_baseline_data$TYPE == "OCD5",]$VALUE,1)
      }
      else {
        baseline_lead = NA;
      }

      # Plot the HFIP baseline
      if(length(baseline_lead) > 0) {
        cat(paste("baseline_lead is",baseline_lead,"for lead",lead,"\n"))
        points(lead, baseline_lead, pch=9, col="blue")
      }
    }
    
  } # end for lead

  # Add a legend
  if(cur_baseline == "no") {
    legend(x="topleft",
         legend=legend_list,
         col=rep(plot_opts$col_list, n_series)[1:n_series],
         lty=rep(plot_opts$lty_list, n_series)[1:n_series],
         lwd=rep(plot_opts$lwd_list, n_series)[1:n_series],
         pch=rep(plot_opts$pch_list, n_series)[1:n_series],
         bty="n");
  } else {
    legend(x="topleft",
         legend=c(legend_list, cur_baseline),
         col=c(rep(plot_opts$col_list, n_series)[1:n_series], "blue"),
         lty=c(rep(plot_opts$lty_list, n_series)[1:n_series], NA),
         lwd=c(rep(plot_opts$lwd_list, n_series)[1:n_series], 1),
         pch=c(rep(plot_opts$pch_list, n_series)[1:n_series], 9),
         bty="n");
  }  

  # Write the log data out to the log file
  if(do_log) {
    cat(paste("Writing log file:", log_file, "\n"));
    write.table(log_data, file=log_file, sep=',',
                row.names=FALSE, col.names=FALSE, quote=FALSE);
  }
}

########################################################################
#
# Create time series of relative performance.
#
########################################################################

plot_relperf = function(dep, horz, vert, plot_opts) {

  # Check that event equalization has been applied
  if(event_equal == FALSE) {
    cat(paste("ERROR: Cannot plot relative performance when event",
              "equalization is disabled.\n"));
    quit(status=1);
  }

  # Check that series_list equals series_plot
  for(i in 1:n_series) {
    if(series_list[i] != series_plot[[i]]) {
      cat(paste("ERROR: Cannot plot relative performance when using",
                "series aggregations.\n"));
      quit(status=1);
    }
  } # end for i

  # Draw a reference line at 0 and 100
  abline(h=c(0, 100), lwd=2.0, col="gray");
  
  # Get the case data
  case_data = get_case_data();

  # Loop over the ties followed by the the series list entries
  for(i in 0:n_series) {

    # Set variables for plotting the ties:
    #   color, series value, and horizontal offset
    if(i == 0) {
      cur_col    = "gray";
      series_val = "TIE";
      h_off      = 0;
    }
    else {
    
      # Get the current plotting information
      cur_i      = i%%length(plot_opts$col_list);
      cur_i      = ifelse(cur_i == 0, length(plot_opts$col_list), cur_i);
      cur_col    = plot_opts$col_list[cur_i];
      series_val = series_list[i];
      h_off      = horz[i];
    }

    # Compute statistic for each lead time
    stat_val = stat_ncl = stat_ncu = rep(NA, length(lead_list));

    # Prepare the data for each lead time
    for(j in 1:length(lead_list)) {

      ind = (case_data$LEAD_HR == lead_list[j]);

      # Get counts
      n_cur = sum(case_data$PLOT[ind] == series_val, na.rm=TRUE);
      n_tot = sum(!is.na(case_data$PLOT[ind]));

      # Compute the current relative performance and CI
      s = get_prop_ci(n_cur, n_tot);

      # Store stats for this lead time
      stat_ncl[j] = s$ncl;
      stat_val[j] = s$val;
      stat_ncu[j] = s$ncu;

    } # end for j

    # Plot the relative performance
    ind = !is.na(stat_val);
    points(lead_list[ind]+h_off, stat_val[ind],
           pch=8, type='b', col=cur_col);

    # Plot relative performance confidence intervals
    if(i == 0 || series_ci[i]) {
      points(lead_list[ind]+h_off, stat_ncl[ind],
             type='l', lty=3, col=cur_col);
      points(lead_list[ind]+h_off, stat_ncu[ind],
             type='l', lty=3, col=cur_col);
    }

    # Plot the valid data counts
    if(i == 1) { plot_valid_counts(case_data, cur_col, vert[n_series+1-i]); }

  } # end for i

  # Plot threshold information across the top
  if(length(unique(rp_diff_list)) > 1) {
     axis(3, at=lead_list, tick=FALSE, padj=vert[2], cex.axis=0.75,
          labels=paste(rp_diff_list, col$units, sep=''));
  }

  # Legend for relative performance
  legend_str = c(paste(legend_list, "Better"), "TIE");
  if(series_ci[i]) legend_str = c(legend_str, paste(100*(1-alpha), "% CI", sep=''));
  legend(x="topleft",
         legend=legend_str,
         col=c(plot_opts$col_list[1:n_series], "gray", "gray"),
         lty=c(rep(1, n_series+1), 3),
         lwd=2, pch=c(rep(8, n_series+1), NA), bty="n");
}

########################################################################
#
# Create time series of rank frequency.
#
########################################################################

plot_rank = function(dep, horz, vert, plot_opts) {

  # Check that event equalization has been applied
  if(event_equal == FALSE) {
    cat(paste("ERROR: Cannot plot relative rank frequency when event",
              "equalization is disabled.\n"));
    quit(status=1);
  }

  # Check that series_list equals series_plot
  for(i in 1:n_series) {
    if(series_list[i] != series_plot[[i]]) {
      cat(paste("ERROR: Cannot plot rank frequencey when using series",
                "aggregations.\n"));
      quit(status=1);
    }
  } # end for i

  # Draw a reference line at 100/n_series
  abline(h=100/n_series, lwd=2.0, col="gray");

  # Get the case data
  case_data = get_case_data();

  # Loop over the series list entries
  for(i in 1:n_series) {

    # Get the current plotting information
    cur_i   = i%%length(plot_opts$col_list);
    cur_i   = ifelse(cur_i == 0, length(plot_opts$col_list), cur_i);
    cur_col = plot_opts$col_list[cur_i];

    # Compute statistic for each lead time
    stat_val = stat_ncl = stat_ncu = rep(NA, length(lead_list));
    rank_min_val = rep(NA, length(lead_list));

    # Prepare the data for each lead time
    for(j in 1:length(lead_list)) {

      ind = (case_data$LEAD_HR == lead_list[j]);

      # Get counts
      n_cur = sum(case_data$RANK_RANDOM[ind] == i, na.rm=TRUE);
      n_tot = sum(!is.na(case_data$RANK_RANDOM[ind]));

      # Compute the current rank value's frequency and CI
      s = get_prop_ci(n_cur, n_tot);

      # Store stats for this lead time
      stat_ncl[j] = s$ncl;
      stat_val[j] = s$val;
      stat_ncu[j] = s$ncu;

      # Compute the RANK_MIN value for the first and last series
      if(i == 1 || i == n_series) {
        rank_min_val[j] = 100*
          sum(case_data$RANK_MIN[ind] == i, na.rm=TRUE)/
          sum(!is.na(case_data$RANK_MIN[ind]));
      }
    } # end for j

    # Plot the statistics
    ind = !is.na(stat_val);
    points(lead_list[ind]+horz[i], stat_val[ind],
           pch=as.character(i), type='b', col=cur_col);

    # Plot rank confidence intervals
    if(series_ci[i]) {
      points(lead_list[ind]+horz[i], stat_ncl[ind],
             type='l', lty=3, col=cur_col);
      points(lead_list[ind]+horz[i], stat_ncu[ind],
             type='l', lty=3, col=cur_col);
    }

    # For the BEST and WORST series, plot the RANK_MIN values
    if(i == 1 || i == n_series) {
       points(lead_list[ind]+horz[i], rank_min_val[ind],
              pch=as.character(i), type='p', col="black");
    }

    # Plot the valid data counts
    if(i == 1) { plot_valid_counts(case_data, cur_col, vert[n_series+1-i]); }

  } # end for i

  # Legend for rank frequency
  rank_str   = c("Best", "2nd", "3rd", "Worst");
  legend_str = c(rank_str[1:min(3, n_series-1)]);
  if(n_series >= 5) legend_str = c(legend_str, paste(4:(n_series-1), "th", sep=''));
  legend_str = c(legend_str, rank_str[4]);
  if(series_ci[i]) legend_str = c(legend_str, paste(100*(1-alpha), "% CI", sep=''));
  legend(x="topleft",
         legend=legend_str,
         col=c(rep(plot_opts$col_list, n_series)[1:n_series], "gray"),
         lty=c(rep(1, n_series), 3),
         lwd=rep(2, n_series),
         pch=c(as.character(1:n_series), NA), bty="n");
}

########################################################################
#
# Plot the valid data counts.
#
########################################################################

plot_valid_counts = function(data, color, vert) {

  # Reset the color to black for event equalized data
  color = ifelse(event_equal, "black", color);

  # Aggregate the values to be plotted by lead hour
  d = aggregate(!is.na(data$PLOT), list(data$LEAD_HR), sum);

  # Initialize valid counts
  n = rep(0, length(lead_list));

  # Store valid counts for the correct lead hour
  n[which(lead_list%in%d$Group.1)] = d$x

  # Plot valid counts on the top axis
  axis(3, at=lead_list, tick=FALSE, labels=n,
       padj=vert, cex.axis=0.75, col.axis=color);
}

