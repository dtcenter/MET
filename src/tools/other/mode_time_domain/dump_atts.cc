

////////////////////////////////////////////////////////////////////////


static const int default_min_volume          = 1000;

static const double default_min_intensity    = 0.0;

static const char default_output_filename [] = "att.out";


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
#include "vx_math.h"
#include "mtd_file.h"
#include "partition.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static CommandLine cline;

static ConcatString output_filename = default_output_filename;

static int min_volume = default_min_volume;

static double min_intensity = default_min_intensity;

static ofstream out;


////////////////////////////////////////////////////////////////////////


static void set_outfile       (const StringArray &);
static void set_min_volume    (const StringArray &);
static void set_min_intensity (const StringArray &);

static void usage();

static void process(const char * mtd_filename);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

cline.set(argc, argv);

cline.set_usage(usage);

cline.add(set_outfile,       "-outfile",       1);
cline.add(set_min_volume,    "-min_volume",    1);
cline.add(set_min_intensity, "-min_intensity", 1);

cline.parse();

if ( cline.n() == 0 )  usage();

int j, k;
int max_len;

max_len = cline.max_length() + 3;


out.open(output_filename);

if ( !out )  {

   cerr << "\n\n  " << program_name << ": unable to open output file \"" << output_filename << "\"\n\n";

   exit ( 1 );

}

cout << "\n\n";

for (j=0; j<(cline.n()); ++j)  {

   cout << "Processing " << cline[j] << ' ';

   for (k=(cline.length(j)); k<max_len; ++k)  cout << '.';

   cout << ' ' << (j + 1) << " of " << cline.n() << "\n";

   if ( (j%5) == 4 )  cout << '\n';

   cout.flush();

   process(cline[j]);

}






   //
   //  done
   //

out.close();

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

cerr << "\n\n   usage:  " << program_name << " [ -outfile path ] [ -min_volume n ] [ -min_intensity value ] mtd_file_list\n\n"

     << "     where\n\n"

     << " the output filename (outfile) defaults to \"" << default_output_filename << "\" if not specified,\n\n"

     << " the minimum intensity threshold (min_intensity) defaults to " << default_min_intensity << " if not specified,\n\n"

     << " the minimum volume threshold (min_volume) defaults to " << default_min_volume << " if not specified.\n\n"

     << "\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void set_outfile (const StringArray & a)

{

output_filename = a[0];

return;

}


////////////////////////////////////////////////////////////////////////


void set_min_volume (const StringArray & a)

{

min_volume = atoi(a[0]);

if ( min_volume < 0 )  {

   cerr << "\n\n  " << program_name << ": bad value for min volume: " << min_volume << "\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////


void set_min_intensity (const StringArray & a)

{

min_intensity = atof(a[0]);

if ( min_intensity < 0 )  {

   cerr << "\n\n  " << program_name << ": bad value for min intensity: " << min_intensity << "\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////


void process(const char * mtd_filename)

{

int j;
int t;
int r, c;
int volume, bbox_volume;
int xmin, xmax, ymin, ymax, tmin, tmax;
double xbar, ybar, tbar;
double vx, vy, ratio;
double max_intensity;
ModeTimeDomainFile tdf;
ModeTimeDomainFile f;
char junk[256];
AsciiTable table;
bool status = false;



if ( !(tdf.read(mtd_filename)) )  {

   cerr << "\n\n  " << program_name << ": process() -> unable to open mtd file \"" << mtd_filename << "\"\n\n";

   exit ( 1 );

}

out << "\n\n"
    << "File       = " << get_short_name(mtd_filename) << '\n'
    << "Radius     = " << (tdf.radius()) << '\n'
    << "Threshold  = " << (tdf.threshold()) << '\n'
    << "Min Volume = " << min_volume << '\n';

tdf.toss_small_objects     (min_volume);
tdf.toss_small_intensities (min_intensity);


out << "# objects = " << tdf.n_objects() << '\n';

if ( tdf.n_objects() == 0 )  return;

table.set_size(tdf.n_objects(), 12 + 2*(tdf.nt() - 1));

table.set_ics(3);


for (j=0; j<(tdf.n_objects()); ++j)  {

   r = j;

   c = 0;

   // f = select(tdf, j);
   f = select(tdf, j + 1);

      //
      //  max conv intensity
      //

   max_intensity = tdf.max_conv_intensity(j);

   sprintf(junk, "%.2f", max_intensity);

   table.set_entry(r, c++, junk);

      //
      //  volume
      //

   volume = tdf.volume(j);

   table.set_entry(r, c++, volume);

      //
      //  centroid
      //

   f.centroid(xbar, ybar, tbar);

   sprintf(junk, "%.2f", xbar);
   table.set_entry(r, c++, junk);

   sprintf(junk, "%.2f", ybar);
   table.set_entry(r, c++, junk);

   sprintf(junk, "%.2f", tbar);
   table.set_entry(r, c++, junk);

      //
      //  bounding box
      //

   f.bbox(xmin, xmax, ymin, ymax, tmin, tmax);

   table.set_entry(r, c++, xmin);
   table.set_entry(r, c++, xmax);

   table.set_entry(r, c++, ymin);
   table.set_entry(r, c++, ymax);

   table.set_entry(r, c++, tmin);
   table.set_entry(r, c++, tmax);

      //
      //  complexity
      //

   bbox_volume = (xmax - xmin + 1)*(ymax - ymin + 1)*(tmax - tmin + 1);

   ratio = ((double) volume)/((double) bbox_volume);

   sprintf(junk, "%.3f", ratio);

   table.set_entry(r, c++, junk);

      //
      //  centroid velocity
      //

   for (t=0; t<(f.nt() - 1); ++t)  {

      status = f.centroid_velocity(t, vx, vy);

      if ( !status )  vx = vy = -9999.0;

      sprintf(junk, "%.2f", vx);

      table.set_entry(r, c++, junk);

      sprintf(junk, "%.2f", vy);

      table.set_entry(r, c++, junk);

   }

}   //  for j

   //
   //  write the table
   //

out << "\n\n";

out << table;

out << "\n\n";





   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////




