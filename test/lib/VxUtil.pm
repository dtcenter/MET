#!/usr/bin/perl

#######################################################################
#
#   VxUtil.pm - v0.2.0 - 2012-06-08
#
#   Verification script utilities, including functionality for logging,
#   parsing contants file, filename templates, GEMPAK file conversion,
#   date parsing and manipulation and MET verification tool utilies.
#
#######################################################################

package VxUtil;

use strict;
use Exporter;
use File::Path qw(mkpath);
use POSIX;
use vars qw($VERSION @ISA @EXPORT @EXPORT_OK %EXPORT_TAGS);

$VERSION     = 1.00;
@ISA         = qw(Exporter);
@EXPORT      = qw(vx_const_init vx_const_get vx_const_def
                  vx_log_open vx_log vx_log_set_email vx_log_close 
                  vx_file_status vx_file_mkdir vx_file_tmpl_gen 
                  vx_util_unique vx_util_seq vx_util_perm
                  vx_gem_conv vx_gem_print_parm_map
                  vx_date_to_array vx_date_parse_date vx_date_parse_lead vx_date_format_lead 
                    vx_date_calc_add vx_date_calc_sub vx_date_calc_init vx_date_calc_valid
                    vx_date_build_list
                  vx_wgrib_extract
                  vx_met_config_var vx_met_ens_stat vx_met_grid_stat vx_met_point_stat
                    vx_met_mode
                  vx_pcp_int vx_pcp_build_map vx_pcp_print_map vx_pcp_print_int 
                    vx_pcp_wizard_add vx_pcp_wizard_sub vx_pcp_wizard);

@EXPORT_OK   = qw(vx_const_init vx_const_get vx_const_def 
                  vx_log_open vx_log vx_log_set_email vx_log_close 
                  vx_file_status vx_file_mkdir vx_file_tmpl_gen 
                  vx_util_unique vx_util_seq vx_util_perm
                  vx_gem_conv vx_gem_print_parm_map
                  vx_date_to_array vx_date_parse_date vx_date_parse_lead vx_date_format_lead
                    vx_date_calc_add vx_date_calc_sub vx_date_calc_init vx_date_calc_valid
                    vx_date_build_list
                  vx_wgrib_extract
                  vx_met_config_var vx_met_ens_stat vx_met_grid_stat vx_met_point_stat
                    vx_met_mode
                  vx_pcp_int vx_pcp_build_map vx_pcp_print_map vx_pcp_print_int 
                    vx_pcp_wizard_add vx_pcp_wizard_sub vx_pcp_wizard);

%EXPORT_TAGS = ( DEFAULT => [
               qw(&vx_const_init &vx_const_get &vx_const_def 
                  &vx_log_open &vx_log &vx_log_set_email &vx_log_close 
                  &vx_file_status &vx_file_mkdir &vx_file_tmpl_gen 
                  &vx_util_unique &vx_util_seq &vx_util_perm
                  &vx_gem_conv &vx_gem_print_parm_map
                  &vx_date_to_array &vx_date_parse_date &vx_date_parse_lead &vx_date_format_lead 
                    &vx_date_calc_add &vx_date_calc_sub &vx_date_calc_init &vx_date_calc_valid
                    &vx_date_build_list
                  &vx_wgrib_extract
                  &vx_met_config_var &vx_met_ens_stat &vx_met_grid_stat &vx_met_point_stat
                    &vx_met_mode
                  &vx_pcp_int &vx_pcp_build_map &vx_pcp_print_map &vx_pcp_print_int 
                    &vx_pcp_wizard_add &vx_pcp_wizard_sub &vx_pcp_wizard)] );



#######################################################################
#
#   Constants functions
#
#######################################################################

# data structure to hold the constants
my %mapConst;

# constants file
my $strVxConstFile;


#######################################################################
# vx_const_init()
#
#   This function assumes that the specified constants file is a 
#   VxUtil constants file, parses it and loads the mapConst data 
#   member with the information contained therein.
#
#   Arguments:
#      const_file = path and filename of the constants file to parse
#
#######################################################################

