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

get_series_sub = function(cur, cur_plot, diff) {

  # Check for series entry differences
  if(diff == TRUE) {

    # Initialize to the first entry
    sub = tcst[tcst[,series] == cur_plot[1],]

    # Loop over the remaining series entries
    i = 2
    while(i <= length(cur_plot)) {
      diff_sub = tcst[tcst[,series] == cur_plot[i],]

      # Sanity check the dimensions
      if(dim(sub)[1] != dim(diff_sub)[1]) {
        cat("ERROR: When computing series differences for",
            cur, "the dimensions do not match:",
            dim(sub), "!=", dim(diff_sub))
        quit(1)
      }

      # Compute series difference
      sub$PLOT = sub$PLOT - diff_sub$PLOT

      i = i+1
    } # end while
  }
  # Otherwise, just subset based on a single series entry
  else {
    sub = tcst[tcst[,series]%in%cur_plot,]
  }

  # Subset based on requested lead times
  sub = sub[sub$LEAD_HR%in%lead_list,]

  return(sub)
}

########################################################################
#
# Update the range of the data based on the plot type.
#
########################################################################

set_range = function(sub, plot_type, yrange) {
      
  # Get the data range based on plot type
  if(plot_type == mean_str) {
    sub_yrange = range(aggregate(sub$PLOT, list(sub$LEAD_HR),
                                 mean, na.rm=TRUE)$x)
  }
  else if(plot_type == median_str) {
    sub_yrange = range(aggregate(sub$PLOT, list(sub$LEAD_HR),
                                 median, na.rm=TRUE)$x)
  }
  else {
    sub_yrange = range(sub$PLOT, na.rm=TRUE)
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
# Plot the valid data counts.
#
########################################################################

plot_valid_counts = function(sub, color, vert) {

  # Aggregate the values to be plotted by lead hour
  d = aggregate(!is.na(sub$PLOT), list(sub$LEAD_HR), sum)

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
# Create time series of boxplots or scatter plots.
#
########################################################################

plot_box_scatter = function(dep, plot_type, horz, vert) {

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
    sub = get_series_sub(series_list[i], series_plot[[i]],
                         diff_flag[i])

    # Add boxplots or scatter plots for each lead time
    for(lead in lead_list) {

      # Scatter plot
      if(plot_type == scatter_str ||
         sum(!is.na(sub$PLOT[sub$LEAD_HR==lead])) < n_min) {

         # Create a scatter plot
         points(rep(lead+horz[i], length(sub$PLOT[sub$LEAD_HR==lead])),
                sub$PLOT[sub$LEAD_HR==lead],
                pch=1, col=color)
       }
       # Boxplot
       else if(plot_type == boxplot_str) {

         # Create a boxplot
         b = boxplot(sub$PLOT[sub$LEAD_HR==lead],
                     add=TRUE, notch=TRUE, boxwex=0.4, outpch=1, outcex=1.0,
                     varwidth=TRUE, at=lead+horz[i],
                     xaxt="n", yaxt="n", col=color)
       }

       # Plot the mean value
       points(lead+horz[i],
              mean(sub$PLOT[sub$LEAD_HR==lead], na.rm=TRUE),
              pch=8, col=color)

    } # end for lead

    # Plot the valid data counts
    plot_valid_counts(sub, color, vert[i])

  } # end for i

  # Add a legend
  legend(x="topleft",
         legend=c(series_list, "Mean"),
         col=c(rep(color_list, n_series)[1:n_series], "black"),
         lty=c(rep(1, n_series),NA),
         lwd=c(rep(2, n_series),NA),
         pch=c(rep(NA,n_series),8),
         bty="n")
}

########################################################################
#
# Create time series of boxplots or scatter plots.
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
    sub = get_series_sub(series_list[i], series_plot[[i]], diff_flag[i])

    # Prepare the data
    if(plot_type == mean_str) {
      d = aggregate(sub$PLOT, list(sub$LEAD_HR), mean, na.rm=TRUE)
      # JHG
      #s = Compute_STDerr_from_mean(sub, "ML",  )
    }
    else if(plot_type == median_str) {
      d = aggregate(sub$PLOT, list(sub$LEAD_HR), median, na.rm=TRUE)
      # JHG s = Compute_STDerr_from_median()
    }

    # Plot the data
    points(d$Group.1, d$x, type='b', col=color)

    # Plot the valid data counts
    plot_valid_counts(sub, color, vert[i])

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
      sub = get_series_sub(series_list[i], series_plot[[i]],
                           diff_flag[i])

      # Update the plotting range
      yrange = set_range(sub, plot_type, yrange)

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

########################################################################
# Function to compute the Standard Error of an uncorrelated time series.
#
# Remove the correlated portion of a time series, using a first order auto-correlation coefficient
# to help compute an inflated variance factor for the uncorrelated portion of the time series.
#
# method = "ML" or "CSS".
#
# Originator:  Eric Gilleland and Andrew Loughe, 08 JUL 2008
#
########################################################################

Compute_STDerr_from_median <- function ( data, method, index, model, e_type, basin, vx_type, e_ylab, e_unit, data_type )
{

   n_min = 20

   RATIO_flag = 0;     ## Problem computing the vif ratio?
   y_label = (j-1)*6

   if ( sum(ifelse(data != 0,1,0)) >= n_min) {

   if ( IQR(data) > 0.0 & length(data) > 2 ) {

      ## Compute the first order auto-correlation coefficient
      ## using a vector that is the same size as "data", but represents
      ## represents excusions from the median of the data.

      ## Use excursions from the median to compute the first order auto-correlation coefficient.
      data_excursions = c();
      for( i in 1:length(data) ) { data_excursions <- c( data_excursions, as.numeric( data[i]>=round(median(data),1) ) ); }

      data.arima <- try( arima( data_excursions, order=c(1,0,0), method=method ), silent=TRUE)

      ## Catch an error from arima
      if(class(data.arima) != "try-error") {

      rho1 <- coef(data.arima)[1]

         ## Use higher order arima coefficients.
         ##       data.arima <- arima( data_excursions, order=c(5,0,0), method=method )
         ##      for (i in 1:5) {
         ##           p <- coef(data.arima)[i];
         ##           z <- 2.0 * sqrt(diag ( data.arim$var.coef ))[i]
         ##           if ( ((abs(p)+z) > 0. & (abs(p)-z) < 0.) ) {
         ##              break
         ##           }
         ##      }
         ##      ORD <- i-1;
         ##      if ( ORD < 1 ) { variance_inflation_factor = 1.0; }

         ## If the AR1 coefficient is out-of-bounds, try a different ARIMA method.
         ## What about higher-order coefficients?  Should we fit a higher-order function
         ## if those coefficients are large?
         if ( coef(data.arima)[1] < 0.3 | coef(data.arima)[1] >= 0.99 ) {
            if ( method == 'ML' ) { method = "CSS"; } else { method = "ML"; }
            data.arima2 <- try( arima( data_excursions, order=c(1,0,0), method=method ), silent=TRUE)
            ## Catch an error from arima2
            if(class(data.arima2) == "try-error") {
            print(paste("VIF|median|",y_label,"|",model,"|",e_type,"|",basin,"|",vx_type,"||","Initial AR1 coefficient is ", rho1," using ", method, " method...(failed to fit [coefficient is out-of-bounds] AR1 w/ other method)"))
#             print(paste("initial AR1 fit is ,", rho1))
#             print(paste("failed to fit AR1 w/ other method"))
            RATIO2 <- -1
            } else {
            RATIO2 = ( 1 + coef(data.arima2)[1] ) / ( 1 - coef(data.arima2)[1] )
            rho2 <- coef(data.arima2)[1]
            }

         } else {
         RATIO2 <- -1
         }

         ## Compute an variance inflation factor (having removed that portion of the time series that was correlated).
         RATIO1 = ( 1 + coef(data.arima)[1] ) / ( 1 - coef(data.arima)[1] )

         ## Check for a zero RATIO, that will then be operated on by SQRT.
         ## If necessary, try a different arima method, or just set RATIO to zero.
         if ( RATIO1 < 0.0 & RATIO2 < 0.0 ) {
         RATIO <- 1
         RATIO_flag <- 1
         rho <- rho1
         } else if ( RATIO1 < 0.0 ) {
         RATIO <- RATIO2
         rho <- rho2
         } else {
         RATIO <- RATIO1
         rho <- rho1
         }
         if ( RATIO < 0.0 ) { RATIO = 1.0; RATIO_flag = 1; }

         variance_inflation_factor = sqrt( RATIO );

         ## If the AR1 coefficient is less than 0.3, then don't even use a vif!  Set vif = 1.0
         if ( rho < 0.3 | rho >= 0.99  ) { variance_inflation_factor = 1.0; }

         AR1 = rho;

         yrange_data = range(data, na.rm=TRUE)
         yrange_data_excursions = range(data_excursions, na.rm=TRUE)
         mn_data = round(unlist(lapply(list(data), mean, na.rm=TRUE)),1)
         mn_data_excursions = round(unlist(lapply(list(data_excursions), mean, na.rm=TRUE)),1)
         mn_data[mn_data == "NaN"] = NA
         mn_data_excursions[mn_data_excursions == "NaN"] = NA

         # Create an empty plot
         par(mfrow=c(1,1), mai=c(1.5, 1.5, 2.0, 0.5), cex=1.5)
#          par(mfrow=c(2,2))
         plot(x=1, y=1, type="n",
              xlab="Lead Time (h)",
              ylab=paste(e_ylab, e_unit),
              xlim=c(0.5, 1.5), ylim=yrange_data,
              xaxt='n', col=0, col.axis="black")

         # Draw the X-axis
         axis(1, at=1, tick=TRUE, labels=FALSE)
         axis(1, at=1, tick=FALSE, labels=c(y_label))

         # Add the title
         title(paste("Data Dump: Median CIs w VIF", "\n", basin, vx_type))
         abline(h=0, lty=3, lwd=2.0)



         # Add boxplots or scatter plots for each lead time
            if(sum(!is.na(data)) < n_min) {

               # Create a scatter plot
               points(rep(1, length(data)), data, pch=1)

            } else {

               # Create a boxplot
               b = boxplot(data,
                     add=TRUE, notch=TRUE, boxwex=0.4, outpch=1, outcex=1.0,
                     varwidth=TRUE, at=1, col=0, xaxt="n", yaxt="n")
            }

         # Add a legend
         model_data = paste(model, "(data)")
         legend(x="topleft", legend=c(model_data, "Mean"),
                col=c(1,1), lty=c(1,NA), lwd=c(2,NA),
                pch=c(NA,8), bty="n", cex=0.8)

         # Add the means to the plot
         points(1, mn_data, pch=8)


         #---------------------------------------------------------

         # Create an empty plot
         par(mfrow=c(1,1), mai=c(1.5, 1.5, 2.0, 0.5), cex=1.5)
         plot(x=1, y=1, type="n",
              xlab="Lead Time (h)",
              ylab=paste(e_ylab, e_unit),
              xlim=c(0.5, 1.5), ylim=yrange_data_excursions,
              xaxt='n', col=0, col.axis="black")

         # Draw the X-axis
         axis(1, at=1, tick=TRUE, labels=FALSE)
         axis(1, at=1, tick=FALSE, labels=c(y_label))

         # Add the title
         title(paste("Data Dump: Median CIs w VIF", "\n", basin, vx_type))
         abline(h=0, lty=3, lwd=2.0)



         # Add boxplots or scatter plots for each lead time
            if(sum(!is.na(data_excursions)) < n_min) {

               # Create a scatter plot
               points(rep(1, length(data_excursions)), data_excursions, pch=1)

            } else {

               # Create a boxplot
               b = boxplot(data_excursions,
                     add=TRUE, notch=TRUE, boxwex=0.4, outpch=1, outcex=1.0,
                     varwidth=TRUE, at=1, col=0, xaxt="n", yaxt="n")
            }

         # Add a legend
         model_data_excursions = paste(model, "(data excursions)")
         legend(x="topleft", legend=c(model_data_excursions, "Mean"),
                col=c(1,1), lty=c(1,NA), lwd=c(2,NA),
                pch=c(NA,8), bty="n")

         # Add the means to the plot
         points(1, mn_data_excursions, pch=8)

#          dev.off(3)


      } else {
         print(paste("VIF|median|",y_label,"|",model,"|",e_type,"|",basin,"|",vx_type,"||","Failed to fit AR1 using ", method, " method...number of values less than the median is ", sum( data_excursions == 0, na.rm=TRUE), sep=""))
#          print(paste("Failed to fit AR1"))
#   print( paste( "Number of values less than the median is ", sum( data_excursions == 0, na.rm=TRUE), sep=""))
         variance_inflation_factor = 1.0;
         AR1 = 0.0;

         yrange_data = range(data, na.rm=TRUE)
         yrange_data_excursions = range(data_excursions, na.rm=TRUE)
         mn_data = round(unlist(lapply(list(data), mean, na.rm=TRUE)),1)
         mn_data_excursions = round(unlist(lapply(list(data_excursions), mean, na.rm=TRUE)),1)
         mn_data[mn_data == "NaN"] = NA
         mn_data_excursions[mn_data_excursions == "NaN"] = NA

         # Create an empty plot
         par(mfrow=c(1,1), mai=c(1.5, 1.5, 2.0, 0.5), cex=1.5)
#          par(mfrow=c(2,2))
         plot(x=1, y=1, type="n",
              xlab="Lead Time (h)",
              ylab=paste(e_ylab, e_unit),
              xlim=c(0.5, 1.5), ylim=yrange_data,
              xaxt='n', col=0, col.axis="black")

         # Draw the X-axis
         axis(1, at=1, tick=TRUE, labels=FALSE)
         axis(1, at=1, tick=FALSE, labels=c(y_label))

         # Add the title
         title(paste("Data Dump: Median CIs w VIF", "\n", basin, vx_type, "\n", "Failed to fit AR1"))
         abline(h=0, lty=3, lwd=2.0)



         # Add boxplots or scatter plots for each lead time
            if(sum(!is.na(data)) < n_min) {

               # Create a scatter plot
               points(rep(1, length(data)), data, pch=1)

            } else {

               # Create a boxplot
               b = boxplot(data,
                     add=TRUE, notch=TRUE, boxwex=0.4, outpch=1, outcex=1.0,
                     varwidth=TRUE, at=1, col=0, xaxt="n", yaxt="n")
            }

         # Add a legend
         model_data = paste(model, "(data)")
         legend(x="topleft", legend=c(model_data, "Mean"),
                col=c(1,1), lty=c(1,NA), lwd=c(2,NA),
                pch=c(NA,8), bty="n")

         # Add the means to the plot
         points(1, mn_data, pch=8)


      }

      ## Compute the Standard Error using the variance inflation factor.
      STDerr_data <- round(variance_inflation_factor * ( IQR(data) * sqrt(pi/2.) ) / ( 1.349 * sqrt( length(data) ) ),1)
      ## STDerr_data <- ( 1.58 * IQR(data) / sqrt( length(data) ) );

      if(class(data.arima) != "try-error") {
      print(paste("VIF|median|",y_label,"|",model,"|",e_type,"|",basin,"|",vx_type,"||","Computed the standard error (",STDerr_data,") using the vif (",variance_inflation_factor,") for AR1 coefficient or rho of ", rho,", AR1 = ", AR1))
      } else {
      print(paste("VIF|median|",y_label,"|",model,"|",e_type,"|",basin,"|",vx_type,"||","Computed the standard error (",STDerr_data,") using the vif (",variance_inflation_factor,") [should be 1.0] for AR1 (coefficient or rho) of ", AR1," [should be 0.0]"))
      }

   } else {

      STDerr_data = 0.0;
      AR1 = 0.0;

      print(paste("VIF|median|",y_label,"|",model,"|",e_type,"|",basin,"|",vx_type,"||","Set standard error and AR1 equal to 0.0 b/c following condition is FALSE:  ( IQR(data) > 0.0 & length(data) > 2 )"))
   }
      print(paste(data))
   if ( IQR(data) > 0.0 & length(data) > 2 ) {
      print(paste(data_excursions))
   }

   } else {

      STDerr_data = 0.0;
      AR1 = 0.0;

      print(paste("VIF|median|",y_label,"|",model,"|",e_type,"|",basin,"|",vx_type,"||","Set standard error and AR1 equal to 0.0 b/c following condition is FALSE:  ( sum(ifelse(data != 0,1,0)) >= n_min ); sum equals ",sum(ifelse(data != 0,1,0))))
      print(paste(data))
   }

   return ( c(STDerr_data, RATIO_flag, AR1, length(data)) );

}

Compute_STDerr_from_mean <- function ( data, method, index, model, e_type, basin, vx_type, e_ylab, e_unit, data_type )
{

   n_min = 20

   RATIO_flag = 0;     ## Problem computing the vif ratio?
   y_label = (j-1)*6

   if ( sum(ifelse(data != 0,1,0)) >= n_min ) {

   if ( var(data) > 0.0 & length(data) > 2 ) {

      ## Compute the first order auto-correlation coefficient.
      data.arima <- try( arima( data, order=c(1,0,0), method=method ) )
      print(data.arima)
      ## Catch an error from arima
      if(class(data.arima) != "try-error") {

      rho1 <- coef(data.arima)[1]

         ## If the AR1 coefficient is out-of-bounds, try a different ARIMA method.
         ## What about higher-order coefficients?  Should we fit a higher-order function
         ## if those coefficients are large?
         if ( coef(data.arima)[1] < 0.3 | coef(data.arima)[1] >= 0.99 ) {
            if ( method == 'ML' ) { method = "CSS"; } else { method = "ML"; }
            data.arima2 <- try( arima( data, order=c(1,0,0), method=method ), silent=TRUE)
            ## Catch an error from arima2
            if(class(data.arima2) == "try-error") {
            print(paste("VIF|mean|",y_label,"|",model,"|",e_type,"|",basin,"|",vx_type,"||","Initial AR1 coefficient is ", rho1," using ", method, " method...(failed to fit [coefficient is out-of-bounds] AR1 w/ other method)"))
#             print(paste("initial AR1 fit is ,", rho1))
#             print(paste("failed to fit AR1 w/ other method"))
            RATIO2 <- -1
            } else {
            RATIO2 = ( 1 + coef(data.arima2)[1] ) / ( 1 - coef(data.arima2)[1] )
            rho2 <- coef(data.arima2)[1]
            print(rho2)
            print(data.arima2)
            }

         } else {
         RATIO2 <- -1
         }

         ## Compute a variance inflation factor (having removed that portion of the time series that was correlated).
         RATIO1 = ( 1 + coef(data.arima)[1] ) / ( 1 - coef(data.arima)[1] )

         ## Check for a zero RATIO, that will then be operated on by SQRT.
         ## If necessary, try a different arima method, or just set RATIO to one.
         if ( RATIO1 < 0.0 & RATIO2 < 0.0 ) {
         RATIO <- 1
         RATIO_flag <- 1
         rho <- rho1
         } else if ( RATIO1 < 0.0 ) {
         RATIO <- RATIO2
         rho <- rho2
         } else {
         RATIO <- RATIO1
         rho <- rho1
         }
         if ( RATIO < 0.0 ) { RATIO = 1.0; RATIO_flag = 1; }

         variance_inflation_factor = sqrt( RATIO );

         ## If the AR1 coefficient is less than 0.3, then don't even use a vif!  Set vif = 1.0
         if ( rho < 0.3 | rho >= 0.99  ) { variance_inflation_factor = 1.0; }

         AR1 = rho;

         yrange_data = range(data, na.rm=TRUE)
         mn_data = round(unlist(lapply(list(data), mean, na.rm=TRUE)),1)
         mn_data[mn_data == "NaN"] = NA

         # Create an empty plot
         par(mfrow=c(1,1), mai=c(1.5, 1.5, 2.0, 0.5), cex=1.5)
#          par(mfrow=c(2,2))
         plot(x=1, y=1, type="n",
              xlab="Lead Time (h)",
              ylab=paste(e_ylab, e_unit),
              xlim=c(0.5, 1.5), ylim=yrange_data,
              xaxt='n', col=0, col.axis="black")

         # Draw the X-axis
         axis(1, at=1, tick=TRUE, labels=FALSE)
         axis(1, at=1, tick=FALSE, labels=c(y_label))

         # Add the title
         title(paste("Data Dump: Mean CIs w VIF", "\n", basin, vx_type))
         abline(h=0, lty=3, lwd=2.0)



         # Add boxplots or scatter plots for each lead time
            if(sum(!is.na(data)) < n_min) {

               # Create a scatter plot
               points(rep(1, length(data)), data, pch=1)

            } else {

               # Create a boxplot
               b = boxplot(data,
                     add=TRUE, notch=TRUE, boxwex=0.4, outpch=1, outcex=1.0,
                     varwidth=TRUE, at=1, col=0, xaxt="n", yaxt="n")
            }

         # Add a legend
         model_data = paste(model, "(data)")
         legend(x="topleft", legend=c(model_data, "Mean"),
                col=c(1,1), lty=c(1,NA), lwd=c(2,NA),
                pch=c(NA,8), bty="n")

         # Add the means to the plot
         points(1, mn_data, pch=8)


      } else {
         print(paste("VIF|mean|",y_label,"|",model,"|",e_type,"|",basin,"|",vx_type,"||","Failed to fit AR1 using ", method, " method...number of values less than the mean is...no data excursions for the mean, only median!!"))
#          print(paste("Failed to fit AR1"))
#   print( paste( "Number of values less than the mean is...no data excursions for the mean, only median!!"))
         variance_inflation_factor = 1.0;
         AR1 = 0.0;

         yrange_data = range(data, na.rm=TRUE)
         mn_data = round(unlist(lapply(list(data), mean, na.rm=TRUE)),1)
         mn_data[mn_data == "NaN"] = NA

         # Create an empty plot
         par(mfrow=c(1,1), mai=c(1.5, 1.5, 2.0, 0.5), cex=1.5)
#          par(mfrow=c(2,2))
         plot(x=1, y=1, type="n",
              xlab="Lead Time (h)",
              ylab=paste(e_ylab, e_unit),
              xlim=c(0.5, 1.5), ylim=yrange_data,
              xaxt='n', col=0, col.axis="black")

         # Draw the X-axis
         axis(1, at=1, tick=TRUE, labels=FALSE)
         axis(1, at=1, tick=FALSE, labels=c(y_label))

         # Add the title
         title(paste("Data Dump: Mean CIs w VIF", "\n", basin, vx_type, "\n", "Failed to fit AR1"))
         abline(h=0, lty=3, lwd=2.0)



         # Add boxplots or scatter plots for each lead time
            if(sum(!is.na(data)) < n_min) {

               # Create a scatter plot
               points(rep(1, length(data)), data, pch=1)

            } else {

               # Create a boxplot
               b = boxplot(data,
                     add=TRUE, notch=TRUE, boxwex=0.4, outpch=1, outcex=1.0,
                     varwidth=TRUE, at=1, col=0, xaxt="n", yaxt="n")
            }

         # Add a legend
         model_data = paste(model, "(data)")
         legend(x="topleft", legend=c(model_data, "Mean"),
                col=c(1,1), lty=c(1,NA), lwd=c(2,NA),
                pch=c(NA,8), bty="n")

         # Add the means to the plot
         points(1, mn_data, pch=8)


      }

      ## Compute the Standard Error using the variance inflation factor.
      STDerr_data <- round(variance_inflation_factor * sqrt( var(data) ) / sqrt( length(data) ),1);

      if(class(data.arima) != "try-error") {
      print(paste("VIF|mean|",y_label,"|",model,"|",e_type,"|",basin,"|",vx_type,"||","Computed the standard error (",STDerr_data,") using the vif (",variance_inflation_factor,") for AR1 coefficient or rho of ", rho,", AR1 = ", AR1))
      } else {
      print(paste("VIF|mean|",y_label,"|",model,"|",e_type,"|",basin,"|",vx_type,"||","Computed the standard error (",STDerr_data,") using the vif (",variance_inflation_factor,") [should be 1.0] for AR1 (coefficient or rho) of ", AR1," [should be 0.0]"))
      }

   } else {

      STDerr_data = 0.0;
      AR1 = 0.0;

      print(paste("VIF|mean|",y_label,"|",model,"|",e_type,"|",basin,"|",vx_type,"||","Set standard error and AR1 equal to 0.0 b/c following condition is FALSE:  ( IQR(data) > 0.0 & length(data) > 2 )"))
   }

   } else {

      STDerr_data = 0.0;
      AR1 = 0.0;

      print(paste("VIF|mean|",y_label,"|",model,"|",e_type,"|",basin,"|",vx_type,"||","Set standard error and AR1 equal to 0.0 b/c following condition is FALSE:  ( sum(ifelse(data != 0,1,0)) >= n_min ); sum equals ",sum(ifelse(data != 0,1,0))))
   }
      print(paste(data))

   return ( c(STDerr_data, RATIO_flag, AR1, length(data)) );

}
