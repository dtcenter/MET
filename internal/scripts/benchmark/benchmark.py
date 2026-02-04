

# ============================*
# ** Copyright UCAR (c) 2026
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# ============================*


import os
import sys
import shutil
import re
import subprocess
from datetime import datetime

import yaml
import pandas as pd

from constants import TO_NANOSEC
from constants import ALL_METRICS_COLS

"""
   Extracts the CTRACK benchmarking information from running MET C++ code via METplus wrapper.
   Benchmark values are collected by running use cases to facilitate optimizing the MET code for
   use cases.
   
   Refer to https://github.com/Compaile/ctrack/tree/main for more information on using CTRACK to
   benchmark your C++ code.

   Two output files are generated when running the use cases:
   1) Summary file
   2) Details file
   These files are located where the use case code is invoked.
   The benchmarking information is retrieved from each file above, then consolidated into one file.  A CSV
   (comma separated values) and tabular file are also generated to enable plotting of the results. These files
   are saved under a subdirectory named after the use case config file (without the extension). The consolidated
   file is named by generating a timestamp and converting it to ISO 8601 Datetime with the appropriate file
   extension (.csv, .txt). The summary and detail file are copied into the use case subdirectory.
   
   This script has an accompanying YAML config file, benchmark.yaml in which the user can specify (explicitly or by 
   setting env variables) the following:
   
   - output path
     - the base output directory, subdirectories based on use case will be created under this directory
     - if the output path does not exist, it will be created
   - filename
       - if specified, the timestamp will be appended
       - if unspecified, the timestamp will be used as the filename
   - number of times to run the use case
   - the system.conf file to be used 
   - a list of one or more use case config files to be used
      - a subdirectory will be created based on the use case config file name where output will be saved   

    ****
   * Usage:
    ****
       -update the benchmark.yaml file to indicate:
          - output base directory
          - filename to apply to the benchmark output files (the timestamp will be added)
             - if no filename is specified, the ISO 1806 formatted timestamp will be used
          - a list of 1 or more use case config files
             - full file path and name
          - the number of times each use case is to be run
          - the system.conf file location (full path)
"""


def extract_detail_info(infile:str, subdir:str) -> pd.DataFrame:
    """
     Extract the benchmarking information in the detail_output.txt file produced by CTRACK.

    :param infile: The detail_output.txt file containing benchmarking information
    :param subdir: The subdirectory for the use case
    :return details_df: A pandas dataframe containing the detail output from CTRACK
    """

    # Open the file and parse information
    with open(infile, 'r') as f:
        lines = f.readlines()

    df_list = []
    details_df = []
    for line in lines:
        details_dict = {}
        kv_pairs = line.split("|")
        for kv in kv_pairs:
          key_values = kv.split(": ")
          cur_key = key_values[0].replace(" ", "_")

          # save each value as a list to facilitate the creation of a pandas dataframe from the dictionary
          details_dict[cur_key] = [ key_values[1]]

        # Create a list of pandas dataframes, each one corresponding to a line of benchmark data
        cur_df = pd.DataFrame(details_dict)
        df_list.append(cur_df)

        # Concatenate each dataframe into one

        counter = 0
        for cur in df_list:
            if counter == 0:
                details_df = cur
                counter += 1
            else:
                details_df = pd.concat([details_df, cur])

    # copy the detail_output.txt file to the use case subdirectory
    cur_dir = os.path.dirname(__file__)
    infile_path = os.path.join(cur_dir, infile)
    shutil.copy(infile_path, os.path.join(subdir, "detail_output.txt"))

    return details_df


def extract_summary_info(infile:str, subdir:str) -> pd.DataFrame:
    """
     Extracts the information from the summary_output.txt file into a csv formatted and
     tabular file

     :param infile: the summary_output.txt file containing the CTRACK summary information
     :param: subdir: the use case subdirectory or MET tool name subdirectory
     :return: a pandas dataframe containing the summary values
     """
    # Open the file and parse information
    with open(infile, 'r') as f:
        lines = f.readlines()

    df_list = []
    for line in lines:
        summary_dict = {}
        kv_pairs = line.split("|")
        for kv in kv_pairs:
            key_values = kv.split(": ")
            cur_key = key_values[0].replace(" ", "_")

            # make the value a list to enable creating a pandas dataframe directly from the summary dictionary
            summary_dict[cur_key] = [key_values[1]]

        # save each summary dict as a dataframe and add it to the list
        cur_df = pd.DataFrame(summary_dict)
        df_list.append(cur_df)

    # concatenate the list of dataframes into one dataframe
    counter = 0
    summary_df = pd.DataFrame()
    for cur_df in df_list:
        if counter == 0:
            summary_df = cur_df
            counter += 1
        else:
            summary_df = pd.concat([summary_df, cur_df])

    # copy the summary_output.txt file to the use case subdirectory
    cur_dir = os.path.dirname(__file__)
    infile_path = os.path.join(cur_dir, infile)
    shutil.copy(infile_path, os.path.join(subdir, "summary_output.txt"))


    return summary_df


