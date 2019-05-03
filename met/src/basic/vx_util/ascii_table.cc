// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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

#include "vx_log.h"

#include "ascii_table.h"
#include "comma_string.h"
#include "fix_float.h"


////////////////////////////////////////////////////////////////////////


static const char radix_marker = '.';   //  some counties use ','
static const char exp_char     = 'e';


////////////////////////////////////////////////////////////////////////


static void do_blank_line(ostream & out, bool FillBlank, int width);

static bool all_blanks(const char *);

static void n_figures(const char * text, int & left, int & right);


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

DecimalPointsAligned = false;

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::empty()

{

int j;

const int NRC = Nrows*Ncols;

for (j=0; j<NRC; ++j)  {
   if ( e[j] )  { delete [] e[j];  e[j] = (char *) 0; }
}

return;

}


////////////////////////////////////////////////////////////////////////


int AsciiTable::rc_to_n(int r, int c) const

{

if ( !e )  {

   mlog << Error << "\nAsciiTable::rc_to_n() -> empty table!\n\n";

   exit ( 1 );

}

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

int j;
const int NRC = NR*NC;

e = new char * [NRC];

if ( !e )  {

   mlog << Error << "\nAsciiTable::set_size_rc() -> memory allocation error 1\n\n";

   exit ( 1 );

}

for (j=0; j<NRC; ++j)  e[j] = (char *) 0;

Nrows = NR;
Ncols = NC;

ColWidth = new int [Ncols];

if ( !ColWidth )  {

   mlog << Error << "\nAsciiTable::set_size_rc() -> memory allocation error 2\n\n";

   exit ( 1 );

}

for (j=0; j<Ncols; ++j)  ColWidth[j] = 0;

InterColumnSpace = new int [Ncols - 1];

for (j=0; j<(Ncols - 1); ++j)  InterColumnSpace[j] = default_ics;

InterRowSpace = new int [Nrows - 1];

for (j=0; j<(Nrows - 1); ++j)  InterRowSpace[j] = default_irs;

Just = new AsciiTableJust [Nrows*Ncols];

if ( !Just )  {

   mlog << Error << "\nAsciiTable::set_size_rc() -> memory allocation error 3\n\n";

   exit ( 1 );

}

for (j=0; j<(Nrows*Ncols); ++j)  Just[j] = default_justification;

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
char ** c = (char **) 0;
AsciiTableJust * atj = (AsciiTableJust *) 0;
int * irs = (int *) 0;

c = new char * [nrc_new];

atj = new AsciiTableJust [nrc_new];

irs = new int [n_rows_new];

memset(c, 0, nrc_new*sizeof(char *));

memset(irs, 0, n_rows_new*sizeof(int));

   //
   //  copy the old values into the new memory
   //

memcpy(c, e, Nrows*Ncols*sizeof(char *));

memcpy(atj, Just, Nrows*Ncols*sizeof(AsciiTableJust));

if ( Nrows > 1 )  memcpy(irs, InterRowSpace, (Nrows-1)*sizeof(int));

   //
   //  deallocate the old memory
   //

delete [] e;  e = (char **) 0;

e = c;   c = (char **) 0;

delete [] Just;  Just = (AsciiTableJust *) 0;

Just = atj;   atj = (AsciiTableJust *) 0;

delete [] InterRowSpace;  InterRowSpace = (int *) 0;

InterRowSpace = irs;  irs = (int *) 0;

Nrows = n_rows_new;

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

if ( !InterColumnSpace )  return ( 0 );

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

if ( !InterRowSpace )  return ( 0 );

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

sprintf(f_FloatFormat, "%%.%df", Precision);
sprintf(g_FloatFormat, "%%.%dg", Precision);

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


void AsciiTable::set_entry(const int r, const int c, const char * text)

{


int n, k;
char junk[256];

n = rc_to_n(r, c);   //  "rc_to_n" does range checking on r and c,
                     //    so we don't need to do that here

   //
   //  delete old entry, if any
   //

if ( e[n] )  { delete [] e[n];  e[n] = (char *) 0; }

if ( !text )  return;

   //
   //  check for bad data value
   //

sprintf(junk, "%.0f", BadDataValue);
if ( strncmp(text, junk, strlen(junk)) == 0 ) text = BadDataStr;

k = strlen(text);

e[n] = new char [1 + k];

if ( !(e[n]) )  {

   mlog << Error << "\nAsciiTable::set_entry() -> memory allocation error\n\n";

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

if ( fabs(a - BadDataValue) < 0.0001 )  strcpy(junk, BadDataStr);
else if ( DoCommaString )              ::comma_string(a, junk);
else                                   sprintf(junk, "%d", a);

set_entry(r, c, junk);

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::set_entry(const int r, const int c, double x)

{

char junk[256];

if ( fabs(x - BadDataValue) < 0.0001 )  strcpy(junk, BadDataStr);
else  {

   if ( fabs(x) >= 1.0 )  sprintf(junk, f_FloatFormat, x);
   else                   sprintf(junk, g_FloatFormat, x);

}

fix_float(junk);

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

   mlog << Error << "\nAsciiTable::col_width(int) const -> range check error\n\n";

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

   mlog << Error << "\nAsciiTable::row_is_blank(int) const -> range check error\n\n";

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

if ( !(e[k]) )  return;

ConcatString s = e[k];

for (j=0; j<n; ++j)  s << fill_char;

set_entry(r, c, s.text());

return;

}


////////////////////////////////////////////////////////////////////////


void AsciiTable::line_up_decimal_points()

{

int r, c, n, k;
int max_left, max_right;
int w_old, w_new;
int * left = 0;
int * right = 0;
// const char fill_char = '*';
const char fill_char = ' ';
const int r_start = 1;   //  skip the header row

left  = new int [Nrows];
right = new int [Nrows];

for (c=0; c<Ncols; ++c)  {

   w_old = ColWidth[c];

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

   w_new = max_left + max_right;

   if ( w_new < w_old )  w_new = w_old;

/*
   cout << "\n"
        << "col       = " << c         << '\n'
        << "max_left  = " << max_left  << '\n'
        << "max_right = " << max_right << '\n'
        << "w_old     = " << w_old     << "\n"
        << "w_new     = " << w_new     << "\n\n";
*/

      //
      //  pad each entry in that column
      //

   for (r=r_start; r<Nrows; ++r)  {

      n = rc_to_n(r, c);

      // len = (e[n] == (char *) 0 ? 0 : strlen(e[n]));

      k = max_right - right[r];

      // k = w_new - len - 1;

      if ( k > 0 )  pad_entry_right(r, c, k, fill_char);

   }

}   //  for c


   //
   //  done
   //

if ( left  )  { delete [] left;   left  = 0; }
if ( right )  { delete [] right;  right = 0; }

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

   set_entry(row, j, s.text());

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


void n_figures(const char * text, int & left, int & right)

{

int j;
char c;
int N = (text == (char *) 0 ? 0 : strlen(text));

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

// cout << "text = \"" << text  << "\"\n";
// cout << "L    = "   << left  << "\n";
// cout << "R    = "   << right << "\n\n";

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

