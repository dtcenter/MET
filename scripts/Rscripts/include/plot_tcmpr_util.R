########################################################################
#
# Get a column of data to be plotted, handling absolute values and
# differences.
#
########################################################################

get_dep_column = function(dep) {

  # Check for aboslute value
  abs_flag = (substring(dep, 1, 3) == "ABS")

  # Strip off the absolute value
  if(abs_flag) {
    dep = unlist(strsplit(dep, "[(|)]"))[2]
  }

  # Split based on differences
  diff_list = unlist(strsplit(dep, "[-]"))

  # Initialize output
  col       = c()
  col$val   = tcst[,diff_list[1]]
  col$desc  = column_info[diff_list[1], "DESCRIPTION"]
  col$units = column_info[diff_list[1], "UNITS"]

  # Loop over any remaining entries
  i = 2
  while(i <= length(diff_list)) {
    col$val   = col$val - tcst[,diff_list[i]]
    col$desc  = paste(col$desc, "-",
                      column_info[diff_list[i], "DESCRIPTION"])
    col$units = paste(col$units, "-",
                      column_info[diff_list[i], "UNITS"])
    i = i+1
  }

  # Apply absolute value
  if(abs_flag) {
    col$val  = abs(col$val)
    col$desc = paste("Absolute Value of", col$desc)
  }

  return(col)
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
    series_data = tcst[tcst[,series] == cur_plot[1],]

    # Loop over the remaining series entries
    i = 2
    while(i <= length(cur_plot)) {
      series_diff = tcst[tcst[,series] == cur_plot[i],]

      # Sanity check the dimensions
      if(dim(series_data)[1] != dim(series_diff)[1]) {
        cat("ERROR: When computing series differences for",
            cur, "the dimensions do not match:",
            dim(series_data), "!=", dim(series_diff))
        quit(1)
      }

      # Compute series difference
      series_data$PLOT = series_data$PLOT - series_diff$PLOT

      i = i+1
    } # end while
  }
  # Otherwise, just subset based on a single series entry
  else {
    series_data = tcst[tcst[,series]%in%cur_plot,]
  }

  # Subset based on requested lead times
  series_data = series_data[series_data$LEAD_HR%in%lead_list,]
  
  return(series_data)
}

########################################################################
#
# Build a table with summary information for each case.
#
########################################################################

get_case_data = function() {

  # Initialize
  series_data = c();

  # Get the data for all series to be used in order
  for(i in 1:length(series_list)) {
    series_data =
      rbind(series_data,
            get_series_data(series_list[i], series_list[i], FALSE));
  } # end for i

  # Define a case column
  series_data$CASE = paste(series_data$BMODEL,
                           series_data$STORM_ID,
                           series_data$INIT,
                           series_data$LEAD_HR,
                           series_data$VALID,
                           sep=':');

  # Build a set of unique cases
  case_data = unique(data.frame(CASE=series_data$CASE,
                                LEAD_HR=series_data$LEAD_HR,
                                MIN=NA, MAX=NA, DIFF=NA, WIN=NA, TEST=NA,
                                RES=NA, PLOT=NA, RANK=NA));

  # Check for equal numbers of entries for each case
  if(sum(aggregate(series_data$PLOT, by=list(series_data$CASE), length)$x != n_series)) {
      cat(paste("ERROR: Must have the same number of entries for each case.\n"));
      quit(status=1);
  }

  # Loop through the cases
  for(i in 1:dim(case_data)[1]) {

    # Build indicator for the current case
    ind = (series_data$CASE == case_data$CASE[i]);

    # Check for the expected number of entries
    if(sum(ind) != n_series) {
      cat(paste("ERROR: Unexpected number of entries for case",
                case_data$CASE[i], ".\n"));
      cat(series_data[ind,]);
      quit(status=1);
    }

    # Compute the winner for each case
    case_data$MIN[i]  = min(series_data[ind,]$PLOT);
    case_data$MAX[i]  = max(series_data[ind,]$PLOT);
    case_data$DIFF[i] = case_data$MAX[i] - case_data$MIN[i];
    case_data$WIN[i]  = as.character(series_data[ind,series][which.min(series_data[ind,]$PLOT)]);
    case_data$TEST[i] = paste(case_data$DIFF[i], rp_thresh, sep='');
    case_data$RES[i]  = eval(parse(text=case_data$TEST[i]));
    case_data$PLOT[i] = ifelse(case_data$RES[i], case_data$WIN[i], "TIE");
    case_data$RANK[i] = rank(series_data[ind,]$PLOT, na.last="keep", ties.method="random")[1];
  }

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
  bound  = (zval * ((phat * (1 - phat) + (zval^2)/(4 * n))/n)^(1/2))/(1 + (zval^2)/n);
  midpnt = (phat + (zval^2)/(2 * n))/(1 + (zval^2)/n);

  # Return the statistic and confidence interval
  return(100*c(round(midpnt - bound, 4),
               round(phat,           4),
               round(midpnt + bound, 4)));
}

