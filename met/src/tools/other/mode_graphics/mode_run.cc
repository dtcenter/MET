

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_util.h"
#include "vx_math.h"
#include "mode_run.h"


////////////////////////////////////////////////////////////////////////


static int simple_single_object_number(const char * text);

static int composite_single_object_number(const char * text);

static FO_Pair pair_object_number(const char * text);

static bool calc_median(const double * sorted_values, const int n, double & out);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ModeRun
   //


////////////////////////////////////////////////////////////////////////


ModeRun::ModeRun()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ModeRun::~ModeRun()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ModeRun::ModeRun(const ModeRun & m)

{

init_from_scratch();

assign(m);

}


////////////////////////////////////////////////////////////////////////


ModeRun & ModeRun::operator=(const ModeRun & m)

{

if ( this == &m )  return ( * this );

assign(m);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void ModeRun::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void ModeRun::clear()

{

a.clear();

FcstValid = ObsValid = (unixtime) 0;

FcstLead = ObsLead = FcstAccum = ObsAccum = 0;

Model.erase();

Filename.erase();

LineFirst = LineLast = 0;

FcstRadius = ObsRadius = 0;

FcstThresh.erase();
 ObsThresh.erase();

FcstVariable.erase();
 ObsVariable.erase();

FcstLevel.erase();
 ObsLevel.erase();

N_FcstSimpleObjs = 0;
N_ObsSimpleObjs  = 0;

N_FcstCompositeObjs = 0;
N_ObsCompositeObjs  = 0;

N_SimplePairs = 0;
N_CompositePairs = 0;


fcst_simple_single_indices.clear();
 obs_simple_single_indices.clear();

fcst_composite_single_indices.clear();
 obs_composite_single_indices.clear();

simple_pair_indices.clear();

composite_pair_indices.clear();


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ModeRun::assign(const ModeRun & m)

{

clear();


a = m.a;

FcstValid = m.FcstValid;
 ObsValid = m.ObsValid;

FcstLead = m.FcstLead;
 ObsLead = m.ObsLead;

FcstAccum = m.FcstAccum;
 ObsAccum = m.ObsAccum;

Model = m.Model;

Filename = m.Filename;

LineFirst = m.LineFirst;
LineLast  = m.LineLast;

FcstRadius   = m.FcstRadius;
FcstThresh   = m.FcstThresh;
FcstVariable = m.FcstVariable;
FcstLevel    = m.FcstLevel;

ObsRadius   = m.ObsRadius;
ObsThresh   = m.ObsThresh;
ObsVariable = m.ObsVariable;
ObsLevel    = m.ObsLevel;

N_FcstSimpleObjs = m.N_FcstSimpleObjs;
N_ObsSimpleObjs = m.N_ObsSimpleObjs;

N_FcstCompositeObjs = m.N_FcstCompositeObjs;
N_ObsCompositeObjs = m.N_ObsCompositeObjs;

N_SimplePairs = m.N_SimplePairs;
N_CompositePairs = m.N_CompositePairs;

fcst_simple_single_indices = m.fcst_simple_single_indices;
 obs_simple_single_indices = m.obs_simple_single_indices;

fcst_composite_single_indices = m.fcst_composite_single_indices;
 obs_composite_single_indices = m.obs_composite_single_indices;

simple_pair_indices = m.simple_pair_indices;

composite_pair_indices = m.composite_pair_indices;


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


int ModeRun::two_to_one(int fnum, int onum, int Nf, int No) const

{

if ( (fnum < 0) || (fnum >= Nf) )  {

   cerr << "\n\n  ModeRun::two_to_one() -> range check error on fnum\n\n";

   exit ( 1 );

}


if ( (onum < 0) || (onum >= No) )  {

   cerr << "\n\n  ModeRun::two_to_one() -> range check error on onum\n\n";

   exit ( 1 );

}


int n;

n = onum*Nf + fnum;

return ( n );

}


////////////////////////////////////////////////////////////////////////


void ModeRun::dump(ostream & out, int depth) const

{

Indent prefix(depth);
int month, day, year, hour, minute, second;
char junk[256];


out << prefix << "FileName          = ";
if ( Filename.length() == 0 )  out << "(nul)\n";
else                           out << '\"' << Filename << "\"\n";
out << prefix << "\n";

out << prefix << "LineFirst         = " << LineFirst << "\n";
out << prefix << "LineLast          = " << LineLast  << "\n";
out << prefix << "\n";

out << prefix << "# Mode Lines      = " << (a.n_elements()) << "\n";
out << prefix << "\n";

out << prefix << "FcstValid         = ";
if ( FcstValid == 0 )  out << "0\n";
else {

   unix_to_mdyhms(FcstValid, month, day, year, hour, minute, second);

   sprintf(junk, "%s %2d, %4d   %02d:%02d:%02d", 
                 short_month_name[month], day, year, 
                 hour, minute, second);

   out << junk << "\n";

}

out << prefix << "FcstLead          = ";
if ( FcstLead == 0 )  out << "0\n";
else {

   hour   = FcstLead/3600;
   minute = (FcstLead%3600)/60;
   second = FcstLead%60;

   sprintf(junk, "%02d:%02d:%02d", hour, minute, second);

   out << junk << "\n";

}

out << prefix << "FcstAccum         = ";
if ( FcstAccum == 0 )  out << "0\n";
else {

   hour   = FcstAccum/3600;
   minute = (FcstAccum%3600)/60;
   second = FcstAccum%60;

   sprintf(junk, "%02d:%02d:%02d", hour, minute, second);

   out << junk << "\n";

}

out << prefix << "\n";

out << prefix << "ObsValid          = ";
if ( ObsValid == 0 )  out << "0\n";
else {

   unix_to_mdyhms(ObsValid, month, day, year, hour, minute, second);

   sprintf(junk, "%s %2d, %4d   %02d:%02d:%02d", 
                 short_month_name[month], day, year, 
                 hour, minute, second);

   out << junk << "\n";

}

out << prefix << "ObsLead           = ";
if ( ObsLead == 0 )  out << "0\n";
else {

   hour   = ObsLead/3600;
   minute = (ObsLead%3600)/60;
   second = ObsLead%60;

   sprintf(junk, "%02d:%02d:%02d", hour, minute, second);

   out << junk << "\n";

}

out << prefix << "ObsAccum          = ";
if ( ObsAccum == 0 )  out << "0\n";
else {

   hour   = ObsAccum/3600;
   minute = (ObsAccum%3600)/60;
   second = ObsAccum%60;

   sprintf(junk, "%02d:%02d:%02d", hour, minute, second);

   out << junk << "\n";

}

out << prefix << "\n";

out << prefix << "Model             = ";
if ( Model.length() == 0 )  out << "(nul)\n";
else                        out << '\"' << Model << "\"\n";
out << prefix << "\n";


out << prefix << "FcstRadius        = " << FcstRadius << "\n";

out << prefix << "FcstThresh        = ";
if ( FcstThresh.length() == 0 )  out << "(nul)\n";
else                             out << '\"' << FcstThresh << "\"\n";

out << prefix << "FcstVariable      = ";
if ( FcstVariable.length() == 0 )  out << "(nul)\n";
else                               out << '\"' << FcstVariable << "\"\n";

out << prefix << "FcstLevel         = ";
if ( FcstLevel.length() == 0 )  out << "(nul)\n";
else                            out << '\"' << FcstLevel << "\"\n";

out << prefix << "\n";


out << prefix << "ObsRadius         = " << ObsRadius << "\n";

out << prefix << "ObsThresh         = ";
if ( ObsThresh.length() == 0 )  out << "(nul)\n";
else                            out << '\"' << ObsThresh << "\"\n";

out << prefix << "ObsVariable       = ";
if ( ObsVariable.length() == 0 )  out << "(nul)\n";
else                              out << '\"' << ObsVariable << "\"\n";

out << prefix << "ObsLevel          = ";
if ( ObsLevel.length() == 0 )  out << "(nul)\n";
else                           out << '\"' << ObsLevel << "\"\n";

out << prefix << "\n";

out << prefix << "# Fcst Simple     = " << N_FcstSimpleObjs << "\n";
out << prefix << "# Obs  Simple     = " << N_ObsSimpleObjs  << "\n";
out << prefix << "\n";
out << prefix << "# Fcst Composite  = " << N_FcstCompositeObjs << "\n";
out << prefix << "# Obs  Composite  = " << N_ObsCompositeObjs  << "\n";
out << prefix << "\n";
out << prefix << "# Simple Pairs    = " << N_SimplePairs << "\n";
out << prefix << "# Composite Pairs = " << N_CompositePairs << "\n";


out << prefix << "\n";

out << prefix << "Fcst Simple Single Indices ...\n";

fcst_simple_single_indices.dump(out, depth + 1);


out << prefix << "\n";

out << prefix << "Obs Simple Single Indices ...\n";

obs_simple_single_indices.dump(out, depth + 1);


out << prefix << "\n";

out << prefix << "Fcst Composite Single Indices ...\n";

fcst_composite_single_indices.dump(out, depth + 1);


out << prefix << "\n";

out << prefix << "Obs Composite Single Indices ...\n";

obs_composite_single_indices.dump(out, depth + 1);


out << prefix << "\n";

out << prefix << "Simple Pair Indices ...\n";

simple_pair_indices.dump(out, depth + 1);


out << prefix << "\n";

out << prefix << "Composite Pair Indices ...\n";

composite_pair_indices.dump(out, depth + 1);





   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString ModeRun::short_filename() const

{

ConcatString s;

if ( Filename.length() > 0 )  s = get_short_name(Filename);

return ( s );

}


////////////////////////////////////////////////////////////////////////


void ModeRun::add(const ModeLine & L)

{

   //
   //  if it's the first line, then initialize some stuff
   //

if ( a.n_elements() == 0 )  {

   FcstValid  = L.fcst_valid ();
   FcstLead   = L.fcst_lead  ();
   FcstAccum  = L.fcst_accum ();

   ObsValid   = L.obs_valid ();
   ObsLead    = L.obs_lead  ();
   ObsAccum   = L.obs_accum ();

   Model = L.model();

   LineFirst = L.line_number ();

   FcstRadius   = L.fcst_rad ();
   FcstThresh   = L.fcst_thr ();
   FcstVariable = L.fcst_var ();
   FcstLevel    = L.fcst_lev ();

   ObsRadius    = L.obs_rad ();
   ObsThresh    = L.obs_thr ();
   ObsVariable  = L.obs_var ();
   ObsLevel     = L.obs_lev ();

}

   //
   //  ok
   //

LineLast = L.line_number();

a.add(L);

return;

}


////////////////////////////////////////////////////////////////////////


bool ModeRun::is_same_run(const ModeLine & L) const

{

if ( a.n_elements() == 0 )  return ( true );

   //
   //  check stuff
   //

if ( FcstValid  != L.fcst_valid () )  return ( false );
if ( FcstLead   != L.fcst_lead  () )  return ( false );
if ( FcstAccum  != L.fcst_accum () )  return ( false );

if ( ObsValid   != L.obs_valid () )  return ( false );
if ( ObsLead    != L.obs_lead  () )  return ( false );
if ( ObsAccum   != L.obs_accum () )  return ( false );

if ( strcmp(Model, L.model()) != 0 )  return ( false );

if ( FcstRadius   != L.fcst_rad () )  return ( false );

if ( strcmp(FcstThresh,   L.fcst_thr ()) != 0 )  return ( false );
if ( strcmp(FcstVariable, L.fcst_var ()) != 0 )  return ( false );
if ( strcmp(FcstLevel,    L.fcst_lev ()) != 0 )  return ( false );

if ( ObsRadius    != L.obs_rad () )  return ( false );

if ( strcmp(ObsThresh,   L.obs_thr ()) != 0 )  return ( false );
if ( strcmp(ObsVariable, L.obs_var ()) != 0 )  return ( false );
if ( strcmp(ObsLevel,    L.obs_lev ()) != 0 )  return ( false );

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


void ModeRun::patch_up()

{

int j, n;
int index;
ModeLine L;
FO_Pair p;
const char * c = (const char *) 0;


n = n_lines();

N_FcstSimpleObjs    = 0;
N_ObsSimpleObjs     = 0;

N_FcstCompositeObjs = 0;
N_ObsCompositeObjs  = 0;

N_SimplePairs       = 0;
N_CompositePairs    = 0;


for (j=0; j<n; ++j)  {

   L = a[j];

   if ( L.is_fcst() && L.is_simple() )  ++N_FcstSimpleObjs;
   if ( L.is_obs () && L.is_simple() )  ++N_ObsSimpleObjs;

   if ( L.is_fcst() && L.is_cluster() )  ++N_FcstCompositeObjs;
   if ( L.is_obs () && L.is_cluster() )  ++N_ObsCompositeObjs;

   if ( L.is_pair () && L.is_simple()    )  ++N_SimplePairs;
   if ( L.is_pair () && L.is_cluster() )  ++N_CompositePairs;

}

fcst_simple_single_indices.set_size(N_FcstSimpleObjs);
 obs_simple_single_indices.set_size(N_ObsSimpleObjs);

fcst_composite_single_indices.set_size(N_FcstCompositeObjs);
 obs_composite_single_indices.set_size(N_ObsCompositeObjs);

simple_pair_indices.clear();
composite_pair_indices.clear();

for (j=0; j<n; ++j)  {

   L = a[j];

   c = L.object_id();

         ///////////////////////////////////

   if ( L.is_fcst() && L.is_simple() )  {

      index = simple_single_object_number(c);

      fcst_simple_single_indices.put(index, j);

   }

         ///////////////////////////////////

   if ( L.is_obs() && L.is_simple() )  {

      index = simple_single_object_number(c);

      obs_simple_single_indices.put(index, j);

   }

         ///////////////////////////////////

   if ( L.is_fcst() && L.is_cluster() )  {

      index = composite_single_object_number(c);

      fcst_composite_single_indices.put(index, j);

   }

         ///////////////////////////////////

   if ( L.is_obs() && L.is_cluster() )  {

      index = composite_single_object_number(c);

      obs_composite_single_indices.put(index, j);

   }

         ///////////////////////////////////

   if ( L.is_pair() && L.is_simple() )  {

      p = pair_object_number(c);

      p.index = j;

      simple_pair_indices.add(p);

   }

         ///////////////////////////////////

   if ( L.is_pair() && L.is_cluster() )  {

      p = pair_object_number(c);

      p.index = j;

      composite_pair_indices.add(p);

   }

}   //  for j




// cerr << "\n\n  ModeRun::patch_up() -> indices not patched up yet!\n\n";

// exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


ModeLine ModeRun::get_fcst_simple_single(int n) const

{

if ( (n < 0) || (n >= N_FcstSimpleObjs) )  {

   cerr << "\n\n  ModeRun::get_fcst_simple_single(int) const -> range check error\n\n";

   exit ( 1 );

}

int index;

index = fcst_simple_single_indices[n];

return ( a[index] );

}


////////////////////////////////////////////////////////////////////////


ModeLine ModeRun::get_obs_simple_single(int n) const

{

if ( (n < 0) || (n >= N_ObsSimpleObjs) )  {

   cerr << "\n\n  ModeRun::get_obs_simple_single(int) const -> range check error\n\n";

   exit ( 1 );

}

int index;

index = obs_simple_single_indices[n];

return ( a[index] );

}


////////////////////////////////////////////////////////////////////////


ModeLine ModeRun::get_fcst_composite_single(int n) const

{

if ( (n < 0) || (n >= N_FcstCompositeObjs) )  {

   cerr << "\n\n  ModeRun::get_fcst_composite_single(int) const -> range check error\n\n";

   exit ( 1 );

}

int index;

index = fcst_composite_single_indices[n];

return ( a[index] );

}


////////////////////////////////////////////////////////////////////////


ModeLine ModeRun::get_obs_composite_single(int n) const

{

if ( (n < 0) || (n >= N_ObsCompositeObjs) )  {

   cerr << "\n\n  ModeRun::get_obs_composite_single(int) const -> range check error\n\n";

   exit ( 1 );

}

int index;

index = obs_composite_single_indices[n];

return ( a[index] );

}


////////////////////////////////////////////////////////////////////////


ModeLine ModeRun::get_simple_pair(int fnum, int onum) const

{

int j, k;
FO_Pair p;


k = -1;

for (j=0; j<N_SimplePairs; ++j)  {

   p = simple_pair_indices[j];

   if ( (p.nf == fnum) && (p.no == onum) )  { k = p.index; break; }

}


if ( k < 0 )  {

   cerr << "\n\n  ModeRun::get_simple_pair(int fnum, int onum) const -> "
        << "pair (" << fnum << ", " << onum << ") not found\n\n";

   exit ( 1 );

}


return ( a[k] );

}


////////////////////////////////////////////////////////////////////////


ModeLine ModeRun::get_composite_pair(int fnum, int onum) const

{

int j, k;
FO_Pair p;


k = -1;

for (j=0; j<N_CompositePairs; ++j)  {

   p = composite_pair_indices[j];

   if ( (p.nf == fnum) && (p.no == onum) )  { k = p.index; break; }

}


if ( k < 0 )  {

   cerr << "\n\n  ModeRun::get_composite_pair(int fnum, int onum) const -> "
        << "pair (" << fnum << ", " << onum << ") not found\n\n";

   exit ( 1 );

}


return ( a[k] );

}


////////////////////////////////////////////////////////////////////////


bool ModeRun::has_simple_pair(int fnum, int onum, int & index) const

{

int j;
FO_Pair p;

index = -1;

for (j=0; j<N_SimplePairs; ++j)  {

   p = simple_pair_indices[j];

   if ( (p.nf == fnum) && (p.no == onum) )  { index = p.index;  return ( true ); }

}


return ( false );

}


////////////////////////////////////////////////////////////////////////


bool ModeRun::has_simple_pair(int fnum, int onum) const

{

int index;
bool status = false;

status = has_simple_pair(fnum, onum, index);

return ( status );

}


////////////////////////////////////////////////////////////////////////


bool ModeRun::has_composite_pair(int fnum, int onum, int & index) const

{

int j;
FO_Pair p;

index = -1;

for (j=0; j<N_CompositePairs; ++j)  {

   p = composite_pair_indices[j];

   if ( (p.nf == fnum) && (p.no == onum) )  { index = p.index;  return ( true ); }

}


return ( false );

}


////////////////////////////////////////////////////////////////////////


bool ModeRun::has_composite_pair(int fnum, int onum) const

{

int index;
bool status = false;

status = has_composite_pair(fnum, onum, index);

return ( status );

}


////////////////////////////////////////////////////////////////////////


MetIntArray ModeRun::fcst_composite_obj_numbers(int fnum) const

{

int j;
int nf, ncf;
int index;
MetIntArray ia;
ModeLine L;


for (j=0; j<N_FcstSimpleObjs; ++j)  {

   index = fcst_simple_single_indices[j];

   L = a[index];

   nf  = simple_single_object_number(L.object_id());
   ncf = composite_single_object_number(L.object_cat());

   if ( ncf != fnum )  continue;

   ia.add(nf);

}


return ( ia );

}


////////////////////////////////////////////////////////////////////////


MetIntArray ModeRun::obs_composite_obj_numbers(int onum) const

{

int j, index;
int no, nco;
MetIntArray ia;
ModeLine L;


for (j=0; j<N_ObsSimpleObjs; ++j)  {

   index = obs_simple_single_indices[j];

   L = a[index];

   no  = simple_single_object_number(L.object_id());
   nco = composite_single_object_number(L.object_cat());

   if ( nco != onum )  continue;

   ia.add(no);

}


return ( ia );

}


////////////////////////////////////////////////////////////////////////


FO_Pair ModeRun::simple_pair(int n) const

{

if ( (n < 0) || (n >= N_SimplePairs) )  {

   cerr << "\n\n  ModeRun::simple_pair(int n) const -> range check error\n\n";

   exit ( 1 );

}

return ( simple_pair_indices[n] );

}


////////////////////////////////////////////////////////////////////////


FO_Pair ModeRun::composite_pair(int n) const

{

if ( (n < 0) || (n >= N_CompositePairs) )  {

   cerr << "\n\n  ModeRun::composite_pair(int n) const -> range check error\n\n";

   exit ( 1 );

}

return ( composite_pair_indices[n] );

}


////////////////////////////////////////////////////////////////////////


ModeLineArray ModeRun::get_fcst_composite(int fnum) const

{

if ( (fnum < 0) || (fnum >= N_FcstCompositeObjs) )  {

   cerr << "\n\n  ModeRun::get_fcst_composite(int fnum) const -> range check error\n\n";

   exit ( 1 );

}

int j, k, k2, n;
MetIntArray ia;
ModeLineArray mla;

ia = fcst_composite_obj_numbers(fnum);

n = ia.n_elements();

for (j=0; j<n; ++j)  {

   k = ia[j];

   k2 = fcst_simple_single_indices[k];

   mla.add(a[k2]);

}   //  for j

   //
   //  done
   //

return ( mla );

}


////////////////////////////////////////////////////////////////////////


ModeLineArray ModeRun::get_obs_composite(int onum) const

{

if ( (onum < 0) || (onum >= N_ObsCompositeObjs) )  {

   cerr << "\n\n  ModeRun::get_obs_composite(int onum) const -> range check error\n\n";

   exit ( 1 );

}

int j, k, k2, n;
MetIntArray ia;
ModeLineArray mla;

ia = obs_composite_obj_numbers(onum);

n = ia.n_elements();

for (j=0; j<n; ++j)  {

   k = ia[j];

   k2 = obs_simple_single_indices[k];

   mla.add(a[k2]);

}   //  for j

   //
   //  done
   //

return ( mla );

}


////////////////////////////////////////////////////////////////////////


void ModeRun::calc_mmi(double & fcst_mmi, double & obs_mmi, double & fo_mmi) const

{

const int NF = N_FcstSimpleObjs;
const int NO = N_ObsSimpleObjs;

const int N = NF + NO;

fcst_mmi = obs_mmi = fo_mmi = 0.0;

if ( N == 0 )  return;

int j;
int nf, no;
double interest;
double * f_ivals = (double *) 0;
double * o_ivals = (double *) 0;
double *   ivals = (double *) 0;
ModeLine L;
bool status = false;


f_ivals = new double [NF];
o_ivals = new double [NO];

ivals = new double [N];

for (j=0; j<NF; ++j)  f_ivals[j] = 0.0;
for (j=0; j<NO; ++j)  o_ivals[j] = 0.0;

for (j=0; j<N; ++j)  ivals[j] = 0.0;   //  not strictly needed

for (j=0; j<(a.n_elements()); ++j)  {

   L = a[j];

   if ( ! L.is_pair() )  continue;

   if ( ! L.is_simple() )  continue;

   (void) L.pair_obj_numbers(nf, no);

   interest = L.interest();

   if ( interest > o_ivals[no] )  o_ivals[no] = interest;
   if ( interest > f_ivals[nf] )  f_ivals[nf] = interest;

}   //  for j;


for (j=0; j<NF; ++j)  ivals[j] = f_ivals[j];

for (j=0; j<NO; ++j)  ivals[j + NF] = o_ivals[j];


sort(f_ivals, NF);

status = calc_median(f_ivals, NF, fcst_mmi);

if ( ! status )  fcst_mmi = -1.0;


sort(o_ivals, NO);

status = calc_median(o_ivals, NO, obs_mmi);

if ( ! status )  obs_mmi = -1.0;


sort(ivals, N);

status = calc_median(ivals, N, fo_mmi);

if ( ! status )  fo_mmi = -1.0;

   //
   //  done
   //

if ( f_ivals )  { delete [] f_ivals;  f_ivals = (double *) 0; }
if ( o_ivals )  { delete [] o_ivals;  o_ivals = (double *) 0; }
if (   ivals )  { delete []   ivals;    ivals = (double *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


int simple_single_object_number(const char * text)

{

const char * c = text + 1;   //  skip the leading "F" or "O"

int k = atoi(c);

--k;   //  object numbers start at zero


return ( k );

}


////////////////////////////////////////////////////////////////////////


int composite_single_object_number(const char * text)

{

const char * c = text + 2;   //  skip the leading "CF" or "CO"

int k = atoi(c);

--k;   //  object numbers start at zero


return ( k );

}


////////////////////////////////////////////////////////////////////////


FO_Pair pair_object_number(const char * text)

{

int n;
FO_Pair p;
char line[256];
char * c = (char *) 0;
const char delim [] = "CFO_";


n = strlen(text);

if ( n >= (-1 + (int) sizeof(line)) )  {

   cerr << "\n\n  pair_object_number() -> string \"" << text << "\" too long!\n\n";

   exit ( 1 );

}

memset(line, 0, sizeof(line));

strncpy(line, text, n);

c = strtok(line, delim);

if ( !c )  {

   cerr << "\n\n  pair_object_number() -> can't get 1st number from string \"" << text << "\"\n\n";

   exit ( 1 );

}

p.nf = -1 + atoi(c);   //  object numbers start at zero


c = strtok(0, delim);

if ( !c )  {

   cerr << "\n\n  pair_object_number() -> can't get 2nd number from string \"" << text << "\"\n\n";

   exit ( 1 );

}

p.no = -1 + atoi(c);   //  object numbers start at zero

   //
   //  done
   //

return ( p );

}


////////////////////////////////////////////////////////////////////////


bool calc_median(const double * sorted_values, const int n, double & value)

{

if ( n <= 0 )  {

   cerr << "\n\n  calc_median() -> bad N value ... " << n << "\n\n";

   // exit ( 1 );

   value = -1.0;

   return ( false );

}

if ( n == 1 )  {

   value = sorted_values[0];

   return ( true );

}

int k;


if ( n%2 )   { //  n odd, n >= 3

   k = (n - 1)/2;

   value = sorted_values[k];

} else {   // n even, n >= 2

   k = n/2;

   value = 0.5*(sorted_values[k - 1] + sorted_values[k]);

}


return ( true );

}


////////////////////////////////////////////////////////////////////////