sub vx_const_init {
  open(my $fhConst, "<", $_[0]) or die "ERROR: vx_const_init() failed to open constants file $_[0]: $!\n\n";

  # initialize the file state variables
  my $intLine = 0;
  my $boolOpenBracket = 0;
  my $strConstName = ""; 
  my @listVal = (); 

  # consider each line of the constants file
  while( <$fhConst> ){
    chomp();
    ++$intLine;
    my $strVal = ""; 

    # ignore comment or space lines
    if( /^\s*#/ || /^\s*$/ ){ next; } 

    # remove inline comments
    s/([^\\])#.*/$1/;

    # if there is no open list, parse a new item
    if( ! $boolOpenBracket ){
      unless( /^(\w+)\s*=\s*(.*)$/ ){ die "ERROR: vx_const_init() could not parse new item on line $intLine: $_\n\n"; }

      # state variables for constant settings
      $strConstName = $1; 
      @listVal = (); 
      $strVal = $2; 
      #print "constName = $strConstName\n";

      # parse the values of the new item
      if( $strVal =~ /^\[\s*(.*[^\\])\s*\]\s*$/ ){
        #print "list val = $1\n";
        $strVal = $1; 
      } elsif( $strVal =~ /^\[\s*([^\s].*)$/ ){
        #print "list start val = $1\n";
        $strVal = $1; 
        $boolOpenBracket = 1;
      } elsif( $strVal =~ /^\[\s*$/ ){
        #print "list start empty\n";
        $boolOpenBracket = 1;
        next;
      } else {
        #print "single val = $strVal\n";
      }   
    }   
    
    # otherwise, parse a continuing list of values
    else {
      if( /^\s*(.*[^\\])\]\s*$/ ){
        #print "list cont close val = $1\n";
        $strVal = $1; 
        $boolOpenBracket = 0;
      } elsif( /^\s*\]\s*$/ ){
        #print "list close empty\n";
        $boolOpenBracket = 0;
      } elsif( /^\s*(.*)$/ ){
        $strVal = $1; 
        #print "list cont open val = $strVal\n";
      }
    }

    # interpolate constants present in the string of items
    $strVal = vx_const_interp($strVal);
   
    # parse each item in the string of items
    my $strItem;
    while( length($strVal) ){

      # parse ticked items
      if( $strVal =~ /^'(.*?[^\\])'(?:\s+(.*))?$/ ){
        $strItem = $1;
        $strVal = ( defined($2) ? $2 : "" );
        #print "  ticked item = \'$strItem\'   list = $strVal\n";
      }
      
      # parse non-ticked items
      elsif( $strVal =~ /^([^'][^\s]*[^'\s]?)(?:\s+(.*))?$/ ){
        #($strItem = $1) =~ s/^ *(.*) *$/\1/;
        $strItem = $1;
        $strVal = ( defined($2) ? $2 : "" );
        #print "  non-ticked item = \'$strItem\'   list = $strVal\n";
      }
      
      # throw an error if the item is unparseable
      else {
        die "ERROR: vx_const_init() unable to parse item list on line $intLine: $strVal\n\n";
      }
  
      # un-escape and trim the item and push it onto the item list
      $strItem =~ s/^ *//g;
      $strItem =~ s/ *$//g;
      $strItem =~ s/\\\\/\\/g;
      $strItem =~ s/\\\[/[/g;
      $strItem =~ s/\\\]/]/g;
      $strItem =~ s/\\'/'/g;

      # add to the list of values
      push( @listVal, $strItem );
    } #  while( length($strVal) )
    #print "\n";

    # if there is not an open list of items, save the constant
    unless( $boolOpenBracket ){
      #print "ITEM: $strConstName\n";
      #foreach(@listVal){ print "  $_\n"; }
      #print "\n";
      vx_const_put($strConstName, @listVal);
    }
    
  }
  
}


#######################################################################
# vx_const_interp()
#
#   This function replaces all instances of embedded constant names in
#   the specified string with their values from the constants map.  If
#   the embedded constant has a value array, the first item is used.
#
#   Arguments:
#     const_int = the string to interpolate constant values into
#
#######################################################################

sub vx_const_interp {
  my $strConstInt = $_[0];

  # check the item for embedded constants to evaluate and replace 
  while( $strConstInt =~ /\$\{(\w+)\}/ ){
    my $strVarName = $1;
    vx_const_def($strVarName) or die "ERROR: vx_const_interp() could not replace variable $strVarName\n\n";
    my $strRef = ref($mapConst{$strVarName});
    if( $strRef eq "SCALAR" ){
      $strConstInt =~ s/\${$strVarName}/$mapConst{$strVarName}[0]/g;
    } elsif( $strRef eq "ARRAY" ){
      my @listVal = vx_const_get($strVarName);
      #my $strRepl = sprintf "@listVal";
      my $strRepl = "@listVal";
      $strConstInt =~ s/\${$strVarName}/$strRepl/g;
    }
  }

  return $strConstInt;
}


#######################################################################
# vx_const_get()
#
#   Retrieves the value(s) of the specified constant and returns them
#   in either scalar or array context.
#
#   Arguments:
#       const_name = name of the constant whose value(s) to return
#
#######################################################################

sub vx_const_get {
  my $strKey = $_[0];
  unless( grep(/$strKey/, keys(%mapConst)) ){
    vx_log("ERROR: key $_[0] not found in constants");
    die;
  }
  my @value = @{ $mapConst{$strKey} };
  if( $#value > 0 ){ return @value;    }
  else             { return $value[0]; }
}


#######################################################################
# vx_const_put()
#
#   Stores a constant with the specified name and value(s).
#
#   Arguments:
#       const_name = name of the constant whose value(s) to return
#        const_val = one or more values to store under the const_name
#
#######################################################################

sub vx_const_put {
  my $strKey = shift;
  $mapConst{$strKey} = [ @_ ];
  #print "vx_const_put() - key = $strKey  val = @listVal\n";
}


#######################################################################
# vx_const_def()
#
#   Searches the list of constant names and returns true if a constnat
#   with the specified name is defined, false otherwise.
#
#   Arguments:
#       const_name = name of constant to test
#
#######################################################################

sub vx_const_def {
  my $strTest = $_[0];
  foreach( keys(%mapConst) ){
    if( /^$strTest$/ ){ return 1; }
  }
  return 0;
}


#######################################################################
#
#   Logging functions
#
#######################################################################

# global state variables for logging functionality 
my $fhMonLog;
my $boolStdOut;
my $boolEmail;
my $boolLogFile;
my $strLogFile;

#######################################################################
# vx_log_open()
#
#   This function should be called at the beginning of a verification
#   script to inialize the logging for the script.  It opens the
#   specified log file and initializes the log related settings like
#   printing to stdout and log file email.  At the end of the 
#   verification script, the vx_log_close() function should be called 
#   to clean up and finalize.
#
#   Arguments:
#       log_file = path and filename of the logfile to write; if
#                  blank, no log file is written
#      overwrite = (optional) specifies whether to overwrite the log
#                  file, defaults to false (0)
#   print_stdout = (optional) specifies whether to print output to 
#                  stdout in addition to printing to the log file,
#                  defaults to true (1)
#     send_email = (optional) specifies whether to email the log 
#                  messages at the end of the logging session,
#                  defaults to false (0)
#
#######################################################################

sub vx_log_open {

  # interpolate variables within and create the directory for the log file
  $strLogFile = vx_const_interp( shift );
  $boolLogFile = ($strLogFile !~ /^$/? 1 : 0);
  if( $boolLogFile ){
    my $strDir = $strLogFile;
    $strDir =~ s/(.*\/).*/$1/;
    mkpath($strDir);
  
    # set the write op
    my $strWriteOp = (($#_ >= 0) && (shift != 0)? ">" : ">>");
  
    # open the log file and initalize the logging settings
    open($fhMonLog, $strWriteOp, $strLogFile) or die "ERROR: could not open log file $strLogFile: $!\n\n";
  } else {
    shift;
  }

  # set the optional logging config variables, if appropriate
  $boolStdOut = ($#_ >= 0? shift : 1);
  $boolEmail  = ($#_ >= 0? shift : 0);
}


#######################################################################
# vx_log()
#
#   This function writes the specified message into the verification
#   log file.  It must be called after vx_log_open in a verification
#   script and before vx_log_close().  If the stdout printing option
#   was specified when vx_log_open() was called.
#
#######################################################################

sub vx_log {
  if( $boolLogFile ){ print $fhMonLog "$_[0]\n"; }
  if( $boolStdOut  ){ print "$_[0]\n";           }
}


#######################################################################
# vx_log_set_email()
#
#   This function sets the internal flag for sending email to the
#   specified value.
#
#   Arguments:
#     email_flag = true to send email, false to not
#
#######################################################################

sub vx_log_set_email {
  $boolEmail = $_[0];
}


#######################################################################
# vx_log_close()
#
#   This function should be called at the end of a logging session in
#   a verification script to finalize the logging and send the log
#   email, if necessary.
#
#   Arguments:
#     email_subj = subject for log email, only required if email active
#       email_to = recipients of log email, only required if email active
#
#######################################################################

sub vx_log_close {

  # close the file handle for the log file
  if( $boolLogFile ){
    if( !close($fhMonLog) ){
      print "ERROR: could not close log file handle in MonLog::close_log(): $!\n\n";
    }

    # if the email setting is true, parse the email parameters and send the log file contents
    if( $boolEmail ){
      if( @_ < 2 ){ die "ERROR: vx_close_log() expected a subject string and one or more recipient email addresses\n\n"; }
      my $strSubject = shift;
      my $strRecipList = vx_const_interp(shift);
      system("mail -s '$strSubject' '$strRecipList' < $strLogFile") == 0 or print "ERROR: call to mail failed in vx_log_close(): $!\n\n";
    }
  }
}


#######################################################################
# sub vx_file_status()
#
#    This function checks the existence if the specified path and
#    file and returns MISSING if the file does not exist, or the
#    human-readable file size, if the file does exist.
#
#    Arguments:
#        file = path and filename to test
#
#######################################################################

sub vx_file_status {
  if( ! -s $_[0] ){ return "MISSING"; }
  my $strFileSize = qx/ls -lh $_[0] | awk '{print \$5}'/;
  chomp($strFileSize);
  return $strFileSize . " " . strftime("%Y-%m-%d %H:%M:%S", gmtime( (stat($_[0]))[9] ));
}  


#######################################################################
# sub vx_file_mkdir()
#
#    This function checks the existence if the specified path and
#    file and builds any portion of the folder structure that does
#    not exist.
#
#    Arguments:
#        file = path and filename to check 
#
#######################################################################

sub vx_file_mkdir {
  while( my $strFile = shift ){
    (my $strDir = $strFile) =~ s/(.*\/).*/$1/;
    qx/mkdir -p $strDir/;
  }
}


#######################################################################
# sub vx_file_tmpl_gen()
#
#    This function populates the specified template with information
#    in the specified map structure.  The template structure is
#    comprised of a fixed string populated with template place-holders
#    inside curly braces, for example {tmpl_key}.  The tmpl_key must
#    be present as a key in the input map, and the value will replace
#    the {tmpl_key} in the returned string.
#
#    In some cases, the template keys can have parameters containing
#    formatting information.  The format of the template in this case
#    is {tmpl_key?parm1=val1&parm2=val2...}.  The supported parameters
#    are:
#
#      init/valid:
#          fmt - specifies a strftime format for the date
#          bin - specifies a lead format bin of time into which the time
#                will be formatted: b - beginnign, e - end
#      lead/accum:
#          fmt - specifies an amount of time in [H]HH[MMSS]  format
#
#    Arguments:
#         tmpl - template string to populate
#         vals - map containing values for each template key
#   warn_undef - (optional) turns on/off warnings for missing template
#                keys, default 1
#
#######################################################################
  
sub vx_file_tmpl_gen {

  # function arguments
  my $strTmpl = shift;
  my %mapTmpl = %{$_[0]}; shift;
  my $boolWarn = (@_? shift : 1);

  # data members for storing work
  my %mapFmt = ();
  my $strData = $strTmpl;

  # perform valid/init/lead calculations, if necessary
  if( (defined $mapTmpl{"valid"}) && (defined $mapTmpl{"lead"}) ){
    $mapTmpl{"init"} = vx_date_calc_init($mapTmpl{"valid"}, $mapTmpl{"lead"});
  } elsif( (defined $mapTmpl{"init"}) && (defined $mapTmpl{"lead"}) ){
    $mapTmpl{"valid"} = vx_date_calc_valid($mapTmpl{"init"}, $mapTmpl{"lead"});
  } elsif( (defined $mapTmpl{"init"}) && (defined $mapTmpl{"valid"}) ){
    $mapTmpl{"lead"} = vx_date_format_lead(
                         vx_date_parse_date( $mapTmpl{"valid"} ) -
                         vx_date_parse_date( $mapTmpl{"init"} ),
                         "HHH"
                       );
  }

  # parse each tag in the input template string
  foreach my $strKey ($strData =~ /[^\\]\{([^\}]+[^\\])\}/g) {
  
    # parse the template parameters
    my $strFmt = "";
    my $strKeyBase = $strKey; 
    if( $strKey =~ /(\w+)\?(.*)/ ){

      # initialize the template data members
      $strKeyBase = $1;
      my %mapFmtKey = ();

      # put each parameter key/value pair into mapFmtKey
      foreach my $strPair ( split(/&/, $2) ) {
        my ($strFmtVar, $strFmtVal) = split(/=/, $strPair);
        #print "  fmt pair: var = $strFmtVar  val = $strFmtVal\n";
        $mapFmtKey{$strFmtVar} = $strFmtVal;
      }

      # add the parameters for the current template key to mapFmt
      $mapFmt{$strKeyBase} = \%mapFmtKey;
    }

    # validate the current tmpl key
    if( ! defined $mapTmpl{$strKeyBase} ){
      print "WARNING: template key $strKeyBase not recognized\n" if $boolWarn;
      next;
    }
  
    # get the tmpl value for the current key
    my $strVal = $mapTmpl{$strKeyBase};
    my $strValFmt = $strVal;
  
    # format the value, if necessary
    if( defined $mapFmt{$strKeyBase} ){
      #print "formatting $strKeyBase\n";
      my %mapFmtKey = %{$mapFmt{$strKeyBase}};

      # for the init and valid templates, parse and apply the formatting options
      if( $strKeyBase =~ /^init$/ || $strKeyBase =~ /^valid$/ ){
        $strValFmt =~ s/_//g;
        my $strValFmtBin = $strValFmt;

        # handle the time bin argument
        if( defined $mapFmtKey{"bin"} ){

          # convert the bin to seconds and ensure that it divides evenly into 24h
          if( $mapFmtKey{'bin'} !~ /^\d+[be]$/ ){ die "ERROR: vx_file_tmpl_gen() could not parse bin format parameter: $mapFmtKey{'bin'}\n\n"; }
          my ($strBinLead, $strBinOrd) = ($mapFmtKey{'bin'} =~ /(\d+)([be])/);
          my $intBinSec = vx_date_parse_lead( $strBinLead ); 
          if( (86400 % $intBinSec) != 0 ){ die "ERROR: vx_file_tmpl_gen() formatting parameter value for bin must be evenly divisible by 24 hours\n\n"; }

          # determine the bin into which the init date/time falls
          my $timeInit = vx_date_parse_date($strValFmt);
          my $intBinDiv = 0;
          if   ( $strBinOrd =~ /b/ ){ $intBinDiv = floor($timeInit / $intBinSec); }
          elsif( $strBinOrd =~ /e/ ){ $intBinDiv = ceil ($timeInit / $intBinSec); }
          else                      { die "ERROR: unrecognized bin ordinal: $strBinOrd\n\n"; }
          $strValFmt = strftime("%Y%m%d%H%M%S", gmtime( $intBinDiv * $intBinSec )); 
        } 
        
        # handle the fmt argument 
        if( defined $mapFmtKey{"fmt"} ){
          my @dateInit = vx_date_to_array($strValFmt);
          $strValFmt = strftime($mapFmtKey{'fmt'}, @dateInit);
        }
      }

      # for the lead templates, parse and apply the formatting
      if( $strKeyBase =~ /^lead$/ || $strKeyBase =~ /^accum$/ ){
        if( defined $mapFmtKey{"fmt"} ){
          $strValFmt = vx_date_format_lead( vx_date_parse_lead($strValFmt), $mapFmtKey{"fmt"} );
        }
      }
    }

    # replace the template key with the formatted value
    (my $strKeyEsc = $strKey) =~ s/\?/\\?/g;
    $strData =~ s/\{$strKeyEsc\}/$strValFmt/;
  }

  return $strData;
}


#######################################################################
# sub vx_util_unique()
#
#   This function constructs an array of unique elements from the
#   input array, in the order of the first occurrence of each unique
#   item.
#
#   Arguments:
#      list = list of items to filter repetetive elements from
#
#######################################################################

sub vx_util_unique {
  my (@listRet, %mapTest) = ((), ());

  foreach (@_){
    next if( defined $mapTest{$_} );
    push(@listRet, $_);
    $mapTest{$_} = 1;
  }
  return @listRet;
}


######################################################################
# vx_util_seq()
#
#   This function behaves like the linux function seq.  It returns a
#   list containin the sequence of integers from the specified 
#   beginning to the specified end, stepping by the optional step
#   argument (default 1).
#
#   Arguments:
#      beg = beginning of sequence
#     step = (optional) step size, default 1 or -1
#      end = end of sequence
#
#######################################################################
sub vx_util_seq {
  return @_ if (2 > @_);
  my ($beg, $inc, $end) = (shift, (2 == @_ ? shift : 1), shift);
  $inc = ($beg > $end && 0 < $inc ? -1 * $inc : $inc);
  my @ret = ();
  if( 0 < $inc ){
    do{ push(@ret, $beg); $beg += $inc; } while( $beg <= $end );
  } elsif( 0 > $inc ) {
    do{ push(@ret, $beg); $beg += $inc; } while( $beg >= $end );
  }
  return @ret;
}


######################################################################
# vx_util_perm()
#
#   This function assumes that the single input argument is a map ref
#   in which keys are mapped to array references.  It builds an array
#   of map refs, each of which contains a single permutation of the
#   values of the input map, referenced by the same set of keys.
#
#   Arguments:
#      map = map whose lists of values will be permuted
#
#######################################################################
sub vx_util_perm {

  my (%val_map, @perms) = %{ $_[0] };
  
  # if the input val_map has only one key, build the start list and return
  if( 1 == keys %val_map ){
    my ($key) = keys %val_map;
    push @perms, { $key => $_ } for @{ $val_map{$key} };
    return @perms;
  }
  
  # select a key over which to permute and build the inner permutation
  my ($key) = keys %val_map;
  my @vals = @{ $val_map{$key} };
  delete $val_map{$key};
  @perms = vx_util_perm( \%val_map );
  
  # create an inner permutation copy for each key value
  my @perms_new;
  for my $val (@vals) {
    for (@perms){
      my %copy = ( %{$_} );
      $copy{$key} = $val;
      push @perms_new, \%copy;
    }
  }
  
  return @perms_new;
  
  return @perms_new;
  
}


######################################################################
#
#   GEMPAK utilities
#
######################################################################

######################################################################
# sub vx_gem_conv()
#
#   This function extracts the forecast fields from the specified
#   GEMPAK file and appends them to the specified GRB file.  The output
#   file is created if not already present.  The field information must
#   be passed in the mapParm parameter as a hash of arrays.  The hash
#   must contain the keys GFUNC, GVCORD, GBTBLS, VERCEN, PDSVAL and
#   PRECSN.  Each underlying array must have the same length.
#
#   Arguments:
#     mapParm     = hash of arrays with the structure described above,
#                   containing the information for gdgrib parameters
#     fileGem     = path and filename of the existing GEMPAK file to
#                   extract data from
#     fileGrb     = path and filename of the output GRIB file to
#                   append fields to
#    leadTime     = lead time of the desired forecast data in HHH
#                   format
#     verbose     = (optional) verbosity level; default 1
#     del_parm    = (optional) if true, parm file is deleted, default 1
#
#######################################################################

sub vx_gem_conv {

  # establish the input parameters
  my %mapParm = %{$_[0]}; shift;
  my ($strFileGem, $strFileGrb, $strLeadTime) = (shift, shift, shift);
  my $boolVerbose = ( $#_ >= 0? shift : 1);
  my $boolDelParm = ( $#_ >= 0? shift : 1);

  # gempak tools
  my $strGemBin    = vx_const_get("GEM_BIN_DIR");
  my $strGemGdgrib = "$strGemBin/gdgrib";
  my $strGemGpend  = "$strGemBin/gpend";

  # print the input parameter information
  if( $boolVerbose >= 2 ){
    vx_log("vx_gem_conv() parameters\n");
    #vx_gem_print_parm_map(\%mapParm);
    vx_log("  fileGem = $strFileGem\n  fileGrb = $strFileGrb\n" .
           "  leadTime = $strLeadTime\n  verbose = $boolVerbose\n");
  }

  # enforce the .grb suffix on the end of the output file, and create the output folder
  unless( $strFileGrb =~ /\.gri?b$/ ){ die "ERROR: invalid output file name $strFileGrb"; }
  vx_file_mkdir($strFileGrb);

  # run the conversion once for each requested field
  for(my $i=0; $i <= $#{$mapParm{"GFUNC"}}; $i++){

    # parse the forecast variable from the parameter list
    my $strFcstVar = $mapParm{"GFUNC"}[$i];
    $strFcstVar =~ s/(\w+).*/$1/;

    # build the path and filename of the parameter file
    my $strFileParm  = $strFileGrb;
    $strFileParm =~ s/\.gri?b$/.$strFcstVar.parm/;

    # print the current variable and parm file
    if( $boolVerbose      ){ vx_log("CONVERTING $mapParm{'GFUNC'}[$i]"); }
    if( $boolVerbose >= 2 ){ vx_log("PARM FILE: $strFileParm");       }

    # if the output files already exist, delete them
    if( -s $strFileParm ){ unlink $strFileParm or die "ERROR: could not delete $strFileParm: $!\n\n"; }

    # write the parameter information to a file
    open(my $fhParm, ">$strFileParm") or die "ERROR: could not open parm file $strFileParm: $!\n\n";
    print $fhParm "GDFILE=$strFileGem\n" .
                  "GFUNC="  . ${$mapParm{"GFUNC"}}[$i] . "\n" .
                  "GDATTIM=f$strLeadTime\n" .
                  "GLEVEL=0\n" .
                  "GVCORD=" . ${$mapParm{"GVCORD"}}[$i] . "\n" .
                  "GBTBLS=" . ${$mapParm{"GBTBLS"}}[$i] . "\n" .
                  "GBFILE=$strFileGrb\n" .
                  "VERCEN=" . ${$mapParm{"VERCEN"}}[$i] . "\n" .
                  "PDSVAL=" . ${$mapParm{"PDSVAL"}}[$i] . "\n" .
                  "PRECSN=" . ${$mapParm{"PRECSN"}}[$i] . "\n" .
                  "WMOHDR=\n" .
                  "CPYFIL=\n" .
                  "PROJ=\n" .
                  "GRDAREA=\n" .
                  "KXKY=\n" .
                  "RUN\n\n";
    close $fhParm;

    # build the commands for gdgrib and gpend
    my $strCmdGdgrib = "$strGemGdgrib < $strFileParm > /dev/null";
    my $strCmdGpend = "$strGemGpend";

    # run gdgrib and gpend
    if( $boolVerbose ){ vx_log("GDGRIB: $strCmdGdgrib; $strCmdGpend"); }
    system("$strCmdGdgrib; $strCmdGpend");

    # delete the parameter file
    $boolDelParm and unlink $strFileParm;
  }

  # if the output file did not appear or is zero-size, complain
  if( ! -e $strFileGrb ){
    vx_log("WARNING: failed to create GRIB output file $strFileGrb\n");
  } elsif( 0 >= (stat($strFileGrb))[7] ){
    vx_log("WARNING: created zero-size GRIB output file $strFileGrb\n");
  }

  if( $boolVerbose >= 2 ){ vx_log("vx_gem_conv() complete\n"); }
}

sub vx_gem_print_parm_map {
  my %mapParm = %{$_[0]};

  vx_log("mapParm = {");
  foreach my $key (keys %mapParm){
    my @listVal = @{ $mapParm{$key} };
    my $strEntry = sprintf('    %-8s => ', $key);
    foreach (@listVal){ $strEntry .= sprintf('%-50s', $_); }
    vx_log($strEntry);
  }
  vx_log("}");
}


######################################################################
#
#  Date/Time Utilities
#
######################################################################

#####################################################################
# sub vx_date_to_array()
#
#   This function returns the input date arranged into an array that
#   is usable by the POSIX time methods like mktime().  The input 
#   date must have the format YYYYmmddHH[MMSS].
#
#   Arguments:
#       date = date to parse and convert to an array
#
######################################################################

sub vx_date_to_array {
  my $strDate = shift;
  $strDate =~ s/_//;
  my ($Y,$m,$d,$H,$M,$S);
  if   ( $strDate =~ /\d{14}/ ){ ($Y,$m,$d,$H,$M,$S) = ($strDate =~ /(\d{4})(\d\d)(\d\d)(\d\d)(\d\d)(\d\d)/); }
  elsif( $strDate =~ /\d{10}/ ){ ($Y,$m,$d,$H,$M,$S) = ($strDate =~ /(\d{4})(\d\d)(\d\d)(\d\d)/, 0, 0);       }
  else { die "ERROR: vx_date_to_array() encountered unexpected date format: $_\n\n";   }
  return ($S,$M,$H,$d,$m-1,$Y-1900);
}


#####################################################################
# sub vx_date_parse_date()
#
#   This function returns the unix time for the input date which is
#   assumed to be in UTC.  The input date must have the format
#   YYYYmmddHH[MMSS].
#
#   Arguments:
#       date = date to parse and convert to unix time
#
######################################################################

sub vx_date_parse_date {
  $ENV{'TZ'} = "UTC"; tzset();
  return mktime( vx_date_to_array(shift) );
}


#####################################################################
# sub vx_date_parse_lead()
#
#   This function returns the number of seconds represented by the
#   input lead time which must have the format [H]HH[MMSS].
#
#   Arguments:
#    lead_time = lead time to parse
#
######################################################################

sub vx_date_parse_lead {
  my $strLead = shift;
  my ($H,$M,$S);
  if   ( $strLead =~ /^\d{7}$/   ){ ($H,$M,$S) = ($strLead =~ /(\d{3})(\d\d)(\d\d)/); }
  elsif( $strLead =~ /^\d{6}$/   ){ ($H,$M,$S) = ($strLead =~ /(\d\d)(\d\d)(\d\d)/);  }
  elsif( $strLead =~ /^\d{1,3}$/ ){ ($H,$M,$S) = ($strLead, 0, 0);            }
  else { die "ERROR: vx_date_parse_lead() found unexpected lead format: $strLead\n\n"; }
  return $H * 3600 + $M * 60 + $S;
}


######################################################################
# sub vx_date_format_lead()
#
#   This function formats the specified lead time, in seconds, into
#   a string conforming to the specified format.  Valid format strings
#   are of the form [H]HH[MMSS].
#
#   Arguments:
#     lead_time = lead time in seconds to format
#     format    = format of the lead time string to return
#
######################################################################

sub vx_date_format_lead {
  my ($intLead, $strFmt) = (shift, shift);

  # parse the components of the input lead time
  my $intLeadS = $intLead;
  my $intLeadH = floor($intLeadS / 3600);
  $intLeadS -= $intLeadH * 3600;
  my $intLeadM = floor($intLeadS / 60);
  $intLeadS -= $intLeadM * 60;

  # parse the lead fmt string
  if( $strFmt !~ /(HHH?)(MMSS)?/ ){ die "ERROR: vx_date_format_lead() found unrecognized lead fmt value $strFmt\n\n"; }
  my ($strFmtH, $strFmtMS) = ($1, $2);
  return sprintf("%0" . length($strFmtH) . "d", $intLeadH) .
         ($strFmtMS? sprintf("%02d%02d", $intLeadM, $intLeadS) : "");
}


#####################################################################
# sub vx_date_calc_add()
#
#   This function calculates a valid time by combining the input
#   init time with the input lead time.  The input init time must
#   be in the format YYYYmmdd[_]HH[MMSS] and the lead time in the 
#   format [H]HH[MMSS].  The date returned will have the same format 
#   as the input date.
#
#   Arguments:
#     init_time = init time from which valid time will be derived
#     lead_time = lead time added to init time
#
######################################################################

sub vx_date_calc_add {
  my $strDate = shift;
  my $intLead = shift;
  my $intDateLen = length($strDate);
  $strDate =~ s/_//;
  my $timeAdd = vx_date_parse_date($strDate) + 
                ( $intLead < 0 ?
                    -1 * vx_date_parse_lead(-1 * $intLead) :
                         vx_date_parse_lead(     $intLead)
                );
  if   ( 15 == $intDateLen ){ return strftime("%Y%m%d_%H%M%S", gmtime($timeAdd)); }
  elsif( 14 == $intDateLen ){ return strftime("%Y%m%d%H%M%S",  gmtime($timeAdd)); }
  elsif( 11 == $intDateLen ){ return strftime("%Y%m%d_%H",     gmtime($timeAdd)); }
  else                      { return strftime("%Y%m%d%H",      gmtime($timeAdd)); }
}
sub vx_date_calc_sub   { return vx_date_calc_add(shift, -1 * shift); }
sub vx_date_calc_valid { return vx_date_calc_add(@_); }
sub vx_date_calc_init  { return vx_date_calc_sub(@_); }


#####################################################################
# sub vx_date_build_list()
#
#   This function builds and returns a list of date strings in
#   the format YYYYmmdd_HH[MMSS].  The begin time and end time must
#   be specified in a format which will be the format used by the
#   returned dates.  The increment must be specified in the format
#   [H]HH[MMSS].
#
#   Arguments:
#     beg = beginning time of the list
#     end = end time of the list
#     inc = time increment between successive list times
#
######################################################################

sub vx_date_build_list {
  my ($beg, $end, $inc, @dates) = @_;
  my ($cur, $end_date) = ($beg, vx_date_parse_date($end));
  while( vx_date_parse_date($cur) <= $end_date ){
    push @dates, $cur;
    $cur = vx_date_calc_add($cur, $inc);
  }
  return @dates;
}


######################################################################
# sub vx_wgrib_extract()
#
#   This function extracts fields from the specified GRIB file
#   according to the specified egrep input string and writes the
#   records to the optional output file.  If no output file is
#   specified, the input file is overwritten.
#
#   Arguments:
#       grib_in = file to extract records from
#     egrep_pat = pattern string that is used to extract desired 
#                 records from wgrib output
#      grib_out = (optional) output file to write records to
#
######################################################################

sub vx_wgrib_extract {

  # parse the input parameters
  my $strFileGrb = shift;
  my $strExtract = shift;
  my $strFileGrbExt = ( $#_ >= 0? shift : $strFileGrb);

  # determine which wgrib instance to use
  my $strWgrib = (vx_const_def("WGRIB_BIN")? vx_const_get("WGRIB_BIN") : qx/which wgrib/);
  if( $strWgrib !~ /.*wgrib$/ ){ die "ERROR: vx_wgrib_extract() could not find wgrib\n\n"; }

  # if the output file was not specified, move the input file
  my $boolRepl = ($strFileGrbExt =~ /^$strFileGrb$/);
  my $strFileGrbOrig = $strFileGrb;
  if( $boolRepl ){
    $strFileGrb =~ s/(\.grb)?$/_preExt.grb/;
    my $strMvCmd = "mv $strFileGrbExt $strFileGrb";
    vx_log("MV: $strMvCmd");
    qx/$strMvCmd/;
  }
  vx_file_mkdir($strFileGrbExt);

  # construct and run the wgrib statement to extract the specified fields
  my $strWgribCmd = "$strWgrib $strFileGrb | egrep \"$strExtract\" |" .
                    " $strWgrib $strFileGrb -i -grib -o $strFileGrbExt";
  vx_log("WGRIB: $strWgribCmd");
  qx/$strWgribCmd/;

  # verify that the output file is present
  if( ! -s $strFileGrbExt ){
    print "WARNING: vx_wgrib_extract() failed to produce output file $strFileGrbExt\n\n";

    # if the output file was to replace the input file, copy the input back into place
    if ( $boolRepl ){
      my $strMvCmd = "mv $strFileGrb $strFileGrbOrig";
      vx_log("MV: $strMvCmd");
      qx/$strMvCmd/;
    }
  }

  # if replacement was used, delete the original
  elsif( $boolRepl ) {
    my $strRmCmd = "rm $strFileGrb";
    vx_log("RM: $strRmCmd");
    qx/$strRmCmd/;
  }

}


######################################################################
#
#  MET Utilities
#
######################################################################

#######################################################################
# sub vx_met_config_var()
#
#   This function parses the specified config file for environment
#   variable place-holders and then verifies that each variable has
#   an entry in the specified configuration map.  Each entry is added
#   to the environment in preparation for running the corresponding
#   MET tool.
#
#   Arguments:
#      config = configuration file to parse for environment variables
#     val_map = map of variable names to values
#
#######################################################################

sub vx_met_config_var {
  my $strConfig = shift;
  my %mapVal = %{$_[0]};

  # open the config file and store all variables
  open my $fhConfig, "<", $strConfig or die "ERROR: vx_config_parse() could not open $strConfig\n\n";
  my @listVar = ();
  while( <$fhConfig> ){
    chomp();
    my @listMat = /\$\{(\w+)\}/g;
    push(@listVar, $_) foreach (@listMat);
  }
  close $fhConfig;

  # check the input map for each variable and add it to the environment
  foreach my $strVar ( vx_util_unique(@listVar) ){
    if( ! defined $mapVal{$strVar} ){
      die "ERROR: vx_met_config_var() found config file variable without setting: $strVar\n\n";
    }
    (my $strValEsc = $mapVal{$strVar}) =~ s/"/\\"/g;
    vx_log("export $strVar=\"$strValEsc\"");
    $ENV{$strVar} = $mapVal{$strVar};
    #qx/export $strVar=$mapVal{$strVar}/;
  }
}


#######################################################################
# sub vx_met_ens_stat()
#
#   This function wraps the MET ensemble_stat tool, which is assumed
#   to be pointed at by the constant MET_ENS_STAT.  The specified
#   models, configuration file, configuration variables and options
#   are build into an ensemble_stat command which is executed.
#
#   Arguments
#      ens_mem = list of ensemble member model data files
#       config = configuration file with variables whose values are
#                stored in config_val map
#   config_val = map of configuration variable/value pairs
#          opt = map of command line options to ensemble_stat
#       no_run = (optional) print the command only and do not run it
#
#######################################################################

sub vx_met_ens_stat {

  # parse the input parameters
  my @listEnsMem   = @{$_[0]}; shift;
  my $strConfig    = shift;
  my %mapConfigVal = %{$_[0]}; shift;
  my %mapOpt       = %{$_[0]}; shift;
  my $boolNoRun    = (@_ ? shift : 0);

  vx_log("\nENSEMBLE_STAT:");

  # verify and establish the configuration environment variables
  vx_met_config_var($strConfig, \%mapConfigVal);

  # build the ensemble_stat command
  my $strEnsCmd = vx_const_get("MET_ENS_STAT") . " \\\n  " . ($#listEnsMem + 1) . " \\\n";
  $strEnsCmd .= "  $_ \\\n" foreach (@listEnsMem);
  $strEnsCmd .= "  $strConfig";

  # add the options from the input options map to the command line
  my @listOpt = qw(grid_obs point_obs ens_valid ens_lead obs_valid_beg obs_valid_end 
                   obs_lead outdir ssvar_mean v);
  foreach my $strOpt (@listOpt){
    next if( ! defined $mapOpt{$strOpt} );
    $strEnsCmd .= " \\\n  -$strOpt $mapOpt{$strOpt}";
    qx/mkdir -p $mapOpt{$strOpt}/ if( $strOpt =~ /^outdir$/ );
  }

  # print and run the ensemble_stat command, logging the output
  vx_log("$strEnsCmd\n");
  unless( $boolNoRun ){
    my @listEnsOut = qx/$strEnsCmd 2>&1/;
    chomp() and vx_log($_) for (@listEnsOut);
  }

}


#######################################################################
# sub vx_met_pgm_stat()
#
#   This function wraps the MET point_stat, grid_stat and MODE tool, 
#   which are assumed to be pointed at by the constant MET_{TOOL}.  
#   The specified model data, obs data, configuration file, 
#   configuration variables and options are built into either a 
#   point_stat, grid_stat or mode command, which is executed.
#
#   Arguments
#         tool = one of "point", "grid" or "mode", depending on the 
#                MET tool to run
#   model_data = path and filename of the model data file
#     obs_data = path and filename of the obs data file
#       config = configuration file with variables whose values are
#                stored in config_val map
#   config_val = map of configuration variable/value pairs
#          opt = map of command line options to point_stat
#       no_run = (optional) print the command only and do not run it
#
#######################################################################

sub vx_met_pgm_stat {

  # parse the input parameters
  my ($tool, $strModel, $strObs, $strConfig) = (shift, shift, shift, shift);
  my %mapConfigVal = %{$_[0]}; shift;
  my %mapOpt       = %{$_[0]}; shift;
  my $boolNoRun    = (@_ ? shift : 0);

  # format the 
  if( $tool !~ /mode/ ){ $tool .= "_stat"; }
  vx_log("\n" . uc($tool) . ":");

  # verify and establish the configuration environment variables
  vx_met_config_var($strConfig, \%mapConfigVal);

  # build the point_stat or grid_stat command
  my $strVxCmd = vx_const_get("MET_" . uc($tool)) . " \\\n" .
                  "  " . $strModel . " \\\n" .
                  "  " . $strObs . " \\\n" .
                  "  " . $strConfig;

  # add the options from the input options map to the command line
  #my @listOpt = qw(fcst_valid fcst_lead obs_valid obs_lead outdir v);
  foreach my $strOpt (keys %mapOpt){
    #next if( ! defined $mapOpt{$strOpt} );
    $strVxCmd .= " \\\n  -$strOpt $mapOpt{$strOpt}";
    qx/mkdir -p $mapOpt{$strOpt}/ if( $strOpt =~ /^outdir$/ );
  }

  # print and run the point_stat command, logging the output
  vx_log("$strVxCmd\n");
  unless( $boolNoRun ){
    my @listVxOut = qx/$strVxCmd 2>&1/;
    do{ chomp(); vx_log("$_"); } foreach (@listVxOut);
  }

}


#######################################################################
# sub vx_met_grid_stat()
#
#   This function wraps the MET grid_stat tool, which is assumed
#   to be pointed at by the constant MET_GRID_STAT.  The specified
#   model data, obs data, configuration file, configuration variables 
#   and options are build into an grid_stat command which is executed.
#
#   Arguments
#   model_data = path and filename of the model data file
#     obs_data = path and filename of the obs data file
#       config = configuration file with variables whose values are
#                stored in config_val map
#   config_val = map of configuration variable/value pairs
#          opt = map of command line options to grid_stat
#       no_run = (optional) print the command only and do not run it
#
#######################################################################

sub vx_met_grid_stat {
  vx_met_pgm_stat("grid", @_);
}


#######################################################################
# sub vx_met_point_stat()
#
#   This function wraps the MET point_stat tool, which is assumed
#   to be pointed at by the constant MET_POINT_STAT.  The specified
#   model data, obs data, configuration file, configuration variables 
#   and options are build into an point_stat command which is executed.
#
#   Arguments
#   model_data = path and filename of the model data file
#     obs_data = path and filename of the obs data file
#       config = configuration file with variables whose values are
#                stored in config_val map
#   config_val = map of configuration variable/value pairs
#          opt = map of command line options to point_stat
#       no_run = (optional) print the command only and do not run it
#
#######################################################################

sub vx_met_point_stat {
  vx_met_pgm_stat("point", @_);
}


#######################################################################
# sub vx_met_mode_stat()
#
#   This function wraps the MET mode tool, which is assumed to be 
#   pointed at by the constant MET_MODE.  The specified model data, 
#   obs data, configuration file, configuration variables and options 
#   are build into mode command which is executed.
#
#   Arguments
#   model_data = path and filename of the model data file
#     obs_data = path and filename of the obs data file
#       config = configuration file with variables whose values are
#                stored in config_val map
#   config_val = map of configuration variable/value pairs
#          opt = map of command line options to mode 
#       no_run = (optional) print the command only and do not run it
#
#######################################################################

sub vx_met_mode {
  vx_met_pgm_stat("mode", @_);
}


#######################################################################
#
#   MET pcp_combine functions - a.k.a. "pcp wizard"
#
#######################################################################

#######################################################################
# vx_pcp_int()
#
#   Search the specified accum/valid map at the specified valid time
#   for an entry with the specified accumulation.  If found, a
#   reference to the interval map is returned, otherwise 0 is returned.
#
#   Arguments:
#           valid = valid time to search for accumulation
#           accum = accumulation to search for
#         map_pcp = accum/valid map to search
#
########################################################################
sub vx_pcp_int {
  my ($strValid, $intAccum) = (shift, shift);
  my %mapPcp = %{$_[0]}; 

  if( ! $mapPcp{$strValid} ){ return; }
  foreach my $intRefPcp ( @{$mapPcp{$strValid}} ){
    return $intRefPcp
      if( $strValid =~ /$intRefPcp->{"vld_end"}/ &&
          $intAccum == $intRefPcp->{"accum"} );
  }
  return 0;
}


#######################################################################
# vx_pcp_build_map()
#
#   This function builds and returns a data structure that contains 
#   accumulation information for the specified list of GRIB files.  
#   The data structure is mapped by valid time to a list of hashes, 
#   each of which contains a single accumulation for a file.  It is
#   assumed that the constants file WGRIB_EXEC setting points at an
#   instance of wgrib.
#
#   Arguments:
#         data_files = list of GRIB files to parse
#
########################################################################
sub vx_pcp_build_map {

  # build a data structure that stores the pcp information
  my %mapPcp = ();
  my $strInit = "";
  foreach my $strDataFile (@_){
    chomp($strDataFile);
  
    # use wgrib to extract the precip information from the data file
    my $strCmdWgrib = vx_const_get("WGRIB_EXEC") . " \\\n  $strDataFile 2>/dev/null \\\n  | egrep ':APCP:'";
    my @listPcpRec = qx/$strCmdWgrib/;
    
    # for each accumulation interval in the file, create a record
    foreach my $strPcpRec (@listPcpRec){
  
      # parse and verify the init date, lead time and accumulation interval
      my ($strInitCur, $strLead, $strPcpInt) = (split(/:/, $strPcpRec))[(2,9,12)];
      if( $strInitCur !~ /^d=(\d{6})(\d{2})$/  ){ vx_log("WARNING: unrecognized init date format: $strInitCur");   next; }
      chomp( $strInitCur = qx/date -d "19700101 \$(date -u -d'$1 $2' +%s) sec" +%Y%m%d_%H/ );
      if( $strLead    !~ /^P2=(\d+)$/          ){ vx_log("WARNING: unrecognized lead time format: $strLead");      next; }
      $strLead = $1;
      if( $strPcpInt  !~ /(\d+)-(\d+)hr\s+acc/ ){ vx_log("WARNING: unrecognized accumulation format: $strPcpInt"); next; }
      my ($intBeg, $intEnd) = ($1, $2);
  
      # check the init time for consistency
      if( $strInit && $strInit !~ /$strInitCur/ ){
        vx_log("ERROR: inconsistent init times\n" . 
          sprintf("%17s: %s\n%17s: %s\n", $strInit, $_[0], $strInitCur, $strDataFile));
        die;
      }
      $strInit = $strInitCur;

      # do not same accumulations of zero length
      my $intAccum = ($intEnd - $intBeg);
      if( 0 >= $intAccum ){ next; }
  
      # build a data structure to store the accumulation interval information
      my $strVldEnd = vx_date_calc_valid($strInit, $intEnd);
      my %mapInt = (
          "int_beg" => $intBeg,
          "int_end" => $intEnd,
          "accum"   => $intAccum,
          "init"    => $strInit,
          "vld_beg" => vx_date_calc_valid($strInit, $intBeg),
          "vld_end" => $strVldEnd,
          "lead"    => $strLead,
          "int"     => $strPcpInt,
          "file"    => $strDataFile
      );
  
      if( $mapPcp{$strVldEnd} ){
        my @listPcpVld = @{$mapPcp{$strVldEnd}};
        foreach my $i (0..@listPcpVld){          
          if( $i == @listPcpVld || $intAccum >= $listPcpVld[$i]->{"accum"} ){ 
            splice(@{$mapPcp{$strVldEnd}}, $i, 0, \%mapInt); 
            last; 
          }
        }
      }
      else { $mapPcp{$strVldEnd} = [ \%mapInt ]; }
  
    }  #  END:  foreach $strPcpRec
  
  }  #  END:  foreach $strDataFile

  return \%mapPcp;
}


#######################################################################
# vx_pcp_print_map()
#
#   This function prints the complete data structure that is built by
#   the vx_pcp_build_map() function, for debugging purposes.  Output 
#   is sent to vx_log().
#
#   Arguments:
#          pcp_map = valid/accum map to print
#
#######################################################################
sub vx_pcp_print_map {
  my %mapPcp = %{ $_[0] };

  vx_log("\n# # # #  Precip Map  # # # #\n");
  foreach my $strVldEnd (sort keys %mapPcp){
    vx_log("  # # mapPcp[ $strVldEnd ]");
    foreach my $intRefInt ( @{$mapPcp{$strVldEnd}} ) {
      vx_pcp_print_int($intRefInt);
    }
    vx_log("");
  }
}


#######################################################################
# vx_pcp_print_int()
#
#   This function prints a single interval map to vx_log().
#
#   Arguments:
#          ref_int = reference to an accumulation interval map
#
#######################################################################
sub vx_pcp_print_int {
  return if( 1 != @_ );
  my $intRefInt = shift;
  vx_log("    {\n" .
         "       int_beg = " . $intRefInt->{"int_beg"}  . "\n" .
         "       int_end = " . $intRefInt->{"int_end"}  . "\n" .
         "         accum = " . $intRefInt->{"accum"}    . "\n" .
         "          init = " . $intRefInt->{"init"}     . "\n" .
         "       vld_beg = " . $intRefInt->{"vld_beg"}  . "\n" .
         "       vld_end = " . $intRefInt->{"vld_end"}  . "\n" .
         "          lead = " . $intRefInt->{"lead"}     . "\n" .
         "           int = " . $intRefInt->{"int"}      . "\n" .
         "          file = " . $intRefInt->{"file"}     . "\n" .
         "    }");
}



sub vx_pcp_wizard {
  my $strAccumIn  = shift;
  my @listValid   = @{ $_[0] }; shift;
  my @listArg     = (@_);

  # set up the verbosity flag
  my $intVerb = 1;
  if( 3 < @listArg ){
    $intVerb = $listArg[3];
    if( $listArg[3] > 0 ){ $listArg[3] *= -1;   }
    else                 { $listArg[3] =  -0.5; }
  } else {
    $listArg[3] = -1;
  }

  # try to build a pcp_combine add command for each valid time
  foreach my $strValidIn (@listValid){

    if( 2 <= $intVerb ){
      vx_log("\n# # # # # # # # # # # # # # # # # # # # # # # # # # # \n" .
              "#  BUILDING $strAccumIn at $strValidIn\n");
    }

    # attempt to build the interval using both addition and subtraction
    next if( vx_pcp_wizard_add($strAccumIn, [$strValidIn], @listArg) );
    next if( vx_pcp_wizard_sub($strAccumIn, [$strValidIn], @listArg) );
    vx_log("WARNING: failed to combine interval $strAccumIn for valid $strValidIn\n");
  }
}


######################################################################
# vx_pcp_wizard_add()
#
#   This function attempts to build and run a pcp_combine add command 
#   for each of the input valid times with the specified accumulation 
#   interval.  It searches the specified valid/accum map for files 
#   with useful accumulations.  This method assumes the constants
#   file contains a MET_PCP_COMBINE setting that points to the 
#   instance of pcp_combine to use.  This method returns the number
#   of successful intervals processed.
#
#   Arguments:
#           accum = accumulation interval to build for each valid time
#           valid = list of valid times to build accumulations for
#         pcp_map = valid/accum map, see vx_pcp_build_map()
#        out_tmpl = pcp_combine output file template, with only valid,
#                   lead, init and accum placeholders
#        pcp_opts = map of pcp_combine options
#            verb = (optional) verbosity of output, default 1
#       accum_fix = (optional) fixed accumulation to use for addition,
#                   -1 to deactivate (default -1)
#         run_pcp = (optional) indicates whether or not to run the
#                   pcp_combine command, default 1
#
#######################################################################
sub vx_pcp_wizard_add {
  my $strAccumIn  = shift;
  my @listValid   = @{ $_[0] }; shift;
  my %mapPcp      = %{ $_[0] }; shift;
  my $strOutTmpl  = shift;
  my %mapOpts     = %{ $_[0] }; shift;
  my $intVerb     = (@_ ? shift : 1);
  my $strAccumFix = (@_ ? shift : -1);
  my $boolRunPcp  = (@_ ? shift : 1);
  my $intNumPcp   = 0;

  # try to build a pcp_combine add command for each valid time
  foreach my $strValidIn (@listValid){

    if( 2 <= abs($intVerb) ){
      vx_log("\n# # # # # # # # # # # # # # # # # # # # # # # # # # # \n" .
              "#  ADDING $strAccumIn at $strValidIn\n");
    }

    # search for accumulations until the interval is accounted for, or failure
    my @listAccum = ();
    my $strAccum = $strAccumIn;
    my $strValid = $strValidIn;
    do {  #  BEGIN:  while( $strAccum > 0 )
    
      # if a useful accumulation interval is found, add it to the list
      if( 2 <= abs($intVerb) ){ vx_log("SEARCHING:  accum = $strAccum  valid = $strValid"); }
      
      # look for an interval to use at the current valid time
      my $intRefInt = 0;
      if( $mapPcp{$strValid} ){

        # look for a file/accumulation that is less than or equal to the input
        my @listAccumChk = ( $strAccumFix == -1 ? vx_util_seq($strAccum, 1) : ($strAccumFix) );
        for my $strAccumChk (@listAccumChk){
          last if( $intRefInt = vx_pcp_int($strValid, $strAccumChk, \%mapPcp) );
        }
      
      }      
      
      # if an accumulation interval is found, apply it
      if( $intRefInt ){
        if( 2 <= abs($intVerb) ){
          vx_log("FOUND:"); 
          vx_pcp_print_int($intRefInt);
          vx_log(""); 
        }
        push( @listAccum, $intRefInt );
        $strAccum -= $intRefInt->{"accum"};
        $strValid = vx_date_calc_sub($strValid, $intRefInt->{"accum"});
      } 
      
      # if a suitable accumulation interval is not found, flag the accum
      else {
        if( 2 <= abs($intVerb) ){ vx_log("NOT FOUND\n"); }
        $strAccum = -1;
      }
    
    } while( $strAccum > 0 );

    # if an accumulation was not build, move on    
    if( 0 > $strAccum ){
      if( 0 <= $intVerb ){
        vx_log("WARNING: accumulation failed for accum = $strAccumIn  valid = $strValidIn\n");
      }
      next;
    } 
    
    # if a file already contains the accumulation, move on
    elsif (1 == @listAccum){
      if( 1 <= abs($intVerb) ){
        vx_log("WARNING: found input GRIB file for accum = $strAccumIn  valid = $strValidIn:\n" .
               "  " . $listAccum[0]->{"file"} . "\n");
      }
      $intNumPcp++;
      next;
    }

    # build the pcp_combine output file
    my $strPcpOut = vx_file_tmpl_gen(
                      $strOutTmpl, 
                      {
                        "init"  => $listAccum[0]->{"init"}, 
                        "valid" => $strValidIn, 
                        "accum" => $strAccumIn
                      });
    vx_file_mkdir($strPcpOut);
                            
    # build the pcp_combine command
    my $strPcpCmd = vx_const_get("MET_PCP_COMBINE") . " -add \\\n";
    foreach (reverse @listAccum){
      $strPcpCmd .= "  " . $_->{"file"} . " " . $_->{"accum"} . " \\\n";
    }
    $strPcpCmd .= "  $strPcpOut";
    foreach (keys %mapOpts){ $strPcpCmd .= " \\\n  -$_ " . $mapOpts{$_}; }
    if( 1 <= abs($intVerb) ){ vx_log("PCP_COMBINE: $strPcpCmd\n"); }
    
    if( $boolRunPcp ){
      # run the pcp_combine command sending output to vx_log
      my @listPcpOut = qx/$strPcpCmd 2>&1/;
      if( 1 <= abs($intVerb) ){ 
        do{ chomp(); vx_log("$_"); } foreach (@listPcpOut);
        vx_log("\n\n");
      }
    }

    $intNumPcp++;
    
  }  #  END:  foreach $strValidIn

  return $intNumPcp;
}


######################################################################
# vx_pcp_wizard_sub()
#
#   This function attempts to build and run a pcp_combine subtract 
#   command for each of the input valid times with the specified 
#   accumulation interval.  It searches the specified valid/accum map
#   for files with useful accumulations.  This method assumes the 
#   constants file contains a MET_PCP_COMBINE setting that points to 
#   the instance of pcp_combine to use.  This method returns the number
#   of successful intervals processed.
#
#   Arguments:
#           accum = accumulation interval to build for each valid time
#           valid = list of valid times to build accumulations for
#         pcp_map = valid/accum map, see vx_pcp_build_map()
#        out_tmpl = pcp_combine output file template, with only valid,
#                   lead, init and accum placeholders
#        pcp_opts = map of pcp_combine options
#            verb = (optional) verbosity of output, default 1
#       accum_fix = (optional) fixed accumulation to use for addition,
#                   -1 to deactivate, default -1
#         run_pcp = (optional) indicates whether or not to run the
#                   pcp_combine command, default 1
#
#######################################################################
sub vx_pcp_wizard_sub {
  my $strAccumIn  = shift;
  my @listValid   = @{ $_[0] }; shift;
  my %mapPcp      = %{ $_[0] }; shift;
  my $strOutTmpl  = shift;
  my %mapOpts     = %{ $_[0] }; shift;
  my $intVerb     = (@_ ? shift : 1);
  my $strAccumFix = (@_ ? shift : -1);
  my $strInit     = $mapPcp{(keys %mapPcp)[0]}->[0]->{"init"};
  my $boolRunPcp  = (@_ ? shift : 1);
  my $intNumPcp   = 0;

  # try to build a pcp_combine add command for each valid time
  foreach my $strValidIn (@listValid){

    if( 2 <= abs($intVerb) ){
      vx_log("\n# # # # # # # # # # # # # # # # # # # # # # # # # # # \n" .
              "#  SUBTRACTING $strAccumIn at $strValidIn\n");
    }

    # search for accumulations until the interval is accounted for, or failure
    my @listAccum = ();
    my $strAccum = $strAccumIn;
    my $strValid = $strValidIn;

    # if a useful accumulation interval is found, add it to the list
    if( 2 <= abs($intVerb) ){ vx_log("SEARCHING:  accum = $strAccum  valid = $strValid"); }

    # calculate the lead time for the current valid time
    my $strLead = vx_date_format_lead(
                    vx_date_parse_date( $strValid ) -
                    vx_date_parse_date( $strInit ),
                    "HHH"
                  );
    
    # look for an interval to subtract from
    my $intRefOp1 = 0;
    my @listAccumChk = ( $strAccumFix == -1 ? vx_util_seq($strAccum, $strLead) : ($strAccumFix) );
    for my $strAccumChk (@listAccumChk){
      last if( $intRefOp1 = vx_pcp_int($strValid, $strAccumChk, \%mapPcp) );
    }

    # validate the interval
    if( ! $intRefOp1 ){
      if( 0 <= $intVerb ){
        vx_log("WARNING: interval to subtract from not found for accum = $strAccum  valid = $strValid");
      }
      next;
    } elsif( $strAccum == $intRefOp1->{"accum"} ) {
      if( 1 <= abs($intVerb) ){
        vx_log("WARNING: found input GRIB file for accum = $strAccumIn  valid = $strValidIn:\n" .
               "  " . $intRefOp1->{"file"} . "\n");
      }
      $intNumPcp++;
      next;
    } elsif( 2 <= abs($intVerb) ){
      vx_log("SUBTRACTING FROM:"); 
      vx_pcp_print_int($intRefOp1);
      vx_log(""); 
    }

    # update the valid and accum to use for the subtraction interval
    $strValid = vx_date_calc_sub($strValid, $strAccum);
    $strAccum = $intRefOp1->{"accum"} - $strAccum;
    my $intRefOp2 = vx_pcp_int($strValid, $strAccum, \%mapPcp);
    
    # validate the interval to subtract
    if( ! $intRefOp2 ){
      vx_log("WARNING: interval to subtract not found for accum = $strAccum  valid = $strValid");
      next;
    } elsif( 2 <= abs($intVerb) ){
      vx_log("SUBTRACTING:"); 
      vx_pcp_print_int($intRefOp2);
      vx_log(""); 
    }

    # build the pcp_combine output file
    my $strPcpOut = vx_file_tmpl_gen(
                      $strOutTmpl, 
                      {
                        "init"  => $strInit, 
                        "valid" => $strValidIn, 
                        "accum" => $strAccumIn
                      });
    vx_file_mkdir($strPcpOut);
                            
    # build the pcp_combine command
    my $strPcpCmd = vx_const_get("MET_PCP_COMBINE") . " -subtract \\\n  " .
                    $intRefOp1->{"file"} . " " . $intRefOp1->{"accum"} . " \\\n  " .
                    $intRefOp2->{"file"} . " " . $intRefOp2->{"accum"} . " \\\n  " .
                    $strPcpOut;
    foreach (keys %mapOpts){ $strPcpCmd .= " \\\n  -$_ " . $mapOpts{$_}; }
    if( 1 <= abs($intVerb) ){ vx_log("PCP_COMBINE: $strPcpCmd\n"); }
    
    if( $boolRunPcp ){
      # run the pcp_combine command sending output to vx_log
      my @listPcpOut = qx/$strPcpCmd 2>&1/;
      if( 1 <= abs($intVerb) ){ 
        do{ chomp(); vx_log("$_"); } foreach (@listPcpOut);
        vx_log("\n\n");
      }
    }

    $intNumPcp++;
    
  }  #  END:  foreach $strValidIn

  return $intNumPcp;
}


#######################################################################
#
#   The BEGIN block checks the VXUTIL_CONST environment variable and
#   then reads the constants file.
#
#######################################################################

BEGIN {
  # set the environment to UTC
  $ENV{"TZ"} = "UTC"; tzset();

  # read the constants file, if present
  $ENV{"VXUTIL_CONST"} and vx_const_init( $ENV{"VXUTIL_CONST"} );

  # if present, initialize the GEMPAK environment
  #my $strGemDir = vx_const_get("GEM_DIR");
  #print "GEM_DIR: $strGemDir\n";
  #if( $strGemDir ne "" ){ print "calling Gemenviron\n"; qx/source $strGemDir\/Gemenviron.profile/; }
}


1;  # module must evaluate to true

