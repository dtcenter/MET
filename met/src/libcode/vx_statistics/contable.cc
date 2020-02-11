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
#include <string.h>
#include <cstdio>
#include <cmath>

#include "contable.h"

#include "vx_util.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


static int table_rc_to_n(int r_table, int c_table, int w, int h);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ContingencyTable
   //


////////////////////////////////////////////////////////////////////////


ContingencyTable::ContingencyTable()
{
    
init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ContingencyTable::~ContingencyTable()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ContingencyTable::ContingencyTable(const ContingencyTable & t)
{

init_from_scratch();

assign(t);

}


////////////////////////////////////////////////////////////////////////


ContingencyTable & ContingencyTable::operator=(const ContingencyTable & t)

{

if ( this == &t )  return ( * this );

assign(t);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


ContingencyTable & ContingencyTable::operator+=(const ContingencyTable & t)

{

if ( Nrows != t.Nrows || Ncols != t.Ncols )  {

   mlog << Error << "\nContingencyTable::operator+=() -> "
        << "table dimensions do not match: (" << Nrows << ", " << Ncols
        << ") != (" << t.Nrows << ", " << t.Ncols << ")\n\n";

   exit ( 1 );

}

if ( E )  {
   for ( int i=0; i<E->size(); ++i )  (*E)[i] += (*t.E)[i];
}

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void ContingencyTable::init_from_scratch()
{
    E = new vector<int>();
    Name.clear();
    Nrows = Ncols = 0;
}


////////////////////////////////////////////////////////////////////////


void ContingencyTable::clear()
{
    if (E) delete E;
    E = new vector<int>();
    
    Name.clear();
    Nrows = Ncols = 0;
    
    return;
    
}


////////////////////////////////////////////////////////////////////////


void ContingencyTable::assign(const ContingencyTable & t)
{
    
    clear();
 
    if(t.E->size() == 0)  return;
 
    ContingencyTable::set_size(t.Nrows, t.Ncols);
    
    if (E) delete E;
    E = new vector<int>(*(t.E));
    Name = t.Name;
    
    //
    //  done
    //
    
    return;
    
}


////////////////////////////////////////////////////////////////////////


void ContingencyTable::zero_out()
{
    
    int n = Nrows*Ncols;
    
    if ( n == 0 )  return;
    
    E->assign(n, 0);
    
    return;
    
}

////////////////////////////////////////////////////////////////////////


void ContingencyTable::dump(ostream & out, int depth) const

{

int r, c;
Indent prefix(depth);
ConcatString junk;

out << prefix << "Name  = ";

if ( Name.length() > 0 )  out << '\"' << Name << "\"\n";
else                      out << "(nul)\n";

out << prefix << "Nrows = " << Nrows << "\n";
out << prefix << "Ncols = " << Ncols << "\n";

out << prefix << "\n";

if ( E->empty() )  { out.flush();  return; }

for (r=0; r<Nrows; ++r)  {

   junk.format("Sum for row %2d is %12d", r, row_total(r));

   out << prefix << junk << "\n";

   if ( ((r%5) == 4) && (r != (Nrows - 1)) )  out.put('\n');

}

out << prefix << "\n";

for (c=0; c<Ncols; ++c)  {

   junk.format("Sum for col %2d is %12d", c, col_total(c));

   out << prefix << junk << "\n";

   if ( ((c%5) == 4) && (c != (Ncols - 1)) )  out.put('\n');

}

out << prefix << "\n";

out << prefix << "Table Total = " << total() << "\n";

out << prefix << "\n";

   //////////////

int n, m, k;
int w, h;
int r_table, c_table;
int * col_width = (int *) 0;
char * table = (char *) 0;
const int hpad = 2;
const int vpad = 1;
const char v_sep      = '|';
const char h_sep      = '-';
const char corner_sep = '+';

col_width = new int[Ncols];

for (c=0; c<Ncols; ++c)  {

   comma_string(c, junk);

   col_width[c] = junk.length();

   junk.format("%d", (int) col_total(c));

   k = junk.length();

   if ( k > col_width[c] )  col_width[c] = k;

   for (r=0; r<Nrows; ++r)  {

      n = rc_to_n(r, c);

      comma_string((*E)[n], junk);

      k = junk.length();

      if ( k > col_width[c] )  col_width[c] = k;

   }

}

w = 2*hpad*Ncols + Ncols + 1;

for (c=0; c<Ncols; ++c)  w += col_width[c];

h = (2*vpad + 2)*Nrows + 1;

table = new char [w*h];

if ( !table )  {

   mlog << Error << "\nContingencyTable::dump() -> "
        << "memory allocation error\n\n";

   exit ( 1 );

}

memset(table, ' ', w*h);

   //
   //  top, bottom
   //

for (c_table=0; c_table<w; ++c_table)  {

   n = table_rc_to_n(0, c_table, w, h);


   table[n] = '=';

   n = table_rc_to_n(h - 1, c_table, w, h);


   table[n] = '=';

}

   //
   //  left, right
   //

for (r_table=1; r_table<(h - 1); ++r_table)  {

   n = table_rc_to_n(r_table, 0, w, h);

   table[n] = v_sep;

   n = table_rc_to_n(r_table, w - 1, w, h);

   table[n] = v_sep;

}

   //
   //  col separators
   //

for (c=1; c<Ncols; ++c)  {

   c_table = 0;

   for (k=0; k<c; ++k)  c_table += 2*hpad + col_width[k] + 1;

   for (r_table=1; r_table<(h - 1); ++r_table)  {

      n = table_rc_to_n(r_table, c_table, w, h);

      table[n] = v_sep;


   }

}

   //
   //  row separators
   //

for (r=1; r<Nrows; ++r)  {

   r_table = (2*vpad + 2)*r;

   for (c_table=1; c_table<(w - 1); ++c_table)  {

      n = table_rc_to_n(r_table, c_table, w, h);

      if ( table[n] == v_sep )  table[n] = corner_sep;
      else                      table[n] = h_sep;

   }

}

   //
   //  entries
   //

for (r=0; r<Nrows; ++r)  {

   r_table = 2 + 4*r;

   for (c=0; c<Ncols; ++c)  {

      c_table = 0;

      for (k=0; k<=c; ++k)  c_table += 2*hpad + col_width[k] + 1;

      n = rc_to_n(r, c);

      junk << cs_erase << (*E)[n];

      k = junk.length();

      c_table -= k + hpad;

      c_table -= (col_width[c] - k)/2;   //  center justified

      for (m=0; m<k; ++m)  {

         n = table_rc_to_n(r_table, c_table + m, w, h);

         table[n] = junk[m];

      }

   }   //  for c

}   //  for r

   //
   //  write table
   //

for (r_table=0; r_table<h; ++r_table)  {

   out << prefix;

   for (c_table=0; c_table<w; ++c_table)  {

      n = table_rc_to_n(r_table, c_table, w, h);

      out.put(table[n]);

   }

   out << "\n";

}

   //
   //  done
   //

out.flush();

if ( table )  { delete [] table;  table = (char *) 0; }

if ( col_width )  { delete [] col_width;  col_width = (int *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////


void ContingencyTable::set_size(int N)

{

ContingencyTable::set_size(N, N);

return;

}


////////////////////////////////////////////////////////////////////////


void ContingencyTable::set_size(int NR, int NC)

{

clear();

if ( (NR < 2) || (NC < 2) )  {

   mlog << Error << "\nContingencyTable::set_size() -> "
        << "# rows (" << NR << ") and # cols (" << NC
        << ") must be at least 2!\n\n";

   exit ( 1 );

}

int n;

n = NR*NC;

E->resize(n, 0);

Nrows = NR;
Ncols = NC;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ContingencyTable::set_name(const char * text)

{

Name = text;

return;

}


////////////////////////////////////////////////////////////////////////


int ContingencyTable::rc_to_n(int r, int c) const

{

if ( (r < 0) || (r >= Nrows) || (c < 0) || (c >= Ncols) )  {

   mlog << Error << "\nContingencyTable::rc_to_n() -> "
        << "range check error!\n\n";

   exit ( 1 );

}

int n;


n = r*Ncols + c;



return ( n );

}


////////////////////////////////////////////////////////////////////////


void ContingencyTable::set_entry(int row, int col, int value)

{

int n;

n = rc_to_n(row, col);

(*E)[n] = value;


return;

}


////////////////////////////////////////////////////////////////////////


void ContingencyTable::inc_entry(int row, int col)

{

int n;

n = rc_to_n(row, col);

++((*E)[n]);



return;

}


////////////////////////////////////////////////////////////////////////


int ContingencyTable::total() const

{

const int n = Nrows*Ncols;

if ( n == 0 )  return ( 0 );

int j, sum;

sum = 0;

for (j=0; j<n; ++j)  sum += (*E)[j];



return ( sum );

}


////////////////////////////////////////////////////////////////////////


int ContingencyTable::row_total(int row) const

{

if ( (row < 0) || (row >= Nrows) )  {

   mlog << Error << "\nContingencyTable::row_total() -> "
        << "range check error!\n\n";

   exit ( 1 );

}

int n, col, sum;


sum = 0;

for (col=0; col<Ncols; ++col)  {

   n = rc_to_n(row, col);

   sum += (*E)[n];

}




return ( sum );

}


////////////////////////////////////////////////////////////////////////


int ContingencyTable::col_total(int col) const

{

if ( (col < 0) || (col >= Ncols) )  {

   mlog << Error << "\nContingencyTable::col_total() -> "
        << "range check error!\n\n";

   exit ( 1 );

}

int n, row, sum;


sum = 0;

for (row=0; row<Nrows; ++row)  {

   n = rc_to_n(row, col);

   sum += (*E)[n];

}




return ( sum );

}


////////////////////////////////////////////////////////////////////////


int ContingencyTable::entry(int row, int col) const

{

int n;

n = rc_to_n(row, col);


return ( (*E)[n] );

}


////////////////////////////////////////////////////////////////////////


int ContingencyTable::largest_entry() const

{

int n = Nrows*Ncols;

if ( n == 0 )  return ( 0 );

int j, a;

a = (*E)[0];

for (j=1; j<n; ++j)  {

   if ( (*E)[n] > a )  a = (*E)[n];

}


return ( a );

}


////////////////////////////////////////////////////////////////////////


int ContingencyTable::smallest_entry() const

{

int n = Nrows*Ncols;

if ( n == 0 )  return ( 0 );

int j, a;

a = (*E)[0];

for (j=1; j<n; ++j)  {

   if ( (*E)[n] < a )  a = (*E)[n];

}


return ( a );

}


////////////////////////////////////////////////////////////////////////


   //
   //  see figure 7.2, page 243 in wilks
   //


TTContingencyTable ContingencyTable::condition_on(int k) const

{

if ( Nrows != Ncols )  {

   mlog << Error << "\nContingencyTable::condition_on() -> "
        << "table not square!\n\n";

   exit ( 1 );

}

if ( (k < 0) || (k >= Nrows) )  {

   mlog << Error << "\nContingencyTable::condition_on() -> "
        << "range check error\n\n";

   exit ( 1 );

}

int r, c;
int n, sum;
TTContingencyTable t;

   //
   //
   //

t.set_entry(0, 0, entry(k, k));

   //
   //
   //

sum = 0;

for (c=0; c<Ncols; ++c)  {

   if ( c == k )  continue;

   n = rc_to_n(k, c);

   sum += (*E)[n];

}

t.set_entry(0, 1, sum);

   //
   //
   //

sum = 0;

for (r=0; r<Nrows; ++r)  {

   if ( r == k )  continue;

   n = rc_to_n(r, k);

   sum += (*E)[n];

}

t.set_entry(1, 0, sum);

   //
   //
   //

sum = 0;

for (r=0; r<Nrows; ++r)  {

   if ( r == k )  continue;

   for (c=0; c<Ncols; ++c)  {

      if ( c == k )  continue;

      n = rc_to_n(r, c);

      sum += (*E)[n];

   }

}

t.set_entry(1, 1, sum);


   //
   //  done
   //

return ( t );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class TTContingencyTable
   //


////////////////////////////////////////////////////////////////////////


TTContingencyTable::TTContingencyTable()

{

ContingencyTable::set_size(2, 2);

}


////////////////////////////////////////////////////////////////////////


TTContingencyTable::~TTContingencyTable()

{


}


////////////////////////////////////////////////////////////////////////


TTContingencyTable::TTContingencyTable(const TTContingencyTable & t)

{

assign(t);

}



////////////////////////////////////////////////////////////////////////


TTContingencyTable & TTContingencyTable::operator=(const TTContingencyTable & t)

{

if ( this == &t )  return ( * this );

assign(t);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void TTContingencyTable::set_fn_on(int k)

{

set_entry(FN_row, ON_col, k);

return;

}


////////////////////////////////////////////////////////////////////////


void TTContingencyTable::set_fy_on(int k)

{

set_entry(FY_row, ON_col, k);

return;

}


////////////////////////////////////////////////////////////////////////


void TTContingencyTable::set_fn_oy(int k)

{

set_entry(FN_row, OY_col, k);

return;

}


////////////////////////////////////////////////////////////////////////


void TTContingencyTable::set_fy_oy(int k)

{

set_entry(FY_row, OY_col, k);

return;

}


////////////////////////////////////////////////////////////////////////


void TTContingencyTable::inc_fn_on()

{

inc_entry(FN_row, ON_col);

return;

}


////////////////////////////////////////////////////////////////////////


void TTContingencyTable::inc_fy_on()

{

inc_entry(FY_row, ON_col);

return;

}


////////////////////////////////////////////////////////////////////////


void TTContingencyTable::inc_fn_oy()

{

inc_entry(FN_row, OY_col);

return;

}


////////////////////////////////////////////////////////////////////////


void TTContingencyTable::inc_fy_oy()

{

inc_entry(FY_row, OY_col);

return;

}


////////////////////////////////////////////////////////////////////////


int TTContingencyTable::fy_oy() const

{

int k;

k = entry(FY_row, OY_col);

return ( k );

}


////////////////////////////////////////////////////////////////////////


int TTContingencyTable::fy_on() const

{

int k;

k = entry(FY_row, ON_col);

return ( k );

}


////////////////////////////////////////////////////////////////////////


int TTContingencyTable::fn_oy() const

{

int k;

k = entry(FN_row, OY_col);

return ( k );

}


////////////////////////////////////////////////////////////////////////


int TTContingencyTable::fn_on() const

{

int k;

k = entry(FN_row, ON_col);

return ( k );

}


////////////////////////////////////////////////////////////////////////


int TTContingencyTable::fy() const

{

int k;

k = row_total(FY_row);

return ( k );

}


////////////////////////////////////////////////////////////////////////


int TTContingencyTable::fn() const

{

int k;

k = row_total(FN_row);

return ( k );

}


////////////////////////////////////////////////////////////////////////


int TTContingencyTable::oy() const

{

int k;

k = col_total(OY_col);

return ( k );

}


////////////////////////////////////////////////////////////////////////


int TTContingencyTable::on() const

{

int k;

k = col_total(ON_col);

return ( k );

}


////////////////////////////////////////////////////////////////////////


int TTContingencyTable::n() const

{

int k;

k = total();

return ( k );

}


////////////////////////////////////////////////////////////////////////


double TTContingencyTable::f_rate() const

{

int N, D;
double num, denom;

N = fy();
D = n();

if ( D == 0 )  return ( bad_data_double );

num   = (double) N;
denom = (double) D;

return ( num/denom );

}


////////////////////////////////////////////////////////////////////////


double TTContingencyTable::h_rate() const

{

int N, D;
double num, denom;

N = fy_oy();
D = n();

if ( D == 0 )  return ( bad_data_double );

num   = (double) N;
denom = (double) D;

return ( num/denom );

}


////////////////////////////////////////////////////////////////////////


double TTContingencyTable::o_rate() const

{

int N, D;
double num, denom;

N = oy();
D = n();

if ( D == 0 )  return ( bad_data_double );

num   = (double) N;
denom = (double) D;

return ( num/denom );

}


////////////////////////////////////////////////////////////////////////


double TTContingencyTable::fy_oy_tp() const

{

int N, D;
double num, denom;

N = fy_oy();
D = n();

if ( D == 0 )  return ( bad_data_double );

num   = (double) N;
denom = (double) D;

return ( num/denom );

}


////////////////////////////////////////////////////////////////////////


double TTContingencyTable::fy_on_tp() const

{

int N, D;
double num, denom;

N = fy_on();
D = n();

if ( D == 0 )  return ( bad_data_double );

num   = (double) N;
denom = (double) D;

return ( num/denom );

}


////////////////////////////////////////////////////////////////////////


double TTContingencyTable::fn_oy_tp() const

{

int N, D;
double num, denom;

N = fn_oy();
D = n();

if ( D == 0 )  return ( bad_data_double );

num   = (double) N;
denom = (double) D;

return ( num/denom );

}


////////////////////////////////////////////////////////////////////////


double TTContingencyTable::fn_on_tp() const

{

int N, D;
double num, denom;

N = fn_on();
D = n();

if ( D == 0 )  return ( bad_data_double );

num   = (double) N;
denom = (double) D;

return ( num/denom );

}


////////////////////////////////////////////////////////////////////////


double TTContingencyTable::fy_tp() const

{

int N, D;
double num, denom;

N = fy();
D = n();

if ( D == 0 )  return ( bad_data_double );

num   = (double) N;
denom = (double) D;

return ( num/denom );

}


////////////////////////////////////////////////////////////////////////


double TTContingencyTable::fn_tp() const

{

int N, D;
double num, denom;

N = fn();
D = n();

if ( D == 0 )  return ( bad_data_double );

num   = (double) N;
denom = (double) D;

return ( num/denom );

}


////////////////////////////////////////////////////////////////////////


double TTContingencyTable::oy_tp() const

{

int N, D;
double num, denom;

N = oy();
D = n();

if ( D == 0 )  return ( bad_data_double );

num   = (double) N;
denom = (double) D;

return ( num/denom );

}


////////////////////////////////////////////////////////////////////////


double TTContingencyTable::on_tp() const

{

int N, D;
double num, denom;

N = on();
D = n();

if ( D == 0 )  return ( bad_data_double );

num   = (double) N;
denom = (double) D;

return ( num/denom );

}


////////////////////////////////////////////////////////////////////////


double TTContingencyTable::fy_oy_fp() const

{

int N, D;
double num, denom;

N = fy_oy();
D = fy();

if ( D == 0 )  return ( bad_data_double );

num   = (double) N;
denom = (double) D;

return ( num/denom );

}


////////////////////////////////////////////////////////////////////////


double TTContingencyTable::fy_on_fp() const

{

int N, D;
double num, denom;

N = fy_on();
D = fy();

if ( D == 0 )  return ( bad_data_double );

num   = (double) N;
denom = (double) D;

return ( num/denom );

}


////////////////////////////////////////////////////////////////////////


double TTContingencyTable::fn_oy_fp() const

{

int N, D;
double num, denom;

N = fn_oy();
D = fn();

if ( D == 0 )  return ( bad_data_double );

num   = (double) N;
denom = (double) D;

return ( num/denom );

}


////////////////////////////////////////////////////////////////////////


double TTContingencyTable::fn_on_fp() const

{

int N, D;
double num, denom;

N = fn_on();
D = fn();

if ( D == 0 )  return ( bad_data_double );

num   = (double) N;
denom = (double) D;

return ( num/denom );

}


////////////////////////////////////////////////////////////////////////


double TTContingencyTable::fy_oy_op() const

{

int N, D;
double num, denom;

N = fy_oy();
D = oy();

if ( D == 0 )  return ( bad_data_double );

num   = (double) N;
denom = (double) D;

return ( num/denom );

}


////////////////////////////////////////////////////////////////////////


double TTContingencyTable::fy_on_op() const

{

int N, D;
double num, denom;

N = fy_on();
D = on();

if ( D == 0 )  return ( bad_data_double );

num   = (double) N;
denom = (double) D;

return ( num/denom );

}


////////////////////////////////////////////////////////////////////////


double TTContingencyTable::fn_oy_op() const

{

int N, D;
double num, denom;

N = fn_oy();
D = oy();

if ( D == 0 )  return ( bad_data_double );

num   = (double) N;
denom = (double) D;

return ( num/denom );
}


////////////////////////////////////////////////////////////////////////


double TTContingencyTable::fn_on_op() const

{

int N, D;
double num, denom;

N = fn_on();
D = on();

if ( D == 0 )  return ( bad_data_double );

num   = (double) N;
denom = (double) D;

return ( num/denom );

}


////////////////////////////////////////////////////////////////////////


void TTContingencyTable::set_size(int N)

{

mlog << Error << "\nTTContingencyTable::set_size(int) -> "
     << "2 x 2 tables cannot be resized!\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void TTContingencyTable::set_size(int NR, int NC)

{

mlog << Error << "\nTTContingencyTable::set_size(int, int) -> "
     << "2 x 2 tables cannot be resized!\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////

   //
   //  see table 7.1a, page 242 in wilks
   //

TTContingencyTable finley()

{

TTContingencyTable t;


t.set_fy_oy(28);
t.set_fn_oy(23);

t.set_fy_on(72);
t.set_fn_on(2680);


t.set_name("Finley Tornado Forecasts (1884)");


return ( t );

}


////////////////////////////////////////////////////////////////////////

   //
   //  see table 7.1b, page 242 in wilks
   //

TTContingencyTable finley_always_no()

{

TTContingencyTable t;


t.set_fy_oy(0);
t.set_fn_oy(51);

t.set_fy_on(0);
t.set_fn_on(2752);


t.set_name("Finley Tornado Forecasts (Always No) (1884)");


return ( t );

}


////////////////////////////////////////////////////////////////////////


int table_rc_to_n(int r_table, int c_table, int w, int h)

{

int n;

n = r_table*w + c_table;

return ( n );

}


////////////////////////////////////////////////////////////////////////
