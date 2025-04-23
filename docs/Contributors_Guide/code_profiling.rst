**************
Code Profiling
**************

Benchmarking measurements are accomplished using the CTRACK tool:
  https://github.com/Compaile/ctrack

This code is licensed under the MIT License:
https://github.com/Compaile/ctrack/blob/main/LICENSE

The benchmarking tool utilizes a macro and C/C++ code is readily instrumented by
including the ctrack.hpp include file and by adding **CTRACK** at the top of the function of interest.
By default, the tool generates summary and detail metrics to stdout (standard output)
in easy to read, well-formatted tables.

.. note::
  The benchmarking tool currently supports **collecting benchmarking metrics for running the code once**
  (i.e. no stress-testing support is available).



Overview
========

Customizations for MET
----------------------

A Python script, benchmark.py is available to exercise the MET source code under consideration
either via MET commands (to replicate command line usage of the MET tool), or via METplus use
case (utilizing the METplus wrapper code and associated configuration files). The Python script consolidates
summary and detail metrics information into csv and tabular text files. The benchmark.py
script has an accompanying configuration file, benchmark.yaml located in the
*$HOME/MET/internal/scripts/benchmark* directory (where \$HOME is the directory where the MET source code is located).

The **ctrack.hpp** header file is modified to allow the summary and detailed reports to be saved as text files to
facilitate the consolidation of information into csv and tabular formats. The summary and detail metrics files are
located in the directory where the benchmark.py script was invoked.  The modified version of ctrack.hpp is located
in the *\${BASE_DIR}/MET/src/basic/vx_util* directory, where ${BASE_DIR} is the full path to where the MET source code
has been cloned or forked.

Code that is currently instrumented
-----------------------------------

The following code is employing the CTRACK macro:

- MET/src/basic/vx_util/main.cpp
   - do_pre_process function
   - do_post_process function
- MET/src/tools/core/ensemble_stat/ensemble_stat.cc
- MET/src/tools/core/ensemble_stat/ensemble_stat_conf.cc

Benchmarking with Python script
-------------------------------

The benchmarking.py script invokes MET code either via **MET command line commands** or **METplus use cases** as
specified by the **run_met_directly** setting in the benchmark.yaml configuration file.
The metrics from the summary and detail tables are consolidated into csv and tabular text files
(the locations of these consolidated metrics text files are specified in the benchmark.yaml configuration file).
The CTRACK summary_output.txt and detail_output.txt reports (containing the performance metrics) are written to the directory
from which the benchmarking.py script was executed.

Overview of Steps for Performing Benchmarking
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. Instrument the MET code of interest
2. Compile MET code
3. Edit the benchmark.yaml configuration file
4. Invoke the Python script *benchmark.py* to collect the benchmarking metrics
5. View results
6. Identify and implement any code changes to improve performance
7. Repeat step 2-5 until desired performance enhancements are achieved

Instrument the MET code of interest
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. note::

   The ctrack.hpp file is saved in the $HOME/MET/src/basic/vx_util directory and does not need to be modified or added to any
   other location.  This version of ctrack.hpp has been modified to write the summary and detail tables to text files.
   By default, CTRACK is disabled and is enabled at compilation time via the **--enable-profiler flag**.

   $HOME refers to the path to where the MET source code is saved.

The ctrack.hpp file must be included in the source code of interest:

.. code-block:: ini

    #ifdef WITH_PROFILER
    #include "ctrack.hpp"
    #endif


The CTRACK directive is placed at the top of the function of interest and the *ctrack::result_print* is placed
within the main()/met_main()functions.  Use the preprocessor directive for WITH_PROFILER:

  e.g. ensemble_stat.cc:

