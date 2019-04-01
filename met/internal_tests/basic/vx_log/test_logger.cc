// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
  ConcatString infile = (string)"test_log_file.txt";
   ConcatString str;
   int n = 4;
   bool b = true;
   double d = 11.4;
   char c = 'A';

   mlog.set_verbosity_level(2);

   mlog.open_log_file(infile);

   mlog << Debug(1) << "";  // try sending an empty string and see if it prints "(NUL)"

   mlog << level(-1) << "\n  Test error message\n\n";

   mlog << level(0) << "\n  Test warning message\n\n";

   mlog << level(1) << "\n  Test debug message level 1\n\n";

   mlog << level(2) << "\n  Test debug message level 2\n\n";

   mlog << level(3) << "\n  Test debug message level 3\n\n";

   mlog << level(4) << "\n  Test debug message level 4\n\n";

   mlog << level(1) << "\n  Does more " << "than one message work?\n\n";

   mlog << level(2) << "\n  Can it print an int: " << n << "?\n\n";

   mlog << level(2) << "\n  Can it print a double: " << d << "?\n\n";

   mlog << level(2) << "\n  Can it print a char: " << c << "?\n\n";

   mlog << level(2) << "\n  Can it print a bool: " << b << "?\n\n";

   mlog << Error << "\nNew type error message\n\n";

   mlog << Warning << "\nNew type warning message\n\n";

   mlog << Debug(1) << "\n  New type debug level 1 message\n\n";
   mlog << Debug(2) << "\n  New type debug level 2 message\n\n";
   mlog << Debug(3) << "\n  New type debug level 3 message\n\n";
   mlog << Debug(4) << "\n  New type debug level 4 message\n\n";

   mlog << Error << "\nNew type err msg " << "with more than " << 1 << " message\n\n";

   mlog << Warning << "\nNew type warn msg " << "with more than " << 1 << " message\n\n";

   mlog << Debug(1) << "\n  New type dbg msg " << "with more than " << 1 << " message\n\n";

   mlog.set_exit_on_warning(true);
   mlog << Warning << "\nNew warning message after setting:\nexit_on_warning = true;\n\n";

   mlog << Debug(1) << "\n  New type debug level 1 message\n\n";
   
   return (0);

}


//////////////////////////////////////////////////////////////////


