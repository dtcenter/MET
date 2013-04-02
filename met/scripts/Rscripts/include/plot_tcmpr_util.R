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

  # Sort the data subset by INIT and VALID columns
  series_data = series_data[with(series_data, order(INIT,VALID)),]
  
  return(series_data)
}

########################################################################
#
# Compute a confidence interval about the mean.
#
########################################################################

get_mean_ci = function(d) {

  # Compute the standard error
  s = Compute_STDerr_from_mean(d, "ML");
  if(length(s) > 1 && s[2] == 0) { stderr = round(zval_bonferroni*s[1], 1); }
  else                           { stderr = 0;                              }

  # Compute the statistic
  stat = mean(d, na.rm=TRUE);

  # Return the statistic and confidence interval
  return(data.frame(val=stat,
                    ncl=round(stat + stderr, 1),
                    ncu=round(stat - stderr, 1)));
}

########################################################################
#
# Compute a confidence interval about the median.
#
########################################################################

get_median_ci = function(d) {

  # Compute the standard error
  s = Compute_STDerr_from_median(d, "ML");
  if(length(s) > 1 && s[2] == 0) { stderr = round(zval_bonferroni*s[1], 1); }
  else                           { stderr = 0;                              }

  # Compute the statistic
  stat = median(d, na.rm=TRUE);

  # Return the statistic and confidence interval
  return(data.frame(val=stat,
                    ncl=round(stat + stderr, 1),
                    ncu=round(stat - stderr, 1)));

  return(stat);
}

########################################################################
#
# Update the range of the data based on the plot type.
#
########################################################################

set_range = function(series_data, plot_type, yrange) {

  # Get the data range based on plot type
  if(plot_type == mean_str || plot_type == median_str) {

    # Initialize
    d = c();

    for(i in 1:length(lead_list)) {

      # Get data for the current lead time
      data = subset(series_data, series_data$LEAD_HR == lead_list[i] &
                    !is.na(series_data$PLOT));

      # Skip lead times for which no data is found
      if(dim(data)[1] == 0) next;

      # Append the plotting limits for each lead time
      if(plot_type == mean_str) { d = c(d, get_mean_ci(data$PLOT));   }
      else                      { d = c(d, get_median_ci(data$PLOT)); }
    } # end for i
  
    sub_yrange = range(d, na.rm=TRUE);
  }
  else {
    sub_yrange = range(series_data$PLOT, na.rm=TRUE)
  }

  # Update the plotting limits
  if(is.na(yrange[1]) || sub_yrange[1] < yrange[1]) {
    yrange[1] = sub_yrange[1]
  }
  if(is.na(yrange[2]) || sub_yrange[2] > yrange[2]) {
    yrange[2] = sub_yrange[2]
  }
      
  return(yrange)
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
  horz = (seq(1, n_series) - n_series/2 - 0.5)*horz_offset
  vert = seq(2.0, 2.0-(n_series*vert_offset), by=-1.0*vert_offset)

  # Set the range for the Y-axis
  if(!is.na(ymin) & !is.na(ymax)) {
    yrange = c(ymin, ymax)
  }
  # Determine the plotting range for the data
  else {

    # Initialize
    yrange = c(NA,NA)

    # Loop over the series list entries
    for(i in 1:n_series) {

      # Get current subset of data
      series_data = get_series_data(series_list[i], series_plot[[i]],
                                    diff_flag[i])

      # Update the plotting range
      yrange = set_range(series_data, plot_type, yrange)

    } # end for i
  } # end else

  cat(paste("Range of ", dep, ":", sep=''),
      paste(yrange, collapse=", "), "\n")

  # Create an empty plot
  par(mfrow=c(1,1), mai=c(1.5, 1.5, 2.0, 0.5), cex=1.5)
  plot(x=seq(0, max(lead_list), 6), type="n",
       xlab="Lead Time (h)",
       ylab=ylab_str,
       main=title_str,
       sub=subtitle_str,
       xlim=c(0+min(horz), max(lead_list)+max(horz)),
       ylim=yrange,
       xaxt='n', col=0, col.axis="black")

  # Draw the X-axis
  axis(1, at=lead_list, tick=TRUE, labels=lead_list)
  abline(h=0, lty=3, lwd=2.0)

  # Check for too few colors
  if(n_series > length(color_list)) {
    cat("WARNING: The number of series (", n_series,
        ") exceeds the number of colors (", length(color_list),
        ").\n", sep='')
  }

  # Populate the plot based on plot type
  if(plot_type == boxplot_str || plot_type == scatter_str) {
    plot_box_scatter(dep, plot_type, horz, vert)
  }
  else if(plot_type == mean_str || plot_type == median_str) {
    plot_mean_median(dep, plot_type, horz, vert)
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

    # Set the valid data counts color
    if(event_equal == TRUE) { vld_col = "black"; }
    else                    { vld_col = color;   }
    
    # Plot the valid data counts    
    if(event_equal == FALSE || i == 1)
      plot_valid_counts(series_data, vld_col, vert[i]);

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
      if(plot_type == mean_str) { s = get_mean_ci(data$PLOT);   }
      else                      { s = get_median_ci(data$PLOT); }

      # Store stats for this lead time
      stat_val[j] = s$val;
      stat_ncl[j] = s$ncl;
      stat_ncu[j] = s$ncu;
    } # end for j

    # Plot the statistics
    ind = !is.na(stat_val)
    points(lead_list[ind]+horz[i], stat_val[ind],
           pch=8, type='b', col=color)

    # Plot the confidence intervals
    arrows(lead_list[ind]+horz[i], stat_ncl[ind],
           lead_list[ind]+horz[i], stat_ncu[ind],
           col=color, length=0.02, angle=90, code=3, lwd=2.0)

    # Set the valid data counts color
    if(event_equal == TRUE) { vld_col = "black"; }
    else                    { vld_col = color;   }

    # Plot the valid data counts
    if(event_equal == FALSE || i == 1)
      plot_valid_counts(series_data, vld_col, vert[i]);

  } # end for i

  # Add a legend
  legend(x="topleft",
         legend=series_list,
         col=rep(color_list, n_series)[1:n_series],
         lty=rep(1, n_series),
         lwd=rep(2, n_series),
         bty="n")
}

########################################################################
#
# Plot the valid data counts.
#
########################################################################

plot_valid_counts = function(data, color, vert) {

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
