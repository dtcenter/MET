// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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
//   001    22-09-29  Prestopnik     MET #2227 Remove namespace std from header files
//
///////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <map>

#include "data_plane.h"

#include "vx_log.h"
#include "vx_math.h"
#include "vx_cal.h"
#include "math_constants.h"

using namespace std;

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

   if(this == &d) return *this;

   assign(d);

   return *this;
}

///////////////////////////////////////////////////////////////////////////////

DataPlane & DataPlane::operator+=(const DataPlane &d) {
   const char *method_name = "DataPlane::operator+=(const DataPlane &) -> ";

   // Check for matching dimensions
   if(Nx != d.Nx || Ny != d.Ny) {
      mlog << Error << "\n" << method_name
           << "the dimensions do not match: ("
           << Nx << ", " << Ny << ") != ("
           << d.Nx << ", " << d.Ny << ")\n\n";
      exit(1);
   }

   // Increment values, checking for bad data
   double v;
   for(int i=0; i<Nxy; i++) {
      v = (is_bad_data(Data[i]) || is_bad_data(d.Data[i]) ?
           bad_data_double : Data[i] + d.Data[i]);
      Data[i] = v;
   }

   return *this;
}

///////////////////////////////////////////////////////////////////////////////

DataPlane & DataPlane::operator/=(const double v) {
   const char *method_name = "DataPlane::operator/=(const double) -> ";

   // Check for matching dimensions
   if(is_eq(v, 0.0)) {
      mlog << Error << "\n" << method_name
           << "divide by 0!\n\n";
      exit(1);
   }

   // Apply the operation, checking for bad data
   for(int i=0; i<Nxy; i++) {
      if(!is_bad_data(Data[i])) Data[i] /= v;
   }

   return *this;
}

