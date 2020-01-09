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
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_util.h"
#include "vx_tc_util.h"

////////////////////////////////////////////////////////////////////////

static ConcatString program_name;

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   program_name = get_short_name(argv[0]);

   if(argc != 2) {
      mlog << Error
           << "\nusage: " << program_name
           << " track_filename\n\n";
      exit(1);
   }

   ConcatString input_filename = (string)argv[1];
   LineDataFile f;
   ATCFTrackLine t_line;
   TrackInfoArray t_array;
   int count, i;

   if(!f.open(input_filename.c_str())) {
      mlog << Error
           << "\n" << program_name
           << ": unable to open input file \"" << input_filename << "\"\n\n";
      exit(1);
   }

   count = 0;
   while(f >> t_line) {

      // Increment the line count
      count++;

      // Add the current line to the TrackInfoArray
      t_array.add(t_line);
   }

   // Dump out summary info
   mlog << Debug(1)
        << "Read " << count << " lines from input file \""
        << input_filename << "\"\n";
   mlog << Debug(1)
        << "TrackInfoArray contains " << t_array.n_tracks()
        << " tracks.\n";

   for(i=0; i<t_array.n_tracks(); i++) {
      mlog << Debug(1)
           << "TrackInfo[" << i+1 << "] contains "
           << t_array[i].n_points() << " points.\n";
   }

   if(mlog.verbosity_level() > 0) {
      cout << "TrackInfoArray::dump() ->\n";
      t_array.dump(cout, 1);
   }

   // Clean up
   f.close();

   return(0);

}

////////////////////////////////////////////////////////////////////////
