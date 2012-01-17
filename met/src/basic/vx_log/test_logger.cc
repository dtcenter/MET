// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


using namespace std;


//////////////////////////////////////////////////////////////////


#include "logger.h"


//////////////////////////////////////////////////////////////////


int main(int argc, char * argv[])
{
   ConcatString infile = "test_log_file.txt";
   ConcatString str;
   int n = 4;
   bool b = true;
   double d = 11.4;
   char c = 'A';

   mlog.set_verbosity_level(2);

   mlog.open_log_file(infile);

   mlog << level(-1) << "\n\n  Test error message\n\n";

   mlog << level(0) << "\n\n  Test warning message\n\n";

   mlog << level(1) << "\n\n  Test debug message level 1\n\n";

   mlog << level(2) << "\n\n  Test debug message level 2\n\n";

   mlog << level(3) << "\n\n  Test debug message level 3\n\n";

   mlog << level(4) << "\n\n  Test debug message level 4\n\n";

   mlog << level(1) << "\n\n  Does more " << "than one message work?\n\n";

   mlog << level(2) << "\n\n  Can it print an int: " << n << "?\n\n";

   mlog << level(2) << "\n\n  Can it print a double: " << d << "?\n\n";

   mlog << level(2) << "\n\n  Can it print a char: " << c << "?\n\n";

   mlog << level(2) << "\n\n  Can it print a bool: " << b << "?\n\n";

   mlog << Error << "\n\n  New type error message\n\n";

   mlog << Warning << "\n\n  New type warning message\n\n";

   mlog << Debug(1) << "\n\n  New type debug level 1 message\n\n";
   mlog << Debug(2) << "\n\n  New type debug level 2 message\n\n";
   mlog << Debug(3) << "\n\n  New type debug level 3 message\n\n";
   mlog << Debug(4) << "\n\n  New type debug level 4 message\n\n";

   mlog << Error << "\n\n  New type err msg " << "with more than " << 1 << " message\n\n";

   mlog << Warning << "\n\n  New type warn msg " << "with more than " << 1 << " message\n\n";

   mlog << Debug(1) << "\n\n  New type dbg msg " << "with more than " << 1 << " message\n\n";

   return (0);

}


//////////////////////////////////////////////////////////////////