def extract_metrics(infile:str, subdir:str)-> pd.DataFrame:
    """
    Extract the metrics from the summary and detail output files generated by the CTRACK tool.
    Separate out the text value+unit into separate columns for values that have units (either time in
    s, ms, mcs, or ns, or %) into a value column and a corresponding unit column.

    :param infile: The input file containing the metrics (summary or detail files)
    :param subdir: The subdirectory to where the finished output should be saved
    :return: A dataframe containing the information from the benchmarking metrics file
    """

    # Open the file and parse information, saving it in a dictionary
    with open(infile, 'r') as f:
       lines = f.readlines()

       df_list = []
       for line in lines:
           metric_dict = {}
           kv_pairs = line.split("|")

           for kv in kv_pairs:
               key_values = kv.split(": ")

               # Replace the whitespace in the metric name with '_' to facilitate
               # pandas operations on column header
               updated_key = key_values[0].replace(" ", "_")

               # Save the values as a list to facilitate creating a pandas dataframe from the dictionary
               if updated_key in ALL_METRICS_COLS:
                   # Extract the numerical value and the units and
                   # save these into separate key:value pairs
                   match = re.match(r"(\d+(\.\d*)?)\s?(%|s|ms|mcs|ns)", key_values[1])
                   value = match.group(1)
                   units = match.group(3)

                   # save the value as a list to facilitate creating a pandas
                   # dataframe from a dictionary
                   metric_dict[updated_key] = [value]
                   units_col = updated_key  + "_units"
                   metric_dict[units_col] = units
               else:
                   # Add the value string to the value (one of the unit less metrics)
                   # Save the value as a list to facilitate creating a pandas
                   # dataframe from a dictionary
                   metric_dict[updated_key] = [key_values[1]]


           # Save each dictionary as a dataframe, adding it to a list of
           # dataframes for concatenation into one dataframe after all
           # rows of metrics have been extracted.
           df_list.append(pd.DataFrame(metric_dict))

       # Concatenate the list of dataframes into one dataframe
       counter = 0
       metric_df = pd.DataFrame()
       for cur_df in df_list:
           if counter == 0:
               metric_df = cur_df
               counter +=1
           else:
               metric_df = pd.concat([metric_df, cur_df])




    return metric_df



def consolidate_info(summary_df:pd.DataFrame, detail_df:pd.DataFrame) -> pd.DataFrame:
    """
       Consolidate the summary and detail information for each filename/function/line/start_time/end_time combination.


    :param summary_df: pandas dataframe containing summary benchmark info
    :param detail_df: pandas dataframe containing detail benchmark info
    :return:  pandas Dataframe
   """
    summary_df['profiler_index'] = summary_df.groupby(['filename', 'function', 'line','start_time', 'end_time']).ngroup()
    detail_df['profiler_index'] = detail_df.groupby(['filename', 'function', 'line', 'start_time', 'end_time']).ngroup()

    merged = summary_df.merge(detail_df, on=["profiler_index"], how="left")

    # clean up column names (since summary and detail tables have
    # the same columns names for filen, function, line, etc.  pandas renames
    # the resulting merged columns with _x appended
    merged.rename(columns={'filename_x': 'filename', 'function_x':'function',
                           'line_x':'line', 'start_time_x':'start_time', 'end_time_x': 'end_time'}, inplace=True)

    return merged


def get_timestamp() -> str:
    """
       Get the timestamp for the time that this is running and use this to name the output file

        :param: None
        :return: The ISO 8601 datetime representation of the timestamp (string)

    """

    # non-aware time because using utcnow() is not recommended and datetime.UTC is not supported for
    # this version (3.10) of Python
    dt = datetime.now()
    ts = dt.isoformat(timespec="seconds")

    return ts



