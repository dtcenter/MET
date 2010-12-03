// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_util/ascii_table.h"
#include "vx_util/comma_string.h"


////////////////////////////////////////////////////////////////////////


static void do_blank_line(ostream & out, bool FillBlank, int width);

static bool all_blanks(const char *);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class AsciiTable
   //


////////////////////////////////////////////////////////////////////////


AsciiTable::AsciiTable()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


AsciiTable::~AsciiTable()

{

clear();

}


////////////////////////////////////////////////////////////////////////


AsciiTable::AsciiTable(const AsciiTable & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


AsciiTable & AsciiTable::operator=(const AsciiTable & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::init_from_scratch()

{

e = (char **) 0;

ColWidth = (int *) 0;

InterColumnSpace = (int *) 0;

InterRowSpace = (int *) 0;

BadDataStr = (char *) 0;

Just = (AsciiTableJust *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::clear()

{

char tmp_str[512];

if ( e )  {

   int j, n;

   n = Nrows*Ncols;

   for (j=0; j<n; ++j)  {

      if ( e[j] )  { delete [] e[j];  e[j] = (char *) 0; }

   }

   delete [] e;  e = (char **) 0;

}

if ( BadDataStr )  {

   delete [] BadDataStr;  BadDataStr = (char *) 0;

}

if ( ColWidth )  { delete [] ColWidth;  ColWidth = (int *) 0; }

if ( Just )  { delete [] Just;  Just = (AsciiTableJust *) 0; }

if ( InterColumnSpace )  { delete [] InterColumnSpace;  InterColumnSpace = (int *) 0; }

if ( InterRowSpace )  { delete [] InterRowSpace;  InterRowSpace = (int *) 0; }

TableIndent = default_table_indent;

FillBlank = default_fill_blank;

ColSepChar = default_col_sep_char;

PadChar = default_table_pad_char;

Nrows = Ncols = 0;

set_precision(ascii_table_default_precision);

set_bad_data_value(ascii_table_default_bad_data_value);

sprintf(tmp_str, "%.0f", ascii_table_default_bad_data_value);

set_bad_data_str(tmp_str);

DoCommaString = false;

DeleteTrailingBlankRows = false;

ElimTrailingWhitespace = true;

return;

}


////////////////////////////////////////////////////////////////////////


int AsciiTable::rc_to_n(int r, int c) const

{

if ( !e )  {

   cerr << "\n\n  AsciiTable::rc_to_n() -> empty table!\n\n";

   exit ( 1 );

}

if ( (r < 0) || (r >= Nrows) || (c < 0) || (c >= Ncols) )  {

   cerr << "\n\n"
        << "  AsciiTable::rc_to_n() -> range check error ... \n"
        << "                           (Nrows, Ncols) = (" << Nrows << ", " << Ncols << ")\n"
        << "                           (r, c) = (" << r << ", " << c << ")\n"
        << "\n\n";

   exit ( 1 );

}

int n;

n = r*Ncols + c;

return ( n );

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::assign(const AsciiTable & a)

{

clear();

if ( !(a.e) )  return;

set_size(a.nrows(), a.ncols());


int j, r, c;


for (j=0; j<Ncols; ++j)  ColWidth[j] = a.ColWidth[j];

for (j=0; j<(Ncols - 1); ++j)  InterColumnSpace [j] = a.InterColumnSpace [j];
for (j=0; j<(Nrows - 1); ++j)  InterRowSpace    [j] = a.InterRowSpace    [j];

for (j=0; j<(Nrows*Ncols); ++j)  Just[j] = a.Just[j];

TableIndent  = a.TableIndent;

FillBlank    = a.FillBlank;

ColSepChar   = a.ColSepChar;

PadChar      = a.PadChar;

Precision    = a.Precision;

BadDataValue = a.BadDataValue;

set_bad_data_str(a.BadDataStr);

memcpy(FloatFormat, a.FloatFormat, sizeof(FloatFormat));

DoCommaString = a.DoCommaString;

DeleteTrailingBlankRows = a.DeleteTrailingBlankRows;

ElimTrailingWhitespace = a.ElimTrailingWhitespace;


for (r=0; r<Nrows; ++r)  {

   for (c=0; c<Ncols; ++c)  {

      set_entry(r, c, a(r, c));

   }

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_size(const int NR, const int NC)

{

if ( (NR <= 0) || (NC <= 0) )  {

   cerr << "\n\n  AsciiTable::set_size() -> bad size\n\n";

   exit ( 1 );

}

clear();

int j;
const int NRC = NR*NC;

e = new char * [NRC];

if ( !e )  {

   cerr << "\n\n  AsciiTable::set_size_rc() -> memory allocation error 1\n\n";

   exit ( 1 );

}

for (j=0; j<NRC; ++j)  e[j] = (char *) 0;

Nrows = NR;
Ncols = NC;

ColWidth = new int [Ncols];

if ( !ColWidth )  {

   cerr << "\n\n  AsciiTable::set_size_rc() -> memory allocation error 2\n\n";

   exit ( 1 );

}

for (j=0; j<Ncols; ++j)  ColWidth[j] = 0;

InterColumnSpace = new int [Ncols - 1];

for (j=0; j<(Ncols - 1); ++j)  InterColumnSpace[j] = default_ics;

InterRowSpace = new int [Nrows - 1];

for (j=0; j<(Nrows - 1); ++j)  InterRowSpace[j] = default_irs;

Just = new AsciiTableJust [Nrows*Ncols];

if ( !Just )  {

   cerr << "\n\n  AsciiTable::set_size_rc() -> memory allocation error 3\n\n";

   exit ( 1 );

}

for (j=0; j<(Nrows*Ncols); ++j)  Just[j] = default_justification;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_ics(int value)

{

if ( (value < 0) || (value > max_ics) )  {

   cerr << "\n\n   AsciiTable::set_ics(int) -> bad value ... " << value << "\n\n";

   exit ( 1 );

}

int j;

for (j=0; j<(Ncols - 1); ++j)  InterColumnSpace[j] = value;

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_ics(int col_left, int value)

{

if ( (value < 0) || (value > max_ics) )  {

   cerr << "\n\n   AsciiTable::set_ics(int, int) -> bad value ... " << value << "\n\n";

   exit ( 1 );

}

if ( (col_left < 0) || (col_left >= (Ncols - 1)) )  {

   cerr << "\n\n   AsciiTable::set_ics(int, int) -> bad column ... " << col_left << "\n\n";

   exit ( 1 );

}


InterColumnSpace[col_left] = value;

return;

}


////////////////////////////////////////////////////////////////////////


int AsciiTable::inter_column_space(int col_left) const

{

if ( (col_left < 0) || (col_left >= (Ncols - 1)) )  {

   cerr << "\n\n   AsciiTable::inter_column_space() -> bad column ... " << col_left << "\n\n";

   exit ( 1 );

}

if ( !InterColumnSpace )  return ( 0 );

return ( InterColumnSpace[col_left] );

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_irs(int value)

{

if ( (value < 0) || (value > max_irs) )  {

   cerr << "\n\n   AsciiTable::set_irs(int) -> bad value ... " << value << "\n\n";

   exit ( 1 );

}

int j;

for (j=0; j<(Nrows - 1); ++j)  InterRowSpace[j] = value;

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_irs(int row_top, int value)

{

if ( (value < 0) || (value > max_irs) )  {

   cerr << "\n\n   AsciiTable::set_irs(int, int) -> bad value ... " << value << "\n\n";

   exit ( 1 );

}

if ( (row_top < 0) || (row_top >= (Nrows - 1)) )  {

   cerr << "\n\n   AsciiTable::set_irs(int, int) -> bad row ... " << row_top << "\n\n";

   exit ( 1 );

}

InterRowSpace[row_top] = value;

return;

}


////////////////////////////////////////////////////////////////////////


int AsciiTable::inter_row_space(int row_top) const

{

if ( (row_top < 0) || (row_top >= (Nrows - 1)) )  {

   cerr << "\n\n   AsciiTable::inter_row_space(int) -> bad row ... " << row_top << "\n\n";

   exit ( 1 );

}

if ( !InterRowSpace )  return ( 0 );

return ( InterRowSpace[row_top] );

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_table_indent(int value)

{

if ( (value < 0) || (value > max_table_indent) )  {

   cerr << "\n\n  AsciiTable::set_table_indent(int) -> bad value\n\n";

   exit ( 1 );

}

TableIndent = value;

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_col_sep_char(const char c)

{

ColSepChar = c;

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_pad_char(const char c)

{

PadChar = c;

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_precision(int k)

{

if ( (k < 0) || (k > ascii_table_max_precision) )  {

   cerr << "\n\n  AsciiTable::set_precision(int) -> bad value\n\n";

   exit ( 1 );

}

Precision = k;

memset(FloatFormat, 0, sizeof(FloatFormat));

sprintf(FloatFormat, "%%.%df", Precision);

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_bad_data_value(double d)

{

BadDataValue = d;

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_bad_data_str(const char *str)

{

if ( BadDataStr ) { delete [] BadDataStr; BadDataStr = (char *) 0; }

BadDataStr = new char [strlen(str)+1];

strcpy(BadDataStr, str);

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_comma_string(bool tf)

{

DoCommaString = tf;

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_delete_trailing_blank_rows(bool tf)

{

DeleteTrailingBlankRows = tf;

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_elim_trailing_whitespace(bool tf)

{

ElimTrailingWhitespace = tf;

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_table_just(const AsciiTableJust just)

{

int j;
const int N = Nrows*Ncols;

for (j=0; j<N; ++j)  Just[j] = just;

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_column_just(const int c, const AsciiTableJust just)

{

if ( (c < 0) || (c >= Ncols) )  {

   cerr << "\n\n  AsciiTable::set_col_just() -> range check error!\n\n";

   exit ( 1 );

}

int r, n;

for (r=0; r<Nrows; ++r)  {

   n = rc_to_n(r, c);

   Just[n] = just;

}

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_row_just(const int r, const AsciiTableJust just)

{

if ( (r < 0) || (r >= Nrows) )  {

   cerr << "\n\n  AsciiTable::set_row_just() -> range check error!\n\n";

   exit ( 1 );

}

int c, n;

for (c=0; c<Ncols; ++c)  {

   n = rc_to_n(r, c);

   Just[n] = just;

}

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_entry_just(const int r, const int c, const AsciiTableJust just)

{

int n;

n = rc_to_n(r, c);   //  "rc_to_n" does range checking on r and c,
                     //    so we don't need to do that here

Just[n] = just;

return;

}


////////////////////////////////////////////////////////////////////////


AsciiTableJust AsciiTable::entry_just(const int r, const int c) const

{

int n;

n = rc_to_n(r, c);   //  "rc_to_n" does range checking on r and c,
                     //    so we don't need to do that here


return ( Just[n] );

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_entry(const int r, const int c, const char * text)

{


int n, k;

n = rc_to_n(r, c);   //  "rc_to_n" does range checking on r and c,
                     //    so we don't need to do that here

   //
   //  delete old entry, if any
   //

if ( e[n] )  { delete [] e[n];  e[n] = (char *) 0; }

if ( !text )  return;

k = strlen(text);

e[n] = new char [1 + k];

if ( !(e[n]) )  {

   cerr << "\n\n  AsciiTable::set_entry() -> memory allocation error\n\n";

   exit ( 1 );

}

memset(e[n], 0, 1 + k);

strcpy(e[n], text);

if ( ColWidth[c] < k )  ColWidth[c] = k;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_entry(const int r, const int c, int a)

{

char junk[256];

if ( abs(a - BadDataValue) < 0.0001 )  strcpy(junk, BadDataStr);
else if ( DoCommaString )              ::comma_string(a, junk);
else                                   sprintf(junk, "%d", a);

set_entry(r, c, junk);

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_entry(const int r, const int c, double x)

{

char junk[256];

if ( abs(x - BadDataValue) < 0.0001 )  strcpy(junk, BadDataStr);
else                                   sprintf(junk, FloatFormat, x);

if ( DoCommaString )  {

   char * p = (char *) 0;
   long X;
   ConcatString s;
   char j2[256];

   p = strchr(junk, '.');

   if ( p )  *p = (char) 0;

   ++p;

   X = atol(junk);

   ::comma_string(X, j2);

   if ( (X == 0) && (x < 0.0) )  s << '-';

   s << j2;

   if ( Precision > 0 )  s << '.' << p;

   set_entry(r, c, s);

} else set_entry(r, c, junk);

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_entry(const int r, const int c, char a)

{

char junk[32];

junk[0] = a;

junk[1] = (char) 0;

set_entry(r, c, junk);

return;

}


////////////////////////////////////////////////////////////////////////


const char * AsciiTable::operator()(int r, int c) const

{

int n;

n = rc_to_n(r, c);   //  "rc_to_n" does range checking on r and c,
                     //    so we don't need to do that here

return ( e[n] );     //  might be null

}


////////////////////////////////////////////////////////////////////////


int AsciiTable::col_width(int k) const

{

if ( !e )  return ( 0 );

if ( (k < 0) || (k >= Ncols) )  {

   cerr << "\n\n  AsciiTable::col_width(int) const -> range check error\n\n";

   exit ( 1 );

}

return ( ColWidth[k] );

}


////////////////////////////////////////////////////////////////////////


int AsciiTable::max_col_width() const

{

if ( !e )  return ( 0 );

int c, w, k;

k = 0;

for (c=0; c<Ncols; ++c)  {

   w = ColWidth[c];

   if ( w > k )  k = w;

}

return ( k );

}


////////////////////////////////////////////////////////////////////////


bool AsciiTable::row_is_blank(int r) const

{

if ( (r < 0) || (r >= Nrows) )  {

   cerr << "\n\n  AsciiTable::row_is_blank(int) const -> range check error\n\n";

   exit ( 1 );

}

int c, n;

for (c=0; c<Ncols; ++c)  {

   n = rc_to_n(r, c);

   if ( e[n] )  {

      if ( !all_blanks(e[n]) )  return ( false );

   }

}


return ( true );

}


////////////////////////////////////////////////////////////////////////


ConcatString AsciiTable::padded_row(const int r) const

{

int c, j, k;
ConcatString s;


for (c=0; c<Ncols; ++c)  {

   if ( c > 0 )  {

      k = InterColumnSpace[c - 1];

      for (j=0; j<k; ++j)  s << ColSepChar;

   }

   s << padded_entry(r, c);

}   //  for c


if ( ElimTrailingWhitespace )  s.elim_trailing_whitespace();


return ( s );

}


////////////////////////////////////////////////////////////////////////


ConcatString AsciiTable::padded_entry(const int r, const int c) const

{

int n, w;
ConcatString s;

n = rc_to_n(r, c);   //  "rc_to_n" does range checking on r and c,
                     //            so we don't need to do that here

w = ColWidth[c];

if ( w == 0 )  {

   char junk[2];

   junk[0] = (char) 0;

   s = junk;

   return ( s );

}

   //
   //  so now we know the column width is > 0
   //

if ( !(e[n]) )  {

   s.set_repeat(' ', w);

   return ( s );

}

char * out = (char *) 0;

out = new char [10 + w];   //  just to be safe

justified_item(e[n], ColWidth[c], PadChar, Just[n], out);

s = out;

   //
   //  done
   //

if ( out )  { delete [] out;  out = (char *) 0; }

return ( s );

}


////////////////////////////////////////////////////////////////////////


int AsciiTable::table_width() const

{

if ( !e )  return ( 0 );

int j, W;

W = 0;

for (j=0; j<Ncols; ++j)  W += ColWidth[j];

for (j=0; j<(Ncols - 1); ++j)  W += InterColumnSpace[j];

return ( W );

}


////////////////////////////////////////////////////////////////////////


int AsciiTable::table_height() const

{

if ( !e )  return ( 0 );

int j, H;

H = Nrows;

for (j=0; j<(Nrows - 1); ++j)  H += InterRowSpace[j];

return ( H );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream & out, AsciiTable & t)

{

int j, r, c, n;
int rmax;
const int indent = t.table_indent();
const int W      = t.table_width();

rmax = t.nrows() - 1;

if ( t.delete_trailing_blank_rows() )  {

   while ( (rmax >= 0) && (t.row_is_blank(rmax)) )  --rmax;

}

for (r=0; r<=rmax; ++r)  {

   if ( r > 0 )  {

      n = t.inter_row_space(r - 1);

      for (j=0; j<n; ++j)  do_blank_line(out, t.fill_blank(), W + indent);

   }

   for (c=0; c<indent; ++c)  out.put(' ');

   out << t.padded_row(r) << '\n';

}   //  for r

   //
   //  done
   //

out.flush();

return ( out );

}


////////////////////////////////////////////////////////////////////////


void justified_item(const char * text, const int field_width, const AsciiTableJust just, char * out)

{

const char pad = ' ';


justified_item(text, field_width, pad, just, out);


return;

}


////////////////////////////////////////////////////////////////////////


void justified_item(const char * text, const int field_width, const char pad, const AsciiTableJust just, char * out)

{

   //
   //  sanity check some of the inputs  (although "text" can be null)
   //

if ( field_width <= 0 )  {

   cerr << "\n\n  justified_item() -> field_width must be > 0\n\n";

   exit ( 1 );

}


if ( !out )  {

   cerr << "\n\n  justified_item() -> null output string\n\n";

   exit ( 1 );

}

   //
   //  get to work
   //

int j, len, offset;

   //
   //  fill the output field with the pad character
   //

for (j=0; j<field_width; ++j)  out[j] = pad;

out[field_width] = (char) 0;   //  end-of-string marker

   //
   //  if there's no text, then we're done
   //

if ( !text )  return;

len = strlen(text);

if ( len == 0 )  return;

   //
   //  check that the field is wide enough to hold the text
   //

if ( len > field_width )  {

   cerr << "\n\n  justified_item() -> item too wide for field\n\n";

   exit ( 1 );

}

   //
   //  compute an offset into the field where we start writing the text
   //

switch ( just )  {

   case RightJust:   offset = field_width - len;       break;

   case LeftJust:    offset = 0;                       break;

   case CenterJust:  offset = (field_width - len)/2;   break;

   default:
      cerr << "\n\n  justified_item() -> bad justification value\n\n";
      exit ( 1 );
      break;

}   //  switch


   //
   //  write the text into the field
   //

for (j=0; j<len; ++j)     out[offset + j] = text[j];

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void do_blank_line(ostream & out, bool FillBlank, int width)

{

if ( !FillBlank )  { out.put('\n');  return; }

int j;

for (j=0; j<width; ++j)  out.put(' ');

out.put('\n');

return;

}


////////////////////////////////////////////////////////////////////////


bool all_blanks(const char * text)

{

while ( text )  {

   if ( *text++ != ' ' )  return ( false );

}

return ( true );

}


////////////////////////////////////////////////////////////////////////


void copy_ascii_table_row(const AsciiTable &at_from, const int r_from,
                          AsciiTable &at_to,         const int r_to)
{
int i;

for ( i=0; i<at_from.ncols() && i<at_to.ncols(); i++ )  {

   at_to.set_entry ( r_to, i, at_from(r_from, i) );

}

return;

}


////////////////////////////////////////////////////////////////////////


