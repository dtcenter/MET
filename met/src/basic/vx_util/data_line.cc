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

LineNumber = N_items = 0;

N_items = 0;

N_chars = N_ints = 0;

Delimiter.assign(dataline_default_delim);

File = (LineDataFile *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void DataLine::clear()

{

Line.clear();
Items.clear();
Offset.clear();

LineNumber = 0;

N_items = 0;

File = (LineDataFile *) 0;


return;

}


////////////////////////////////////////////////////////////////////////


void DataLine::assign(const DataLine & a)

{

clear();

//extend_char(a.N_chars);

Line = a.Line;
Items = a.Items;

//extend_int(a.N_items);

Offset = a.Offset;

N_items = a.N_items;

LineNumber = a.LineNumber;

File = a.File;


return;

}


////////////////////////////////////////////////////////////////////////


void DataLine::dump(ostream & out, int depth) const

{

int j;
//char junk[256];
Indent prefix(depth);


out << prefix << "N_items        = " << N_items     << "\n";
out << prefix << "N_chars        = " << N_chars     << "\n";
out << prefix << "N_ints         = " << N_ints      << "\n";
out << prefix << "LineNumber     = " << LineNumber  << "\n";

out << prefix << "\n";

if ( N_items == 0 )  { out.flush();  return; }

std::ostringstream sstream;
sstream.width(2);

for (j=0; j<N_items; ++j)  {

   //snprintf(junk, sizeof(junk), "Item[%2d]       = \"", j);

   sstream << "Item[" << j << "]       = \"";
   out << prefix << sstream.str() << Line.substr(j) << "\"\n";

   if ( (j%5) == 4 )  out << prefix << "\n";

}

out << prefix << "\n";


sstream.str("");
sstream.clear();

for (j=0; j<N_items; ++j)  {

   //snprintf(junk, sizeof(junk), "Offset[%2d]     = ", j);
   sstream << "Offset[" << j << "]     = ";
   out << prefix << sstream.str() << Offset[j] << "\n";

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

return Items[k].c_str();

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

/*
void DataLine::extend_char(int n)

{

++n;   //  add room for trailing nul, if needed

if ( N_chars >= n )  return;

int k;

k = n/dataline_charextend_alloc_inc;

if ( n%dataline_charextend_alloc_inc ) ++k;

n = k*dataline_charextend_alloc_inc;

Line.reserve(n);
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

Offset.reserve(n);

N_ints = n;

   //
   //  done
   //

return;

}
*/

////////////////////////////////////////////////////////////////////////


int DataLine::max_item_width() const

{

if ( Line.empty() )  return ( 0 );

int j, n, w;

n = 0;

for (j=0; j<N_items; ++j)  {

   w = Items[j].size();

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
size_t pos, count;


   //
   //  get a line from the file
   //

pos = 0;
count = 0;

while ( f.get(c) )  {

   if ( !f )  { clear();  return ( 0 ); }

   //extend_char(pos + 5);   //  better safe than sorry

   if ( c == '\n' )  { break; }

   Line += c;

}

if ( !f )  { clear();  return ( 0 ); }

   //
   //  parse the line with strtok
   //

size_t len, tpos = std::string::npos;

if (!Line.find_first_not_of(Delimiter)) { // no leading delimiter
    ++count;
    //extend_int(count);
    Offset.push_back(pos);
    Items.push_back(Line.substr(pos, Line.find_first_of(Delimiter, pos) - pos));
}
while ((tpos = Line.substr(pos).find_first_of(Delimiter)) != std::string::npos)  {
    len = Line.substr(pos+tpos).find_first_not_of(Delimiter);
    if (len == std::string::npos)  // delims at the end of the line, or delim-only line
        break;
    pos += tpos + len;
    
    ++count;
    //extend_int(count);
    Offset.push_back(pos);
    Items.push_back(Line.substr(pos, Line.find_first_of(Delimiter, pos) - pos));
}

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
int start, pos, count;
int null_char_count;

   //
   //  get a line from the file using the specified widths
   //

null_char_count = pos = count = 0;

for( i=0; i<n_wdth; i++ )  {

   if ( !f )  { clear();  return ( 0 ); }

   //extend_char(pos + wdth[i] + 1);   //  better safe than sorry
   //extend_int(++count);
   ++count;

   //
   //  get the next entry
   //
   f.read(buf, wdth[i]);

   //
   //  store the offset to this entry
   //
   start = pos;
   Offset.push_back(pos);

   //
   //  store this entry
   //
   for( j=0; j<wdth[i]; j++ )  {

     Line += buf[j]; pos++;

     //
     //  check for embedded newline
     //
     if(buf[j] == '\0') {
        Line[pos-1] = ' ';
        null_char_count++;
     }
     else if(buf[j] == '\n') {
        mlog << Warning
             << "\nDataLine::read_fwf_line() -> "
             << "Encountered newline while parsing line "
             << ldf->last_line_number() + 1 << ".\n\n";
     }
   }
   
   Items.push_back(Line.substr(start, pos-start));

   //
   //  null terminate the entry
   //
   Line += '\0';
   pos++;
}

if (null_char_count > 0)
   mlog << Warning
        << "\nDataLine::read_fwf_line() -> "
        << "Encountered " << null_char_count << " null character"
        << (null_char_count==1?"":"s") << " while parsing line "
        << ldf->last_line_number() + 1 << ".\n\n";
   
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
  Delimiter.assign(delimiter);
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

met_open((*in), path);

if ( !(*in) )  {

   return ( 0 );

}

   //
   //  get filename
   //

int j, n;

n = strlen(path);

Filename.assign(path);
ShortFilename.assign(get_short_name(path));


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

int j;

for (j = 0; j < L.Items.size(); j++) {
    out << L.Items[j];
    if (j < (L.Items.size() - 1))
        out << ' ';
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


// k = L.N_items - 1;   //  last item
// 
// N = L.Offset[k] + strlen(L[k]);
// 
// for (j=0; j<N; ++j)  {
// 
//    c = L.Line[j];
// 
//       //
//       //  patch the nul chars put there by strtok
//       //
// 
//    if ( c == 0 )  lgr << ' ';
//    else           lgr << c;
// 
// }

for (j = 0; j < L.Items.size(); j++) {
    lgr << L.Items[j];
    if (j < (L.Items.size() - 1))
        lgr << ' ';
}


lgr << '\n';

return ( lgr );

}


////////////////////////////////////////////////////////////////////////