def parse_config(path=None, data=None, tag='!ENV'):
    """
    Load a yaml configuration file and resolve any environment variables
    The environment variables must have !ENV before them and be in this format
    to be parsed: ${VAR_NAME}.
    E.g.:

    database:
        host: !ENV ${HOST}
        port: !ENV ${PORT}
    app:
        log_path: !ENV '/var/${LOG_PATH}'
        something_else: !ENV '${AWESOME_ENV_VAR}/var/${A_SECOND_AWESOME_VAR}'

    :param str path: the path to the yaml file
    :param str data: the yaml data itself as a stream
    :param str tag: the tag to look for
    """
    # pattern for global vars: look for ${word}
    pattern = re.compile(r'.*?\${(\w+)}.*?')
    loader = yaml.SafeLoader

    # the tag will be used to mark where to start searching for the pattern
    # e.g. somekey: !ENV somestring${MYENVVAR}blah blah blah
    loader.add_implicit_resolver(tag, pattern, None)

    def constructor_env_variables(loader, node):
        """
        Extracts the environment variable from the node's value
        :param yaml.Loader loader: the yaml loader
        :param node: the current node in the yaml
        :return: the parsed string that contains the value of the environment
        variable
        """
        value = loader.construct_scalar(node)
        match = pattern.findall(value)  # to find all env variables in line
        if match:
            full_value = value
            for g in match:
                full_value = full_value.replace(
                    f'${{{g}}}', os.environ.get(g, g)
                )
            return full_value
        return value

    loader.add_constructor(tag, constructor_env_variables)

    if path:
        with open(path) as conf_data:
            return yaml.load(conf_data, Loader=loader)
    elif data:
        return yaml.load(data, Loader=loader)
    else:
        raise ValueError('Either a path or data should be defined as input')

def save_results(consolidated:pd.DataFrame, output_dir:str, ts:str, filename , subdir:str) -> None:
    """
       Save the consolidated results into a csv file and a tabular file

    :param consolidated: pandas dataframe containing the summary and detail report results from CTRACK
    :param output_dir: the directory where the consolidated file(s) is/are saved
    :param ts: The timestamp used to create the output filename
    :param filename: The user-specified filename, if empty string, then the timestamp will be used
    :param subdir: The subdir corresponding to this use case (based on the wrapper conf file name or specified
    subdir name for MET cli)
    :return: None
    """

    # extensions for csv and tabular files
    c_ext = ".csv"
    t_ext = ".txt"

    # filename is either filename_<timestamp>.ext
    # or <timestamp>.ext, where ext is .csv or .txt
    if filename == "" or filename == " " or filename is None:
       c_filename = ts + c_ext
       t_filename = ts + t_ext
    else:
        c_filename = filename + "_" + ts + c_ext
        t_filename = filename + "_" + ts + t_ext

    # save the csv and txt files to the use case directory
    full_out_dir = os.path.join(output_dir, subdir)
    full_csv_output_file = os.path.join(full_out_dir, c_filename)
    full_table_output_file = os.path.join(full_out_dir, t_filename)

    # Keep header and do not write out an unnamed index column (i.e. index=False,
    # to avoid the unwanted Unnamed 0 column in the final output)
    consolidated.to_csv(full_csv_output_file,header=True, index=False)
    consolidated.to_csv(full_table_output_file, header=True, sep="\t", index=False)

    # Do some checking to make sure the files were actually created and they aren't empty
    assert os.path.isfile(full_csv_output_file), "WARNING: csv output file not created"
    assert os.path.isfile(full_table_output_file), "WARNING: tabular output file not created"



