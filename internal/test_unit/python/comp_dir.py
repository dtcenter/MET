#! /usr/bin/env python3

import os
import shutil
import sys

metplus_util_path = os.getenv('METPLUS_UTIL')
sys.path.append(metplus_util_path)

from diff_util import compare_dir

# note: this is how's it done in run_diff_docker.py in METplus...
# GITHUB_WORKSPACE = os.environ.get('GITHUB_WORKSPACE')
# # add util directory to sys path to get diff utility
# diff_util_dir = os.path.join(GITHUB_WORKSPACE,
#                              'metplus',
#                              'util')
# sys.path.insert(0, diff_util_dir)
# from diff_util import compare_dir

# Assume the command run is ./comp_dir.py MET-${1}/test_output MET-${2}/test_output

def comp_dir(truth_dir, output_dir, diff_dir='', debug=True, save_diff=True):
    print('******************************')
    print("Comparing output to truth data")
    diff_files = compare_dir(truth_dir, output_dir,
                             debug=True,
                             save_diff=True)

    # copy difference files into directory
    # so it can be easily downloaded and compared
    if diff_files:
        for truth_file, out_file, _, diff_file in diff_files:
            if truth_file:
                data_dir = truth_dir
                label = '_truth'
            if out_file:
                data_dir = output_dir
                label = '_output'
            if diff_file:
                data_dir = output_dir
                label = ''

    if not diff_dir:
        os.path.join(output_dir, 'diff')

    # replace data dir with diff directory
    # add data type identifier to filename before extension (blank for diff output)
    diff_out = file_path.replace(data_dir, diff_dir)
    output_path, extension = os.path.splitext(diff_out)
    output_path = f'{output_path}{label}{extension}'

    # create output directory if it doesn't exist
    output_dir = os.path.dirname(output_path)
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    try:
        shutil.copyfile(file_path, output_path)
    except OSError as err:
        print(f'Could not copy file. {err}')
        return False

    return True                                


    #=================================================
    ## BELOW IS COPIED FROM comp_dir.R

    # library(ncdf4);

    # # get the MET_TEST_BASE environment variable and include the support scripts
    # met_test_base = system("echo $MET_TEST_BASE", intern=T);
    # if( "" == met_test_base ){
    #     cat("ERROR: environment variable MET_TEST_BASE not set\n\n"); q(status=1);
    # }
    # source(paste(met_test_base, "/R_test/test_const.R", sep=""));
    # source(paste(met_test_base, "/R_test/test_util.R", sep=""));

    # verb = 1;
    # strict = F;
    # hist = 0;		# default histogram plot production
    # file_size_delta = 0.01; # 1% file size difference
    # compare_nc_var = 0;

    # usage = function(){
    #     cat("usage: Rscript comp_dir.R [-v {lev}] [-hist {0|1}] [-strict] [-nc_var] {dir_1} {dir_2}\n",
    #             "  where -v {lev}    indicates verbosity level (0-3), default 1\n",
    #             "        -hist {0|1} 1 to produce histogram error plots for each file, default 0\n",
    #             "        -nc_var     compare NetCDF variables, default: no\n",
    #             "        -comp_nc_var     compare NetCDF variables, default: no\n",
    #             "        -compare_nc_var  compare NetCDF variables, default: no\n",
    #             "        -strict     applies strict equality when comparing numerical values, default false\n\n",
    #             sep="");
    # }

    # # parse and verify command line arguments
    # listArgs = commandArgs(TRUE);
    # while( 2 < length(listArgs) ){
    #     if( "-v" == listArgs[1] ){
    #         verb = listArgs[2];
    #         listArgs = listArgs[3:length(listArgs)];
    #     } else if( "-hist" == listArgs[1] ){
    #         hist = listArgs[2];
    #         listArgs = listArgs[3:length(listArgs)];
    #     } else if( "-strict" == listArgs[1] ){
    #         strict = 1;
    #         listArgs = listArgs[2:length(listArgs)];
    #     } else if( "-compare_nc_var" == listArgs[1] || "-comp_nc_var" == listArgs[1] || "-nc_var" == listArgs[1] ){
    #         compare_nc_var = 1;
    #         listArgs = listArgs[2:length(listArgs)];
    #     } else {
    #         cat("ERROR: unrecognized option:", listArgs[1], "\n\n"); usage(); q(status=1);
    #     }
    # }
    # if( 2 != length(listArgs) ){ 
    #     cat("ERROR: invalid number of arguments\n\n"); usage(); q(status=1);
    # }
    # strDir1 = gsub("/$", "", listArgs[1]);
    # strDir2 = gsub("/$", "", listArgs[2]);

    # # build a list of files in each stat folder
    # listTest1 = system(paste("find", strDir1, "| egrep '\\.stat$|\\.txt$|\\.tcst|\\.nc$|\\.out$|\\.ps$|\\.png$|\\.dat$' | sort"), intern=T);
    # listTest1Files = gsub(paste(strDir1, "/", sep=""), "", listTest1);
    # listTest2 = system(paste("find", strDir2, "| egrep '\\.stat$|\\.txt$|\\.tcst|\\.nc$|\\.out$|\\.ps$|\\.png$|\\.dat$' | sort"), intern=T);
    # listTest2Files = gsub(paste(strDir2, "/", sep=""), "", listTest2);

    # if( 1 <= verb ){ cat("dir1:", strDir1, "contains", length(listTest1Files), "files\n");
    #                 cat("dir2:", strDir2, "contains", length(listTest2Files), "files\n\n"); }

    # if( 5 <= verb ){
    #     boolRmTmp = FALSE;
    # }
                    
    # # report files missing from stat folder 1
    # listMiss = listTest2Files[ !(listTest2Files %in% listTest1Files) ];
    # if( 0 < length(listMiss) ){
    #     if( 1 <= verb ){ 
    #         cat("ERROR: folder", strDir1, "missing", length(listMiss), "files\n");
    #         for(strMiss in listMiss){ cat("   ", strMiss, "\n"); }
    #     } else {
    #         quit(status=1);
    #     }
    # }

    # # report files missing from stat folder 2
    # listMiss = listTest1Files[ !(listTest1Files %in% listTest2Files) ];
    # if( 0 < length(listMiss) ){
    #     if( 1 <= verb ){ 
    #         cat("ERROR: folder", strDir2, "missing", length(listMiss), "files\n");
    #         for(strMiss in listMiss){ cat("   ", strMiss, "\n"); }
    #     } else {
    #         quit(status=1);
    #     }
    # }

    # # compare the files that are common to both stat folders
    # for(strFile in listTest1Files[ listTest1Files %in% listTest2Files ]){

    # if( 1 <= verb ){
    #     cat("\n# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #\n\n",
    #         "COMPARING ", strFile, "\n", sep="");
    # }

    #     # build the two stat file names
    #     strFile1 = paste(strDir1, "/", strFile, sep="");
    #     strFile2 = paste(strDir2, "/", strFile, sep="");

    #     # if the files are NetCDF, compare accordingly
    #     if( TRUE == grepl("\\.nc$", strFile1, perl=T) ){
    #         if( 1 <= verb ){ cat("file1: ", strFile1, "\nfile2: ", strFile2, "\n", sep=""); }
    #         compareNc(strFile1, strFile2, verb, strict, file_size_delta, compare_nc_var);
    #     }

    #     # if the files are PostScript, PNG, or end in .out or .dat, compare accordingly
    #     else if( TRUE == grepl("\\.out$", strFile1, perl=T) ||
    #                 TRUE == grepl("\\.ps$",  strFile1, perl=T) ||
    #                 TRUE == grepl("\\.png$", strFile1, perl=T) || 
    #                 TRUE == grepl("\\.dat$", strFile1, perl=T) ){
    #         if( 1 <= verb ){ cat("file1: ", strFile1, "\nfile2: ", strFile2, "\n", sep=""); }
    #         compareDiff(strFile1, strFile2, verb);
    #     }

    #     # compare the stat files and print a report
    #     else {
    #         strHistFile = "";
    #         if( 1 == hist ){
    #             strHistFile = gsub("\\.stat$", ".png", strFile);
    #             strHistFile = gsub("\\.tcst$", ".png", strHistFile);
    #             strHistFile = gsub("\\.txt$",  ".png", strHistFile);
    #         }
    #         if( 1 <= verb ){ cat("file1: ", strFile1, "\nfile2: ", strFile2, "\n", sep=""); }
    #         status = try(listTest <- compareStat(strFile1, strFile2, verb, strict));
    #         if(class(status) == "try-error") {
    #             cat("ERROR: compareStat() failed\n\n");
    #         } else {
    #             printCompReport(listTest, verb, strHistFile);
    #         }
    #     }

    # }
    # if( 1 <= verb ){
    # cat("\n# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #\n\n",
    #     "comp_dir complete\n");
    # }

    #=================================================


    #=================================================
    ## BELOW IS COPIED from metplus / run_diff_docker.py

    # def main():
    # print('******************************')
    # print("Comparing output to truth data")
    # diff_files = compare_dir(TRUTH_DIR, OUTPUT_DIR,
    #                          debug=True,
    #                          save_diff=True)

    # # copy difference files into directory
    # # so it can be easily downloaded and compared
    # if diff_files:
    #     copy_diff_output(diff_files)

    # def copy_diff_output(diff_files):
    # """!  Loop through difference output and copy files
    # to directory so it can be made available for comparison.
    # Files will be put into the same directory with _truth or
    # _output added before their file extension.

    # @param diff_files list of tuples containing truth file path
    #  and file path of output that was just generated. Either tuple
    #  value may be an empty string if the file was not found.
    # """
    # for truth_file, out_file, _, diff_file in diff_files:
    #     if truth_file:
    #         copy_to_diff_dir(truth_file,
    #                          'truth')
    #     if out_file:
    #         copy_to_diff_dir(out_file,
    #                          'output')
    #     if diff_file:
    #         copy_to_diff_dir(diff_file,
    #                          'diff')

    # def copy_to_diff_dir(file_path, data_type):
    #     """! Generate output path based on input file path,
    #     adding text based on data_type to the filename, then
    #     copy input file to that output path.

    #     @param file_path full path of file to copy
    #     @param data_type data identifier, should be 'truth'
    #     or 'output'
    #     @returns True if success, False if there was a problem
    #     copying the file
    #     """
    #     if data_type == 'truth':
    #         data_dir = TRUTH_DIR
    #     else:
    #         data_dir = OUTPUT_DIR

    #     # replace data dir with diff directory
    #     diff_out = file_path.replace(data_dir, DIFF_DIR)

    #     # add data type identifier to filename before extension
    #     # if data is not difference output
    #     if data_type == 'diff':
    #         output_path = diff_out
    #     else:
    #         output_path, extension = os.path.splitext(diff_out)
    #         output_path = f'{output_path}_{data_type}{extension}'

    #     # create output directory if it doesn't exist
    #     output_dir = os.path.dirname(output_path)
    #     if not os.path.exists(output_dir):
    #         os.makedirs(output_dir)

    #     try:
    #         shutil.copyfile(file_path, output_path)
    #     except OSError as err:
    #         print(f'Could not copy file. {err}')
    #         return False

    #     return True



if __name__ == "__main__":
    import argparse

    # Assume the command run is ./comp_dir.py MET-${1}/test_output MET-${2}/test_output
    parser = argparse.ArgumentParser(description="Compare output files between two directories.")
    parser.add_argument('dir_1',
                        help='"truth" or "ref" version test_output')
    parser.add_argument('dir_2')
    #note: the args below are not currently supported
    parser.add_argument('-v', default=1, choices=[0,1,2,3],
                        help='indicates verbosity level (0-3), default 1')
    parser.add_argument('-hist', default=0, choices=[0,1],
                        help='1 to produce histogram error plots for each file')
    parser.add_argument('-nc_var', action='store_true',
                        help='if present, compare NetCDF variables')
    parser.add_argument('-strict', action='store_true',
                        help='applies strict equality when comparing numerical values')
    args = parser.parse_args()

    comp_dir(truth_dir=args.dir_1, output_dir=args.dir_2)

