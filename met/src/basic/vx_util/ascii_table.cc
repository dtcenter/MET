// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <cstdio>
#include <cmath>
#include <iostream>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "vx_log.h"
#include "vx_cal.h"

#include "ascii_table.h"
#include "comma_string.h"
#include "fix_float.h"
#include "util_constants.h"

////////////////////////////////////////////////////////////////////////


static const char radix_marker = '.';   //  some counties use ','
static const char exp_char     = 'e';


////////////////////////////////////////////////////////////////////////


static void do_blank_line(ostream & out, bool FillBlank, int width);

static bool all_blanks(const std::string);

static void n_figures(const std::string text, int & left, int & right);


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

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::clear()

{

char tmp_str[512];

if ( !e.empty() )  {

   e.clear();

}

if ( !BadDataStr.empty() )  {

  BadDataStr.clear();

}

ColWidth.clear();

Just.clear();

InterColumnSpace.clear();

InterRowSpace.clear();

TableIndent = default_table_indent;

FillBlank = default_fill_blank;

ColSepChar = default_col_sep_char;

PadChar = default_table_pad_char;

Nrows = Ncols = 0;

set_precision(ascii_table_default_precision);

set_bad_data_value(ascii_table_default_bad_data_value);

snprintf(tmp_str, sizeof(tmp_str), "%.0f", ascii_table_default_bad_data_value);

set_bad_data_str(tmp_str);

DoCommaString = false;

DeleteTrailingBlankRows = false;

ElimTrailingWhitespace = true;

DecimalPointsAligned = false;

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::erase()

{

const int NRC = Nrows*Ncols;

e.clear();
e.resize(NRC);

return;

}


////////////////////////////////////////////////////////////////////////


int AsciiTable::rc_to_n(int r, int c) const

{

// if ( !e )  {

//    mlog << Error << "\nAsciiTable::rc_to_n() -> empty table!\n\n";

//    exit ( 1 );

// }

if ( (r < 0) || (r >= Nrows) || (c < 0) || (c >= Ncols) )  {

   mlog << Error << "\n"
        << "  AsciiTable::rc_to_n() -> range check error ... \n"
        << "                           (Nrows, Ncols) = (" << Nrows << ", " << Ncols << ")\n"
        << "                           (r, c) = (" << r << ", " << c << ")\n\n";

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

if ( a.e.empty() )  return;

set_size(a.nrows(), a.ncols());


int r, c;


ColWidth = a.ColWidth;

InterColumnSpace = a.InterColumnSpace;
InterRowSpace = a.InterRowSpace;

Just = a.Just;

TableIndent  = a.TableIndent;

FillBlank    = a.FillBlank;

ColSepChar   = a.ColSepChar;

PadChar      = a.PadChar;

Precision    = a.Precision;

BadDataValue = a.BadDataValue;

set_bad_data_str(a.BadDataStr);

memcpy(f_FloatFormat, a.f_FloatFormat, sizeof(f_FloatFormat));
memcpy(g_FloatFormat, a.g_FloatFormat, sizeof(g_FloatFormat));

DoCommaString = a.DoCommaString;

DeleteTrailingBlankRows = a.DeleteTrailingBlankRows;

ElimTrailingWhitespace = a.ElimTrailingWhitespace;

DecimalPointsAligned = a.DecimalPointsAligned;


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

   mlog << Error << "\nAsciiTable::set_size() -> bad size\n\n";

   exit ( 1 );

}

clear();

const int NRC = NR*NC;

e.resize(NRC);

if ( e.size() != NRC )  {

   mlog << Error << "\nAsciiTable::set_size_rc() -> memory allocation error 1\n\n";

   exit ( 1 );

}

//for (j=0; j<NRC; ++j)  e[j] = "";

Nrows = NR;
Ncols = NC;

ColWidth.resize(Ncols, 0);

if ( !ColWidth.size() )  {

   mlog << Error << "\nAsciiTable::set_size_rc() -> memory allocation error 2\n\n";

   exit ( 1 );

}

//for (j=0; j<Ncols; ++j)  ColWidth[j] = 0;

InterColumnSpace.resize(Ncols - 1, default_ics);
InterRowSpace.resize(Nrows - 1, default_irs);

Just.resize(Nrows*Ncols, default_justification);

if ( !Just.size() )  {

   mlog << Error << "\nAsciiTable::set_size_rc() -> memory allocation error 3\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::add_rows(const int NR)

{

   //
   //  sanity check the input value
   //

if ( NR <= 0 )  {

   mlog << Error
        << "AsciiTable::add_rows(const int) -> bad number of rows ... " << NR << "\n";

   exit ( 1 );

}

   //
   //  allocate some new memory
   //

int jr, jc;
const int n_rows_new = Nrows + NR;
const int n_rows_old = Nrows;
const int nrc_new = n_rows_new*Ncols;

Nrows = n_rows_new;

InterRowSpace.reserve(n_rows_new);
InterRowSpace.resize(n_rows_new, default_irs);

Just.reserve(nrc_new);
Just.resize(nrc_new, default_justification);

e.reserve(nrc_new);
e.resize(nrc_new);

   //
   //  copy the justification information from the old last
   //
   //    row into all of the new rows
   //

for (jr=0; jr<NR; ++jr)  {

   for (jc=0; jc<Ncols; ++jc)  {

     Just[rc_to_n(jr + n_rows_old, jc)] = Just[rc_to_n(n_rows_old - 1, jc)];

   }

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_ics(int value)

{

if ( (value < 0) || (value > max_ics) )  {

   mlog << Error << "\nAsciiTable::set_ics(int) -> bad value ... " << value << "\n\n";

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

   mlog << Error << "\nAsciiTable::set_ics(int, int) -> bad value ... " << value << "\n\n";

   exit ( 1 );

}

if ( (col_left < 0) || (col_left >= (Ncols - 1)) )  {

   mlog << Error << "\nAsciiTable::set_ics(int, int) -> bad column ... " << col_left << "\n\n";

   exit ( 1 );

}


InterColumnSpace[col_left] = value;

return;

}


////////////////////////////////////////////////////////////////////////


int AsciiTable::inter_column_space(int col_left) const

{

if ( (col_left < 0) || (col_left >= (Ncols - 1)) )  {

   mlog << Error << "\nAsciiTable::inter_column_space() -> bad column ... " << col_left << "\n\n";

   exit ( 1 );

}

if ( InterColumnSpace.empty() )  return ( 0 );

return ( InterColumnSpace[col_left] );

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_irs(int value)

{

if ( (value < 0) || (value > max_irs) )  {

   mlog << Error << "\nAsciiTable::set_irs(int) -> bad value ... " << value << "\n\n";

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

   mlog << Error << "\nAsciiTable::set_irs(int, int) -> bad value ... " << value << "\n\n";

   exit ( 1 );

}

if ( (row_top < 0) || (row_top >= (Nrows - 1)) )  {

   mlog << Error << "\nAsciiTable::set_irs(int, int) -> bad row ... " << row_top << "\n\n";

   exit ( 1 );

}

InterRowSpace[row_top] = value;

return;

}


////////////////////////////////////////////////////////////////////////


int AsciiTable::inter_row_space(int row_top) const

{

if ( (row_top < 0) || (row_top >= (Nrows - 1)) )  {

   mlog << Error << "\nAsciiTable::inter_row_space(int) -> bad row ... " << row_top << "\n\n";

   exit ( 1 );

}

if ( InterRowSpace.empty() )  return ( 0 );

return ( InterRowSpace[row_top] );

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_table_indent(int value)

{

if ( (value < 0) || (value > max_table_indent) )  {

   mlog << Error << "\nAsciiTable::set_table_indent(int) -> bad value\n\n";

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

   mlog << Error << "\nAsciiTable::set_precision(int) -> bad value\n\n";

   exit ( 1 );

}

Precision = k;

memset(f_FloatFormat, 0, sizeof(f_FloatFormat));
memset(g_FloatFormat, 0, sizeof(g_FloatFormat));

snprintf(f_FloatFormat, sizeof(f_FloatFormat), "%%.%df", Precision);
snprintf(g_FloatFormat, sizeof(g_FloatFormat), "%%.%dg", Precision);

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_bad_data_value(double d)

{

BadDataValue = d;

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_bad_data_str(const std::string str)

{

BadDataStr = str;

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

   mlog << Error << "\nAsciiTable::set_col_just() -> range check error!\n\n";

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

   mlog << Error << "\nAsciiTable::set_row_just() -> range check error!\n\n";

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


void AsciiTable::set_entry(const int r, const int c, const ConcatString &text)

{


int n, k;
char junk[256];

n = rc_to_n(r, c);   //  "rc_to_n" does range checking on r and c,
                     //    so we don't need to do that here

   //
   //  delete old entry, if any
   //

if ( e[n].size() )  { e[n].clear(); }

if ( text.empty() )  return;

   //
   //  check for bad data value
   //

snprintf(junk, sizeof(junk), "%.0f", BadDataValue);
if ( text == junk ) {
   e[n] = BadDataStr;
} else {
   e[n] = text;
}

k = text.length();
if ( ColWidth[c] < k )  ColWidth[c] = k;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_entry(const int r, const int c, const char* text)

{
  set_entry(r, c, (string)text);
}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_entry(const int r, const int c, int a)

{
ConcatString junk;

if ( fabs(a - BadDataValue) < 0.0001 )  {
   set_entry(r, c, BadDataStr);
   return;

} else if ( DoCommaString )  {
   ::comma_string(a, junk);
}  else  {
  junk.format("%d", a);
}

set_entry(r, c, junk);

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_entry(const int r, const int c, double x)

{

ConcatString str;

if ( fabs(x - BadDataValue) < 0.0001 )  str = BadDataStr;
else  {

  if ( fabs(x) >= 1.0 )   str.format(f_FloatFormat, x);
  else                    str.format(g_FloatFormat, x);

}

fix_float(str);

if ( DoCommaString )  {
   char junk[256];
   strncpy(junk, str.c_str(), str.length());
   char * p = (char *) 0;
   long X;
   ConcatString s;
   ConcatString j2;

   p = strchr(junk, '.');

   if ( p )  *p = (char) 0;

   ++p;

   X = atol(junk);

   ::comma_string(X, j2);

   if ( (X == 0) && (x < 0.0) )  s << '-';

   s << j2;

   if ( Precision > 0 )  s << '.' << p;

   set_entry(r, c, s.string());

} else set_entry(r, c, str);

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_entry(const int r, const int c, char a)

{

ConcatString junk;

junk = a;

set_entry(r, c, junk);

return;

}


////////////////////////////////////////////////////////////////////////


const ConcatString AsciiTable::operator()(int r, int c) const

{

int n;

n = rc_to_n(r, c);   //  "rc_to_n" does range checking on r and c,
                     //    so we don't need to do that here

return ( e[n] );     //  might be null

}


////////////////////////////////////////////////////////////////////////


int AsciiTable::col_width(int k) const

{

if ( e.empty() )  return ( 0 );

if ( (k < 0) || (k >= Ncols) )  {

   mlog << Error << "\nAsciiTable::col_width(int) const -> range check error\n\n";

   exit ( 1 );

}

return ( ColWidth[k] );

}


////////////////////////////////////////////////////////////////////////


int AsciiTable::max_col_width() const

{

if ( e.empty() )  return ( 0 );

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

   mlog << Error << "\nAsciiTable::row_is_blank(int) const -> range check error\n\n";

   exit ( 1 );

}

int c, n;

for (c=0; c<Ncols; ++c)  {

   n = rc_to_n(r, c);

   if ( !e[n].empty() )  {

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

   return ( ConcatString() );

}

   //
   //  so now we know the column width is > 0
   //

if ( e[n].empty() )  {

   s.set_repeat(' ', w);

   return ( s );

}

char * out = new char [10 + w];   //  just to be safe

justified_item(e[n].c_str(), ColWidth[c], PadChar, Just[n], out);

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

if ( e.empty() )  return ( 0 );

int j, W;

W = 0;

for (j=0; j<Ncols; ++j)  W += ColWidth[j];

for (j=0; j<(Ncols - 1); ++j)  W += InterColumnSpace[j];

return ( W );

}


////////////////////////////////////////////////////////////////////////


int AsciiTable::table_height() const

{

if ( e.empty() )  return ( 0 );

int j, H;

H = Nrows;

for (j=0; j<(Nrows - 1); ++j)  H += InterRowSpace[j];

return ( H );

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::pad_entry_right(const int r, const int c, const int n, const char fill_char)

{

if ( n < 0 )  {

   mlog << Error
        << "bad pad value ... " << n << "\n\n";

   exit ( 1 );

}

if ( n == 0 )  return;

int j, k;

k = rc_to_n(r, c);   //  "rc_to_n" does range checking on r and c,
                     //    so we don't need to do that here

if ( e[k].empty() )  return;

ConcatString s = e[k];

for (j=0; j<n; ++j)  s << fill_char;

set_entry(r, c, s);

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::line_up_decimal_points()

{

if ( Nrows < 0 || Nrows >= INT_MAX )  {

   mlog << Error << "\nAsciiTable::line_up_decimal_points() -> "
        << "invalid number of rows ... " << Nrows << "\n\n";

   exit ( 1 );
}

int left[Nrows];
int right[Nrows];

int r, c, n, k;
int max_left, max_right;
const char fill_char = ' ';
const int r_start = 1;   //  skip the header row

for (c=0; c<Ncols; ++c)  {

      //  get the pad size for that column

   for (r=r_start; r<Nrows; ++r)  {

      n = rc_to_n(r, c);

      n_figures(e[n], left[r], right[r]);

   }

   max_left  = left  [r_start];
   max_right = right [r_start];

   for (r=r_start+1; r<Nrows; ++r)  {

      if ( left  [r] > max_left  )  max_left  =  left[r];
      if ( right [r] > max_right )  max_right = right[r];

   }

      //
      //  pad each entry in that column
      //

   for (r=r_start; r<Nrows; ++r)  {

      k = max_right - right[r];

      if ( k > 0 )  pad_entry_right(r, c, k, fill_char);

   }

}   //  for c


   //
   //  done
   //


DecimalPointsAligned = true;

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::underline_row(const int row, const char underline_char)

{

int j;
ConcatString s;

for (j=0; j<Ncols; ++j)  {

   s.set_repeat(underline_char, ColWidth[j]);

   set_entry(row, j, s);

}   //  for j



return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream & out, AsciiTable & t)

{

if ( !t.decimal_points_aligned() ) t.line_up_decimal_points();

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

   mlog << Error << "\njustified_item() -> field_width must be > 0\n\n";

   exit ( 1 );

}


if ( !out )  {

   mlog << Error << "\njustified_item() -> null output string\n\n";

   exit ( 1 );

}

   //
   //  get to work
   //

int j, len;
int offset = 0;

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

   mlog << Error << "\njustified_item() -> item too wide for field\n\n";

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
      mlog << Error << "\njustified_item() -> bad justification value\n\n";
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


bool all_blanks(std::string text)

{

if (text.find_first_not_of(' ') != std::string::npos) return false;

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


void n_figures(const std::string text, int & left, int & right)

{

int j;
char c;
int N = text.length();

   //
   //  I wasn't able to find a string library function
   //     that did quite what I wanted here
   //

for (j=0; j<N; ++j)  {

   c = text[j];

   if ( (c == radix_marker) || (c == exp_char) )  break;

}

left  = j + 1;
right = N - left;

return;

}


////////////////////////////////////////////////////////////////////////


void justify_met_at(AsciiTable &at, const int n_hdr_cols) {
   int i;

   // Check for minimum number of columns
   if(at.ncols() < n_hdr_cols) {
      mlog << Error << "\njustify_met_at() -> "
           << "AsciiTable object has fewer columns ("
           << at.ncols() << ") than the number of header columns ("
           << n_hdr_cols << ").\n\n";
      exit(1);
   }

   // Left-justify header columns and right-justify data columns
   for(i=0; i<at.ncols(); i++) {
      if(i < n_hdr_cols) at.set_column_just(i, LeftJust);
      else               at.set_column_just(i, RightJust);
   }

   // Left-justify the header row
   at.set_row_just(0, LeftJust);

   return;
}


////////////////////////////////////////////////////////////////////////


ConcatString check_hdr_str(const ConcatString s, bool space_to_underscore) {
   ConcatString s_tmp = s;

   if(space_to_underscore) s_tmp.replace(" ", "_", false);

   // Check for empty string
   if(s_tmp.length() == 0) {
      mlog << Warning << "\ncheck_hdr_str() -> "
           << "null string!\n\n";
      return(na_string);
   }

   // Check for embedded whitespace
   if(check_reg_exp(ws_reg_exp, s_tmp.c_str())) {
      mlog << Error << "\ncheck_hdr_str() -> "
           << "output header column value (\"" << s_tmp
           << "\") should contain no embedded whitespace!\n\n";
      exit(1);
   }

   return(s_tmp);
}


////////////////////////////////////////////////////////////////////////