def check_settings(settings:dict) -> None:
    """
      Check that paths specified in the config file exist and set any default values, create
      any needed directories,

      :param settings: dictionary representation of settings specified in the YAML config file
      :return:  None
    """

    metplus_dir = settings['metplus_base']
    sys_conf = settings['system_conf']
    wrapper_confs = settings['wrapper_conf']

    if settings['run_met_directly']:
        # Check that the script is invoked within the MET/internal/scripts/benchmark directory
        expected_path = os.path.abspath(os.path.dirname(__file__))   
        invoked_path = os.getcwd()
        assert invoked_path == expected_path, f"ERROR You must invoke benchmark.py within the {expected_path} directory"

        # Check for empty string or only whitespace for MET command
        assert len(settings['met_command']) > 0
        assert not settings['met_command'].isspace()
    else:
        assert os.path.exists(metplus_dir), f"ERROR|benchmark.yaml::The METplus base directory {metplus_dir} does not exist"
        assert os.path.exists(sys_conf), f"ERROR|benchmark.yaml::The system.conf file {sys_conf}  does not exist."

        for cur_conf in wrapper_confs:
            assert os.path.exists(cur_conf), f"ERROR|benchmark.yaml:: The {cur_conf} use case config file does not exist. "

    output_base = settings['benchmark_output_path']
    # if the base output dir does not exist, create it
    os.makedirs(output_base, exist_ok=True)

def generate_info(settings:dict, ts:str, description: str, subdir: str, additional:str=None) -> None:
    """
       Generate a text file with information on the current benchmark run
    :param settings: dictionary representation of the settings specified in the YAML config file
    :param ts: timestamp
    :param description: a description of the run
    :param subdir: the use case subdirectory (full path)
    :param additional: any other descriptive information, such as current env variables. Default is None
    :return: None, write an output text file in the output path specified in the YAML config file
    """
    info_file = "info_" + ts + ".txt"
    full_path = os.path.join(subdir, info_file)
    with open(full_path, 'w') as f:
        f.write(f"Python version info: {sys.version}\n")
        f.write(f"Timestamp: {ts}\n")
        f.write(f"Description of Use case or MET invocation (optional) : {description}\n")
        if additional:
            f.write(f"Additional information: {additional}\n")
        f.write(f"Number of times run: {settings['num_runs']}\n")

def run_usecases(settings:dict, ts:str, files_from_ctrack:tuple)->None:
    """
      Run the usecases based on settings specified in the benchmark.yaml config file.
      Currently, the use case(s) are run only once (i.e. no stress-testing support is currently
      available).

      :param settings: the dictionary representation of the settings from the benchmark.yaml file
      :param ts: the timestamp (string) used to create the filename
      :param files_from_ctrack: tuple containing the names of the summary and detail output files generated
                              by CTRACK
    :return: None
    """

    # Retrieve the settings from benchmark.yaml
    output_base = settings['benchmark_output_path']

    # Run the use case using the METplus wrapper and the use case and system config files
    # (for the specified number of times)
    wrapper_confs = settings['wrapper_conf']
    summary_filename, details_filename = files_from_ctrack

    filename = settings['filename']

    for use_case in wrapper_confs:
        # Create the subdirectory for this use case using the use case config file name
        usecase_file = os.path.basename(use_case)
        usecase_subdir_name = usecase_file.split(".")[0]
        usecase_subdir = str(os.path.join(output_base, usecase_subdir_name))
        os.makedirs(usecase_subdir, exist_ok=True)

        # use cases can only be run once for now
        # for _ in range(0, num_of_runs):
        #    metplus_str = os.path.join(settings['metplus_base'], 'ush/run_metplus.py')
        #    subprocess.run(['python', metplus_str, use_case, settings['system_conf']])
        metplus_str = os.path.join(settings['metplus_base'], 'ush/run_metplus.py')
        subprocess.run(['python', metplus_str, use_case, settings['system_conf']])

        # Extract the benchmark data
        summary_info = extract_summary_info(summary_filename, usecase_subdir)
        detail_info = extract_detail_info(details_filename, usecase_subdir)

        # hard-code the run_number, no multiple runs are currently supported
        run_number = 1
        consolidated_df = consolidate_info(summary_info, detail_info, run_number)
        save_results(consolidated_df, output_base, ts, filename, usecase_subdir)

        # provide information about this run: Python version, etc.
        generate_info(settings, ts, usecase_subdir_name, usecase_subdir)


