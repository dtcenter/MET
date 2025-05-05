// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
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
//   000    01-11-11  Halley Gotway
//   001    09-29-22  Prestopnik     MET #2227 Remove namespace std from header files
//   002    04-18-25  Halley Gotway  MET #3132 OpenMP
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
   int nxy = nx*ny;
   
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

#pragma omp parallel default(none) \
   shared(Data, v, nx, ny, nxy, DefaultTO)
   {

      // Note: v should be a row first & the size is (nx * ny).
      //       implemented based on two_to_one("n = y*Nx + x").
#pragma omp for schedule(static)
      for(int offset=0; offset < nxy; offset++) {
         int x;
         int y;
         DefaultTO.one_to_two(nx, ny, offset, x, y);
         Data[two_to_one(x, y)] = v[offset];
      }
   } // End of omp parallel

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

void DataPlane::set_times(const DataPlane &dp) {
   InitTime  = dp.InitTime;
   ValidTime = dp.ValidTime;
   LeadTime  = dp.LeadTime;
   AccumTime = dp.AccumTime;
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

#pragma omp parallel default(none) \
   shared(Data, data, nx, ny)
   {

#pragma omp for schedule(static) \
                collapse(2)
      for(int x=0; x<nx; ++x) {
         for(int y=0; y<ny; ++y) {
            int index = two_to_one(x, y);
            Data[index] = data[index];
         }
      }
   } // End omp parallel
}

///////////////////////////////////////////////////////////////////////////////

bool DataPlane::is_all_bad_data() const {
   bool status = true;

   // Check for no valid data
   for(int j=0; j<Nxy; ++j) {
      if(!is_bad_data(Data[j])) {
         status = false;
         break;
      }
   }

   return status;
}

///////////////////////////////////////////////////////////////////////////////

