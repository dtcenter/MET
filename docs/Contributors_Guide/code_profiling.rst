**************
Code Profiling
**************

Benchmarking (also referred to as profiling) of MET tools is accomplished using the CTRACK tool:
  https://github.com/Compaile/ctrack

This code is licensed under the MIT License:
https://github.com/Compaile/ctrack/blob/main/LICENSE

Benchmarking uses a macro and the C++ source code is readily instrumented by
including ctrack.hpp and by adding **CTRACK** at the top of the function of interest.
By default, the tool generates summary and detail metrics to stdout (standard output)
in easy to read, well-formatted tables.  The ctrack.hpp file has been modified to permit
saving these tables to their respective text files (summary_output.txt and detail_output.txt).

.. note::
  The benchmarking tool currently supports **collecting benchmarking metrics for running the code once**
  (i.e. no stress-testing support is available).



Overview
========

Customizations for MET
----------------------

A Python script, benchmark.py is available to exercise the MET source code under consideration
either via MET commands (to replicate command line usage of the MET tool), or via METplus use
case (utilizing the METplus wrapper code and associated configuration files). The Python script consolidates the
summary and detail metrics information (as text files) into csv and tabular text files. The benchmark.py
script has an accompanying configuration file, benchmark.yaml located in the
*$HOME/MET/internal/scripts/benchmark* directory (where \$HOME is the directory where the MET source code is located).

The **ctrack.hpp** header file is modified to allow the summary and detailed reports to be saved as text files to
facilitate the consolidation of information into csv and tabular formats. The summary and detail metrics files are
located in the directory where the benchmark.py script was invoked.  The modified version of ctrack.hpp is located
in the *\${BASE_DIR}/MET/src/basic/vx_util* directory, where ${BASE_DIR} is the full path to where the MET source code
has been cloned or forked.

Code that is currently instrumented
-----------------------------------

The following code is instrumented using CTRACK:

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
from which the benchmarking.py script was executed.  An information file is also generated that captures the version
of Python used, a timestamp, and any other relevant information for capturing the environment under which the code was
profiled/benchmarked.

