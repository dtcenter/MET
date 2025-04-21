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
The CTRACK summary.txt and details.txt reports (containing the performance metrics) are written to the directory
from which the benchmarking.py script was executed.

Overview of Steps for Performing Benchmarking
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. Instrument the C/C++ code of interest
2. Compile MET code
3. Modify the benchmark.yaml config file based on one of the two methods to execute MET code and to indicate location
   of input data, etc.:
   - MET commands
   - METplus use case (METplus wrapper code)

4. Invoke the Python script *benchmark.py* to collect the benchmarking metrics

Instrumenting the MET code of interest
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The ctrack.hpp file is saved in the $HOME/MET/src/basic/vx_util directory and does not need to be modified or added to any
other location.  This version of ctrack.hpp has been modified to write the summary and detail tables to text files.
By default, CTRACK is disabled and is enabled at compilation time via the **--enable-profiler flag**.

The ctrack.hpp file must be included in the source code of interest:

.. code-block:: ini

    #ifdef WITH_PROFILER
    #include "ctrack.hpp"
    #endif


The CTRACK directive is placed at the top of the function of interest and the ctrack::result_print is placed
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

 The summary_output.txt and detail_output.txt files will only be saved when the ctrack::result_print() function is called.


Instrumenting the C/C++ MET code
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* For each .cpp file to be considered for benchmarking, include the ctrack.hpp header:

.. code-block:: ini

    #ifdef WITH_PROFILER
    #include "ctrack.hpp"
    #endif

* Add the **CTRACK** directive to any functions of interest:

.. code-block:: ini

      void some_interesting_function(){
         #ifdef WITH_PROFILER
         CTRACK;
         #endif

         //Do some stuff...
         return something;
      }

* To write the summary and detail benchmark metrics to text files:

  In the main() or met_main() function, invoke the ctrack::result_print()

  .. code-block:: ini

       void met_main(){

          //Do some stuff...
          #ifdef WITH_PROFILER
          ctrack::result_print();
          #endif

         //Do cleanup
         return return_code
       }

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

From the command line:

* *cat make.log*


.. note::

   **Concatenation** (vs viewing via a text editor like vim) **must** be used when viewing the logs,
   as the CTRACK reformatted output is formatted using *BeautifulTable*.


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
     - setting the METPLUS_BASE environment variable and use the current environment syntax (!ENV '$ENV'
- system.conf
  - file location of the system_conf
  - full path and file name
  - pre-condition: generate a system.conf file
- wrapper_conf
  - the location of the METplus wrapper use case config file(s)
  - more than one use case can be run
  - full path and file name
  - pre-condition: generate the necessary wrapper config file(s)


Run the Python code to consolidate benchmarking metrics
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Run the following from the command line (from the location where the benchmark.py file is located):

*python benchmark.py*

or, if running from any other directory:

*python /path/to/benchmark.py*

replacing */path/to* with the full path to the benchmark.py code from your current working directory

.. note::

   The intermediate summary.txt and details.txt files that contain the CTRACK benchmark metrics are found in the
   directory from which the benchmark.py script was invoked.  The final, consolidated report is saved as a .csv and
   a tabular .txt file as specified in the **benchmark_output_path** setting.




Keywords
========

.. note::

 - CTRACK
 - benchmarking
 - profiling
 - code profiler
 - code profiling



