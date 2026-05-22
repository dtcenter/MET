.. _grid-diag:

**************
Grid-Diag Tool
**************

Introduction
============

The Grid-Diag tool creates histograms (probability distributions when normalized) for an arbitrary collection of data fields and levels. Joint histograms are created for all possible pairs of variables. If no masking region is specified to subset the data fields spatially, then all points in the input domain are used. However, an arbitrary number of masking regions can be specified and output is created for each one. The histograms are accumulated over all of the input data files. Typically this tool is run with a time series of input data files, similar to Series-Analysis.

The Grid-Diag tool also uses the histograms to derive information theory statistics. Entropy is derived from each 1-dimensional histogram, and joint entropy and mutual information are derived from each 2-dimensional joint histogram. These statistics are defined using log base 2, rather than the natural logarithm which is also commonly used.

Practical Information
=====================

grid_diag Usage
---------------

The following sections describe the usage statement, required arguments, and optional arguments for **grid_diag**.

.. code-block:: none

  Usage: grid_diag
         -data file_1 ... file_n | file_list
         -out file
         -config file
         [-log file]
         [-v level]
         [-compress level]

.. note::

  The "-data" option can be used once to read all fields from each input file or once for each field to be processed.

grid_diag has required arguments and can accept several optional arguments.

Required Arguments for grid_diag
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. The **-data file_1 ... file_n | file_list** options specify the gridded data files or an ASCII file containing a list of file names to be used, as described in :numref:`ascii_file_lists`.

When **-data** is used once, all fields are read from each input file. When used multiple times, it must match the number of fields to be processed.
In this case the first field in the config data field list is read from the files designated by the first **-data**, the second field in the field list is read from files designated by the second **-data**, and so forth.  All files within each set must be of the same file type, but the file types of each set may differ.
A typical use case for this option is for the first **-data** to specify forecast data files and the second **-data** the observation data files.

2. The **-out** argument is the NetCDF output file.

3. The **-config file** is the configuration file to be used. The contents of the configuration file are discussed below.

Optional Arguments for grid_diag
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

4. The **-log file** option directs output and errors to the specified log file. All messages will be written to that file as well as standard out and error. Thus, users can save the messages without having to redirect the output on the command line. The default behavior is no log file.

5. The **-v level** option indicates the desired level of verbosity. The contents of "level" will override the default setting of 2. Setting the verbosity to 0 will make the tool run with no log messages, while increasing the verbosity above 1 will increase the amount of logging.

6. The **-compress level** option indicates the desired level of compression (deflate level) for NetCDF variables. The valid level is between 0 and 9. The value of "level" will override the default setting of 0 from the configuration file or the environment variable MET_NC_COMPRESS. Setting the compression level to 0 will make no compression for the NetCDF output. Lower number is for fast compression and higher number is for better compression.

grid_diag Configuration File
----------------------------

The default configuration file for the Grid-Diag tool named **GridDiagConfig_default** can be found in the installed *share/met/config/* directory. It is encouraged for users to copy these default files before modifying their contents. The contents of the configuration file are described in the subsections below.

_____________________

.. code-block:: none

  desc          = "GFS";
  regrid        = { ... }
  censor_thresh = [];
  censor_val    = [];
  mask          = { grid = []; poly = []; }
  version       = "VN.N";

The configuration options listed above are common to many MET tools and are described in :numref:`config_options`.

_____________________

.. code-block:: none

  power_spectrum_missing_flag  = NONE;
  power_spectrum_missing_value = 0.0;

The **power_spectrum_missing_flag** and **power_spectrum_missing_value** entries define how bad data values should be handled when computing power spectra. For all other output types, bad data values are ignored but they are problematic for power spectra. Set **power_spectrum_missing_flag** to **NONE** (default) to skip power spectrum when bad data is present, to **MEAN** to replace bad data with the mean of each input field, or to **VALUE** to replace bad data with the constant numeric value specified by **power_spectrum_missing_value**. Note that these configuration options can be specified separately for each input field.

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

The **name** and **level** entries in the **data** dictionary define the data to be processed. The **n_bins** parameter specifies the number of histogram bins for that variable, and the **range** parameter the lower and upper bounds of the histogram. The interval length is the upper and lower difference divided by **n_bins**. Each bin is inclusive on the left side and exclusive on the right, such as [a,b).

Grid-Diag prints a warning message if the actual range of data values falls outside the range defined for that variable in the configuration file. Any data values less than the configured range are counted in the first bin, while values greater than the configured range are counted in the last bin.

_____________________

.. code-block:: none

   output_flag = {
      histogram_1d   = TRUE;
      histogram_2d   = TRUE;
      info_theory    = FALSE;
      power_spectrum = FALSE;
   }

The **output_flag** dictionary controls the type of output that the Grid-Diag tool generates. Each flag should be set to **TRUE** or **FALSE** to enable the computation and writing of one or more variables to the output NetCDF file, as described below:

1. **histogram_1d** for 1-dimensional histograms for each **data.field** entry, including minimum, maxmimum, and midpoint values for each histogram bin.

2. **histogram_2d** for 2-dimensional histograms for each pair of **data.field** entries, including minimum, maxmimum, and midpoint values for each histogram bin.

3. **info_theory** for information theory metrics, including entropy for each **data.field** entry and mutual information and joint entropy for each pair of entries.

4. **power_spectrum** for power spectrum output.

grid_diag Output File
---------------------

The NetCDF file has dimensions for the number of masking regions and one for each of the specified data variable and level combinations, e.g. APCP_L0 and PWAT_L0. If histogram output is requested, the bin minimum and maximum values are indicated with an _min or _max appended to the variable/level. For each variable and level combination, a coordinate variable is written to indicate the midpoint value for each histogram bin.

The output variables for **grid_size** and **n_series** specify the number of points in the grid and the number of files that were processed, respectively. The range of the initialization, valid, and lead times processed is written to the global attributes.

The **mask_name** and **mask_size** variables have dimensions based on the number of masking regions and indicate the name of each masking region and the number of grid points it includes, respectively.

If 1-dimensional histograms are requested, a corresponding **hist_** variable is written for each variable/level in the data dictionary. This variable has dimensions for the number of masking regions and for the number of bins specified in the data dictionary. For example, hist_APCP_L0 and hist_PWAT_L0 are the counts of all data values falling within each bin for a given spatial masking region. Data values below the minimum or above the maximum are included in the lowest and highest bins, respectively. A warning message is printed when the range of the data falls outside the range defined in the configuration file.

If 2-dimensional joint historgrams are requested, a corresponding **hist_** variable is written for each combination of variable/level entries in the data dictionary. This variable has dimensions for the number of masking regions and for the number of bins specified for the two data dictionary entries. For example, hist_APCP_L0_PWAT_L0 is the joint histogram for those two variables/levels for a given spatial masking region.

If information theory output is requested, **entropy_**, **joint_entropy_**, and **mutual_information_** variables are written. Shannon entropy is derived from each 1-dimensional histogram, while joint entropy and mutual information are derived from each 2-dimensional joint histogram. These variables have one dimension for the number of masking regions and are computed using log base 2 rather than the natural logarithm. As such, their units are specified in the output as "bits" rather than "nats".

