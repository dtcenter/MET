########################################################################
#
# Default values for command-line arguments.
#
########################################################################

file_list      = "";
outdir         = ".";
prefix         = "";
title_str      = c();
subtitle_str   = c();
xlab_str       = c();
ylab_str       = c();
xmin           = NA;
xmax           = NA;
ymin           = NA;
ymax           = NA;
filter_opts    = "";
tcst_file      = "";
dep_list       = c("TK_ERR");
scatter_x_list = c("");
scatter_y_list = c("");
skill_ref      = c("");
series         = "AMODEL";
series_list    = c();
series_ci      = c(TRUE);
legend_list    = c();
lead_list      = c(0, 12, 24, 36, 48, 60, 72, 84, 96, 108, 120);
plot_list      = c("BOXPLOT");
rp_diff_list   = c(">=100");
demo_yr        = NA;
hfip_bsln      = "no";
footnote_flag  = FALSE;
event_equal    = TRUE;
log_flag       = TRUE;
plot_config    = "";
save_data      = "";
save           = FALSE;

########################################################################
#
# Default values for constants used by the plot_tcmpr.R script.
#
########################################################################

# Build unique tc_stat output file name
tcst_tmp_file = paste("/tmp/plot_tcmpr_", Sys.getpid(), ".tcst", sep='');

# Output image file format
img_fmt  = "png256";
img_ext  = "png";
img_hgt  = 8.5;
img_wdth = 11.0;
img_res  = 300;

# List of colors to be used for each plot type
default_color_list  = c("black", "red", "blue", "green", "purple", "orange");
boxplot_color_list  = default_color_list;
point_color_list    = default_color_list;
mean_color_list     = default_color_list;
median_color_list   = default_color_list;
skill_mn_color_list = default_color_list;
skill_md_color_list = default_color_list;
relperf_color_list  = default_color_list;
rank_color_list     = c("green", "blue", "orange", "red", "purple");
scatter_color_list  = c("black", "brown", "green", "blue", "orange", "red", "purple", "magenta", "yellow", "steelblue1", "violetred4", "gray", "bisque2");
default_color      = "gray";
default_pch        = 1;
default_lwd        = 2;
default_lty        = 1;

# Minimum number of values for boxplots and confidence intervals.
n_min=11;

# Plotting offset value
horz_offset  = 2.00;
vert_offset  = 1.25;

# Horizontal offset for relative performance and rank plots
relperf_rank_horz_offset = 0.50;

# Alpha and z-value
alpha = 0.05;
zval  = qnorm(1 - (alpha/2));

