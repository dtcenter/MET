// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <cstdio>
#include <cmath>

#include "vx_util.h"

#include "data_line.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class DataLine
   //


////////////////////////////////////////////////////////////////////////


DataLine::DataLine()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


DataLine::~DataLine()

{



if ( Line )  { delete [] Line;  Line = (char *) 0; }

if ( Offset )  { delete [] Offset;  Offset = (int *) 0; }

LineNumber = N_items = N_ints = N_chars = 0;

File = (LineDataFile *) 0;

}


////////////////////////////////////////////////////////////////////////


DataLine::DataLine(const DataLine & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


DataLine & DataLine::operator=(const DataLine & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void DataLine::init_from_scratch()

{

Line = (char *) 0;

Offset = (int *) 0;


LineNumber = N_items = 0;

N_items = 0;

N_chars = N_ints = 0;

Delimiter = new char[2];
strcpy(Delimiter, " ");

File = (LineDataFile *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void DataLine::clear()

{

if ( Line )  {

   memset(Line, 0, N_chars);

}

if ( Offset )  {

   memset(Offset, 0, N_ints*sizeof(int));

}


LineNumber = 0;

N_items = 0;

File = (LineDataFile *) 0;


return;

}


////////////////////////////////////////////////////////////////////////


void DataLine::assign(const DataLine & a)

{

clear();

extend_char(a.N_chars);

memcpy(Line, a.Line, a.N_chars);

extend_int(a.N_items);

int j;

for (j=0; j<(a.N_items); ++j)  {

   Offset[j] = a.Offset[j];

}

N_items = a.N_items;

LineNumber = a.LineNumber;

File = a.File;


return;

}


////////////////////////////////////////////////////////////////////////


void DataLine::dump(ostream & out, int depth) const

{

int j;
char junk[256];
Indent prefix(depth);


out << prefix << "N_items        = " << N_items     << "\n";
out << prefix << "N_chars        = " << N_chars     << "\n";
out << prefix << "N_ints         = " << N_ints      << "\n";
out << prefix << "LineNumber     = " << LineNumber  << "\n";

out << prefix << "\n";

if ( N_items == 0 )  { out.flush();  return; }


for (j=0; j<N_items; ++j)  {

   sprintf(junk, "Item[%2d]       = \"", j);

   out << prefix << junk << (Line + Offset[j]) << "\"\n";

   if ( (j%5) == 4 )  out << prefix << "\n";

}

out << prefix << "\n";

for (j=0; j<N_items; ++j)  {

   sprintf(junk, "Offset[%2d]     = ", j);

   out << prefix << junk << (Offset[j]) << "\n";

   if ( (j%5) == 4 )  out << prefix << "\n";

}


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


const char * DataLine::get_item(int k) const

{

if ( (k < 0) || (k >= N_items) )  {

   mlog << Error << "\nDataLine::get_item(int) -> range check error\n\n";

   exit ( 1 );

}

const char * c = Line + Offset[k];

return ( c );

}


////////////////////////////////////////////////////////////////////////


const LineDataFile * DataLine::get_file() const

{

  return File;

}


////////////////////////////////////////////////////////////////////////


const char * DataLine::operator[](int k) const

{

const char * c = get_item(k);

return ( c );

}


////////////////////////////////////////////////////////////////////////


void DataLine::extend_char(int n)

{

++n;   //  add room for trailing nul, if needed

if ( N_chars >= n )  return;

int k;

k = n/dataline_charextend_alloc_inc;

if ( n%dataline_charextend_alloc_inc ) ++k;

n = k*dataline_charextend_alloc_inc;

char * u = (char *) 0;

u = new char [n];

if ( !u )  {

   mlog << Error << "\nDataLine::extend_char() -> memory allocation error\n\n";

   exit ( 1 );

}

memset(u, 0, n);

if ( Line )  {

   memcpy(u, Line, N_chars);

   delete [] Line;  Line = (char *) 0;

}

Line = u;  u = (char *) 0;

N_chars = n;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void DataLine::extend_int(int n)

{

if ( N_ints >= n )  return;

int k;

k = n/dataline_intextend_alloc_inc;

if ( n%dataline_intextend_alloc_inc ) ++k;

n = k*dataline_intextend_alloc_inc;

int * u = (int *) 0;

u = new int [n];

if ( !u )  {

   mlog << Error << "\nDataLine::extend_int() -> memory allocation error\n\n";

   exit ( 1 );

}

memset(u, 0, n);

if ( Offset )  {

   memcpy(u, Offset, N_ints*sizeof(int));

   delete [] Offset;  Offset = (int *) 0;

}

Offset = u;  u = (int *) 0;

N_ints = n;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


int DataLine::max_item_width() const

{

if ( !Line )  return ( 0 );

int j, n, w;

n = 0;

for (j=0; j<N_items; ++j)  {

   w = strlen(Line + Offset[j]);

   if ( w > n )  n = w;

}


return ( n );

}


////////////////////////////////////////////////////////////////////////


int DataLine::read_line(LineDataFile * ldf)

{

clear();

ifstream & f = *(ldf->in);

if ( !f )  return ( 0 );

File = ldf;

char c;
char * s = (char *) 0;
char * p = (char *) 0;

int pos, count;


   //
   //  get a line from the file
   //

pos = 0;

while ( f.get(c) )  {

   if ( !f )  { clear();  return ( 0 ); }

   extend_char(pos + 5);   //  better safe than sorry

   if ( c == '\n' )  { Line[pos] = (char) 0;    break; }

   Line[pos++] = c;

}

if ( !f )  { clear();  return ( 0 ); }

   //
   //  parse the line with strtok
   //

s = Line;

count = 0;

while ( (p = strtok(s, Delimiter)) != NULL )  {

   pos = (int) (p - Line);

   ++count;

   extend_int(count);

   Offset[count - 1] = pos;

   s = (char *) 0;

}   //  while


N_items = count;

LineNumber = ldf->last_line_number() + 1;


return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int DataLine::read_fwf_line(LineDataFile * ldf, const int *wdth, int n_wdth)

{

clear();

ifstream & f = *(ldf->in);

if ( !f )  return ( 0 );

File = ldf;

int i, j;
char buf[max_str_len];
char c;
int pos, count;

   //
   //  get a line from the file using the specified widths
   //
   
pos = count = 0;

for( i=0; i<n_wdth; i++ )  {

   if ( !f )  { clear();  return ( 0 ); }

   extend_char(pos + wdth[i] + 1);   //  better safe than sorry
   extend_int(++count);
   
   //
   //  get the next entry
   //
   f.read(buf, wdth[i]);             

   //
   //  store the offset to this entry
   //
   Offset[count - 1] = pos;
   
   //
   //  store this entry
   //
   for( j=0; j<wdth[i]; j++ )  {

     Line[pos++] = buf[j];
     
     //
     //  check for embedded newline
     //
     if(buf[j] == '\n') {
        mlog << Warning
             << "\nDataLine::read_fwf_line() -> "
             << "Encountered newline while parsing line "
             << ldf->last_line_number() + 1 << ".\n\n";
     }
   }

   //
   //  null terminate the entry
   //
   Line[pos++] = '\0';
}

   //
   //  advance to the end of the line
   //
   
while ( f.get(c) )  {

   if ( !f )  { clear();  return ( 0 ); }

   if ( c == '\n' )  break;
}


if ( !f )  { clear();  return ( 0 ); }

N_items = count;

LineNumber = ldf->last_line_number() + 1;


return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int DataLine::is_ok() const

{

if ( N_items == 0 )  return ( 0 );

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int DataLine::is_header() const

{

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void DataLine::set_delimiter(const char *delimiter)

{
  delete [] Delimiter;
  
  Delimiter = new char[strlen(delimiter) + 1];
  strcpy(Delimiter, delimiter);
}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class LineDataFile
   //


////////////////////////////////////////////////////////////////////////


LineDataFile::LineDataFile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


LineDataFile::~LineDataFile()

{

close();

}


////////////////////////////////////////////////////////////////////////


LineDataFile::LineDataFile(const LineDataFile &)

{

mlog << Error << "\nLineDataFile::LineDataFile(const LineDataFile &) -> should never be called!\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


LineDataFile & LineDataFile::operator=(const LineDataFile &)

{

mlog << Error << "\nLineDataFile::operator=(const LineDataFile &) -> should never be called!\n\n";

exit ( 1 );


return ( * this );

}


////////////////////////////////////////////////////////////////////////


void LineDataFile::init_from_scratch()

{

Filename = (char *) 0;

ShortFilename = (char *) 0;

in = (ifstream *) 0;

Last_Line_Number = 0;

Header.clear();

Header.set_ignore_case(true);


return;

}


////////////////////////////////////////////////////////////////////////


int LineDataFile::open(const char * path)

{

close();

in = new ifstream;

if ( !in )  {

   mlog << Error << "\nLineDataFile::open(const char *) -> can't allocate input stream\n\n";

   exit ( 1 );

}

in->open(path);

if ( !(*in) )  {

   return ( 0 );

}

   //
   //  get filename
   //

int j, n;

n = strlen(path);

Filename = new char [1 + n];

memset(Filename, 0, 1 + n);

strncpy(Filename, path, n);

j = n - 1;

while ( (j > 0) && (Filename[j] != '/') )  --j;

++j;

n = strlen(Filename + j);

ShortFilename = new char [1 + n];

memset(ShortFilename, 0, 1 + n);

strncpy(ShortFilename, Filename + j, n);


   //
   //  done
   //

Last_Line_Number = 0;

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


void LineDataFile::close()

{

if ( in )  {

   in->close();

   delete in;  in = (ifstream *) 0;

}

if ( Filename )  { delete [] Filename;  Filename = (char *) 0; }

if ( ShortFilename )  { delete [] ShortFilename;  ShortFilename = (char *) 0; }


Last_Line_Number = 0;

return;

}

////////////////////////////////////////////////////////////////////////


void LineDataFile::rewind()

{
  
   //
   //  clear any error status and rewind to beginning
   //
   
if ( in )  {

   in->clear();

   in->seekg(0, ios::beg);

   Last_Line_Number = 0;
   
}

return;

}


////////////////////////////////////////////////////////////////////////


int LineDataFile::ok() const

{

if ( !in )  return ( 0 );

if ( !(*in) )  return ( 0 );


return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int LineDataFile::operator>>(DataLine & a)

{

int status;

do { 

   status = a.read_line(this);

   if ( !status ) return ( 0 );

   ++Last_Line_Number;

   if ( a.is_header() ) set_header(a);

} while ( !(a.is_ok()) );


return ( 1 );

}


////////////////////////////////////////////////////////////////////////


void LineDataFile::set_header(DataLine & a)

{
   
int j;

Header.clear();

for (j=0; j<a.n_items(); ++j)  {
   
   Header.add(a.get_item(j));
   
}

return;

}


////////////////////////////////////////////////////////////////////////


int LineDataFile::read_fwf_line(DataLine & a, const int *wdth, int n_wdth)

{

int status;

do {

   status = a.read_fwf_line(this, wdth, n_wdth);

   if ( !status ) return ( 0 );

   ++Last_Line_Number;

} while ( !(a.is_ok()) );


return ( 1 );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream & out, const DataLine & L)

{

if ( L.n_items() == 0 )  return ( out );

int j, k, N;
char c;


k = L.N_items - 1;   //  last item

N = L.Offset[k] + strlen(L[k]);

for (j=0; j<N; ++j)  {

   c = L.Line[j];

      //
      //  patch the nul chars put there by strtok
      //

   if ( c == 0 )  out.put(' ');
   else           out.put(c);

}

out.put('\n');

out.flush();

return ( out );

}


////////////////////////////////////////////////////////////////////////


Logger & operator<<(Logger & lgr, const DataLine & L)

{

if ( L.n_items() == 0 )  return ( lgr );

int j, k, N;
char c;


k = L.N_items - 1;   //  last item

N = L.Offset[k] + strlen(L[k]);

for (j=0; j<N; ++j)  {

   c = L.Line[j];

      //
      //  patch the nul chars put there by strtok
      //

   if ( c == 0 )  lgr << ' ';
   else           lgr << c;

}

lgr << '\n';

return ( lgr );

}


////////////////////////////////////////////////////////////////////////




