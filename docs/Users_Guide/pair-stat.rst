.. _pair-stat:

***************
Pair-Stat Tool
***************

Introduction
============

The Pair-Stat tool provides verification statistics for forecast and observation data that has already been paired. While no smoothing, regridding, or interpolation methods apply, the Pair-Stat tool filters and groups the pairs temporally and spatially. It then computes continuous, categorical, and probabilistic verification statistics. The categorical and probabilistic statistics generally are derived by applying a threshold to the forecast and observation values. Confidence intervals - representing the uncertainty in the verification measures - are computed for the verification statistics.

Scientific and statistical aspects of the Pair-Stat tool are discussed in the following section. Practical aspects of the Pair-Stat tool are described in :numref:`pair-stat_practical_info`.

Scientific and Statistical Aspects
==================================

The statistical methods and measures computed by the Pair-Stat tool are described briefly in this section. In addition, :numref:`matching-methods` discusses the various interpolation options available for matching the forecast grid point values to the observation points. The statistical measures computed by the Pair-Stat tool are described briefly in :numref:`PS_Statistical-measures` and in more detail in :numref:`Appendix C, Section %s <appendixC>`. :numref:`PS_Statistical-confidence-intervals` describes the methods for computing confidence intervals that are applied to some of the measures computed by the Pair-Stat tool; more detail on confidence intervals is provided in :numref:`Appendix D, Section %s <App_D-Confidence-Intervals>`.

.. _pair-stat_practical_info:

Practical Information
=====================

.. note::

  Only **-format mpr** is supported at this time. Support for **-format python** and **-format ioda** will be added in future development cycles.


The Pair-Stat tool is used to perform verification of a gridded model field using point observations. The gridded model field to be verified must be in one of the supported file formats. The point observations must be formatted as the NetCDF output of the point reformatting tools described in :numref:`reformat_point`. The Pair-Stat tool provides the capability of interpolating the gridded forecast data to the observation points using a variety of methods as described in :numref:`matching-methods`. The Pair-Stat tool computes a number of continuous statistics on the matched pair data as well as discrete statistics once the matched pair data have been thresholded.

If no matched pairs are found for a particular verification task, a report listing counts for reasons why the observations were not used is written to the log output at the default verbosity level of 2. If matched pairs are found, this report is written at verbosity level 3. Inspecting these rejection reason counts is the first step in determining why Pair-Stat found no matched pairs. The order of the log messages matches the order in which the processing logic is applied. Start from the last log message and work your way up, considering each of the non-zero rejection reason counts. Verbosity level 9 prints a very detailed explanation about why each observation is used or skipped for each verification task.

pair_stat Usage
---------------

The usage statement for the Pair-Stat tool is shown below:

.. code-block:: none

  Usage: pair_stat
         -pairs file_1 ... file_n | file_list
         -format type
         -config config_file
         [-outdir path]
         [-log file]
         [-v level]

pair_stat has three required arguments and accepts optional ones.

Required Arguments for pair_stat
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. The **-pairs** argument defines one or more input files containing forecast/observation pairs.
   May be set as a list of file names (**file_1 ... file_n**) or as an ASCII file containing
   a list file names (**file_list**). May be used multiple times (required).

2. The **format type** argument defines the input pairs file format and may be set to
   "mpr" or "ioda" (required).

3. The **-config config_file** argument is a PairStatConfig file containing the desired
   configuration settings (required).

Optional Arguments for pair_stat
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

4. The **-outdir path** option overrides the output directory which defaults to the current
   working directory (optional).

5. The **-log file** option directs output and errors to the specified log file.
   All messages will be written to that file as well as standard out and error.
   Thus, users can save the messages without having to redirect the output on the command line.
   The default behavior is no log file.

6. The **-v level** option indicates the desired level of verbosity.
   The value of "level" will override the default setting of 2.
   Setting the verbosity to 0 will make the tool run with no log messages,
   while increasing the verbosity will increase the amount of logging.

An example of the pair_stat calling sequence is shown below:

.. code-block:: none

  pair_stat \
  -pairs point_stat_mpr.txt \
  -format mpr \
  -config PairStatConfig

In this example, the Pair-Stat tool reads matched pair data (**-format mpr**) from **point_stat_mpr.txt**
and applies the configuration options specified in the **PairStatConfig** file.

pair_stat Configuration File
-----------------------------

The default configuration file for the Pair-Stat tool named **PairStatConfig_default** can be found in the installed *share/met/config* directory. Users are encouraged to make a copy prior to modifying its contents. The configuration file options are described in the subsections below.

Note that environment variables may be used when editing configuration files, as described in the :numref:`config_env_vars`.

TODO: Complete documentation