def run_met_cli(settings:dict, ts, files_from_ctrack:tuple) -> None:
    """
       Runs MET for one instance, using the command specified in the benchmarking.yaml config
       file. Stress-testing support is available, the number of times the MET command line command
       is to be run is set in the benchmarking.yaml config file (num_of_runs). By default, the number of runs is
       set to 1.

    :param settings: A dictionary representation of the settings specified in the benchmark.yaml
                     config file.
    :param ts: The timestamp used to generate the filename of the consolidated information
    :param files_from_ctrack: tuple of summary and details files generated by CTRACK
    :return: None The CTRACK benchmarking results are saved to the summary_output.txt and
             detail_output.txt files.
    """

    met_command = settings['met_command']
    summary_filename, details_filename = files_from_ctrack
    full_benchmark_path = str(os.path.join(settings['benchmark_output_path'], settings['met_subdir_name']))
    os.makedirs(full_benchmark_path, exist_ok=True)

    #
    # Run the MET command for the specified number of runs and
    # consolidate all the information for all the runs into one pandas dataframe
    #

    # If the num_runs setting is missing or has no value assigned, set to default
    # value of 1.
    if 'num_runs' in settings and settings['num_runs'] :
        num_of_runs = int(settings['num_runs'])
    else:
        num_of_runs = 1

    for idx, _ in enumerate(range(num_of_runs)):
        print(f"Running MET command, run number: {idx+1}")

        # Remove any leftover summary and detail files from a previous run
        if idx == 0:
            if os.path.isfile(summary_filename) or os.path.isfile(details_filename):
                os.remove(summary_filename)
                os.remove(details_filename)
        subprocess.run(met_command, shell=True)

    # Extract the benchmark data for each run and each metric table generated
    # by CTRACK.
    # **NOTE**: The order of the information from the summary table and detail table
    # generated by CTRACK are NOT identical (i.e. the first row of the summary
    # table is not necessarily the same information as the first row of the
    # detail table).
    summary_info:pd.DataFrame = extract_metrics(summary_filename, full_benchmark_path)
    detail_info:pd.DataFrame = extract_metrics(details_filename, full_benchmark_path)

    # Consolidate all the summary and detail metrics
    consolidated_df:pd.DataFrame = consolidate_info(summary_info, detail_info)

    # get any METPLUS_envs used to run the MET command
    all_envs = os.environ
    mp_envs: str = "MET Environment variables in this run: "
    newline = '\n'

    for cur in all_envs:
        if cur.startswith("MET_"):
             mp_envs = f"{mp_envs} {newline} {cur}: {os.environ[cur]}"


    # Calculate the means of all the metrics in the cols_to_avg list
    if int(settings['num_runs']) > 1:
        avgd_metrics = calc_means_for_metrics(consolidated_df)
    else:
        avgd_metrics = consolidated_df.copy(deep=True)

    # Write out the averaged metrics and any other useful information
    save_results(avgd_metrics, settings['benchmark_output_path'], ts, settings['filename'], settings['met_subdir_name'])
    generate_info(settings, ts, met_command, full_benchmark_path, mp_envs)

    
def calc_means_for_metrics(input_df: pd.DataFrame) -> pd.DataFrame:
    """
    Calculate the mean value for the total_time
    :param input_df: The consolidated summary and detail metrics for each run
    :return: a dataframe containing the average/mean values

    """

    final_mean_df = pd.DataFrame()

    # Create a new index for unique filename/function/line combination for all
    # the runs corresponding to that unique combination
    input_df['new_index'] = input_df.groupby(['filename', 'function', 'line']).ngroup()
    assert input_df.shape[0] >= 1,   "WARNING: input_df is empty "
    unique_indexes = input_df['new_index'].unique()

    # Create a working dataframe that contains only the metrics for this
    # unique filename/function/line number combination.
    # Store the calculated mean values in a new dataframe that will be
    # concatenated into the final_mean_df dataframe.
    for cur_unique in unique_indexes:

        unique_df = input_df.loc[input_df['new_index'] == cur_unique]
        assert  unique_df.shape[0] >= 1,   "WARNING: unique_shape_df is empty "

        for col in ALL_METRICS_COLS:
            # Check for mixed units (i.e. mixture of sec, microsecs, milliseconds, etc.)
            # in the current column of data
            homogeneous_df = homogeneous_units(unique_df, col)

            # Calculate the mean values for metrics based on unique
            # filename/function/line number combination
            assert homogeneous_df.shape[0] >= 1,  "WARNING: homogeneous_df is empty "
            unique_df = homogeneous_df.copy(deep=True)
            # Retrieve only the first row of this initial unique dataframe, this
            # will hold the mean values and any updated units
            num_rows = 1
            unique_df.head(num_rows)
            mean_df = unique_df.iloc[:num_rows, :]
            mean_time = unique_df[col].astype(float).mean()
            idx = mean_df[col].index[0]

            # Make sure any modified units are the best units.
            # In an effort to limit checking every metric and its corresponding unit,
            # only check units that are in 'ns'.  These are the
            # most likely candidates for conversion
            # during the check for homogeneous units
            col_units = col + "_units"
            if mean_df.loc[idx, col_units] == 'ns':
               best_mean_value, best_units =  get_best_units(mean_time, mean_df.loc[idx, col_units])
               mean_df.loc[idx, col] = best_mean_value
               mean_df.loc[idx, col_units] = best_units
            else:
                mean_df.loc[idx, col] = mean_time

        # add this unique filename/function/line  dataframe to the final dataframe
        final_mean_df = pd.concat([final_mean_df, mean_df])
        final_mean_df.drop(['new_index'], axis=1, inplace=True)

    return final_mean_df


