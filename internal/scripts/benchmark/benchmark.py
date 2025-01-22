

# ============================*
# ** Copyright UCAR (c) 2020
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
     :param: subdir: the use case subdirectory
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


def consolidate_info(summary_df:pd.DataFrame, detail_df:pd.DataFrame) -> pd.DataFrame:
    """
       Consolidate the summary and detail information for each filename/function/line combination.
       Create a

    :param summary_df: pandas dataframe containing summary benchmark info
    :param detail_df: pandas dataframe containing detail benchmark info
    :return:  pandas Dataframe
   """

    merged = summary_df.merge(detail_df, on=["filename", "function", "line"], how="left")

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
    pattern = re.compile('.*?\${(\w+)}.*?')
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

def save_results(consolidated:pd.DataFrame, output_dir:str, ts:str, filename, subdir:str) -> None:
    """
       Save the consolidated results into a csv file and a tabular file

    :param consolidated: pandas dataframe containing the summary and detail report results from CTRACK
    :param output_dir: the directory where the consolidated file(s) is/are saved
    :param ts: The timestamp used to create the output filename
    :param filename: The user-specified filename, if empty string, then the timestamp will be used
    :param subdir: The subdir corresponding to this use case (based on the wrapper conf file name)
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

    consolidated.to_csv(full_csv_output_file,header=True)
    consolidated.to_csv(full_table_output_file, header=True, sep="\t")

    # Do some checking to make sure the files were actually created and they aren't empty
    assert os.path.isfile(full_csv_output_file), "WARNING: csv output file not created"
    assert os.path.isfile(full_table_output_file), "WARNING: tabular output file not created"



def check_settings(settings:dict) -> None:
    """
      Check that paths specified in the config file exist

      :param settings: dictionary representation of settings specified in the YAML config file
      :return:  None
    """

    metplus_dir = settings['metplus_base']
    sys_conf = settings['system_conf']
    wrapper_confs = settings['wrapper_conf']

    assert os.path.exists(metplus_dir), "fERROR|benchmark.yaml::The METplus base directory {metplus_dir} does not exist"
    assert os.path.exists(sys_conf), "fERROR|benchmark.yaml::The system.conf file {sys_conf}  does not exist."

    for cur_conf in wrapper_confs:
        assert os.path.exists(cur_conf), "fERROR|benchmark.yaml:: The {cur_conf} use case config file does not exist. "


def generate_info(settings:dict, ts:str, usecase: str, subdir: str) -> None:
    """
       Generate a text file with information on the current benchmark run
    :param settings: dictionary representation of the settings specified in the YAML config file
    :param ts: timestamp
    :param usecase: current use case
    :param subdir: the use case subdirectory (full path)
    :return: None, write an output text file in the output path specified in the YAML config file
    """
    info_file = "info_"+ usecase+ '_' + ts + ".txt"
    full_path = os.path.join(subdir, info_file)
    with open(full_path, 'w') as f:
        f.write(f"Python version info: {sys.version}\n")
        f.write(f"Timestamp: {ts}\n")
        f.write(f"Use case : {usecase}\n")
        f.write(f"Number of times run: {settings['num_runs']}\n")




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

    output_base = settings['benchmark_output_path']
    # if the base output dir does not exist, create it
    os.makedirs(output_base, exist_ok=True)

    num_of_runs = settings['num_runs']
    # Set the number of times to run to 1 if this value isn't set in the YAML config file
    if num_of_runs == '' or num_of_runs is None:
        num_of_runs = 1

    filename = settings['filename']

    # Run the use case using the METplus wrapper and the use case and system config files
    # (for the specified number of times)
    wrapper_confs = settings['wrapper_conf']
    for use_case in wrapper_confs:
        # Create the subdirectory for this use case using the use case config file name
        usecase_file = os.path.basename(use_case)
        usecase_subdir_name = usecase_file.split(".")[0]
        usecase_subdir = str(os.path.join(output_base, usecase_subdir_name))
        os.makedirs(usecase_subdir, exist_ok=True)

        for _ in range(0, num_of_runs):
           metplus_str = os.path.join(settings['metplus_base'], 'ush/run_metplus.py')
           subprocess.run(['python', metplus_str, use_case, settings['system_conf']])

        # Extract the benchmark data
        summary_info = extract_summary_info(summary_filename, usecase_subdir)
        detail_info = extract_detail_info(details_filename, usecase_subdir)
        consolidated_df = consolidate_info(summary_info, detail_info)
        save_results(consolidated_df, output_base, ts, filename, usecase_subdir)

        # provide information about this run: Python version, etc.
        generate_info(settings, ts, usecase_subdir_name, usecase_subdir)


if __name__ == "__main__":
    run_benchmark()
