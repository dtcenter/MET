# do we need this? / is this right?
#!/usr/bin/python

#add imports...
#import lxml     #note: developing with v 4.9.3
import os
import re
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

    # read command arguments

    # parse the test xml file parameter

    # # if command  only mode is enabled, disable logging
    # $cmd_only and $file_log = 0;

    # # open the log file, if requested
    # open(my $fh_log, ">", $file_log) or die "ERROR: unable to open log file $file_log\n"
    #   if $file_log;

    
    # parse xml file
    test_root = ET.parse(test_xml)

    # # report any error that stopped parsing
    # if( $@ ){
    #   $@ =~ s/at \/.*?$//s;
    #   die "ERROR: parsing error $@\n";
    # }

    # # parse the children of the met_test element
    # $tree->[0] eq "met_test" or die "ERROR: unexpected top-level element " . $tree->[0] . "\n";
    # my @tests = build_tests( @{ $tree->[1] } );
    tests = build_tests(test_root)

    return tests

# # determine the max length of the test names
# my $name_wid = 12;
# $name_wid < length($_->{"name"}) and $name_wid = length($_->{"name"}) for @tests;

# # default return value
# my $ret_val = 0;
# my $VALGRIND_OPT_MEM ="--leak-check=full --show-leak-kinds=all --error-limit=no -v";
# my $VALGRIND_OPT_CALL ="--tool=callgrind --dump-instr=yes --simulate-cache=yes --collect-jumps=yes";


# # run each test
# for my $test (@tests){

#   # print the test name
#   $cmd_only or printf "TEST: %-*s - ", $name_wid, $test->{"name"};

#   # prepare the output space
#   my @outputs = ( @{$test->{"out_pnc"}}, @{$test->{"out_gnc"}}, @{$test->{"out_stat"}}, @{$test->{"out_ps"}}, @{$test->{"out_exist"}}, @{$test->{"out_not_exist"}} );
#   for my $output ( @outputs ){
#     -s $output and qx/rm -rf $output/;
#     vx_file_mkdir($output);
#   }

#   # set the test environment variables
#   $ENV{$_} = $test->{"env"}{$_} for keys %{ $test->{"env"} };

#   # build the text command
#   my ($cmd, $ret_ok, $out_ok) = ($test->{"exec"} . $test->{"param"});
#   $cmd =~ s/[ \n\t]+$//m;
#   if ($memchk) {
#     $cmd = "valgrind ".$VALGRIND_OPT_MEM." ".$cmd;
#   }
#   elsif ($callchk) {
#     $cmd = "valgrind ".$VALGRIND_OPT_CALL." ".$cmd;
#   }

#   # if writing a command file, print the environment and command, then loop
#   if( $cmd_only ){
#     print "export $_=\'" . $test->{"env"}{$_} . "\'\n" for sort keys %{ $test->{"env"} };
#     print "$cmd\n";
#     print "unset $_\n" for sort keys %{ $test->{"env"} };
#     print "\n";
#     next;
#   }
  
#   # run and time the test command
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

    test_list = []
    # read list of tests from test_xml
    # for each test
    for test_el in test_root.iter('test'):
        test = {}
        test['name'] = test_el.attrib['name']
        # check that name exists or "ERROR: name attribute not found for test ", $test_idx++ . "\n";

        for el in test_el:
            if (el.tag=='exec' or el.tag=='param'):
                test[el.tag] = repl_env(el.text)
                # handle env variable in el.text
            elif el.tag=='output':
                # build output file array (filename by output type)
                pass
            elif el.tag=='env':
                # set env variables from name/value pairs
                pass

        #   validate test format/details
        test_list.append(test)

    return test_list

#       # build the output file arrays
#       if( $test_child eq "output" ){
#         local @_ = @{ $childs[0] };
#         shift and shift @childs;

#         my (@out_pncs, @out_gncs, @out_stats, @out_pss, @out_exists, @out_not_exists);
#         while( @_ ){
#           my $out_child = shift;
#           $out_child or shift and next;
#           "point_nc"  eq $out_child and push @out_pncs,       repl_env($_[0][2]);
#           "grid_nc"   eq $out_child and push @out_gncs,       repl_env($_[0][2]);
#           "stat"      eq $out_child and push @out_stats,      repl_env($_[0][2]);   
#           "ps"        eq $out_child and push @out_pss,        repl_env($_[0][2]);
#           "exist"     eq $out_child and push @out_exists,     repl_env($_[0][2]);
#           "not_exist" eq $out_child and push @out_not_exists, repl_env($_[0][2]);          
#           shift;
#         }

#         @test{ ("out_pnc",  "out_gnc",   "out_stat",  "out_ps",  "out_exist",  "out_not_exist") } = 
#                (\@out_pncs, \@out_gncs,  \@out_stats, \@out_pss, \@out_exists, \@out_not_exists);
#       }

#       # build the environment map
#       elsif( $test_child eq "env" ){
#         local @_ = @{ $childs[0] };
#         shift and shift @childs;

#         my %env;
#         while( @_ ){
#           my $env_child = shift;
#           $env_child or shift and next;

#           # parse the pair name and value
#           if( "pair" eq $env_child ){
#             my @pair_childs = @{ $_[0] };
#             shift @pair_childs;
#             my ($pair_name, $pair_value);
#             while( @pair_childs ){
#               my $pair_child = shift @pair_childs;
#               $pair_child or shift @pair_childs and next;
#               "name"  eq $pair_child and $pair_name  = $pair_childs[0][2];
#               "value" eq $pair_child and $pair_value = $pair_childs[0][2];
#             }
#             $pair_name or die "ERROR: env pair in test \"" . $test{"name"} . "\" " .
#                               "missing name or value\n";            
#             $env{$pair_name} = defined($pair_value) ? $pair_value : "";
#           }
#           $test{"env"} = \%env;
#         }
#       }

#     }

#     # verify the structure of the test element
#     $test{"exec"}   or die "ERROR: test " . $test{"name"} . " missing exec element\n";
#     $test{"param"}  or die "ERROR: test " . $test{"name"} . " missing param element\n";
#     ( $test{"out_pnc"} && $test{"out_gnc"} && $test{"out_stat"} && $test{"out_ps"} && $test{"out_exist"} && $test{"out_not_exist"} ) 
#                     or die "ERROR: test " . $test{"name"} . " missing output element\n";

#     # add the test to the list of parsed tests
#     push @tests, \%test;

#   }  # END: while( @_ )

#   return @tests;
# }


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


