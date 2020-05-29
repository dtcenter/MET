.. _grid-diag:

Chapter 14 Grid-Diag Tool
=========================

14.1 Introduction
_________________

The Grid-Diag tool creates histograms (probability distributions when normalized) for an arbitrary collection of data fields and levels. Joint histograms will be created for all possible pairs of variables. Masks can be used to subset the data fields spatially. The histograms are accumulated over a time series of input data files, similar to Series-Analysis.

14.2 Practical information

14.2.1 grid_diag usage

The following sections describe the usage statement, required arguments, and optional arguments for grid_diag.

Usage: grid_diag

{\hskip 0.5in}-data file_1 ... file_n | data_file_list

{\hskip 0.5in}-out file

{\hskip 0.5in}-config file

{\hskip 0.5in}[-log file]

{\hskip 0.5in}[-v level]

grid_diag has required arguments and can accept several optional arguments.

Required arguments for grid_diag

1. The -data file_1 ... file_n | data_file_list options specify the gridded data files or an ASCII file containing list of file names to be used.

2. The -out argument is the NetCDF output file.

3. The -config file is the configuration file to be used. The contents of the configuration file are discussed below.

Optional arguments for grid_diag

4. The -log file option directs output and errors to the specified log file. All messages will be written to that file as well as standard out and error. Thus, users can save the messages without having to redirect the output on the command line. The default behavior is no log file. 

5. The -v level option indicates the desired level of verbosity. The contents of “level” will override the default setting of 2. Setting the verbosity to 0 will make the tool run with no log messages, while increasing the verbosity above 1 will increase the amount of logging.

14.2.2 grid_diag configuration file

The default configuration file for the Grid-Diag tool named 'GridDiagConfig_default' can be found in the installed share/met/config/ directory. It is encouraged for users to copy these default files before modifying their contents. The contents of the configuration file are described in the subsections below.



model         = "GFS";

regrid        = { ... }

censor_thresh = [];

censor_val    = [];

mask          = { grid = [ "FULL" ]; poly = []; }

version       = "VN.N";

The configuration options listed above are common to many MET tools and are described in Section [subsec:IO_General-MET-Config-Options].



data = {

   field = [

      {

         name   = "APCP";

         level  = ["L0"];

         n_bins = 30;

         range  = [0, 12];

      },

      {

         name   = "PWAT";

         level  = ["L0"];

         n_bins = 35;

         range  = [35, 70];

      }

   ];

}

The name and level entries in the data dictionary define the data to be processed. The n_bins parameter specifies the number of histogram bins for that variable, and the range parameter the lower and upper bounds of the histogram. The interval length is the upper and lower difference divided by n_bins.

14.2.3 grid_diag output file

The NetCDF file has a dimension for each of the specified data variable and level combinations, e.g. APCP_L0 and PWAT_L0. The bin minimum, midpoint, and maximum values are indicated with an _min, _min, or _max appended to the variable/level.

For each variable/level combination in the data dictionary a corresponding histogram will be output to the NetCDF file. For example, hist_APCP_L0 and hist_PWAT_L0. These are the counts of all data values falling within the bin. Data values below the minimum or above the maximum are included in the lowest and highest bins, respectively. In addition to 1D histograms, 2D histograms for all variable/level pairs are written. For example, hist_APCP_L0_PWAT_L0 is the joint histogram for those two variables/levels.