########################################################################
#
# Compute a confidence interval about the mean.
# When diff is false, use the bonferroni correction.
#
########################################################################

get_mean_ci = function(d, diff) {

  # Compute the standard error
  s = Compute_STDerr_from_mean(d, "ML");
  if(length(s) > 1 && s[2] == 0) {
    stderr = ifelse(diff, round(zval*s[1], 1),
                          round(zval_bonferroni*s[1], 1));
  }
  else {
    stderr = 0;
  }

  # Compute the statistic
  stat = mean(d, na.rm=TRUE);

  # Return the statistic and confidence interval
  return(c(round(stat - stderr, 1), stat, round(stat + stderr, 1)));
}

########################################################################
#
# Compute a confidence interval about the median.
# When diff is false, use the bonferroni correction.
#
########################################################################

get_median_ci = function(d, diff) {

  # Compute the standard error
  s = Compute_STDerr_from_median(d, "ML");
  if(length(s) > 1 && s[2] == 0) {
    stderr = ifelse(diff, round(zval*s[1], 1),
                          round(zval_bonferroni*s[1], 1));
  }
  else {
    stderr = 0;
  }

  # Compute the statistic
  stat = median(d, na.rm=TRUE);

  # Return the statistic and confidence interval
  return(c(round(stat - stderr, 1), stat, round(stat + stderr, 1)));
}

########################################################################
#
# Get the range of the data based on the plot type.
#
########################################################################

get_yrange = function(plot_type) {

  # Initialize
  ylim = c(NA,NA)

  # Loop over the series list entries
  for(i in 1:n_series) {

    # Get current subset of data
    series_data = get_series_data(series_list[i], series_plot[[i]],
                                  diff_flag[i])

    # Initialize
    cur = c();

    # Get the data range based on plot type
    if(plot_type == mean_str || plot_type == median_str) {

      for(j in 1:length(lead_list)) {

        # Get data for the current lead time
        data = subset(series_data, series_data$LEAD_HR == lead_list[j] &
                      !is.na(series_data$PLOT));

        # Skip lead times for which no data is found
        if(dim(data)[1] == 0) next;

        # Append the plotting limits for each lead time
        if(plot_type == mean_str) { cur = c(cur,   get_mean_ci(data$PLOT, diff_flag[i])); }
        else                      { cur = c(cur, get_median_ci(data$PLOT, diff_flag[i])); }

      } # end for j
    }
    else if(plot_type == relperf_str || plot_type == rank_str) {

      # Get the case data
      case_data = get_case_data();

      for(j in 1:length(lead_list)) {

        ind = (case_data$LEAD_HR == lead_list[j]);

        # Append the plotting limits for each lead time
        if(plot_type == relperf_str) {
          cur = c(cur, round(100*sum(case_data$PLOT[ind] ==
                                     series_list[i])/sum(ind), 0));
        }
        else {

          # Get counts
          n_cur = sum(case_data$RANK[ind] == i);
          n_tot = sum(ind);

          # Compute the current rank value's frequency and CI
          cur = c(cur, get_prop_ci(n_cur, n_tot));
        }

      } # end for j
    }
    else {
      cur = range(series_data$PLOT, na.rm=TRUE)
    }

    # Update the plotting limits
    ylim = range(c(ylim, cur), na.rm=TRUE);

  } # end for i

  return(ylim);
}

########################################################################
#
# Plot time-series of data.
#
########################################################################

