

////////////////////////////////////////////////////////////////////////


static const bool verbose = true;


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <cmath>

#include "vx_cal.h"
#include "vx_util.h"

#include "mtd_file.h"
#include "mtd_nc_defs.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Default values for command-line switches
   //

static double Threshold = -1.0;

static ConcatString output_directory = ".";


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static CommandLine cline;


////////////////////////////////////////////////////////////////////////


static void usage();

static void set_thresh (const StringArray &);
static void set_outdir (const StringArray &);

static void process(const char *);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

cline.set(argc, argv);

cline.set_usage(usage);

cline.add(set_thresh,    "-thresh",    1);
cline.add(set_outdir,    "-outdir",    1);

cline.parse();

if ( cline.n() == 0 )  usage();

if ( Threshold < 0.0 )  usage();

int j;


for (j=0; j<(cline.n()); ++j)  {

   cout << "processing " << (cline[j]) << " at threshold " << Threshold << "\n" << flush;

   process(cline[j]);

}



   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

cerr << "\n\n   usage:  " << program_name << " -thresh value [ -outdir path ] conv_3d_file_list\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void set_thresh(const StringArray & a)

{

Threshold = atof(a[0]);

if ( Threshold < 0.0 )  {

   cerr << "\n\n  " << program_name << ": bad threshold ... " << Threshold << "\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////


void set_outdir(const StringArray & a)

{

output_directory = a[0];

return;

}


////////////////////////////////////////////////////////////////////////


void process(const char * input_filename)

{

int x, y, t, j, n;
double value;
ModeTimeDomainFile in, out;
ConcatString output_filename;
ConcatString s;
char junk[256];
double * m = (double *) 0;


   //
   //  open the input file
   //

if ( ! in.read(input_filename) )  {

   cerr << "\n\n  " << program_name << ": process() -> unable to open input file \"" << input_filename << "\"\n\n";

   exit ( 1 );

}

const int nx = in.nx();
const int ny = in.ny();
const int nt = in.nt();

out = in;

out.do_threshold(Threshold);

out.split();

   //
   //  fill in the max conv intensities
   //

m = new double [out.n_objects()];

for (j=0; j<(out.n_objects()); ++j)  m[j] = 0.0;

for (x=0; x<nx; ++x)  {

   for (y=0; y<ny; ++y)  {

      for (t=0; t<nt; ++t)  {

         n = out.ival(x, y, t);

         if ( n == 0 )  continue;

         --n;

         value = in.dval(x, y, t);

         if ( value > m[n] )  m[n] = value;

      }

   }

}

out.set_max_conv_intensities(m);

   //
   //  make the output filename
   //

s = get_short_name(input_filename);

s.chomp(mtd_file_suffix);

sprintf(junk, "%.2fT", Threshold);

s << '.' << junk << mtd_file_suffix;

output_filename << cs_erase << output_directory << '/' << s;

   //
   //  write the file
   //

if ( ! out.write(output_filename) )  {

   cerr << "\n\n  " << program_name << ": process() -> trouble writing output file \"" << output_filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  done
   //

if ( m )  { delete [] m;   m = (double *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////