int DataPlane::n_good_data() const {
   int n = 0;

#pragma omp parallel default(none) \
   shared(Data, n)
   {

      // Count number of good data values
#pragma omp for schedule(static) \
                reduction(+:n)
      for(int j=0; j<Nxy; ++j) {
         if(!is_bad_data(Data[j])) n++;
      }
   } // End omp parallel

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

   //
   // Loop through the data and apply the threshold to all valid values
   //   1.0 if it meets the threshold criteria
   //   0.0 if it does not
   //

#pragma omp parallel default(none) \
   shared(Data, Nxy, st)
   {

#pragma omp for schedule(static)
      for(int j=0; j<Nxy; ++j) {
         if(is_bad_data(Data[j])) continue;
         if(st.check(Data[j]))    Data[j] = 1.0;
         else                     Data[j] = 0.0;
      }
   } // End omp parallel

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::convert(const UserFunc_1Arg &convert_fx) {

   if(!convert_fx.is_set()) return;

   mlog << Debug(3) << "Applying conversion function.\n";
 
   for(int i=0; i<Nxy; i++) {
      if(!is_bad_data(Data[i])) Data[i] = convert_fx(Data[i]);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::censor(const ThreshArray &censor_thresh,
                       const NumArray &censor_val) {
   ThreshArray ta = censor_thresh;

   // Check for no work to do
   if(censor_thresh.n_elements() == 0) return;

   // Check for percentile thresholds
   if(ta.need_perc()) {
      NumArray d;
      d.extend(Nxy);

      // Store valid data values
      for(int i=0; i<Nxy; i++) {
         if(!is_bad_data(Data[i])) d.add(Data[i]);
      }

      ta.set_perc(&d, &d, &d, &d);
   }

   mlog << Debug(3)
        << "Applying censor thresholds \"" << ta.get_str(" ")
        << "\" and replacing with values \"" << censor_val.serialize()
        << "\".\n";

   int count = 0;

   // Loop through the points and apply all the censor thresholds
   for(int i=0; i<Nxy; i++) {
      for(int j=0; j<ta.n_elements(); j++) {

         // Break out after the first match
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

#pragma omp parallel default(none) \
   shared(Data, Nxy, mn)
   {

      // Subtract the mean
#pragma omp for schedule(static)
      for(int i=0; i<Nxy; i++) {
         if(is_bad_data(Data[i]) || is_bad_data(mn.Data[i])) {
            Data[i] = bad_data_double;
         }
         else {
            Data[i] -= mn.Data[i];
         }
      }
   } // End omp parallel

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

#pragma omp parallel default(none) \
   shared(Data, Nxy, mn, sd)
   {

      // Subtract the mean and divide by the standard deviation
#pragma omp for schedule(static)
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
   } // End omp parallel

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::replace_bad_data(const double value) {

#pragma omp parallel default(none) \
   shared(Data, Nxy, value)
   {

#pragma omp for schedule(static)
      for(int i=0; i<Nxy; i++) {
         if(is_bad_data(Data[i])) Data[i] = value;
      }
   } // End omp parallel

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::set_all_to_bad_data() {

#pragma omp parallel default(none) \
   shared(Data, Nxy)
   {

#pragma omp for schedule(static)
      for(int i=0; i<Nxy; i++) {
         Data[i] = bad_data_double;
      }
   } // End omp parallel
 
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

   //  don't change this!  lots of downstream code depends on this!
   n = (to_north ? y : (Ny-1-y))*Nx + x;

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

   // Initialize
   data_min =  1.0e30;
   data_max = -1.0e30;

#pragma omp parallel default(none) \
   shared(Data, Nxy, data_min, data_max)
   {

#pragma omp for schedule(static) \
                reduction(min: data_min) \
                reduction(max: data_max)
      for(int j=0; j<Nxy; ++j) {

         if(is_bad_data(Data[j])) continue;

         data_min = min(data_min, Data[j]);
         data_max = max(data_max, Data[j]);
      }
   } // end omp parallel

   // Check for all bad data
   if(is_eq(data_min,  1.0e30)) data_min = bad_data_double;
   if(is_eq(data_max, -1.0e30)) data_max = bad_data_double;

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

#pragma omp parallel default(none) \
   shared(Data, Nxy, mp)
   {

#pragma omp for schedule(static)
      for(int i=0; i<Nxy; i++) {
         mp.buf()[i] = (is_bad_data(Data[i]) ? false : !is_eq(Data[i], 0.0));
      }
   } // End omp parallel

   return mp;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::shift_right(int N) {
   const char *method_name = "DataPlane::shift_right(int) -> ";

   mlog << Debug(3) << "Shifting dataplane to the right " << N
        << " grid squares.\n";

   // Check some stuff
   if(Data.empty()) {
      mlog << Error << "\n" << method_name
           << "data plane is empty!\n\n";
      exit(1);
   }

   N %= Nx;

   if(N < 0) N += Nx;

   if(N == 0) return;   //  no shift, so do nothing

   //
   //  ok, get to work
   //

   vector<double> new_data(Nxy);

#pragma omp parallel default(none)   \
   shared(Data, new_data, N, Nx, Ny)
   {

#pragma omp for schedule(static)
      for(int x=0; x<Nx; ++x) {

         int x_new = (x + N)%Nx;

         for(int y=0; y<Ny; ++y) {
            int index_old = two_to_one(x,     y);
            int index_new = two_to_one(x_new, y);
            new_data[index_new] = Data[index_old];
         }
      }
   } // End omp parallel

   Data = new_data;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::destagger(bool x_stag, bool y_stag) {

   // don't do anything if data is not staggered in x or y dimension
   if(!x_stag && !y_stag) return;

   const char *method_name = "DataPlane::destagger(bool, bool) -> ";

   if(Data.empty()) {
      mlog << Error << "\n" << method_name
           << " data plane is empty!\n\n";
      exit(1);
   } 

   int nx_new = Nx;
   int ny_new = Ny;
   int weight = 0;

   // set nx and weight based on which dimensions are staggered
   if(x_stag) {
      mlog << Debug(3) << "De-staggering dataplane in X dimension\n";
      nx_new = Nx - 1;
      weight += 2;
   }

   if(y_stag) {
      mlog << Debug(3) << "De-staggering dataplane in Y dimension\n";
      ny_new = Ny - 1;
      weight += 2;
   }

   // allocate vector to store output data
   int nxy_new = nx_new * ny_new;
   vector<double> new_data(nxy_new);

#pragma omp parallel default(none) \
   shared(Data, new_data, ny_new, nx_new, x_stag, y_stag, weight)
   {

#pragma omp for schedule(static) \
                collapse(2)
      for(int y=0; y<ny_new; y++) {
         for(int x=0; x<nx_new; x++) {

            int index_new = y*nx_new + x;

            // always add data from current grid point
            double total = Data[two_to_one(x, y)];

            // add data from neighboring grid points based on staggered dimension
            if(x_stag) total += Data[two_to_one(x+1,y)];
            if(y_stag) total += Data[two_to_one(x,y+1)];

            // add diagonal point if staggered in both dimensions (may not occur)
            if(x_stag && y_stag) total += Data[two_to_one(x+1,y+1)];

            // divide the sum of the values by the weight to compute the average
            new_data[index_new] = total / weight;
         }
      }
   } // End omp parallel

   // Replace data vector and size variables
   Data = new_data;
   Nx = nx_new;
   Ny = ny_new;
   Nxy = nxy_new;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::put(const double value, const int x, const int y) {

   if(Data.empty()) {
      mlog << Error << "\nDataPlane::put() -> "
           << "no data plane allocated!\n\n";
      exit(1);
   }

   //  the two_to_one function does range checking on x and y
   const int n = two_to_one(x, y);

   Data[n] = value;

   return;
}

///////////////////////////////////////////////////////////////////////////////

bool DataPlane::fitwav_1d(const int start_wave, const int end_wave) {
   const int unsigned mnw = (Nx + 1)/2;

   // Check for bad data
   for(int i=0; i<Nxy; ++i) {
      if(is_bad_data(Data[i])) return false;
   }

   // Range check the requested wave numbers
   if(start_wave < 0   || end_wave < 0 ||
      start_wave > mnw || end_wave > mnw) {
      mlog << Error << "\nDataPlane::fitwav_1d() -> "
           << "Requested wave numbers (" << start_wave << " to " << end_wave
           << ") must be between 0 and " << mnw << " for data with dimension "
           << "(Nx, Ny) = (" << Nx << ", " << Ny << ")!\n\n";
      exit(1);
   }

   // Working vectors
   vector<double> C(Nx);
   vector<double> S(Nx);

#pragma omp parallel default(none) \
shared(Nx, Ny, mnw, start_wave, end_wave, C, S)
   {

#pragma omp for schedule(static)
      for(int x=0; x<Nx; ++x) {
         double angle = (twopi*x)/Nx;
         C[x] = cos(angle);
         S[x] = sin(angle);
      } // for x

#pragma omp for schedule(static)
      for(int y=0; y<Ny; ++y) {
         double xa0 = 0.0;
         for(int x=0; x<Nx; ++x) {
            xa0 += get(x, y);
         } // for x

         // Working vectors
         vector<double> a (mnw + 1);
         vector<double> b (mnw + 1);
         vector<double> xa(mnw + 1);
         vector<double> xb(mnw + 1);

         a[0] = xa0/Nx;
         b[0] = 0.0;

         for(int i=1; i<=mnw; ++i) {
            xa[i] = xb[i] = 0.0;
            for(int x=0; x<Nx; ++x)  {
               int m = (i*x)%Nx;
               xa[i] += (get(x, y))*(C[m]);
               xb[i] += (get(x, y))*(S[m]);
            } // for x

            a[i] = (2.0*xa[i])/Nx;
            b[i] = (2.0*xb[i])/Nx;

         } // for i

         for(int x=0; x<Nx; ++x) {
            double value = 0.0;
            for(int i=start_wave; i<=end_wave; ++i) {
               int m = (i*x)%Nx;
               value += (a[i])*(C[m]);
               value += (b[i])*(S[m]);
            } // for i

            put(value, x, y);

         } // for x
      } // for y 
   } // End omp parallel

   return true;
}

///////////////////////////////////////////////////////////////////////////////
//
// Begin Code for class DataPlaneArray
//
///////////////////////////////////////////////////////////////////////////////

DataPlaneArray::DataPlaneArray() {
   init_from_scratch();
}

///////////////////////////////////////////////////////////////////////////////

DataPlaneArray::~DataPlaneArray() {
   clear();
}

///////////////////////////////////////////////////////////////////////////////

DataPlaneArray::DataPlaneArray(const DataPlaneArray & a) {
   init_from_scratch();
   assign(a);
}

///////////////////////////////////////////////////////////////////////////////

DataPlaneArray & DataPlaneArray::operator=(const DataPlaneArray & a) {
   if(this == &a) return *this;
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

#pragma omp parallel default(none) \
   shared(mlog, Error, method_name) \
   shared(Nplanes, Lower, Upper, Plane, d)
   {

#pragma omp for schedule(static)
      for(int i=0; i<Nplanes; i++) {

         // Check for matching level values
         if(Lower[i] != d.Lower[i] || Upper[i] != d.Upper[i]) {
            mlog << Error << "\n" << method_name
                 << "for level " << i+1
                 << " the lower and upper values do not match: ("
                 << Lower[i] << ", " << Upper[i] << ") != ("
                 << d.Lower[i] << ", " << d.Upper[i] << ")\n\n";
            exit(1);
         }

         // Increment values for each level
         Plane[i] += d.Plane[i];
      }
   } // End omp parallel

   return *this;
}

///////////////////////////////////////////////////////////////////////////////

DataPlaneArray & DataPlaneArray::operator/=(const double v) {

#pragma omp parallel default(none) \
   shared(Nplanes, Plane, v)
   {

#pragma omp for schedule(static)
      for(int i=0; i<Nplanes; i++) Plane[i] /= v;

   } // End omp parallel

   return *this;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlaneArray::init_from_scratch() {

   clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlaneArray::clear() {

   Lower.clear();
   Upper.clear();
   Plane.clear();
   Nplanes = 0;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlaneArray::assign(const DataPlaneArray & a) {

   Lower   = a.Lower;
   Upper   = a.Upper;
   Plane   = a.Plane;
   Nplanes = a.Nplanes;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlaneArray::extend(int n) {

   if(n <= Nplanes)  return;

   Lower.reserve(n);
   Upper.reserve(n);
   Plane.reserve(n);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlaneArray::add(const DataPlane & p, double _low, double _up) {

   check_xy_size(p);

   Lower.emplace_back(_low);
   Upper.emplace_back(_up);
   Plane.emplace_back(p);

   Nplanes++;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlaneArray::check_xy_size(const DataPlane & p) const {

   if(Nplanes == 0) return;

   if((p.nx() != Plane[0].nx()) || (p.ny() != Plane[0].ny())) {
      mlog << Error << "\nDataPlaneArray::check_xy_size(const DataPlane &) const -> "
           << "(Nx, Ny) dimensions does not match ("
           << p.nx() << ", " << p.ny() << ") != ("
           << Plane[0].nx() << ", " << Plane[0].ny() << ")!\n\n";
      exit(1);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

double DataPlaneArray::data(int p, int x, int y) const {

   // Range check
   if((p < 0) || (p >= Nplanes)) {
      mlog << Error << "\nDataPlaneArray::data() -> "
           << "range check error for " << p << " in (0, " << Nplanes
           << ")!\n\n";
      exit(1);
   }

   return Plane[p].get(x,y);
}

///////////////////////////////////////////////////////////////////////////////

void DataPlaneArray::set(double v, int p, int x, int y) {

   // Range check
   if((p < 0) || (p >= Nplanes)) {
      mlog << Error << "\nDataPlaneArray::set() -> "
           << "range check error for " << p << " in (0, " << Nplanes
           << ")!\n\n";
      exit(1);
   }

   Plane[p].set(v, x, y);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlaneArray::set_levels(int n, double _low, double _up) {

   // Range check
   if((n < 0) || (n >= Nplanes)) {
      mlog << Error << "\nDataPlaneArray::set_levels() -> "
           << "range check error for " << n << " in (0, " << Nplanes
           << ")!\n\n";
      exit(1);
   }

   if(_low > _up) {
      mlog << Error << "\nDataPlaneArray::set_levels() -> "
           << "low level (" << _low << ") > up level (" << _up << ")!\n\n";
      exit(1);
   }

   Lower[n] = _low;
   Upper[n] = _up;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlaneArray::levels(int n, double & _low, double & _up) const {

   // Range check
   if((n < 0) || (n >= Nplanes)) {
      mlog << Error << "\nDataPlaneArray::levels() -> "
           << "range check error for " << n << " in (0, " << Nplanes
           << ")!\n\n";
      exit(1);
   }

   _up  = Upper [n];
   _low = Lower [n];

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlaneArray::level_range(double & _low, double & _up) const {

   _low = _up = bad_data_int;

   for(int j=0; j<Nplanes; ++j)  {
      if(is_bad_data(_low) || Lower[j] <= _low) _low = Lower[j];
      if(is_bad_data(_up)  || Upper[j] >= _up ) _up  = Upper[j];
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

int DataPlaneArray::nx() const {

   // Range check
   if(Nplanes == 0) {
      mlog << Error << "\nDataPlaneArray::nx() const -> "
           << "array is empty!\n\n";
      exit(1);
   }

   return Plane[0].nx();
}


///////////////////////////////////////////////////////////////////////////////

int DataPlaneArray::ny() const {

   // Range check
   if(Nplanes == 0) {
      mlog << Error << "\nDataPlaneArray::ny() const -> "
           << "array is empty!\n\n";
      exit(1);
   }

   return Plane[0].ny();
}

///////////////////////////////////////////////////////////////////////////////

void DataPlaneArray::dump(ostream & out, int depth) const {

   Indent prefix(depth);

   out << prefix << "Nplanes  = " << Nplanes                    << '\n';
   out << prefix << "Nx       = " << ((Nplanes > 0) ? nx() : 0) << '\n';
   out << prefix << "Ny       = " << ((Nplanes > 0) ? ny() : 0) << '\n';

   for(int j=0; j<Nplanes; ++j)  {
      out << prefix << "Level " << j << "  = "
          << '[' << Lower[j] << ", " << Upper[j] << ']'
          << '\n';
   } // for j

   out.flush();

   return;
}

///////////////////////////////////////////////////////////////////////////////

double DataPlaneArray::lower(int n) const {

   // Range check
   if((n < 0) || (n >= Nplanes)) {
      mlog << Error << "\nDataPlaneArray::lower() -> "
           << "range check error for " << n << " in (0, " << Nplanes
           << ")!\n\n";
      exit(1);
   }

   return Lower[n];
}

///////////////////////////////////////////////////////////////////////////////

double DataPlaneArray::upper(int n) const {

   // Range check
   if((n < 0) || (n >= Nplanes)) {
      mlog << Error << "\nDataPlaneArray::upper() -> "
           << "range check error for " << n << " in (0, " << Nplanes
           << ")!\n\n";
      exit(1);
   }

   return Upper[n];
}

///////////////////////////////////////////////////////////////////////////////

const DataPlane & DataPlaneArray::operator[](int n) const {

   // Range check
   if((n < 0) || (n >= Nplanes)) {
      mlog << Error << "\nDataPlaneArray::operator[](int) const -> "
           << "range check error for " << n << " in (0, " << Nplanes
           << ")!\n\n";
      exit(1);
   }

   return Plane[n];
}

///////////////////////////////////////////////////////////////////////////////

DataPlane & DataPlaneArray::at(int n) {

   // Range check
   if((n < 0) || (n >= Nplanes)) {
      mlog << Error << "\nDataPlaneArray::at(int) const -> "
           << "range check error for " << n << " in (0, " << Nplanes
           << ")!\n\n";
      exit(1);
   }

   return Plane[n];
}

///////////////////////////////////////////////////////////////////////////////

void DataPlaneArray::replace_bad_data(const double value) {

   for(int j=0; j<Nplanes; ++j)  {
      Plane[j].replace_bad_data(value);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////
