// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_cal.h"
#include "vx_util.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

static void set_regex (const StringArray &);
static void set_test  (const StringArray &);

static void usage();

static ConcatString RegexCS;
static ConcatString TestCS;

////////////////////////////////////////////////////////////////////////

static ConcatString program_name;

////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv []) {

   program_name = get_short_name(argv[0]);

   CommandLine c;

   c.set(argc, argv);

   c.add(set_regex, "-regex", 1);
   c.add(set_test,  "-test",  1);
   c.set_usage(usage);
   c.parse();

   if(RegexCS.length() == 0 || TestCS.length() == 0) {
      usage();
      exit(1);
   }

   mlog << Debug(1) << "Regex  = '" << RegexCS << "'\n";
   mlog << Debug(1) << "Test   = '" << TestCS  << "'\n";
   mlog << Debug(1) << "Result = "
        << bool_to_string(check_reg_exp(RegexCS.c_str(), TestCS.c_str())) << "\n";

   return(0);
}

////////////////////////////////////////////////////////////////////////

void set_regex(const StringArray & a) {
   RegexCS = a[0];
   return;
}

////////////////////////////////////////////////////////////////////////

void set_test(const StringArray & a) {
   TestCS = a[0];
   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {
   mlog << Error
        << "\nusage:  " << program_name
        << " -regex string -test string\n\n";
   exit(1);

   return;
}

////////////////////////////////////////////////////////////////////////