bool DataPlane::operator==(const DataPlane &d) const {

   const char *method_name = "DataPlane::operator==(const DataPlane &) -> ";

   // don't check times, only data
   
   // Check for matching dimensions
   if(Nx != d.Nx || Ny != d.Ny) {
      return false;
   }

   for(int i=0; i<Nxy; i++) {
      if (Data[i] != d.Data[i]) {
         return false;
      }
   }

   return true;
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

void DataPlane::debug_examine(bool show_all_values) const {

   // Nothing to print if verbosity level is below 4
   if(mlog.verbosity_level() < 4) return;

   map<double,int> value_count_map;
   int total_count = 0;

   for(int i=0; i<Data.size(); i++) {

      // Count positive values
      if(Data[i] > 0) total_count++;

      if (show_all_values) {

         // Add a new map entry
         if(value_count_map.count(Data[i]) == 0) {
            value_count_map[Data[i]] = 0;
         }

         // Increment count
         value_count_map[Data[i]] += 1;
      }
   }

   if(show_all_values) {
      for(auto it  = value_count_map.begin();
               it != value_count_map.end(); it++) {
         mlog << Debug(4) << " data value=" << it->first
              << " count=" << it->second << "\n";
      }
   }

   mlog << Debug(4) << "Total count > 0 = " << total_count
        << " of " << Data.size() << "\n";

   return;
}

///////////////////////////////////////////////////////////////////////////////

string DataPlane::sdebug_examine() const{
   ConcatString cs;
   int n = 0;

   // Count positive values
   for(auto it = Data.begin(); it != Data.end(); it++) {
      if(*it > 0) n++;
   }

   cs << "Total count > 0 = " << n << " of " << (int) Data.size();

   return cs;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::set_size(int nx, int ny, double v) {

      //
      //  if already requested size, erase existing data
      //

   if ( Nx == nx && Ny == ny ) {
      erase();
      return;
   }

      //
      //  delete existing data, if necessary
      //

   Nx = nx;
   Ny = ny;

   Nxy = Nx*Ny;

      //
      //  resize and initialize data
      //

   Data.resize(Nxy);
   Data.assign(Nxy, v);

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

   Data.assign(Nxy, v);

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

void DataPlane::set_all(float *data, int nx, int ny) {
   if (nx != Nx || ny != Ny) {
      mlog << Error << "\nDataPlane::set_all() -> "
           << "the data dimensions do not match: ("
           << Nx << ", " << Ny << ") != ("
           << nx << ", " << ny << ")!\n\n";
      exit(1);
   }
   for (int x=0; x<nx; ++x) {
      for (int y=0; y<ny; ++y) {
         int index = two_to_one(x, y);
         Data[index] = data[index];
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

bool DataPlane::is_all_bad_data() const {
   int j;
   bool status = true;

   //
   // Check for no valid data
   //

   for(j=0; j<Nxy; ++j) {
      if( !is_bad_data(Data[j]) ) {
         status = false;
         break;
      }
   }

   return status;
}

///////////////////////////////////////////////////////////////////////////////

int DataPlane::n_good_data() const {
   int j, n;

   //
   // Count number of good data values
   //

   for(j=0,n=0; j<Nxy; ++j) {
      if(!is_bad_data(Data[j])) n++;
   }

   return n;
}

///////////////////////////////////////////////////////////////////////////////

double DataPlane::get(int x, int y) const {
   int n;

   n = two_to_one(x, y);

   return Data[n];
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
      ta.set_perc(&d, &d, &d, &d);
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

void DataPlane::anomaly(const DataPlane &mn) {

   // Check dimensions
   if(Nxy != mn.Nxy) {
      mlog << Error << "\nDataPlane::anomaly() -> "
           << "the data dimensions do not match: ("
           << Nx << ", " << Ny << ") != ("
           << mn.Nx << ", " << mn.Ny << ")!\n\n";
      exit(1);
   }

   // Subtract the mean
   for(int i=0; i<Nxy; i++) {
      if(is_bad_data(Data[i]) || is_bad_data(mn.Data[i])) {
         Data[i] = bad_data_double;
      }
      else {
         Data[i] -= mn.Data[i];
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::standard_anomaly(const DataPlane &mn,
                                 const DataPlane &sd) {

   // Check dimensions
   if(Nxy != mn.Nxy || Nxy != sd.Nxy) {
      mlog << Error << "\nDataPlane::standard_anomaly() -> "
           << "the data dimensions do not match: ("
           << Nx << ", " << Ny << ") != ("
           << mn.Nx << ", " << mn.Ny << ") != ("
           << sd.Nx << ", " << sd.Ny << ")!\n\n";
      exit(1);
   }

   // Subtract the mean and divide by the standard deviation
   for(int i=0; i<Nxy; i++) {
      if(is_bad_data(Data[i])    ||
         is_bad_data(mn.Data[i]) ||
         is_bad_data(sd.Data[i]) ||
         is_eq(sd.Data[i], 0.0)) {
         Data[i] = bad_data_double;
      }
      else {
         Data[i] = (Data[i] - mn.Data[i])/sd.Data[i];
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::replace_bad_data(const double value) {

   for(int i=0; i<Nxy; i++) {
      if(is_bad_data(Data[i])) Data[i] = value;
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::set_all_to_bad_data() {

   for(int i=0; i<Nxy; i++) {
      Data[i] = bad_data_double;
   }
   return;

}   

///////////////////////////////////////////////////////////////////////////////

int DataPlane::two_to_one(int x, int y, bool to_north) const {
   int n;

   if((x < 0) || (x >= Nx) || (y < 0) || (y >= Ny)) {
      mlog << Error << "\nDataPlane::two_to_one() -> "
           << "range check error: (Nx, Ny) = (" << Nx << ", " << Ny
           << "), (x, y) = (" << x << ", " << y << ")\n\n";
      exit(1);
   }

   n = (to_north ? y : (Ny-1-y))*Nx + x;    //  don't change this!  lots of downstream code depends on this!

   return n;
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

   if( s_is_on(x, y) )                                return true;

   if( (x > 0) && s_is_on(x - 1, y) )                 return true;

   if( (x > 0) && (y > 0) && s_is_on(x - 1, y - 1) )  return true;

   if( (y > 0) && s_is_on(x, y - 1) )                 return true;

   return false;
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

   return mp;
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
vector<double> new_data(Nxy);

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


void DataPlane::destagger(bool x_stag, bool y_stag)
{
    // don't do anything if data is not staggered in x or y dimension
    if (!x_stag && !y_stag) return;

    const char *method_name = "DataPlane::destagger(bool, bool) -> ";

    if ( Data.empty() )  {

        mlog << Error << "\n\n  " << method_name << " data plane is empty!\n\n";
        exit ( 1 );

    }

    int nx_new = Nx;
    int ny_new = Ny;
    int nxy_new;
    int weight = 0;
    int x, y, index_new;
    double total;
    vector<double> new_data;

    // set nx and weight based on which dimensions are staggered

    if (x_stag) {

        mlog << Debug(3) << "De-staggering dataplane in X dimension\n";
        nx_new = Nx - 1;
        weight += 2;

    }

    if (y_stag) {

        mlog << Debug(3) << "De-staggering dataplane in Y dimension\n";
        ny_new = Ny - 1;
        weight += 2;

    }

    // allocate vector to store output data

    nxy_new = nx_new * ny_new;
    new_data.resize(nxy_new);

    for (y=0; y < ny_new; y++)  {
        for (x=0; x < nx_new; x++)  {

            index_new = y*nx_new + x;

            // always add data from current grid point
            total = Data[two_to_one(x, y)];

            // add data from neighboring grid points based on staggered dimension

            if (x_stag) {
                total += Data[two_to_one(x+1,y)];
            }
            if (y_stag) {
                total += Data[two_to_one(x,y+1)];
            }

            // add diagonal point if staggered in both dimensions (may not occur)

            if (x_stag && y_stag) {
                total += Data[two_to_one(x+1,y+1)];
            }

            // divide the sum of the values by the weight to compute the average

            new_data[index_new] = total / weight;

        }
    }

    // replace data vector and size variables

    Data = new_data;
    Nx = nx_new;
    Ny = ny_new;
    Nxy = nxy_new;
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
   if (is_bad_data(Data[j])) return false;
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

return true;

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
   if (is_bad_data(Data[i])) return false;
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

return true;

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

if ( this == &a )  return *this;

assign(a);

return *this;

}

///////////////////////////////////////////////////////////////////////////////

DataPlaneArray & DataPlaneArray::operator+=(const DataPlaneArray &d) {
   const char *method_name = "DataPlaneArray::operator+=(const DataPlaneArray &) -> ";

   // Check for matching number of levels
   if(Nplanes != d.Nplanes) {
      mlog << Error << "\n" << method_name
           << "the number of levels do not match: "
           << Nplanes << " != " << d.Nplanes << "\n\n";
      exit(1);
   }

   double v;
   for(int i=0; i<Nplanes; i++) {

      // Check for matching level values
      if(Lower[i] != d.Lower[i] || Upper[i] != d.Upper[i]) {
         mlog << Error << "\n" << method_name
              << "for level " << i+1 << " the lower and upper values do not match: ("
              << Lower[i] << ", " << Upper[i] << ") != ("
              << d.Lower[i] << ", " << d.Upper[i] << ")\n\n";
         exit(1);
      }

      // Increment values for each level
      *Plane[i] += *d.Plane[i];
   }

   return *this;
}

///////////////////////////////////////////////////////////////////////////////

DataPlaneArray & DataPlaneArray::operator/=(const double v) {

   for(int i=0; i<Nplanes; i++) *Plane[i] /= v;

   return *this;
}

///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::init_from_scratch()

{

Lower = (double *) nullptr;
Upper = (double *) nullptr;

Plane = (DataPlane **) nullptr;

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

      if ( Plane[j] )  { delete Plane[j];  Plane[j] = (DataPlane *) nullptr; }

   }

   delete [] Plane;  Plane = (DataPlane **) nullptr;

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
DataPlane ** p = (DataPlane **) nullptr;
double * b = (double *) nullptr;
double * t = (double *) nullptr;

if ( ! exact )  {

   k = (n + AllocInc - 1)/AllocInc;
   n = k*AllocInc;

}

p = new DataPlane * [n];
b = new double      [n];
t = new double      [n];

for (j=0; j<n; ++j)  {

   p[j] = (DataPlane *) nullptr;

   b[j] = t[j] = 0.0;

}

if ( Plane )  {

   for (j=0; j<Nplanes; ++j)  {

      p[j] = Plane[j];

      b[j] = Lower[j];

      t[j] = Upper[j];

   }   //  for j;

   delete [] Plane;  Plane = (DataPlane **) nullptr;
   delete [] Lower;  Lower = (double *)     nullptr;
   delete [] Upper;  Upper = (double *)     nullptr;
}

Plane = p;

Lower = b;

Upper = t;

p = (DataPlane **) nullptr;
b = (double *)     nullptr;
t = (double *)     nullptr;

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

return value;

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