plot_time_series = function(dep, plot_type,
                            title_str, subtitle_str, ylab_str) {

  cat("Plotting", plot_type, "time series by", series, "\n")

  # Open the output device
  cat(paste("Creating file:", out_file, "\n"))
  bitmap(out_file, type=img_fmt,
         height=img_hgt, width=img_wdth, res=img_res)

  # Compute the series offsets
  hoff = ifelse(plot_type == relperf_str ||
                plot_type == rank_str, relperf_horz_offset, horz_offset);
  horz = (seq(1, n_series) - n_series/2 - 0.5)*hoff;
  vert = seq(2.0, 2.0-(n_series*vert_offset), by=-1.0*vert_offset)

  # Set the range for the Y-axis
  if(!is.na(ymin) & !is.na(ymax)) { yrange = c(ymin, ymax);         }
  else                            { yrange = get_yrange(plot_type); }

  cat(paste("Range of ", dep, ":", sep=''),
      paste(yrange, collapse=", "), "\n")
  
  # Create an empty plot
  top_mar = ifelse(event_equal, 4, 4+floor(n_series/2))
  par(mfrow=c(1,1), mar=c(5, 4, top_mar, 2), cex=1.5)
  plot(x=seq(0, max(lead_list), 6), type="n",
       xlab="Lead Time (h)",
       ylab=ylab_str,
       main=NA, sub=subtitle_str,
       xlim=c(0+min(horz), max(lead_list)+max(horz)),
       ylim=yrange,
       xaxt='n', col=0, col.axis="black")
  title(main=title_str, line=top_mar-2)

  # Draw the X-axis
  axis(1, at=lead_list, tick=TRUE, labels=lead_list)

  # Draw a reference line
  if(plot_type == rank_str) { abline(h=100/n_series, lwd=2.0, col="gray"); }
  else                      { abline(h=0, lty=3, lwd=2.0);                 }

  # Check for too few colors
  if(n_series > length(color_list)) {
    cat("WARNING: The number of series (", n_series,
        ") exceeds the number of colors (", length(color_list),
        ").\n", sep='')
  }

  # Populate the plot based on plot type
  if(plot_type == boxplot_str ||
     plot_type == scatter_str) {
    plot_box_scatter(dep, plot_type, horz, vert)
  }
  else if(plot_type == mean_str ||
          plot_type == median_str) {
    plot_mean_median(dep, plot_type, horz, vert)
  }
  else if(plot_type == relperf_str ||
          plot_type == rank_str) {
    plot_relperf_rank(dep, plot_type, horz, vert)
  }

  # Close the output device
  dev.off()
}

########################################################################
#
# Create time series of boxplots or scatter plots.
#
########################################################################

plot_box_scatter = function(dep, plot_type, horz, vert) {

  # Loop over the series list entries
  for(i in 1:n_series) {

    # Get the color index
    i_col = i%%length(color_list);
    if(i_col == 0) {
      color = color_list[length(color_list)];
    }
    else {
      color = color_list[i_col];
    }

    # Get current subset of data
    series_data = get_series_data(series_list[i], series_plot[[i]],
                                  diff_flag[i]);

    # Add boxplots or scatter plots for each lead time
    for(lead in lead_list) {

      # Scatter plot
      if(plot_type == scatter_str ||
         sum(!is.na(series_data$PLOT[series_data$LEAD_HR==lead])) < n_min) {

         # Create a scatter plot
         points(rep(lead+horz[i],
                length(series_data$PLOT[series_data$LEAD_HR==lead])),
                series_data$PLOT[series_data$LEAD_HR==lead],
                pch=1, col=color);
       }
       # Boxplot
       else if(plot_type == boxplot_str) {

         # Set the boxplot color
         bxp_col = ifelse(color == "black", "white", color);
         if(color == "black") { bxp_col = "white"; }
         else                 { bxp_col = color;   }

         # Create a boxplot
         b = boxplot(series_data$PLOT[series_data$LEAD_HR==lead],
                     add=TRUE, notch=TRUE, boxwex=1.5, outpch=1, outcex=1.0,
                     at=lead+horz[i], xaxt="n", yaxt="n", col=bxp_col,
                     varwidth=TRUE);
       }

       # Plot the mean value
       points(lead+horz[i],
              mean(series_data$PLOT[series_data$LEAD_HR==lead], na.rm=TRUE),
              pch=8);

    } # end for lead
    
    # Plot the valid data counts    
    if(event_equal == FALSE || i == 1)
      plot_valid_counts(series_data, color, vert[i]);

  } # end for i

  # Add a legend
  legend(x="topleft",
         legend=c(series_list, "Mean"),
         col=c(rep(color_list, n_series)[1:n_series], "black"),
         lty=c(rep(1, n_series),NA),
         lwd=c(rep(2, n_series),NA),
         pch=c(rep(NA,n_series),8),
         bty="n");
}

