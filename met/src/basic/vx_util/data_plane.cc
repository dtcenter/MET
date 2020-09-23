// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   data_plane.cc
//
//   Description:
//      Contains the definition of the DataPlane class.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    11-01-11  Halley Gotway
//
///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include "data_plane.h"

#include "vx_log.h"
#include "vx_math.h"
#include "vx_cal.h"
#include "math_constants.h"

///////////////////////////////////////////////////////////////////////////////
//
//  Begin Code for class DataPlane
//
///////////////////////////////////////////////////////////////////////////////

DataPlane::DataPlane() {

   init_from_scratch();

}

///////////////////////////////////////////////////////////////////////////////

DataPlane::~DataPlane() {

   clear();
}

///////////////////////////////////////////////////////////////////////////////

DataPlane::DataPlane(const DataPlane &d) {

   init_from_scratch();

   assign(d);
}

///////////////////////////////////////////////////////////////////////////////

DataPlane & DataPlane::operator=(const DataPlane &d) {

   if(this == &d) return(*this);

   assign(d);

   return(*this);
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::init_from_scratch() {

   Nx = 0;
   Ny = 0;
   Nxy = 0;
   clear();

}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::assign(const DataPlane &d) {

   clear();

   set_size(d.nx(), d.ny());

   Data = d.Data;

   InitTime  = d.init();
   ValidTime = d.valid();
   LeadTime  = d.lead();
   AccumTime = d.accum();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::clear() {

   Data.clear();

   Nx = 0;
   Ny = 0;

   Nxy = 0;

   InitTime = ValidTime = (unixtime) 0;
   LeadTime = AccumTime = 0;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::erase() {

   Data.resize(Nxy);
   Data.assign(Nxy, 0);

   InitTime = ValidTime = (unixtime) 0;
   LeadTime = AccumTime = 0;

   return;
}

///////////////////////////////////////////////////////////////////////////////


void DataPlane::dump(ostream & out, int depth) const {
   ConcatString time_str;

   Indent prefix(depth);

   out << prefix << "Nx        = " << Nx  << '\n';
   out << prefix << "Ny        = " << Ny  << '\n';
   out << prefix << "Nxy       = " << Nxy << '\n';

   time_str = unix_to_yyyymmdd_hhmmss(InitTime);
   out << prefix << "InitTime  = " << time_str << " (" << InitTime  << ")\n";

   time_str = unix_to_yyyymmdd_hhmmss(ValidTime);
   out << prefix << "ValidTime = " << time_str << " (" << ValidTime << ")\n";

   time_str = sec_to_hhmmss(LeadTime);
   out << prefix << "LeadTime  = " << time_str << " (" << LeadTime  << ")\n";

   time_str = sec_to_hhmmss(AccumTime);
   out << prefix << "AccumTime = " << time_str << " (" << AccumTime << ")\n";

   //
   //  done
   //

   out.flush();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::set_size(int x, int y) {

      //
      //  if already requested size, erase existing data
      //

   if ( Nx == x && Ny == y ) {
      erase();
      return;
   }

      //
      //  delete exisiting data, if necessary
      //

   Nx = x;
   Ny = y;

   Nxy = Nx*Ny;

      //
      //  resize and initialize data
      //

   Data.resize(Nxy);
   Data.assign(Nxy, 0);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::set(double v, int x, int y) {
   int n;

   n = two_to_one(x, y);

   Data[n] = v;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::set_block(double *v, int nx, int ny) {
   const char *method_name = "DataPlane::set_block() -> ";
   
   if (nx > Nx) {
      mlog << Error << "\n" << method_name << "nx is too big ("
           << nx << " should be equal or less than " << Nx << "\n\n\n";
      exit(1);
   }
   if (ny > Ny) {
      mlog << Error << "\n" << method_name << "ny is too big ("
           << ny << " should be equal or less than " << Ny << "\n\n\n";
      exit(1);
   }
   
   int offset = 0;
   //Note: v should be a row first & the size is (nx * ny).
   //      implemented based on two_to_one("n = y*Nx + x").
   for (int y=0; y < ny; y++) {
      int dp_offset = two_to_one(0, y);
      for (int x=0; x < nx; x++) {
         Data[dp_offset+x] = v[offset++];
      }
   }
   
   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::set_constant(double v) {

   if(Data.empty()) {
      mlog << Error << "\nDataPlane::set_constant(double) -> "
           << "no data buffer allocated!\n\n";
      exit(1);
   }

   int j;

   for(j=0; j<Nxy; ++j) Data[j] = v;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::set_init(unixtime ut) {
   InitTime = ut;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::set_valid(unixtime ut) {
   ValidTime = ut;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::set_lead(int s) {
   LeadTime = s;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::set_accum(int s) {
   AccumTime = s;
   return;
}

///////////////////////////////////////////////////////////////////////////////

double DataPlane::get(int x, int y) const {
   int n;

   n = two_to_one(x, y);

   return(Data[n]);
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::threshold(const SingleThresh &st) {
   int j;

   //
   // Loop through the data and apply the threshold to all valid values
   //   1.0 if it meets the threshold criteria
   //   0.0 if it does not
   //

   for(j=0; j<Nxy; ++j) {

      if( is_bad_data(Data[j]) )  continue;
      if( st.check(Data[j]) )     Data[j] = 1.0;
      else                        Data[j] = 0.0;

   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::convert(const UserFunc_1Arg &convert_fx) {

   if(!convert_fx.is_set()) return;

   mlog << Debug(3) << "Applying conversion function.\n";

   for(int i=0; i<Nxy; i++) {
      if(!is_bad_data(buf()[i])) buf()[i] = convert_fx(buf()[i]);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::censor(const ThreshArray &censor_thresh,
                       const NumArray &censor_val) {
   int i, j, count;
   ThreshArray ta = censor_thresh;

   // Check for no work to do
   if(censor_thresh.n_elements() == 0) return;

   // Check for percentile thresholds
   if(ta.need_perc()) {
      NumArray d;
      d.extend(Nxy);
      for(i=0; i<Nxy; i++) {
         if(!is_bad_data(Data[i])) d.add(Data[i]);
      }
      ta.set_perc(&d, &d, &d);
   }

   mlog << Debug(3)
        << "Applying censor thresholds \"" << ta.get_str(" ")
        << "\" and replacing with values \"" << censor_val.serialize()
        << "\".\n";

   // Loop through the points and apply all the censor thresholds.
   for(i=0,count=0; i<Nxy; i++) {

      for(j=0; j<ta.n_elements(); j++) {

         // Break out after the first match.
         if(ta[j].check(Data[i])) {
            Data[i] = censor_val[j];
            count++;
            break;
         }
      }
   }

   mlog << Debug(3)
        << "Censored values for " << count << " of " << Nxy
        << " grid points.\n";

   return;
}

///////////////////////////////////////////////////////////////////////////////


void DataPlane::replace_bad_data(const double value)

{

int j;

for (j=0; j<Nxy; ++j)  {

   if ( is_bad_data(Data[j]) )  Data[j] = value;

}

return;

}


///////////////////////////////////////////////////////////////////////////////

int DataPlane::two_to_one(int x, int y) const {
   int n;

   if((x < 0) || (x >= Nx) || (y < 0) || (y >= Ny)) {
      mlog << Error << "\nDataPlane::two_to_one() -> "
           << "range check error: (Nx, Ny) = (" << Nx << ", " << Ny
           << "), (x, y) = (" << x << ", " << y << ")\n\n";
      exit(1);
   }

   n = y*Nx + x;   //  don't change this!  lots of downstream code depends on this!

   return(n);
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::one_to_two(int n, int &x, int &y) const {

   if(n < 0 || n >= Nxy) {
      mlog << Error << "\nDataPlane::one_to_two() -> "
           << "range check error: n = " << n << "but Nx*Ny = " << Nxy
           << "\n\n";
      exit(1);
   }

   x = n%Nx;
   y = n/Nx;

   return;
}

///////////////////////////////////////////////////////////////////////////////

bool DataPlane::s_is_on(int x, int y) const {

   //
   // Return true if the current point is non-zero.
   //

   return( !is_eq(get(x, y), 0) );
}

///////////////////////////////////////////////////////////////////////////////

bool DataPlane::f_is_on(int x, int y) const {

   //
   // Consider the box defined by (x,y) as the upper-right corner.
   // Return true if any corner of that box is non-zero.
   //

   if( s_is_on(x, y) )                                return(true);

   if( (x > 0) && s_is_on(x - 1, y) )                 return(true);

   if( (x > 0) && (y > 0) && s_is_on(x - 1, y - 1) )  return(true);

   if( (y > 0) && s_is_on(x, y - 1) )                 return(true);

   return(false);
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::data_range(double & data_min, double & data_max) const {
   int j;
   double value;
   bool first_set = true;

   data_min = data_max = bad_data_double;

   for(j=0; j<Nxy; ++j) {

      value = Data[j];

      if(is_bad_data(value)) continue;

      if(first_set) {
         data_min = data_max = value;
         first_set = false;
      }
      else {
         data_min = min(value, data_min);
         data_max = max(value, data_max);
      }
   }   //  for j

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
// Return a MaskPlane version of the DataPlane object
//
///////////////////////////////////////////////////////////////////////////////

MaskPlane DataPlane::mask_plane() const {
   MaskPlane mp;

   mp.set_size(Nx, Ny);

   for(int i=0; i<Nxy; i++) {
      mp.buf()[i] = (is_bad_data(Data[i]) ? false : !is_eq(Data[i], 0.0));
   }

   return(mp);
}

///////////////////////////////////////////////////////////////////////////////


void DataPlane::shift_right(int N)

{

   mlog << Debug(3)
        << "Shifting dataplane to the right " << N << " grid squares.\n";

   //
   //  check some stuff
   //


if ( Data.empty() )  {

   mlog << Error
        << "\n\n  DataPlane::shift_right(int) -> data plane is empty!\n\n";

   exit ( 1 );

}

N %= Nx;

if ( N < 0 )  N += Nx;

if ( N == 0 )  return;   //  no shift, so do nothing

   //
   //  ok, get to work
   //

int x, y, x_new;
int index_old, index_new;
std::vector<double> new_data(Nxy);

for (x=0; x<Nx; ++x)  {

   x_new = (x + N)%Nx;

   for (y=0; y<Ny; ++y)  {

      index_old = two_to_one(x,     y);
      index_new = two_to_one(x_new, y);

      new_data[index_new] = Data[index_old];

   }

}

Data = new_data;

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void DataPlane::put(const double value, const int x, const int y)

{

if ( Data.empty() )  {

   mlog << Error
        << "\n\n  DataPlane::put() -> no data plane allocated!\n\n";

   exit ( 1 );

}

const int n = two_to_one(x, y);   //  the two_to_one function does range checking on x and y

Data[n] = value;

return;

}


///////////////////////////////////////////////////////////////////////////////

   //
   //  Note: this function could be speeded up a lot, I think
   //

bool DataPlane::fitwav_1d_old(const int start_wave, const int end_wave)

{

int i, j, k;
double * a = 0;
double * b = 0;
double * xa = 0;
double * xb = 0;
double xa0, value, angle;
const unsigned int mnw = (Nx + 1)/2;

   //
   // Check for bad data
   //

for (j=0; j<Nxy; ++j)  {
   if (is_bad_data(Data[j])) return ( false );
}

   //
   // Range check the requested wave numbers
   //

if ( start_wave < 0   || end_wave < 0 ||
     start_wave > mnw || end_wave > mnw )  {

   mlog << Error << "\nDataPlane::fitwav_1d_old() -> "
        << "Requested wave numbers (" << start_wave << " to " << end_wave
        << ") must be between 0 and " << mnw << " for data with dimension "
        << "(Nx, Ny) = (" << Nx << ", " << Ny << ")!\n\n";

   exit ( 1 );

}

   //
   // Allocate memory
   //

a  = new double [mnw+1];
b  = new double [mnw+1];
xa = new double [mnw+1];
xb = new double [mnw+1];

for (j=0; j<Ny; ++j)  {

   xa0 = 0.0;

   for (k=0; k<Nx; ++k)  {

      xa0 += get(k, j);

   }   //  for k

   a[0] = xa0/Nx;
   b[0] = 0.0;

   /////////////////////////////

   for (i=1; i<=mnw; ++i)  {

      xa[i] = xb[i] = 0.0;

      for (k=0; k<Nx; ++k)  {

         angle = (twopi*i*k)/Nx;

         xa[i] += (get(k, j))*cos(angle);
         xb[i] += (get(k, j))*sin(angle);

      }   //  for k

      a[i] = (2.0*xa[i])/Nx;
      b[i] = (2.0*xb[i])/Nx;

   }   //  for i

   /////////////////////////////

   for (k=0; k<Nx; ++k)  {

      value = 0.0;

      for (i=start_wave; i<=end_wave; ++i)  {

         angle = (twopi*i*k)/Nx;

         value += (a[i])*cos(angle);

         value += (a[i])*sin(angle);

      }   //  for i

      put(value, k, j);

   }   //  for k

   /////////////////////////////

}   //  for j


   //
   //  done
   //

if ( a )  { delete [] a;  a = 0; }
if ( b )  { delete [] b;  b = 0; }

if ( xa )  { delete [] xa;  xa = 0; }
if ( xb )  { delete [] xb;  xb = 0; }

return(true);

}


///////////////////////////////////////////////////////////////////////////////


bool DataPlane::fitwav_1d(const int start_wave, const int end_wave)

{

int i, m, x, y;
double *  a = 0;
double *  b = 0;
double * xa = 0;
double * xb = 0;
double *  C = 0;
double *  S = 0;
double xa0, value, angle;
const int unsigned mnw = (Nx + 1)/2;
// const int mnw = Nx - 1;

// const time_t start_time = time(0);

   //
   // Check for bad data
   //

for (i=0; i<Nxy; ++i)  {
   if (is_bad_data(Data[i])) return ( false );
}

   //
   // Range check the requested wave numbers
   //

if ( start_wave < 0   || end_wave < 0 ||
     start_wave > mnw || end_wave > mnw )  {

   mlog << Error << "\nDataPlane::fitwav_1d() -> "
        << "Requested wave numbers (" << start_wave << " to " << end_wave
        << ") must be between 0 and " << mnw << " for data with dimension "
        << "(Nx, Ny) = (" << Nx << ", " << Ny << ")!\n\n";

   exit ( 1 );

}

   //
   // Allocate memory
   //

a  = new double [ mnw + 1 ];
b  = new double [ mnw + 1 ];
xa = new double [ mnw + 1 ];
xb = new double [ mnw + 1 ];

C  = new double [ Nx ];
S  = new double [ Nx ];

for (x=0; x<Nx; ++x)  {

   angle = (twopi*x)/Nx;

   C[x] = cos(angle);

   S[x] = sin(angle);

}



for (y=0; y<Ny; ++y)  {

   xa0 = 0.0;

   for (x=0; x<Nx; ++x)  {

      xa0 += get(x, y);

   }   //  for x

   a[0] = xa0/Nx;
   b[0] = 0.0;

   /////////////////////////////

   for (i=1; i<=mnw; ++i)  {

      xa[i] = xb[i] = 0.0;

      for (x=0; x<Nx; ++x)  {

         m = (i*x)%Nx;

         // angle = (twopi*i*x)/Nx;

         // xa[i] += (get(x, y))*cos(angle);
         // xb[i] += (get(x, y))*sin(angle);

         xa[i] += (get(x, y))*(C[m]);
         xb[i] += (get(x, y))*(S[m]);

      }   //  for x

      a[i] = (2.0*xa[i])/Nx;
      b[i] = (2.0*xb[i])/Nx;

   }   //  for i

   /////////////////////////////

   for (x=0; x<Nx; ++x)  {

      value = 0.0;

      for (i=start_wave; i<=end_wave; ++i)  {

         m = (i*x)%Nx;

         // angle = (twopi*i*x)/Nx;

         // value += (a[i])*cos(angle);
         // value += (b[i])*sin(angle);

         value += (a[i])*(C[m]);
         value += (b[i])*(S[m]);

      }   //  for i

      put(value, x, y);

   }   //  for x

   /////////////////////////////

}   //  for y


   //
   //  done
   //

if ( a )  { delete [] a;  a = 0; }
if ( b )  { delete [] b;  b = 0; }

if ( xa )  { delete [] xa;  xa = 0; }
if ( xb )  { delete [] xb;  xb = 0; }

if ( C )  { delete [] C;  C = 0; }
if ( S )  { delete [] S;  S = 0; }

   //
   //  done
   //

return(true);

}


///////////////////////////////////////////////////////////////////////////////
//
//  End Code for class DataPlane
//
///////////////////////////////////////////////////////////////////////////////

   //
   //  Code for class DataPlaneArray
   //

///////////////////////////////////////////////////////////////////////////////


DataPlaneArray::DataPlaneArray()

{

init_from_scratch();

}


///////////////////////////////////////////////////////////////////////////////


DataPlaneArray::~DataPlaneArray()

{

clear();

}


///////////////////////////////////////////////////////////////////////////////


DataPlaneArray::DataPlaneArray(const DataPlaneArray & a)

{

init_from_scratch();

assign(a);

}


///////////////////////////////////////////////////////////////////////////////


DataPlaneArray & DataPlaneArray::operator=(const DataPlaneArray & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::init_from_scratch()

{

Lower = (double *) 0;
Upper = (double *) 0;

Plane = (DataPlane **) 0;

Nplanes = 0;

clear();

return;

}


///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::clear()

{

if ( Nplanes > 0 )  {

   int j;

   for (j=0; j<Nplanes; ++j)  {

      if ( Plane[j] )  { delete Plane[j];  Plane[j] = (DataPlane *) 0; }

   }

   delete [] Plane;  Plane = (DataPlane **) 0;

}

Nplanes = 0;
Nalloc  = 0;

AllocInc = dataplane_default_alloc_inc;

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::assign(const DataPlaneArray & a)

{

clear();

if ( a.Nplanes == 0 )  return;

extend(a.Nplanes);

AllocInc = a.AllocInc;

int j;

for (j=0; j<a.Nplanes; ++j)  {

   add( *(a.Plane[j]), a.Lower[j], a.Upper[j] );

}

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::extend(int n, bool exact)

{

if ( Nalloc >= n )  return;

int j, k;
DataPlane ** p = (DataPlane **) 0;
double * b = (double *) 0;
double * t = (double *) 0;

if ( ! exact )  {

   k = (n + AllocInc - 1)/AllocInc;
   n = k*AllocInc;

}

p = new DataPlane * [n];
b = new double      [n];
t = new double      [n];

for (j=0; j<n; ++j)  {

   p[j] = (DataPlane *) 0;

   b[j] = t[j] = 0.0;

}

if ( Plane )  {

   for (j=0; j<Nplanes; ++j)  {

      p[j] = Plane[j];

      b[j] = Lower[j];

      t[j] = Upper[j];

   }   //  for j;

   delete [] Plane;  Plane = (DataPlane **) 0;
   delete [] Lower;  Lower = (double *)     0;
   delete [] Upper;  Upper = (double *)     0;
}

Plane = p;

Lower = b;

Upper = t;

p = (DataPlane **) 0;
b = (double *)     0;
t = (double *)     0;

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::add(const DataPlane & p, double _low, double _up)

{

check_xy_size(p);

extend(Nplanes + 1, false);

Plane[Nplanes] = new DataPlane;

*(Plane[Nplanes]) = p;

Lower[Nplanes] = _low;
Upper[Nplanes] = _up;

++Nplanes;


   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::check_xy_size(const DataPlane & p) const

{

if ( Nplanes == 0 )  return;

if ( (p.nx() != Plane[0]->nx()) || (p.ny() != Plane[0]->ny()) )  {

   mlog << Error << "\nDataPlaneArray::check_xy_size(const DataPlane &) const -> wrong size!\n\n";

   exit ( 1 );

}

return;

}


///////////////////////////////////////////////////////////////////////////////


double DataPlaneArray::data(int p, int x, int y) const

{

if ( (p < 0) || (p >= Nplanes) )  {

   mlog << Error << "\nDataPlaneArray::data() -> range check error!\n\n";

   exit ( 1 );

}

double value = Plane[p]->get(x, y);

return ( value );

}



///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::set(double v, int p, int x, int y)

{

if ( (p < 0) || (p >= Nplanes) )  {

   mlog << Error << "\nDataPlaneArray::set() -> range check error!\n\n";

   exit ( 1 );

}

Plane[p]->set(v, x, y);

return;

}


///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::set_alloc_inc(int n)

{

if ( n <= 0 )  {

   mlog << Error << "\nvoid DataPlaneArray::set_alloc_inc(int) -> bad value ... " << n << "\n\n";

   exit ( 1 );

}

AllocInc = n;

return;

}


///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::set_levels(int n, double _low, double _up)

{

if ( (n < 0) || (n >= Nplanes) )  {

   mlog << Error << "\nDataPlaneArray::set_levels(int, double, double) -> bad level index ... " << n << "\n\n";

   exit ( 1 );

}

if ( _low > _up )  {

   mlog << Error << "\nDataPlaneArray::set_levels(int, double, double) -> lowtom level > up level!\n\n";

   exit ( 1 );
}

Lower[n] = _low;
Upper[n] = _up;

return;

}


///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::levels(int n, double & _low, double & _up) const

{

if ( (n < 0) || (n >= Nplanes) )  {

   mlog << Error << "\nDataPlaneArray::level(int, double &, double &) -> bad plane index ... " << n << "\n\n";

   exit ( 1 );

}

_up  = Upper [n];
_low = Lower [n];

return;

}


///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::level_range(double & _low, double & _up) const

{

int j;

_low = _up = bad_data_int;

for (j=0; j<Nplanes; ++j)  {

   if ( is_bad_data(_low) || Lower[j] <= _low ) _low = Lower[j];
   if ( is_bad_data(_up)  || Upper[j] >= _up  ) _up  = Upper[j];

}

return;

}


///////////////////////////////////////////////////////////////////////////////


int DataPlaneArray::nx() const

{

if ( Nplanes == 0 )  {

   mlog << Error << "\nDataPlaneArray::nx() const -> array is empty!\n\n";

   exit ( 1 );

}

return ( Plane[0]->nx() );

}


///////////////////////////////////////////////////////////////////////////////


int DataPlaneArray::ny() const

{

if ( Nplanes == 0 )  {

   mlog << Error << "\nDataPlaneArray::ny() const -> array is empty!\n\n";

   exit ( 1 );

}

return ( Plane[0]->ny() );

}


///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Nplanes  = " << Nplanes    << '\n';
out << prefix << "Nalloc   = " << Nalloc     << '\n';
out << prefix << "AllocInc = " << AllocInc   << '\n';
out << prefix << "Nx       = " << ((Nplanes > 0) ? nx() : 0) << '\n';
out << prefix << "Ny       = " << ((Nplanes > 0) ? ny() : 0) << '\n';

int j;

for (j=0; j<Nplanes; ++j)  {

   out << prefix << "Level " << j << "  = "
       << '[' << Lower[j] << ", " << Upper[j] << ']'
       << '\n';

}   //  for j



   //
   //  done
   //

out.flush();

return;

}


///////////////////////////////////////////////////////////////////////////////


double DataPlaneArray::lower(int n) const

{

if ( (n < 0) || (n >= Nplanes) )  {

   mlog << Error << "\nDataPlaneArray::lower(int) const -> bad plane index ... " << n << "\n\n";

   exit ( 1 );

}

return ( Lower[n] );

}


///////////////////////////////////////////////////////////////////////////////


double DataPlaneArray::upper(int n) const

{

if ( (n < 0) || (n >= Nplanes) )  {

   mlog << Error << "\nDataPlaneArray::upper(int) const -> bad plane index ... " << n << "\n\n";

   exit ( 1 );

}

return ( Upper[n] );

}


///////////////////////////////////////////////////////////////////////////////


DataPlane & DataPlaneArray::operator[](int n) const

{

if ( (n < 0) || (n >= Nplanes) )  {

   mlog << Error << "\nDataPlaneArray::operator[](int) const -> bad plane index ... " << n << "\n\n";

   exit ( 1 );

}

return ( *(Plane[n]) );

}


///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::replace_bad_data(const double value)

{

int j;

for (j=0; j<Nplanes; ++j)  {

   Plane[j]->replace_bad_data(value);

}


return;

}


///////////////////////////////////////////////////////////////////////////////




