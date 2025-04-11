
### Background:


Benchmarking measurements are accomplished with the CTRACK macro:
  https://github.com/Compaile/ctrack

This code is licensed under the MIT License: 
https://github.com/Compaile/ctrack/blob/main/LICENSE

The benchmarking tool utilizes a macro and C/C++ code is readily instrumented by
including the ctrack.hpp include file and by adding **CTRACK** at the top of the function of interest.
By default, the tool generates summary and detail metrics to stdout (standard output)
in easy to read, well-formatted tables. 

**NOTE**

The benchmarking tool currently supports **collecting benchmarking metrics for running the code once** 
(i.e. no stress-testing support is available).  


### Customizations for MET

A Python script, benchmark.py is available to exercise the MET source code under consideration
either via MET commands (to replicate command line usage of the MET tool), or via METplus use
case (utilizing the METplus wrapper code and associated configuration files). The consolidated
summary and detail metrics information is stored as csv and tabular text files. The benchmark.py
script has an accompanying configuration file, benchmark.yaml located in the $HOME/MET/internal/scripts/benchmark
directory (where $HOME is the directory where the MET source code is located).

The **ctrack.hpp** header file is modified to allow the summary and detailed reports to be saved as text files to facilitate the consolidation of information into
csv and tabular formats. The summary and detail metrics files are located in the directory where the benchmark.py script
was invoked.  The modified version of ctrack.hpp is located in the \${BASE_DIR}/MET/src/basic/vx_util directory, 
where ${BASE_DIR} is the full path to where the MET source code has been cloned or forked.


 


##### Performing Benchmarking with Python script

A Python script, benchmarking.py, supports running either **MET command line commands** or **METplus use cases** via
The method is specified in the benchmark.yaml configuration file. The metrics from the summary and detail reports 
are collected into csv and tabular text files (the locations are specified in the benchmark.yaml 
configuration file).  The summary.txt and details.txt reports generated from CTRACK are written to the directory
from which the benchmarking.py script was executed.

##### Instrumenting the MET code of interest

The ctrack.hpp file must be included in the source code of interest. The CTRACK
directive is placed at the top of the function of interest and within the main()/met_main()
function, the 

###### Code that is currently instrumented

The MET/src/basic/vx_util/main.cpp (do_pre_process and do_post_process functions) and 
MET/src/tools/core/ensemble_stat/ensemble_stat.cc source files are instrumented by
copying the ctrack.hpp header file into the MET/src/basic/vx_util directory,
and adding **CTRACK** at the top of the function of interest.


The modified ctrack.hpp file is included in the main.cc source code. The CTRACK directive is
used within the pre-processing and post-processing functions in this code.  The MET ensemble stat tool is
also instrumented.  The ensemble_stat.cc and ensemble_stat_conf_info.cc source code have the CTRACK directive
added to the top of the functions of interest.


**IMPORTANT**

The summary_output.txt and detail_output.txt files will only be saved when the ctrack::result_print() function is called.

 Add the following in the tool's main method (e.g. int main() or int met_main()) :
   *ctrack::result_print(); 

This is already added to the MET/src/basic/vx_util/main.cpp main() function to save the results from the do_pre_proces() and
do_post_process() functions.  The CTRACK macro is incorporated in the do_pre_process() and do_post_process() functions
in the main.cpp code in the basic/vx_util directory. 



### Summary of Steps for Performing Benchmarking

1. Instrument the C/C++ code of interest
2. Recompile MET code
3. Modify the benchmark.yaml config file based on one of the two methods to execute MET code:
   - MET commands
   - METplus use case (METplus wrapper code)
4. Invoke the Python script _benchmark.py_ to collect the benchmarking metrics


#### Instrument the C/C++ MET code

* For each .cpp file to be measured, include the ctrack.hpp header:

   `#include 'ctrack.hpp'`

* Add the **CTRACK** directive to any functions of interest:


     void some_interesting_function(){

         CTRACK;

         //Do some stuff...
         return something;
      }

* To write the summary and detail benchmark metrics to text files:

 ` void met_main(){

     //Do some stuff...
     ctrack::result_print();

     //Do cleanup
     return return_code
  }`

  
  In the main() or met_main() function, invoke the ctrack::result_print() function to consolidate the metrics
  from the summary and detail tables and save them to csv and tabular text files


#### Recompile MET code

Follow instructions for compiling the MET code:

Configure:

From the $HOME/MET directory:

* source ./internal/scripts/environment/development.xyz
* _xyz_ is the name of the host 
* ./configure --prefix=`pwd` --enable-grib2 --enable-modis --enable-mode_graphics --enable-lidar2nc --enable-python --enable-ugrid

Run make install and test, redirect output to a log file named make.log:
* make install test >& make.log &


###### Viewing summary and detail metrics tables generated by CTRACK

The summary and detail tables are generated during the MET build (when running the test target).
These tables created by CTRACK can be viewed in the make.log before they are consolidated. 
Use the _cat_ (concatenation) tool to view the CTRACK-generated metrics tables.

From the command line:

* _cat make.log_



**NOTE:**


_**Concatenation**_ (vs viewing via a text editor like vim) **must** be used when viewing the logs,
as the CTRACK reformatted output is formatted using _BeautifulTable_. 


#### Edit the benchmark.yaml configuration file

**NOTE**

the benchmark.py and benchmark.yaml files **must** reside in the same directory
(the benchmark.yaml file does **NOT** need to be specified at the command line)

The following settings **must** be specified:

- filename
  - the supplied filename prepended with a Timestamp that follows ISO 1806 format 
  - if left empty, the timestamp alone will be used as the filename
- benchmark_output_path
  - output directory where the output files will be saved

**For running MET command, set the following:**

- run_met_directly
  - set to True
- met_command
  - the command to run the MET tool with the appropriate arguments
- met_subdir_name
  - if left empty, the consolidated benchmark metrics will be saved to a subdirectory named after the MET tool

**For running METplus use cases:**
 
**NOTE:** 

A subdirectory under the output base directory (specified in benchmark_output_path) is created for each use case
(based on the use case config filename).  

- run_met_directly
  - set to False
- metplus_base
  - The location of the METplus source code
  - Indicate the full path e.g. /home/username/METplus
- system.conf 
  - file location of the system_conf 
  - full path and file name
  - pre-condition: generate a system.conf file 
- wrapper_conf
  - the location of the METplus wrapper use case config file(s)
    - more than one use case can be run
  - full path and file name
  - pre-condition: generate the necessary wrapper config file(s)
 

#### Run the Python code to consolidate benchmarking metrics

Run the following from the command line (from the location where the benchmark.py file is located):

*python benchmark.py*

or, if running from any other directory:

*python /path/to/benchmark.py*

replacing _/path/to_ with the full path to the benchmark.py code from your current working directory



  