.. code-block:: ini

    void process_grid(const Grid &fcst_grid) {
        #ifdef WITH_PROFILER
        CTRACK;
        #endif
        Grid obs_grid;
       ... more code

    int met_main(int argc, char *argv[]) {

      // Process the command line arguments
      process_command_line(argc, argv);

      // Check for valid ensemble data
      process_n_vld();

     // Perform verification
     process_vx();

    // Save the CTRACK metrics
    #ifdef WITH_PROFILER
    ctrack::result_print();
    #endif


.. note ::

 The summary_output.txt and detail_output.txt files will only be saved when the ctrack::result_print() function is
 called within main() or met_main().

Compile MET code
^^^^^^^^^^^^^^^^

**Configure**

From the $HOME/MET directory:

* source ./internal/scripts/environment/development.xyz
*  *xyz* is the name of the host

By default, CTRACK is disabled. To enable it, use the --enable-profiler argument when configuring.

Run one of the following configure commands (to enable all the components and the CTRACK macro):

.. code-block:: ini

   ./configure --prefix=`pwd` --enable-grib2 --enable-modis --enable-mode_graphics --enable-lidar2nc --enable-python --enable-ugrid --enable-profiler

or

.. code-block:: ini

   ./configure --prefix=`pwd` --enable-all --enable-ugrid --enable-profiler

**Compile**

Run make install and test, redirecting the output to a log file named make.log:

.. code-block:: ini

  make install test >& make.log &
  tail -f make.log


**Verify that the expected code was profiled/instrumented for benchmarking**

The summary and detail tables are generated during the MET build (when running the test target).
These tables created by CTRACK can be viewed in the make.log before they are consolidated.
Use the *cat* (concatenation) tool to view the make.log file to view the CTRACK-generated metrics tables that
correspond to the MET tool that was instrumented.

.. note::

   **Concatenation** (vs viewing via a text editor like vim) **must** be used when viewing the logs,
   as the CTRACK reformatted output is formatted using *BeautifulTable*.


From the command line:

.. code-block:: ini

   cat make.log

.. note::

   There will be an error message like the following:

   make[1]: *** No rule to make target 'profiler', needed by 'all'.  Stop.

   make[1]: Leaving directory '/d1/personal/mwin/AF_optimization/feature_3065_benchmarking_ensemble_stat/MET/scripts'

   make: *** [Makefile:880: test] Error 2


This does not indicate an error with the compilation.  The --enable-profiler option does not have a build target,
it is used to turn on the CTRACK tool.



Edit the benchmark.yaml configuration file
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. note::

   the benchmark.py and benchmark.yaml files **must** reside in the same directory
   (the benchmark.yaml file does **NOT** need to be specified at the command line)


   The following is an example benchmark.yaml config file that utilizes environment variables and full directory paths

.. code-block:: ini

   # Configuration file used to collect benchmarking in MET tools using CTRACK
   #

   #
   # filename
   # Timestamp in ISO 1806 format is used to generate output filename
   # If filename setting is empty string, then timestamp is used.
   # Otherwise, the specified filename followed by the timestamp will
   # be used for the output filename.
   #
   filename: ''

   #
   # Output directory where output files will be saved
   #
   benchmark_output_path: !ENV '${BENCHMARK_OUTPUT_BASE}'

   # -------------------------------
   # FOR Running METPLUS USE CASE(S)
   # -------------------------------

   #
   # location of METplus
   #
   metplus_base: !ENV '${METPLUS_BASE}'

   #
   # location of system.conf file
   #
   system_conf: "/path/to/MET/internal/scripts/benchmark/system.conf"

   #
   # location of METplus wrapper configuration file(s)
   #
   wrapper_conf:
     - "/path/to/usecase_confs/truncated/EnsembleStat_fcstRRFS_obsCCPA_1hrAPCP_truncated.conf"

   # ------------------------
   # FOR RUNNING MET COMMAND
   # ------------------------
   run_met_directly: False
   met_command: ''

   # subdirectory to save the consolidated information, if empty, the
   # MET tool name will be used
   met_subdir_name: 'EnsembleStat_fcstRRFS_obsCCPA_1hrAPCP'

   #-----------------------------------
   # For future stress-testing support
   #-----------------------------------
   # number of times to run the use case for stress-testing
   # The default=1 if this setting is missing or unspecified
   #
   num_runs: 3


The following settings **must** be specified:

- **filename**

  - the supplied filename prepended with a Timestamp that follows ISO 1806 format
  - if left empty, the timestamp alone will be used as the filename

- **benchmark_output_path**

  - output directory where the output files will be saved
  - specify in one of two ways:

    - setting the BENCHMARK_OUTPUT_BASE env variable
    - explicitly setting the full directory path

**For running MET command, set the following:**

- run_met_directly
  - set to True
- met_command
  - the command to run the MET tool with the appropriate arguments
- met_subdir_name
  - if left empty, the consolidated benchmark metrics will be saved to a subdirectory named after the MET tool

**For running METplus use cases:**

.. note::

   A subdirectory under the output base directory (specified in benchmark_output_path) is created for each use case
   (based on the use case config filename).

- run_met_directly
  - set to False
- metplus_base
  - location of the METplus source code, specified by one of the following methods:

     - indicated as a full path e.g. /home/username/METplus
     - setting the METPLUS_BASE environment variable and use the current environment syntax like the following:

        .. code-block:: ini

           !ENV '${SOME_ENV_NAME}'

        Make sure that the SOME_ENV_NAME environment variable is defined

- system.conf

  - file location of the system_conf
  - full path and file name
  - pre-condition: generate a system.conf file

- wrapper_conf

  - the location of the METplus wrapper use case config file(s)
  - more than one use case can be run
  - full path and file name
  - pre-condition: generate the necessary wrapper config file(s)


Invoke the Python script *benchmark.py* to collect the benchmarking metrics
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. note::
 
   Use Python 3.12 or above for running the benchmark.py script


.. note::

   When collecting benchmarking metrics via the MET command line commands, first
   define the necessary environment variables for the corresponding MET tool (e.g. Ensemble-Stat tool environment
   variables specified in the $HOME/METplus/metplus/parm/met_config/EnsembleStatConfig_wrapped)

For example, the following example bash script has been used to set the necessary environment variables for a
specific Ensemble-Stat run:

.. code-block:: ini

   #!/usr/bin/bash

   export METPLUS_CENSOR_THRESH="";
   export METPLUS_CENSOR_VAL="";
   export METPLUS_CI_ALPHA="ci_alpha = [0.05];";
   export METPLUS_CLIMO_CDF_DICT="";
   export METPLUS_CLIMO_MEAN_DICT=“”;
   export METPLUS_CLIMO_STDEV_DICT="";
   export METPLUS_CONTROL_ID="";
   export METPLUS_DESC="desc = \"NA\";";
   export METPLUS_DUPLICATE_FLAG="";
   export METPLUS_ECLV_POINTS="";
   export METPLUS_ENS_MEMBER_IDS="";
   export METPLUS_ENS_PHIST_BIN_SIZE="";
   export METPLUS_ENS_SSVAR_BIN_SIZE="";
   export METPLUS_ENS_THRESH="ens_thresh = 1.0;";
   export METPLUS_FCST_CLIMO_STDEV_DICT="";
   export METPLUS_FCST_FIELD="field = [{ name=\"APCP\"; level=\"A01\"; }];";
   export METPLUS_FCST_FILE_TYPE=""; export METPLUS_GRID_WEIGHT_FLAG="";
   export METPLUS_INTERP_DICT="interp = {vld_thresh = 1.0;shape = SQUARE;type = {method = [NEAREST];width = [1];}}";
   export METPLUS_MASK_GRID="";
   export METPLUS_MASK_POLY="";
   export METPLUS_MESSAGE_TYPE="";
   export METPLUS_MET_CONFIG_OVERRIDES="";
   export METPLUS_MODEL="model = \"RRFS\";";
   export METPLUS_NC_ORANK_FLAG_DICT="nc_orank_flag = {latlon = TRUE;mean = TRUE;raw = TRUE;rank = TRUE;pit = TRUE;vld_count = TRUE;weight = FALSE;}";
   export METPLUS_OBS_CLIMO_MEAN_DICT="";
   export METPLUS_OBS_CLIMO_STDEV_DICT="";
   export METPLUS_OBS_ERROR_FLAG="";
   export METPLUS_OBS_FIELD="field = [{ name=\"APCP\"; level=\"A01\"; }];";
   export METPLUS_OBS_FILE_TYPE=""; export METPLUS_OBS_QUALITY_EXC="";
   export METPLUS_OBS_QUALITY_INC=""; export METPLUS_OBS_THRESH="";
   export METPLUS_OBS_WINDOW_DICT="obs_window = {beg = -1800;end = 1800;}";
   export METPLUS_OBTYPE="obtype = \"CCPA\";";
   export METPLUS_OBTYPE_AS_GROUP_VAL_FLAG="";
   export METPLUS_OUTPUT_FLAG_DICT="output_flag = {ecnt = NONE;rps = NONE;rhist = STAT;phist = STAT;orank = STAT;ssvar = STAT;relp = STAT;}";
   export METPLUS_OUTPUT_PREFIX="";
   export METPLUS_POINT_WEIGHT_FLAG="";
   export METPLUS_PROB_CAT_THRESH="";
   export METPLUS_PROB_PCT_THRESH="";
   export METPLUS_REGRID_DICT="regrid = {to_grid = OBS;method = NEAREST;width = 1;vld_thresh = 0.5;shape = SQUARE;}";
   export METPLUS_SKIP_CONST=""; exp


Run the following from the command line (from the location where the benchmark.py file is located):

.. code-block:: ini

  python benchmark.py

or, if running from any other directory:

.. code-block:: init

  python /path/to/benchmark.py

replacing */path/to* with the full path to the benchmark.py code from your current working directory

.. note::

   The intermediate summary_output.txt and detail_output.txt files that contain the CTRACK benchmark metrics are found in the
   directory from which the benchmark.py script was invoked.  The final, consolidated report is saved as a .csv and
   a tabular .txt file as specified in the **benchmark_output_path** setting.

View the results
^^^^^^^^^^^^^^^^

The benchmark.py script creates .csv and .txt files with consolidated metrics from the summary and details tables
(generated by the CTRACK tool).

View the consolidated metrics to identify potential performance enhancements.  Refer to the CTRACK documentation to
learn about the metrics collected, under the **Metrics & Output** section:

     https://github.com/Compaile/ctrack?tab=readme-ov-file#metrics--output


.. note:


  The consolidated files will be named *filename*_*timestamp*.csv and *filename*_*timestamp*.txt if the filename
  setting is specified in the benchmark.yaml configuration file.  If the filename setting is not specified, then the files
  will be named *timestamp*.csv and *timestamp*.txt.


Keywords
========

.. note::

 - CTRACK
 - benchmarking
 - profiling
 - code profiler
 - code profiling



