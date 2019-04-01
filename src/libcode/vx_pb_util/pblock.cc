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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>

#include "vx_log.h"
#include "pblock.h"
#include "do_blocking.h"
#include "do_unblocking.h"

////////////////////////////////////////////////////////////////////////

void pblock(const char *infile, const char *outfile, Action action) {
   PadSize padsize;
   int in  = -1;
   int out = -1;

   //
   // Open the input file
   //
   if( (in = met_open(infile, O_RDONLY)) < 0)  {
      mlog << Error << "\npblock() -> "
           << "unable to open input file \"" << infile << "\"\n\n";

      exit(1);
   }

   //
   // Open the output file
   //
   if( (out = open(outfile, O_WRONLY | O_CREAT, 0644)) < 0)  {
      mlog << Error << "\npblock() -> "
           << "unable to open output file \"" << outfile << "\"\n\n";

      exit(1);
   }

   //
   // Set the block size for this compiler 
   //
   #ifdef BLOCK4
      padsize = padsize_4;
   #else
      padsize = padsize_8;
   #endif

   //
   // Block or unblock the file
   //
   switch(action)  {

      case block:
         do_blocking(in, out, padsize);
         break;

      case unblock:
         do_unblocking(in, out, padsize);
         break;

      default:
         mlog << Error << "\npblock() ->"
              << "unexpected action requested!\n\n";

         exit(1);
         break;
   }

   //
   // Close in the input and output files
   //
   close(in);  in = -1;
   close(out); out = -1;

   return;
}

////////////////////////////////////////////////////////////////////////