########################################################################
#
# Create time series of mean or median line plots.
#
########################################################################

plot_mean_median = function(dep, plot_type, horz, vert) {

  # Loop over the series list entries
  for(i in 1:n_series) {

    # Get the color index
    i_col = i%%length(color_list)
    if(i_col == 0) {
      color = color_list[length(color_list)]
    }
    else {
      color = color_list[i_col]
    }

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
      if(plot_type == mean_str) { s =   get_mean_ci(data$PLOT, diff_flag[i]); }
      else                      { s = get_median_ci(data$PLOT, diff_flag[i]); }

      # Store stats for this lead time
      stat_ncl[j] = s[1];
      stat_val[j] = s[2];
      stat_ncu[j] = s[3];

    } # end for j

    # Plot the statistics
    ind = !is.na(stat_val)
    points(lead_list[ind]+horz[i], stat_val[ind],
           pch=8, type='b', col=color)

    # Plot the confidence intervals
    arrows(lead_list[ind]+horz[i], stat_ncl[ind],
           lead_list[ind]+horz[i], stat_ncu[ind],
           col=color, length=0.02, angle=90, code=3, lwd=2.0)

    # Plot the valid data counts
    if(event_equal == FALSE || i == 1)
      plot_valid_counts(series_data, color, vert[i]);

  } # end for i

  # Add a legend
  legend(x="topleft",
         legend=series_list,
         col=rep(color_list, n_series)[1:n_series],
         lty=rep(1, n_series),
         lwd=rep(2, n_series),
         pch=8, bty="n")
}

########################################################################
#
# Create time series of relative performance or rank frequency.
#
########################################################################

plot_relperf_rank = function(dep, plot_type, horz, vert) {

  # Check that event equalization has been applied
  if(event_equal == FALSE) {
    cat(paste("ERROR: Cannot plot relative performance or rank",
              "frequency when event equalization is disabled.\n"));
    quit(status=1);
  }

  # Check that series_list equals series_plot
  for(i in 1:n_series) {
    if(series_list[i] != series_plot[[i]]) {
      cat(paste("ERROR: Cannot plot relative performance or rank",
                "frequencey when using series aggregations.\n"));
      quit(status=1);
    }
  } # end for i
  
  # Get the case data
  case_data = get_case_data();
  
  # Loop over the series list entries
  for(i in 1:n_series) {

    # Get the color index
    i_col = i%%length(color_list)
    color = ifelse(i_col == 0, color_list[length(color_list)],
                               color_list[i_col]);

    # Compute statistic for each lead time
    stat_val = stat_ncl = stat_ncu = rep(NA, length(lead_list));

    # Prepare the data for each lead time
    for(j in 1:length(lead_list)) {

      ind = (case_data$LEAD_HR == lead_list[j]);
      
      # Handle the relative performance
      if(plot_type == relperf_str) {

        # Compute the current model's winning frequency
        stat_val[j] = round(100*sum(case_data$PLOT[ind] ==
                                    series_list[i])/sum(ind), 0);
      }
      # Handle the rank frequency
      else {

        # Get counts
        n_cur = sum(case_data$RANK[ind] == i);
        n_tot = sum(ind);

        # Compute the current rank value's frequency and CI
        s = get_prop_ci(n_cur, n_tot);

        # Store stats for this lead time
        stat_ncl[j] = s[1];
        stat_val[j] = s[2];
        stat_ncu[j] = s[3];
      }
    } # end for j

    # Plot the statistics
    ind = !is.na(stat_val);
    pch = ifelse(plot_type == relperf_str, 8, as.character(i));
    points(lead_list[ind]+horz[i], stat_val[ind],
           pch=pch, type='b', col=color);

    # Plot rank confidence intervals
    if(plot_type == rank_str) {
      points(lead_list[ind]+horz[i], stat_ncl[ind],
             type='l', lty=3, col=color);
      points(lead_list[ind]+horz[i], stat_ncu[ind],
             type='l', lty=3, col=color);
    }

    # Plot the valid data counts
    if(event_equal == FALSE || i == 1)
      plot_valid_counts(case_data, color, vert[i]);

  } # end for i

  # Legend for relative performance
  if(plot_type == relperf_str) {
    legend(x="topleft",
           legend=paste(series_list, "Better"),
           col=rep(color_list, n_series)[1:n_series],
           lty=rep(1, n_series),
           lwd=rep(2, n_series),
           pch=8, bty="n");
  }
  # Legend for rank frequency
  else {
    rank_str   = c("Best", "2nd", "3rd", "Worst");
    legend_str = c(rank_str[1:min(3, n_series-1)]);
    if(n_series >= 5) legend_str = c(legend_str, paste(4:(n_series-1), "th", sep=''));
    legend_str = c(legend_str, rank_str[4], paste(100*(1-alpha), "% CI", sep=''));
    legend(x="topleft",
           legend=legend_str,
           col=c(rep(color_list, n_series)[1:n_series], "gray"),
           lty=c(rep(1, n_series), 3),
           lwd=rep(2, n_series),
           pch=c(as.character(1:n_series), NA), bty="n");
  }
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
  d = aggregate(!is.na(data$PLOT), list(data$LEAD_HR), sum)

  # Initialize valid counts
  n = rep(0, length(lead_list))

  # Store valid counts for the correct lead hour
  n[which(lead_list%in%d$Group.1)] = d$x

  # Plot valid counts on the top axis
  axis(3, at=lead_list, tick=FALSE, labels=n,
       padj=vert, cex.axis=0.75, col.axis=color)
}

