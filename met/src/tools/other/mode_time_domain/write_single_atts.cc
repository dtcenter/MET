

////////////////////////////////////////////////////////////////////////


static const char output_filename [] = "single_atts.out";


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "vx_util.h"
#include "mtd_file.h"
#include "att.h"
#include "caps_data_dir.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Default values for command-line switches
   //


static ConcatString output_directory = ".";


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static CommandLine cline;


////////////////////////////////////////////////////////////////////////


static void usage();

static void set_outdir(const StringArray &);

static void process(const char * filename, ostream &);

static void do_object(const ModeTimeDomainFile & f, Unixtime start_time, 
                      const ConcatString & model, int object_number, 
                      int radius, double threshold, 
                      const ModeTimeDomainFile & raw, 
                      ostream & out);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

cline.set(argc, argv);

cline.set_usage(usage);

cline.add(set_outdir, "-outdir", 1);

cline.parse();

if ( cline.n() == 0 )  usage();

   //
   //  open output file
   //

int j;
ConcatString path;
ofstream out;


path << output_directory << '/' << output_filename;

out.open(path);

if ( !out )  {

   cerr << "\n\n  " << program_name << ": unable to open output file \"" << path << "\"\n\n";

   exit ( 1 );

}

   //
   //  process input files
   //

for (j=0; j<(cline.n()); ++j)  {

   if ( (j%5) == 0 )  cout.put('\n');

   cout << "Processing object file \"" 
        << get_short_name(cline[j]) 
        << "\" ... " << (j + 1) << " of " << cline.n() << "\n";
 
   cout.flush();

   process(cline[j], out);

}

   //
   //  close output file
   //

out.close();

   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

cerr << "\n\n   usage:  " << program_name << " [ -outdir path ] obj_file_list\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void set_outdir(const StringArray & a)

{

output_directory = a[0];

return;

}


////////////////////////////////////////////////////////////////////////


void process(const char * filename, ostream & out)

{

int j, n;
int n_shapes;
ModeTimeDomainFile f;
ModeTimeDomainFile s, b;
ModeTimeDomainFile raw;
ConcatString model;
int month, day, year, hour, minute, second;
char junk[256];
const char * const short_name = get_short_name(filename);
char lead_char;
ConcatString raw_filename;


if ( !(f.read(filename)) )  {

   cerr << "\n\n  " << program_name << ": process() -> unable to open input file \"" << filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  get model
   //

model.erase();

n = strlen(short_name);

for (j=0; j<n; ++j)  {

   if ( short_name[j] == '_' )  break;

   model.add(short_name[j]);

}

   //
   //  load raw file
   //

unix_to_mdyhms(f.start_time(), month, day, year, hour, minute, second);

if ( strcmp(model, "obs") == 0 )  {

   lead_char = 'L';

   hour = 1;

} else {

   lead_char = 'I';

   hour = 0;

}

sprintf(junk, "%04d%02d%02d_%02d%c", year, month, day, hour, lead_char);

raw_filename << cs_erase
             << caps_3D_data_dir << '/'
             << "3d_" << model << '/'
             << model << '_'
             << junk << ".nc";


if ( ! (raw.read(raw_filename)) )  {

   cerr << "\n\n  " << program_name << ": process() -> can't open raw file \"" << raw_filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  split
   //

s = split(f, n_shapes);

n = 0;

for (j=0; j<n_shapes; ++j)  {

   b = select(s, j + 1);

   // if ( b.volume() < min_volume )  continue;

   ++n;

   if ( (n%25) == 0 )  cout << "   object " << n << " of " << n_shapes << '\n' << flush;

   do_object(b, f.start_time(), model, n, f.radius(), f.threshold(), raw, out);

}

   //
   //  done
   //


return;

}


////////////////////////////////////////////////////////////////////////


void do_object(const ModeTimeDomainFile & f, Unixtime start_time, 
               const ConcatString & model, int object_number, 
               int radius, double threshold, 
               const ModeTimeDomainFile & raw, 
               ostream & out)

{

int month, day, year, hour, minute, second;
char junk[512];
SingleAttributes a;


   //
   //  calc attributes
   //

a = calc_single_atts(f, raw, model, object_number);

   //
   //  object number
   //

sprintf(junk, "%4d", a.object_number());

out << junk << ' ';

   //
   //  start time
   //

unix_to_mdyhms(f.start_time(), month, day, year, hour, minute, second);

sprintf(junk, "%04d %02d %02d %02d", year, month, day, hour);

out << junk << ' ';

   //
   //  model
   //

sprintf(junk, "%6s", model.text());

out << junk << ' ';

   //
   //  radius, threshold
   //

sprintf(junk, "%4d", radius);

out << junk << ' ';

sprintf(junk, "%6.2f", threshold);

out << junk << ' ';

   //
   //  volume
   //

sprintf(junk, "%10d", a.volume());

out << junk << ' ';

   //
   //  centroid
   //

sprintf(junk, "%8.2f  %8.2f  %8.2f", a.xbar(), a.ybar(), a.tbar());

out << junk << ' ';

   //
   //  bounding box
   //

sprintf(junk, "%5d  %5d  %5d  %5d  %5d  %5d", 
               a.xmin(), a.xmax(),
               a.ymin(), a.ymax(),
               a.tmin(), a.tmax());

out << junk << ' ';

   //
   //  velocity
   //

sprintf(junk, "%7.2f  %7.2f", a.xdot(), a.ydot());

out << junk << ' ';

   //
   //  percentiles
   //

sprintf(junk, "%7.3f  %7.3f  %7.3f  %7.3f  %7.3f", 
               a.ptile_10(), 
               a.ptile_25(), 
               a.ptile_50(), 
               a.ptile_75(), 
               a.ptile_90());

out << junk << ' ';


   //
   //  done
   //

out << '\n' << flush;

return;

}


////////////////////////////////////////////////////////////////////////




