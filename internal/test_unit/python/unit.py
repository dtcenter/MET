#!/usr/bin/python

from datetime import datetime as dt
import logging
import os
from pathlib import Path
import re
import subprocess
import sys
import xml.etree.ElementTree as ET

def unit(test_xml, file_log=None, cmd_only=False, noexit=False, memchk=False, callchk=False):
    """
    unit testing script

    Parameters
    -----------
    mgnc : 
    mpnc : 
    test_xml : pathlike
        path to file containing the unit test(s) to perform
    file_log : pathlike, default None
        if present, write output from each test to the specified file
    cmd_only : bool, default False
        if true, print the test commands but do not run them (overrides file_log)
    noexit : bool, default False
        if true, the unit tester will continue executing subsequent
                tests when a test fails
    memchk : bool, default False
        if true, activate valgrind with memcheck
    callchk : bool, default False
        if true, activate valgrind with callcheck
    """
  
    # initialize logger
    logger = logging.getLogger(__name__)
    logger.setLevel(logging.DEBUG)

    # create/add console handler
    ch = logging.StreamHandler()
    ch.setLevel(logging.INFO)
    logger.addHandler(ch)

    # create/add file handler
    if file_log:
        fh = logging.FileHandler(file_log)
        fh.setLevel(logging.DEBUG)
        logger.addHandler(fh)

    # # if command  only mode is enabled, disable logging
    # $cmd_only and $file_log = 0;
    
    # parse xml file
    try:
        test_root = ET.parse(test_xml)
    except Exception as e:
        logger.exception(f"Unable to parse xml from {test_xml}")
        raise

    # # parse the children of the met_test element
    if test_root.getroot().tag != 'met_test':
        logger.error(f"unexpected top-level element. Expected 'met_test', got '{test_root.tag}'")
        sys.exit(1)
    else:
        # read test_dir
        try:
            test_dir = test_root.find('test_dir').text
            mgnc = repl_env(test_dir + '/bin/mgnc.sh')
            mpnc = repl_env(test_dir + '/bin/mpnc.sh')
        except Exception as e:
            logger.exception(f"unable to read test_dir from {test_xml}")
            raise

        tests = build_tests(test_root)

    # # determine the max length of the test names
    name_wid = max([len(test['name']) for test in tests])
    
    

    # # default return value
    # my $ret_val = 0;
    VALGRIND_OPT_MEM ="--leak-check=full --show-leak-kinds=all --error-limit=no -v"
    VALGRIND_OPT_CALL ="--tool=callgrind --dump-instr=yes --simulate-cache=yes --collect-jumps=yes"

    #for debug:
    #cmd_args_list = []
    
    # # run each test
    for test in tests:
    #   # print the test name
        logger.debug("\n")
        logger.info(f"TEST: {test['name']}")
    #   $cmd_only or printf "TEST: %-*s - ", $name_wid, $test->{"name"};

    #   # prepare the output space
        output_keys = [key for key in test.keys() if key.startswith('out_')]
        outputs = [output for key in output_keys for output in test[key]]
        for output in outputs:
            try:
                Path(output).unlink()
            except FileNotFoundError:
                pass
            except Exception as e:
                logger.exception()
                raise
            # calls a perl function from VxUtil.pm (in test_unit/lib) to make the directory 
            output_dir = Path(output).parent
            output_dir.mkdir(parents=True, exist_ok=True)     #should error be raised if dir already exists? 

    #   # set the test environment variables
        if 'env' in test.keys():
            for key, val in sorted(test['env'].items()):
                #if val:
                    os.environ[key] = val
                    logger.debug(f"export {key}={val}")

    #   # build the text command
        cmd = test['exec'] + test['param']
        cmd = re.sub('[ \n\t]+$', '', cmd)  # not sure this is doing what it should;
                                            #  may need to remove the +$ from the regex?
    #   my ($cmd, $ret_ok, $out_ok) = ($test->{"exec"} . $test->{"param"});
        if memchk:
            cmd = f"valgrind {VALGRIND_OPT_MEM} {cmd}"
        elif callchk:
            cmd = f"valgrind {VALGRIND_OPT_CALL} {cmd}"

        
    #   # if writing a command file, print the environment and command, then loop
        if cmd_only:
            if 'env' in test.keys():
                for key, val in sorted(test['env'].items()):
                    print(f"export {key}={val}")
            print(f"{cmd}")
            if 'env' in test.keys():
                for key, val in sorted(test['env'].items()):
                    print(f"unset {key}")
            print("\n")
    
    #   # run and time the test command
        else:
            logger.debug(f"{cmd}")
            cmd_args = [arg.strip('\\') for arg in cmd.split() if arg!='\\']
            # cmd_args = [arg.strip() for arg in cmd.split('\\\n')]   #this could work also?
            #cmd_args_list.append(cmd_args)  #debug
            t_start = dt.now()
            cmd_return = subprocess.run(cmd_args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)  #should retun STDERR and STDOUT (as list?)
            t_elaps = dt.now() - t_start

            cmd_outs = cmd_return.stdout
            logger.debug(f"{cmd_outs}")
            logger.debug(f"Return code: {cmd_return.returncode}")
            

        #   # check the return status and output files
            ret_ok = not cmd_return.returncode
            if ret_ok:
                out_ok = True

                try:
                    for filepath in test['out_pnc']:
                        result = subprocess.run([mpnc, '-v', filepath], 
                                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
                        cmd_outs += ("\n"+result.stdout)
                        logger.debug(result.stdout)
                        out_ok = not result.returncode
                except KeyError:
                    pass
                
                try:
                    for filepath in test['out_gnc']:
                        result = subprocess.run([mgnc, '-v', filepath], 
                                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
                        cmd_outs += ("\n"+result.stdout)
                        logger.debug(result.stdout)
                        out_ok = not result.returncode
                except KeyError:
                    pass
                
                try:
                    for filepath in test['out_stat']:
                        # check stat file exists and is nonzero size
                        try:
                            filesize = os.stat(filepath).st_size
                            if filesize==0:
                                cmd_outs += (f"\nERROR: stat file empty {filepath}\n")
                                out_ok = False
                                break
                        except FileNotFoundError:
                            cmd_outs += (f"\nERROR: stat file missing {filepath}\n")
                            logger.debug(result.stdout)
                            out_ok = False
                            break
                        # check stat file has non-header lines
                        with open(filepath) as f:
                            numlines = len([l for l in f.readlines() if not l.startswith('VERSION')])
                        if numlines==0:
                            cmd_outs += (f"\nERROR: stat data missing from file {filepath}\n")
                            out_ok = False
                except KeyError:
                    pass

                try:
                    for filepath in test['out_ps']:
                        # check postscript file exists and is nonzero size
                        try:
                            filesize = os.stat(filepath).st_size
                            if filesize==0:
                                cmd_outs += (f"\nERROR: postscript file empty {filepath}\n")
                                out_ok = False
                                break
                        except FileNotFoundError:
                            cmd_outs += (f"\nERROR: postscript file missing {filepath}\n")
                            out_ok = False
                            break
                        # check for ghostscript errors
                        result = subprocess.run(['gs', '-sDEVICE=nullpage', '-dQUIET', '-dNOPAUSE', '-dBATCH', filepath])
                        if result.returncode:
                            cmd_outs += (f"\nERROR: ghostscript error for postscript file {filepath}")
                            out_ok = False
                except KeyError:
                    pass
                
                try:
                    for filepath in test['out_exist']:
                        # check output file exists and is nonzero size
                        try:
                            filesize = os.stat(filepath).st_size
                            if filesize==0:
                                cmd_outs += (f"\nERROR: file empty {filepath}\n")
                                out_ok = False
                                break
                        except FileNotFoundError:
                            cmd_outs += (f"\nERROR: file missing when it should exist {filepath}\n")
                            out_ok = False
                except KeyError:
                    pass

                try:
                    for filepath in test['out_not_exist']:
                        # check output file doesn't exist
                        if os.path.isfile(filepath):
                            cmd_outs += (f"\nERROR: file exists when it should be missing {filepath}\n")
                            out_ok = False
                except KeyError:
                    pass

        #   # unset the test environment variables
        if 'env' in test.keys():
            for key, val in sorted(test['env'].items()):
                # if val:
                    del os.environ[key]
                    logger.debug(f"unset {key}")
        
        #   # print the test result
        test_result = "pass" if (ret_ok and out_ok) else "FAIL"
        logger.info(f"{test_result} - {round(t_elaps.total_seconds(),3)} sec\n")

        #   # if the log file is activated, print the test information
        # would like to redo this so the log is being written as commands are executed... not after
        # also, this could be done simultaneously with cmd_only option
        # might want to improve formatting here too
        # if file_log:
        #     logger.debug("\n\n")
        #     if 'env' in test.keys():
        #         for key, val in sorted(test['env'].items()):
        #             logger.debug(f"export {key}={val}")
        #     logger.debug(f"{cmd}")
        #     if 'env' in test.keys():
        #         for key, val in sorted(test['env'].items()):
        #             logger.debug(f"unset {key}")
        #     logger.debug("\n")

        #   # on failure, print the problematic test and exit, if requested
        if not (ret_ok and out_ok):
            logger.info(cmd + cmd_outs)   #skipping the setting/unsetting envs here ?
        #     print "$_" for (@set_envs, @cmd_outs, @unset_envs);
            if not noexit:
                sys.exit(1)
        #     print "\n\n";

    return tests, #cmd_args_list


def build_tests(test_root):
    """
    # #   This function assumes that the inputs are the body elements of
    # #   a parsed XML file using XML::Parser.  The components of each test
    # #   element are parsed and the test elements are returned as an array
    # #   of hashes.
    # #
    Parameters
    ----------
    test_root : ElementTree element
        parsed from XML file containing the unit test(s) to perform

    """

    # find all tests in test_xml, and create a dictionary of attributes for each test
    test_list = []
    for test_el in test_root.iter('test'):
        test = {}
        try:
            test['name'] = test_el.attrib['name']
        except KeyError:
            logger.error("name attribute not found for test [test index]")
            raise
            # should just fail this one test and not fully exit?

        for el in test_el:
            if (el.tag=='exec' or el.tag=='param'):
                test[el.tag] = repl_env(el.text)
            elif el.tag=='output':
                test['out_pnc'] = []
                test['out_gnc'] = []
                test['out_stat'] = []
                test['out_ps'] = []
                test['out_exist'] = []
                test['out_not_exist'] = []
                output_names = {
                    'point_nc' : 'out_pnc',
                    'grid_nc' : 'out_gnc',
                    'stat' : 'out_stat',
                    'ps' : 'out_ps',
                    'exist' : 'out_exist',
                    'not_exist' : 'out_not_exist',
                }
                for output_el in el:
                    test[output_names[output_el.tag]].append(repl_env(output_el.text))

            elif el.tag=='env':
                env_dict = {}
                for env_el in el:
                    try:
                        env_name = env_el.find('name').text
                        env_dict[env_name] = env_el.find('value').text
                        if not env_dict[env_name]:
                            env_dict[env_name] = ''
                    except AttributeError:
                        logger.error(f"env pair in test \\{test['name']}\\ missing name or value")
                        raise
                        # should just fail this one test and not fully exit?                      

                test['env'] = env_dict

        #   validate test format/details
        test_list.append(test)
        #     # verify the structure of the test element
        #     $test{"exec"}   or die "ERROR: test " . $test{"name"} . " missing exec element\n";
        #     $test{"param"}  or die "ERROR: test " . $test{"name"} . " missing param element\n";
        #     ( $test{"out_pnc"} && $test{"out_gnc"} && $test{"out_stat"} && $test{"out_ps"} && $test{"out_exist"} && $test{"out_not_exist"} ) 
        #                     or die "ERROR: test " . $test{"name"} . " missing output element\n";

    return test_list


def repl_env(string_with_ref):
    """
    Take a string with a placeholder for environment variable with syntax
    ${ENV_NAME} and replace placeholder with corresponding value of environment
    variable.

    Parameters
    ----------
    string_with_ref : str
        A string, generally path-like, that includes substring ${ENV_NAME}

    Returns
    -------
    string_with_ref : str
        The provided string with ${ENV_NAME} replaced by corresponding environment variable
    """

    envar_ref_list = re.findall('\$\{\w+}', string_with_ref)
    envar_ref_unique = [
        envar_ref_list[i] for i in list(range(len(envar_ref_list))) if (
            envar_ref_list[i] not in envar_ref_list[:i])]

    if len(envar_ref_unique)>0:
        for envar_ref in envar_ref_unique:
            envar_name = envar_ref[2:-1]
            envar = os.getenv(envar_name)
            if not envar:
                logger.error(f"ERROR: environment variable {envar_name} not found")
            string_with_ref = string_with_ref.replace(envar_ref, envar)

    return string_with_ref

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Run a unit test.")
    parser.add_argument('test_xml', )
    parser.add_argument('-log', metavar='log_file',
                        help='if present, write output from each test to log_file')
    parser.add_argument('-cmd', action='store_true',
                        help='if present, print the test commands but do not run them, overrides -log')
    parser.add_argument('-memchk', action='store_true',
                        help='if present, activate valgrind with memcheck')
    parser.add_argument('-callchk', action='store_true',
                        help='if present, activate valgrind with callcheck')
    parser.add_argument('-noexit', action='store_true',
                        help='if present, the unit tester will continue executing subsequent tests when a test fails')
    args = parser.parse_args()

    unit(test_xml=args.test_xml, file_log=args.log, cmd_only=args.cmd, noexit=args.noexit, memchk=args.memchk, callchk=args.callchk)


