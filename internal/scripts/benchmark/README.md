Benchmarking measurements using CTRACK macro:
  https://github.com/Compaile/ctrack

Background:
----------

A Python script, benchmarking.py, supports running either MET command line commands or METplus use cases via wrapper 
software (specified in a benchmark.yaml configuration file) and extracts metrics from the summary and detail reports 
into csv and tabular text files. The csv and tabular text file locations are specified in the benchmark.yaml 
configuration file and the summary.txt and details.txt reports generated from CTRACK are written to the directory
from which the benchmarking.py script was executed.


METplus use cases for the ensemble stat tools were run to collect benchmarking baselines.
The MET/src/basic/vx_util/main.cpp (do_pre_process and do_post_process functions) and 
MET/src/tools/core/ensemble_stat/ensemble_stat.cc source files are "instrumented" by
copying the ctrack.hpp header file into the MET/src/basic/vx_util directory,
and adding **CTRACK** at the top of the function of interest.

By default, the CTRACK tool generates a table summary and detail metrics to stdout.  
The ctrack.hpp file in this repository is a modified version of the original version
(located in the Compaile/CTRACK github repository). This modified version writes the summary metrics into a summary.txt
file and the details metrics into a details.txt file.  The location of these file is determined by where the benchmark.py
script was invoked.


**IMPORTANT**

The summary_output.txt and detail_output.txt files will only be saved when the ctrack::result_print() function is called.

 Add the following in the tool's main method (e.g. int main() or int met_main()) :
   *ctrack::result_print(); 

This is already added to the MET/src/basic/vx_util/main.cpp main() function to save the results from the do_pre_proces() and
do_post_process() functions.  The CTRACK macro is incorporated in the do_pre_process() and do_post_process() functions
in the main.cpp code in the basic/vx_util directory. 



Performing Benchmarking
-----------------------

Add the CTRACK directive to any other functions of interest and recompile the MET code.

The benchmarking tool can be run using METplus use case (via wrappers) or using MET command line commands.
The benchmarking code **requires a YAML config file,  benchmark.yaml**. This configuration file is invoked by the
benchmarking.py code. 

A summary_output.txt and detail_output.txt file is
generated for each use case (which can be run one or more times) or the MET command.  The files are saved in the 
directory from where the benchmark.py script was invoked.

**If running METplus use cases:**
A system.conf file and use case configuration files are needed to run a METplus use case.
The YAML config file (benchmark.yaml) contains the name and location of the METplus use case config files,
the system.conf file, and the name of the output filename (containing the benchmarking information and statistics).  
A subdirectory under the output base directory is created for each use case (based on the use case config filename).  

The following settings should be specified in the benchmark.yaml file:
* metplus_base
  * The location of the METplus source code
  * full path e.g. /home/username/METplus
* system_conf
  * the location of the system.conf file
  * full path and file name
* wrapper_conf
  * the usecase configuration file 
  * full path and file name


**If running MET command line commands:**
The benchmark.yaml file is the only configuration file needed. The following
settings need to be specified:

* met_command
  * the command used to run the MET tool with required and optional arguments
* met_subdir_name

**NOTE**
The benchmarking tool currently supports **running code once** (i.e. no stress-testing support is available).  

The benchmarking results are extracted and consolidated into one data structure (pandas dataframe). 
The information is saved as a .csv and a .txt file (comma separated format and tabular format). The location of these
files are specified in the benchmark.yaml config file.


**USAGE:**

*python benchmark.py*

*!!!NOTE!!!* the benchmark.py and benchmark.yaml files **must** reside in the same directory (but the benchmark.yaml file does NOT need to be specified at the command line)
Modify the benchmark.yaml file to indicate the following:

- benchmark output directory (benchmark_output_path)
- METplus base directory (metplus_base)
- use case configuration files for running the use cases via METplus (wrapper_conf)
- system.conf file location (system_conf)
- if you wish to prepend a filename to the consolidated output, do so with the filename setting
   - the timestamp will follow the filename specified
   - otherwise, the timestamp + extension will be created as the filename

  
  
