# do we need this? / is this right?
#!/usr/bin/python

#add imports...
#import lxml     #note: developing with v 4.9.3
from datetime import datetime as dt
import os
from pathlib import Path
import re
import subprocess
import xml.etree.ElementTree as ET

# use lib "$ENV{'MET_TEST_BASE'}/lib";



# this stuff will go into if __name__==__main__...

# sub usage {
#   print "usage: $0 [-log {log_file}] [-cmd] [-memchk] [-callchk] [-noexit] {test_xml}

#   where:
#         log: if present, write output from each test to the specified file
#         cmd: if present, print the test commands but do not run them, 
#                overrides -log
#      memchk: if present, activate valgrind with memcheck
#     callchk: if present, activate valgrind with callcheck
#      noexit: if present, the unit tester will continue executing subsequent
#                tests when a test fails
#    test_xml: file containing the unit test(s) to perform

# ";
# }
# # parse the input options
# my ($mgnc, $mpnc, $file_log, $cmd_only, $noexit, $memchk, $callchk);
# GetOptions("log=s" => \$file_log, "cmd" => \$cmd_only, "noexit" => \$noexit,
#            "memchk" => \$memchk, "callchk" => \$callchk)
#   or die "ERROR: parsing input options";


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

    # read command arguments  --> could do this in if "name"="main"
   
    # # if command  only mode is enabled, disable logging
    # $cmd_only and $file_log = 0;

    # # open the log file, if requested
    # open(my $fh_log, ">", $file_log) or die "ERROR: unable to open log file $file_log\n"
    #   if $file_log;

    
    # parse xml file
    try:
        test_root = ET.parse(test_xml)
    except e:
        print(e)
        # and exit?


    # # parse the children of the met_test element
    if test_root.getroot().tag != 'met_test':
        print(f"ERROR: unexpected top-level element. Expected 'met_test', got '{test_root.tag}'")
        # and exit?
    else:
        tests = build_tests(test_root)

    # # determine the max length of the test names
    name_wid = max([len(test['name']) for test in tests])
    
    

    # # default return value
    # my $ret_val = 0;
    VALGRIND_OPT_MEM ="--leak-check=full --show-leak-kinds=all --error-limit=no -v"
    VALGRIND_OPT_CALL ="--tool=callgrind --dump-instr=yes --simulate-cache=yes --collect-jumps=yes"

    # # run each test
    for test in tests:
    #   # print the test name
        print(f"TEST: {test['name']}")
    #   $cmd_only or printf "TEST: %-*s - ", $name_wid, $test->{"name"};

    #   # prepare the output space
        output_keys = [key for key in test.keys() if key.startswith('out_')]
        outputs = [test[key] for key in output_keys]
        for output in outputs:
            try:
                Path(output).unlink()
            except FileNotFoundError:
                pass
            except e:
                print(e)
            # calls a perl function from VxUtil.pm (in test_unit/lib) to make the directory 
            output_dir = Path(output).parent
            output_dir.mkdir(parents=True, exist_ok=True)     #should error be raised if dir already exists? 

    #   my @outputs = ( @{$test->{"out_pnc"}}, @{$test->{"out_gnc"}}, @{$test->{"out_stat"}}, @{$test->{"out_ps"}}, @{$test->{"out_exist"}}, @{$test->{"out_not_exist"}} );
    #   for my $output ( @outputs ){
    #     -s $output and qx/rm -rf $output/;
    #     vx_file_mkdir($output);
    #   }
        # sub vx_file_mkdir {
        #   while( my $strFile = shift ){
        #     (my $strDir = $strFile) =~ s/(.*\/).*/$1/;
        #     qx/mkdir -p $strDir/;
        #   }
        # }

    #   # set the test environment variables
        if 'env' in test.keys():
            for key, val in test['env'].items():
                if val:
                    os.environ[key] = val

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
            for key, val in sorted(test['env'].items()):
                print(f"export {key}={val}")
            print(f"{cmd}")
            for key, val in sorted(test['env'].items()):
                print(f"unset {key}")
            print("\n")
    
    #   # run and time the test command
        t_start = dt.now()
        cmd_args = [arg for arg in cmd.split() if arg!='\\'] + ['2>&1']
        cmd_outs = subprocess.run(cmd_args, capture_output=True)  #what should this actually contain?
        t_elaps = dt.now() - t_start
        # unshift @cmd_outs, "$cmd\n";

    #   my $t_start = [gettimeofday];
    #   my @cmd_outs = qx{$cmd 2>&1};
    #   my $t_elaps = tv_interval( $t_start );
    #   unshift @cmd_outs, "$cmd\n";

    #   # check the return status and output files
    #   $ret_ok = (! $?);
    #   if( $ret_ok ){
    #     $out_ok = 1;
    #     do{ push @cmd_outs, qx/$mpnc -v $_/; $? and $out_ok = 0 } for ( @{ $test->{"out_pnc"} } );
    #     do{ push @cmd_outs, qx/$mgnc -v $_/; $? and $out_ok = 0 } for ( @{ $test->{"out_gnc"} } );
    #     for my $output ( @{ $test->{"out_stat"} } ){
    #       -s $output or 
    #         push @cmd_outs, "ERROR: stat file missing \"$output\"\n" and $out_ok = 0 and next;
    #       int(qx{cat $output 2>/dev/null | egrep -v '^VERSION' | wc -l}) or
    #         push @cmd_outs, "ERROR: stat data missing from file \"$output\"\n" and $out_ok = 0;
    #     }
    #     for my $output ( @{ $test->{"out_ps"} } ){
    #       -s $output or 
    #         push @cmd_outs, "ERROR: postscript file missing \"$output\"\n" and $out_ok = 0 and next;
    #       !(qx{gs -sDEVICE=nullpage -dQUIET -dNOPAUSE -dBATCH $output 2>/dev/null}) or
    #         push @cmd_outs, "ERROR: ghostscript error for postscript file \"$output\"\n" and $out_ok = 0;
    #     }
    #     for my $output ( @{ $test->{"out_exist"} } ){
    #       -s $output or 
    #         push @cmd_outs, "ERROR: file missing when it should exist \"$output\"\n" and $out_ok = 0 and next;
    #     }
    #     for my $output ( @{ $test->{"out_not_exist"} } ){
    #       !(-s $output) or 
    #         push @cmd_outs, "ERROR: file exists when it should be missing \"$output\"\n" and $out_ok = 0 and next;
    #     }
        
    #   }

    #   # unset the test environment variables
    #   delete $ENV{$_} for keys %{ $test->{"env"} };
    
    #   # print the test result
    #   printf "%s - %7.3f sec\n", $ret_ok && $out_ok ? "pass" : "FAIL", sprintf("%7.3f", $t_elaps);

    #   # build a list of environment variable exports and unsets for reporting
    #   my @set_envs;
    #   push @set_envs, "export $_=\'" . $test->{"env"}{$_} . "\'\n" for sort keys %{ $test->{"env"} };
    #   my @unset_envs;
    #   push @unset_envs, "unset $_\n" for sort keys %{ $test->{"env"} };

    #   # if the log file is activated, print the test information
    #   if( $file_log ){
    #     print $fh_log "\n\n";
    #     print $fh_log "$_" for (@set_envs, @cmd_outs, @unset_envs);
    #     print $fh_log "\n\n";
    #   }

    #   # on failure, print the problematic test and exit, if requested
    #   if( !($ret_ok && $out_ok) ){
    #     print "$_" for (@set_envs, @cmd_outs, @unset_envs);
    #     $noexit or exit 1;
    #     print "\n\n";
    #   }

    # }
    return tests, cmd


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

    # read test_dir
    test_dir = test_root.find('test_dir').text
    #mgnc = ?
    #mpnc = ?

    # find all tests in test_xml, and create a dictionary of attributes for each test
    test_list = []
    for test_el in test_root.iter('test'):
        test = {}
        test['name'] = test_el.attrib['name']
        # check that name exists or "ERROR: name attribute not found for test ", $test_idx++ . "\n";

        for el in test_el:
            if (el.tag=='exec' or el.tag=='param'):
                test[el.tag] = repl_env(el.text)
            elif el.tag=='output':
                output_names = {
                    'point_nc' : 'out_pnc',
                    'grid_nc' : 'out_gnc',
                    'stat' : 'out_stat',
                    'ps' : 'out_ps',
                    'exist' : 'out_exist',
                    'not_exist' : 'out_not_exist',
                }
                for output_el in el:
                    test[output_names[output_el.tag]] = repl_env(output_el.text)

            elif el.tag=='env':
                env_dict = {}
                for env_el in el:
                    env_dict[env_el.find('name').text] = env_el.find('value').text
                    # check for missing names/values:
#                   $pair_name or die "ERROR: env pair in test \"" . $test{"name"} . "\" " .
#                               "missing name or value\n";                          

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
    string_with_env : str
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
            # will want to check for no envar found and log:
            #   "ERROR: environment variable $1 not found\n"
            string_with_ref = string_with_ref.replace(envar_ref, envar)

    return string_with_ref


