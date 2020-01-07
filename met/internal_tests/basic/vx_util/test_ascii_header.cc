// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

#include "vx_util.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

static void usage();

////////////////////////////////////////////////////////////////////////

static ConcatString program_name;

ConcatString Version, DataType, LineType;
StringArray  ColNames;
IntArray     ColOffsets;
int          Dim = 0;

static void set_verbosity   (const StringArray &);
static void set_name        (const StringArray &);
static void set_offset      (const StringArray &);
static void set_dim         (const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv []) {
   int i;

   program_name = get_short_name(argv[0]);

   CommandLine cline;

   cline.set(argc, argv);
   cline.set_usage(usage);
   cline.add(set_name,        "-name",        1);
   cline.add(set_offset,      "-offset",     -1);
   cline.add(set_dim,         "-dim",         1);
   cline.add(set_verbosity,   "-v",           1);
   cline.parse();

   // Check for required arguments: version, data_type, line_type
   if(cline.n() != 3) usage();

   // Store the input file names
   Version  = cline[0];
   DataType = cline[1];
   LineType = cline[2];

   // AsciiHeader object
   AsciiHeader hdr;
   const AsciiHeaderLine *line = hdr.header(Version.c_str(), DataType.c_str(), LineType.c_str());

   mlog << Debug(1)
        << "Version:  " << Version << "\n"
        << "DataType: " << DataType << "\n"
        << "LineType: " << LineType << "\n";

   // Process each column name
   for(i=0; i<ColNames.n_elements(); i++) {
      mlog << Debug(1)
           << ColNames[i] << " Offset = "
           << line->col_offset(ColNames[i].c_str(), Dim) << "\n";
   }

   // Process each column offset
   for(i=0; i<ColOffsets.n_elements(); i++) {
      mlog << Debug(1)
           << "Offset " << ColOffsets[i] << " = "
           << line->col_name(ColOffsets[i], Dim) << "\n";
   }

   return(0);
}

////////////////////////////////////////////////////////////////////////

void usage() {

   mlog << Error
        << "\nUsage: " << program_name << "\n"
        << "\tversion\n"
        << "\tdata_type\n"
        << "\tline_type\n"
        << "\t[-dim    n]\n"
        << "\t[-name   str]\n"
        << "\t[-offset beg [end]]\n\n";
   exit(1);

   return;
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a) {
   mlog.set_verbosity_level(atoi(a[0].c_str()));
}

////////////////////////////////////////////////////////////////////////

void set_name(const StringArray & a) {
   ColNames.add_css(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_offset(const StringArray & a) {
        if(a.n_elements() == 1) ColOffsets.add(atoi(a[0].c_str()));
   else if(a.n_elements() == 2) {
      for(int i=atoi(a[0].c_str()); i<=atoi(a[1].c_str()); i++) ColOffsets.add(i);
   }
   else {
      usage();
   }
}

////////////////////////////////////////////////////////////////////////

void set_dim(const StringArray & a) {
   Dim = atoi(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////