def get_best_units(mean_time: [float, int], col_units: str) :
    """
     Convert time values and time units to the most appropriate value to
     avoid very large values (e.g. 1039740 ns, which would be converted to
     1.039740 ms)

     :param mean_time:  The mean value
     :param col_units: The units: 'ns', 'mcs', 'ms', or 'sec'

     :Return:
      tuple of the best time value and time units
    """


    best_time = mean_time

    # nested dictionary, to determine best units ( outer key is the starting unit)
    final_units = {'ns': {0: 'ns', 1: 'mcs', 2: 'ms', 3: 'sec'},
                   'mcs': {0: 'mcs', 1: 'ms', 2: 'sec'},
                   'ms': {0: 'ms', 1: 'sec'}}

    counter = 0
    while best_time > 1000:
        best_time = best_time / 1000
        # increment counter, which will be used to determine the best units
        counter = counter + 1


    best_units = final_units[col_units][counter]



    return best_time, best_units

def homogeneous_units(input_df: pd.DataFrame, time_column: str) -> pd.DataFrame:
    """
      Determine if the units for this particular column are homogeneous (e.g. all
      units are mcs or all units are sec, etc.). If they are homogeneous, return the
      input df. Otherwise, convert the values and units to ns and return the
      updated dataframe.

      :param input_df:  input dataframe
      : param time_column: name of time column to evaluate
      :return:  a dataframe with the columns in ns and values converted to ns.
    """

    units_col = time_column + "_units"
    unique_units: list = input_df[units_col].unique()

    if len(unique_units) == 1:
        return input_df

    else:
        print("Units are not uniform for time column.  Converting all values to nanoseconds...")

        # Do any modifications on a copy of the input df
        working_df = input_df.copy(deep=True)
        units = ['sec', 'ms', 'mcs']

        for u in units:
            # match the unit u to what is in the unit column and replace the
            # existing value with the converted value (in ns) and update the
            # corresponding units to nanoseconds
            if len((working_df.loc[working_df[units_col] == u, time_column]).index) > 0:
                index = ((working_df.loc[working_df[units_col] == u, time_column]).index)[0]
                working_df.loc[index, time_column] = float(working_df.loc[index, time_column]) * TO_NANOSEC[u]
                working_df.loc[index, units_col] = 'ns'

        return working_df


def run_benchmark():
    """
       Run the METplus use case to get benchmark information.
       Extract information from the CTRACK summary and detail files and consolidate into one file.
    """

    # get the timestamp, to be used in naming the output file
    ts = get_timestamp()

    # read in the YAML config file
    settings = {}
    try:
        # get the path to the YAML file
        yaml_path = os.path.dirname(__file__)
        config = os.path.join(yaml_path, "benchmark.yaml")
        benchmark_config = os.getenv("BENCHMARK_YAML_CONFIG_NAME", config)
        settings = parse_config(benchmark_config)
        check_settings(settings)
    except yaml.YAMLError as ye:
        print(ye)

    # The summary_output.txt and detail_output.txt files are saved in the directory where
    # the wrapper command is run
    ctrack_path = os.path.dirname(__file__)
    summary_filename = os.path.join(ctrack_path, "summary_output.txt")
    details_filename = os.path.join(ctrack_path, "detail_output.txt")
    files_from_ctrack = (summary_filename, details_filename)

    run_met = settings['run_met_directly']

    if run_met is True:
        run_met_cli(settings, ts, files_from_ctrack)
    else:
        run_usecases(settings, ts, files_from_ctrack)


if __name__ == "__main__":
    run_benchmark()