########################################################################
#
# Function used for outputting outlier case info for each lead time.
#
########################################################################

boxplot_outlier = function(a, b, c, d, e, f) {

   a_tmp  = unique(a)
   a_stuq = sort(a_tmp, decreasing = TRUE)

   lines_onePlus = -1
   len_b_out = length(a_stuq)
   for(p in 1:length(a_stuq)) {
      row_ind_mNUM_StormList <- which(b[,j+2]==a_stuq[p])
      len_ind_mNUM_StormList = length(row_ind_mNUM_StormList)
      lines_onePlus = lines_onePlus + len_ind_mNUM_StormList
   }
   lines = lines_onePlus + len_b_out*2
   cat(paste(lines, c, d,e[j],f,"(outlier values and case info)"))
   cat(paste("\n"))
   for(p in 1:length(a_stuq)) {
      row_ind_mNUM_StormList <- which(b[,j+2]==a_stuq[p])
      print(b[row_ind_mNUM_StormList,c(1,2,j+2)])
      cat(paste("\n"))
   }

}

boxplot_outlier_mdiff = function(a, b, c, d, e, f, g, h, i, k) {

   a_tmp  = unique(a)
   a_stuq = sort(a_tmp, decreasing = TRUE)

   lines_onePlus = -1
   lines_mNUM1 = -1
   lines_mNUM2 = -1
   len_b_out = length(a_stuq)
   for(p in 1:length(a_stuq)) {
#       row_ind_mNUM1_StormList <- which(g[,j+2]==a[p])
#       len_ind_mNUM1_StormList = length(row_ind_mNUM1_StormList)
#       row_ind_mNUM2_StormList <- which(i[,j+2]==a[p])
#       len_ind_mNUM2_StormList = length(row_ind_mNUM2_StormList)
#       lines_onePlus = lines_onePlus + len_ind_mNUM1_StormList + len_ind_mNUM2_StormList
#       lines_mNUM1= lines_mNUM1 + len_ind_mNUM1_StormList
#       lines_mNUM2= lines_mNUM2 + len_ind_mNUM2_StormList
      row_ind_StormList <- which(b[,j]==a_stuq[p])
      len_ind_StormList = length(row_ind_StormList)
      lines_onePlus = lines_onePlus + len_ind_StormList*2
   }
   lines = lines_onePlus + len_b_out*5
   cat(paste(lines, h,"-",k,"= mdiff", d,e[j],f,"(outlier values and case info)"))
   cat(paste("\n"))
   for(p in 1:length(a_stuq)) {
      lines_onePlus = -1
      row_ind_StormList <- which(b[,j]==a_stuq[p])
      len_ind_StormList = length(row_ind_StormList)
      lines_onePlus = lines_onePlus + len_ind_StormList*2
      lines_indv = lines_onePlus + 4

      row_ind_StormList <- which(b[,j]==a_stuq[p])
      cat(paste(lines_indv,"!",p, "Individual Outlier Line Count"))
      cat(paste("\n"))
      print(g[row_ind_StormList,c(1,2,j+2)])
      print(i[row_ind_StormList,c(1,2,j+2)])
      print(b[row_ind_StormList,c(j)])
      cat(paste("\n"))
   }

}
