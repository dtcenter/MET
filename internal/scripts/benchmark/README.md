Benchmarking measurements using CTRACK macro:
  https://github.com/Compaile/ctrack


METplus use cases using the grid stat, point stat, and ensemble stat tools were run to collect benchmarking baselines.
The MET/src/tools/core/grid_stat, MET/src/tools/core/point_stat, and MET/src/tools/core/ensemble_stat code was "instrumented" by
copying the ctrack.hpp header file into each tool's directory, including ctrack.hpp in the .cpp files of interest, 
and placing 'CTRACK;' at the beginning of the functions of interest.  The CTRACK tool generates a table of results (for both
summary and detail) to stdout.  The ctrack.hpp file was modified to instead create summary_output.txt and detail_output.txt files.
In the tool's main method (e.g. int main() or int met_main()), be sure to add the *ctrack::result_print(); * before exiting to ensure
that the summary_output.txt and detail_output.txt files are being saved.

The MET tool was then rebuilt and used to run the METplus use cases.  

The benchmarking code uses a YAML config file: benchmark.yaml, that accompanies the benchmarking.py code. A summary_output.txt and detail_output.txt file is
generated for each use case (which can be run one or more times) in the directory from where the benchmark.py script was invoked.

The benchmark.py script reads the YAML config file (benchmark.yaml) to obtain the name and location of the METplus use case config files, the system.conf file,
the name of the output filename (containing the benchmarking information and statistics).  A subdirectory under the output base directory is created
for each use case (based on the use case config filename).  

The benchmarking results are extracted and consolidated into one data structure (pandas dataframe).  The information is saved as a .csv and a .txt file 
(comma separated format and tabular format).  


**USAGE:**

*python benchmark.py*

*!!!NOTE!!!* the benchmark.py and benchmark.yaml files **must** reside in the same directory
Modify the benchmark.yaml file to indicate the following:

- base output directory
- METplus base directory
- use case configuration files for running the use cases via METplus 
- 
