.. _grid-diag:

Grid-Diag Tool
==============

Introduction
____________

The Grid-Diag tool creates histograms (probability distributions when normalized) for an arbitrary collection of data fields and levels. Joint histograms will be created for all possible pairs of variables. Masks can be used to subset the data fields spatially. The histograms are accumulated over a time series of input data files, similar to Series-Analysis.

Practical information
_____________________

grid_diag usage
~~~~~~~~~~~~~~~

The following sections describe the usage statement, required arguments, and optional arguments for **grid_diag**.

.. code-block:: none

  Usage: grid_diag
         -data file_1 ... file_n | data_file_list
         -out file
         -config file
         [-log file]
         [-v level]
         [-compress level]

    NOTE: The "-data" option can be used once to read all fields from each input file or once for each field to be processed.

grid_diag has required arguments and can accept several optional arguments.

Required arguments for grid_diag
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. The **-data file_1 ... file_n | data_file_list** options specify the gridded data files or an ASCII file containing list of file names to be used.

When **-data** is used once, all fields are read from each input file. When used multiple times, it must match the number of fields to be processed.

2. The **-out** argument is the NetCDF output file.

3. The **-config file** is the configuration file to be used. The contents of the configuration file are discussed below.

Optional arguments for grid_diag
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

4. The **-log file** option directs output and errors to the specified log file. All messages will be written to that file as well as standard out and error. Thus, users can save the messages without having to redirect the output on the command line. The default behavior is no log file.

5. The **-v level** option indicates the desired level of verbosity. The contents of “level” will override the default setting of 2. Setting the verbosity to 0 will make the tool run with no log messages, while increasing the verbosity above 1 will increase the amount of logging.

6. The **-compress level** option indicates the desired level of compression (deflate level) for NetCDF variables. The valid level is between 0 and 9. The value of “level” will override the default setting of 0 from the configuration file or the environment variable MET_NC_COMPRESS. Setting the compression level to 0 will make no compression for the NetCDF output. Lower number is for fast compression and higher number is for better compression.

grid_diag configuration file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The default configuration file for the Grid-Diag tool named 'GridDiagConfig_default' can be found in the installed **share/met/config/ directory**. It is encouraged for users to copy these default files before modifying their contents. The contents of the configuration file are described in the subsections below.

_____________________

.. code-block:: none

  desc          = "GFS";
  regrid        = { ... }
  censor_thresh = [];
  censor_val    = [];
  mask          = { grid = ""; poly = ""; }
  version       = "VN.N";

The configuration options listed above are common to many MET tools and are described in :numref:`Data IO MET Configuration File Options`.

_____________________

.. code-block:: none

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

The **name** and **level** entries in the **data** dictionary define the data to be processed. The **n_bins** parameter specifies the number of histogram bins for that variable, and the **range** parameter the lower and upper bounds of the histogram. The interval length is the upper and lower difference divided by **n_bins**.

grid_diag output file
~~~~~~~~~~~~~~~~~~~~~

The NetCDF file has a dimension for each of the specified data variable and level combinations, e.g. APCP_L0 and PWAT_L0. The bin minimum, midpoint, and maximum values are indicated with an _min, _mid, or _max appended to the variable/level.

For each variable/level combination in the data dictionary, a corresponding histogram will be written to the NetCDF output file. For example, hist_APCP_L0 and hist_PWAT_L0 are the counts of all data values falling within the bin. Data values below the minimum or above the maximum are included in the lowest and highest bins, respectively. A warning message is printed when the range of the data falls outside the range defined in the configuration file. In addition to 1D histograms, 2D histograms for all variable/level pairs are written. For example, hist_APCP_L0_PWAT_L0 is the joint histogram for those two variables/levels. The output variables for grid_size, mask_size, and n_series specify the number of points in the grid, the number of grid points in the mask, and the number of files that were processed, respectively. The range of the initialization, valid, and lead times processed is written to the global attributes.