Overview of Steps for Performing Benchmarking
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. .. dropdown:: Instrument the MET code of interest

       .. note::

         The ctrack.hpp file is saved in the $HOME/MET/src/basic/vx_util directory and does not need to be modified
         or added to any other location.  This version of ctrack.hpp has been modified to write the summary and detail
         tables to text files. By default, CTRACK is disabled and is enabled at compilation time via the
         **--enable-profiler flag**.

         $HOME refers to the path to where the MET source code is saved.

       The ctrack.hpp file must be included in the source code of interest:

       .. code-block:: ini

         #ifdef WITH_PROFILER
         #include "ctrack.hpp"
         #endif


       The CTRACK directive is placed at the top of the function of interest.  Use the preprocessor directive for WITH_PROFILER:

       e.g. ensemble_stat.cc:

        .. code-block:: ini

           void process_grid(const Grid &fcst_grid) {
               #ifdef WITH_PROFILER
               CTRACK;
               #endif
               Grid obs_grid;
              ... more code

      and the *ctrack::result_print* is placed within the corresponding MET tool's
      **main()/met_main()** function

       e.g. ensemble_stat.cc

         .. code-block:: ini

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

2. .. dropdown::  Compile MET code

    .. dropdown:: Configure

        From the $HOME/MET directory:

          * source ./internal/scripts/environment/development.xyz
          *  *xyz* is the name of the host

        **By default, CTRACK is disabled**. Enable it with the --enable-profiler option.

         Run one of the following configure commands (to enable all the components and the CTRACK macro):

           .. code-block:: ini

              ./configure --prefix=`pwd` --enable-grib2 --enable-modis --enable-mode_graphics --enable-lidar2nc --enable-python --enable-ugrid --enable-profiler

               or

           .. code-block:: ini

               ./configure --prefix=`pwd` --enable-all --enable-ugrid --enable-profiler

    .. dropdown:: Run make install and test

        Redirect the output to a log file named make.log:

          .. code-block:: ini

             make install test >& make.log &
             tail -f make.log



    .. dropdown:: Verify that the expected code is being measured

      The summary and detail tables are generated during the MET build (when running the test target).
      These tables created by CTRACK can be viewed in the make.log before they are consolidated.
      Use the *cat* (concatenation) tool to view the make.log file to view the CTRACK-generated metrics tables that
      correspond to the MET tool that was instrumented.

      .. note::

         The CTRACK output is formatted using *BeautifulTable*.
         Therefore **concatenation** (vs viewing via a text editor like vim) facilitates viewing the human-readable
         version of the tables. The human-readable form of the tables is also available while running the
         *tail -f* command when viewing the make.log during compilation.

      From the command line:

      .. code-block:: ini

        cat make.log

      CTRACK summary and detail tables will appear in the make.log file.  A
      summary table will look like the following:

      .. code-block:: ini

        Summary
        +---------------------+---------------------+------------+---------------+-----------------+
        |        Start        |         End         | time total | time ctracked | time ctracked % |
        +---------------------+---------------------+------------+---------------+-----------------+
        | 2025-04-22 22:43:22 | 2025-04-22 22:44:31 |    69.43 s |    353.42 mcs |           0.00% |
        +---------------------+---------------------+------------+---------------+-----------------+
        +----------+-----------------+------+-------+-----------+------------+----------------+---------------+
        | filename |    function     | line | calls | ae[1-99]% | ae[0-100]% | time ae[0-100] | time a[0-100] |
        +----------+-----------------+------+-------+-----------+------------+----------------+---------------+
        |  main.cc |  do_pre_process |   97 |     1 |     0.00% |      0.00% |     322.08 mcs |    322.08 mcs |
        +----------+-----------------+------+-------+-----------+------------+----------------+---------------+
        |  main.cc | do_post_process |  119 |     1 |     0.00% |      0.00% |      31.34 mcs |     31.34 mcs |
        +----------+-----------------+------+-------+-----------+------------+----------------+---------------+





3. .. dropdown:: Edit the benchmark.yaml configuration file

     .. note::

        the benchmark.py and benchmark.yaml files **must** reside in the same directory
        (the benchmark.yaml file does **NOT** need to be specified at the command line)


     .. dropdown:: The following is an example benchmark.yaml config file that utilizes environment variables and full directory paths

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
         num_runs: 1


     .. dropdown::  Config settings for running via MET command:

        - benchmark_output_path

          - **required**
          - output directory where the output files will be saved
          - specify in one of two ways:

            - setting the BENCHMARK_OUTPUT_BASE env variable
            - explicitly setting the **full** directory path

        - filename

          - **optional**
          - the supplied filename prepended with a Timestamp that follows ISO 1806 format
          - if left empty, the timestamp alone will be used as the filename

        - run_met_directly

          - **required**
          - set to **True**

        - met_command

          - **required**
          - the command to run the MET tool with the appropriate arguments
          - this is the same command that would be ordinarily used when running a
            MET tool from the command line
          - make sure the specified *-outdir* directory exists

        - met_subdir_name

          - **optional**
          - if left empty, the consolidated benchmark metrics will be saved to a subdirectory (in the
            benchmark_output_path) named after the MET tool

        - num_runs

          - **not yet supported**
          - to be used for stress-testing/running command multiple times
          - set to 1

     .. dropdown:: Config settings for running via METplus usecase(s):

        - benchmark_output_path

          - **required**
          - output directory where the output files will be saved
          - specify in one of two ways:

            - setting the BENCHMARK_OUTPUT_BASE env variable
            - explicitly setting the full directory path

        - filename

          - **optional**
          - the supplied filename prepended with a Timestamp that follows ISO 1806 format
          - if left empty, the timestamp alone will be used as the filename


        - run_met_directly

          - **required**
          - set to **False**

        - metplus_base

          - **required**
          - location of the METplus source code, specified by one of the following methods:

             - indicated as a full path e.g. /home/username/METplus
             - setting the METPLUS_BASE environment variable and use the current environment syntax like the following:

              .. code-block:: ini

                 !ENV '${SOME_ENV_NAME}'

              Make sure that the *SOME_ENV_NAME* environment variable is defined

        - system.conf

          - **required**
          - file location of the system.conf file
          - full path and file name
          - pre-condition: generate a valid system.conf file

        - wrapper_conf

          - **required**
          - the location of the METplus wrapper use case config file(s)
          - more than one use case can be run
          - full path and file name
          - pre-condition: generate the necessary wrapper config file(s)

        - num_runs

          - **not yet supported**
          - to be used for stress-testing/running command multiple times
          - set to 1

        .. note::

         A subdirectory under the output base directory (specified in benchmark_output_path) is created for each use case
         (based on the use case config filename).



4. .. dropdown::  Invoke the Python script *benchmark.py* to collect the benchmarking metrics

    .. note::

       Use Python 3.12 or above for running the benchmark.py script


    **Pre-conditions:**

    .. dropdown::  Running MET command

       Define any necessary environment variables for the corresponding MET tool (e.g. Ensemble-Stat tool environment
       variables specified in the $HOME/METplus/metplus/parm/met_config/EnsembleStatConfig_wrapped)

      .. dropdown:: Example Ensemble-Stat config

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


    .. dropdown:: Running via METplus Usecase(s)

      Define the necessary environment variables that are required for running any METplus use case.


    **Running the Python script**

    Run the following from the command line (from the location where the benchmark.py file is located):

    .. code-block:: ini

      python benchmark.py

    or, if running from any other directory:

    .. code-block:: ini

      python /path/to/benchmark.py

    replacing */path/to* with the full path to the benchmark.py code from your current working directory

    .. note::

       The intermediate summary_output.txt and detail_output.txt files that contain the CTRACK benchmark metrics are found in the
       directory from which the benchmark.py script was invoked (while the MET code is being run).
       The final, consolidated report is saved as a .csv and a tabular .txt file as specified in the
       **benchmark_output_path** setting.


5. .. dropdown:: View results


     The benchmark.py script creates .csv and .txt files with consolidated metrics from the summary and details tables
     (generated by the CTRACK tool).  The summary_output.txt and detail_output.txt files generated
     during benchmarking are moved to the output directory.

     View the consolidated metrics to identify potential performance enhancements.  Refer to the CTRACK documentation to
     learn about the metrics collected, under the **Metrics & Output** section:

          https://github.com/Compaile/ctrack?tab=readme-ov-file#metrics--output


     .. note::

       The consolidated files will be named *filename_<timestamp>*.csv and *filename_<timestamp>*.txt if the filename
       setting is specified in the benchmark.yaml configuration file.  If the filename setting is not specified, then the files
       will be named *<timestamp>*.csv and *<timestamp>*.txt. An information file is also
       generated, capturing details about the benchmarking/profiling run such as Python version,
       timestamp, and other useful information to assist in trouble-shooting or re-creating
       a particular benchmarking run.  The information file is named info_*<timestamp>*.txt, where
       *<timestamp>* is the timestamp of the benchmarking run.





6. **Identify and implement any code changes to improve performance**
7. **Repeat steps 2-5 until desired performance enhancements are achieved**
















Keywords
========

.. note::

 - CTRACK
 - benchmarking
 - profiling
 - code profiler
 - code profiling



