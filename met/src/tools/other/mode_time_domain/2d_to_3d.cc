

////////////////////////////////////////////////////////////////////////


static const bool verbose = true;


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <cmath>

#include "vx_util.h"

#include "mode_2d_input_file.h"
#include "mtd_file.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Default values for command-line switches
   //

static ConcatString output_filename;

static ConcatString field_name = "APCP_01";


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static CommandLine cline;

static Grid G;

static bool got_grid = false;

static Unixtime start_time;

static int delta_t;   //  seconds

static bool outfile_set   = false;

static StringArray input_filenames;

static double total_data_min =  1.0e30;
static double total_data_max = -1.0e30;

static int Nx = 0;
static int Ny = 0;


////////////////////////////////////////////////////////////////////////


static void usage();

static void set_outfile   (const StringArray &);
static void set_field     (const StringArray &);

static void check_times();

static bool is_arith_prog(const Unixtime *, const int n);

static Unixtime get_valid_time(const char * mode_2d_filename);

static void open_input(Mode2DInputFile &, const char * path);

static void do_mtd_file();

static ConcatString timestring(const Unixtime);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

cline.set(argc, argv);

cline.set_usage(usage);

cline.add(set_outfile,   "-outfile",   1);
cline.add(set_field,     "-field",     1);

cline.parse();

if ( cline.n() == 0 )  usage();

if ( !outfile_set )  usage();

int j;

for (j=0; j<(cline.n()); ++j)  {

   input_filenames.add(cline[j]);

}

if ( input_filenames.n_elements() < 3 )  {

   cerr << "\n\n  " << program_name <<": gotta have at least 3 input files\n\n";

   exit ( 1 );

}

check_times();

do_mtd_file();


   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

cerr << "\n\n   usage:  " << program_name << " -outfile path [ -field name ] file_list\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void set_outfile(const StringArray & a)

{

output_filename = a[0];

outfile_set = true;

return;

}


////////////////////////////////////////////////////////////////////////


void set_field(const StringArray & a)

{

field_name = a[0];

return;

}


////////////////////////////////////////////////////////////////////////


void check_times()

{

int j;
Unixtime * valid_times;
const int Nt = input_filenames.n_elements();
bool status = false;


valid_times = new Unixtime [Nt];

for (j=0; j<Nt; ++j)  { valid_times[j] = (Unixtime) 0; }


   //
   //  get the valid times
   //

for (j=0; j<Nt; ++j)  {

   valid_times[j] = get_valid_time(input_filenames[j]);

}

start_time = valid_times[0];

delta_t = (int) (valid_times[1] - valid_times[0]);

   //
   //  check that they form an arithmetic progression
   //

status = is_arith_prog(valid_times, Nt);

if ( !status )  {

   ConcatString s;

   cerr << "\n\n  " << program_name << ": valid times do not form an arithmetic progression\n\n";

   for (j=0; j<Nt; ++j)  {

      s = timestring(valid_times[j]);

      cerr << s << '\n';

   }

   exit ( 1 );

}

   //
   //  done
   //

if ( valid_times )  { delete [] valid_times;  valid_times = (Unixtime *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////


bool is_arith_prog(const Unixtime * t, const int n)

{

if ( n < 3 )  return ( false );

int j;
Unixtime d;
const Unixtime delta = t[1] - t[0];


if ( delta <= 0 )  return ( false );

for (j=1; j<(n - 1); ++j)  {

   d = t[j + 1] - t[j];

   if ( d != delta )  return ( false );

}


   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


Unixtime get_valid_time(const char * mode_2d_filename)

{

Mode2DInputFile f;

if ( ! f.open(mode_2d_filename, field_name) )  {

   cerr << "\n\n  " << program_name << ": get_valid_time() -> unable to open mode 2d file \"" << mode_2d_filename << "\"\n\n";

   exit ( 1 );

}

Unixtime valid = f.valid_time();

   //
   //  check grid
   //

if ( got_grid )  {

   if ( ! f.same_grid(G) )  {

      cerr << "\n\n  " << program_name << ": get_valid_time() -> grid not same in input file \"" << mode_2d_filename << "\"\n\n";

      exit ( 1 );

   }

} else {

   G = f.grid();

   Nx = G.nx();
   Ny = G.ny();

   got_grid = true;

}

   //
   //  data min/max values
   //

total_data_min = min(total_data_min, f.data_min());
total_data_max = max(total_data_max, f.data_max());

   //
   //  done
   //

f.close();

return ( valid );

}


////////////////////////////////////////////////////////////////////////


void open_input(Mode2DInputFile & f, const char * path)

{

f.close();

bool status = false;

status = f.open(path, field_name);

if ( !status )  {

   cerr << "\n\n  open_input() -> unable to open input file \"" << path << "\"\n\n";

   exit ( 1 );

}


return;

}


////////////////////////////////////////////////////////////////////////


void do_mtd_file()

{

int x, y, t;
// int bytes;
double value, denom;
double m, b;
double min_value, max_value;
ModeTimeDomainFile out;
const int Nt = input_filenames.n_elements();
Mode2DInputFile in;


   //
   //  setup the output file
   //

// bytes = (int) sizeof(MtdDataType);

denom = (double) (mtd_max_ival);

m = (total_data_max - total_data_min)/denom;

b = total_data_min;


out.set_size(Nx, Ny, Nt);

out.set_grid(G);

out.set_radius(-1);

out.set_threshold(-1.0);

out.set_start_time(start_time);

out.set_delta_t(delta_t);

out.set_mb(m, b);


   //
   //  get the data values
   //

min_value =  1.0e30;
max_value = -1.0e30;

for (t=0; t<Nt; ++t)  {

   if ( verbose )  cout << "opening input file \"" << input_filenames[t] << "\"\n" << flush;

   open_input(in, input_filenames[t]);

   for (x=0; x<Nx; ++x)  {

      for (y=0; y<Ny; ++y)  {

         value = in(x, y);

         min_value = min(min_value, value);
         max_value = max(max_value, value);

         out.put_dval(value, x, y, t);

      }   //  for y

   }   //  for x

   in.close();

}   //  for t

if ( verbose )  cout << "\n   Data range is " << min_value << " to " << max_value << "\n\n" << flush;

   //
   //  write the file
   //

if ( ! out.write(output_filename) )  {

   cerr << "\n\n  " << program_name << ": do_mtd_file() -> trouble writing output file \"" << output_filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString timestring(const Unixtime t)

{

int month, day, year, hour, minute, second;
char junk[256];
ConcatString s;

unix_to_mdyhms(t, month, day, year, hour, minute, second);

sprintf(junk, "%s %2d, %4d  %02d:%02d:%02d", short_month_name[month], day, year, hour, minute, second);

   //
   //  done
   //

s = junk;

return ( s );

}


////////////////////////////////////////////////////////////////////////




