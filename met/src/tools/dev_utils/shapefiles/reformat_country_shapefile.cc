

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>
#include <cmath>

#include "vx_util.h"

#include "shp_file.h"
#include "dbf_file.h"


////////////////////////////////////////////////////////////////////////

   //
   //  constants
   //

static const char country_name_tag    [] = "CNTRY_NAME";
static const char   admin_name_tag    [] = "ADMIN_NAME";
static const int max_records             = 100;

   //
   //  command line arguments
   //

static ConcatString shp_filename;
static ConcatString dbf_filename;
static ConcatString target_country_name;


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

struct RecInfo {

   int rec_num;

   ConcatString admin;

};

static RecInfo records[max_records];

static int n_records = 0;

static const int buf_size = 500000;

static unsigned char buf[buf_size];


////////////////////////////////////////////////////////////////////////


static int get_index(const int recnum);

static void write_record(ostream &, const ShpPolygonRecord &, const int recnum, const ConcatString & admin);

static void write_part(ostream & out, const ShpPolygonRecord & gr, const int recnum, const ConcatString & admin, const int part);


////////////////////////////////////////////////////////////////////////


static void usage();


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

if ( argc != 4 )  usage();

int fd = -1;
int j, n_read, bytes;
DbfHeader dh;
DbfSubRecord * country_name_rec = 0;
DbfSubRecord *   admin_name_rec = 0;
DbfSubRecord * r = 0;
ConcatString s;
char junk[512];

shp_filename        = argv[1];
dbf_filename        = argv[2];
target_country_name = argv[3];

   //
   //  get records
   //

n_records = 0;

if ( (fd = open(dbf_filename, O_RDONLY)) < 0 )  {

   cerr << "\n\n  " << program_name << ": unable to open dbf file \""
        << dbf_filename << "\"\n\n";

   exit ( 1 );

}

bytes = 32;

if ( (n_read = read(fd, buf, bytes)) != bytes )  {

   cerr << "\n\n  " << program_name << ": trouble reading main file header from input_file "
        << dbf_filename << "\"\n\n";

   exit ( 1 );

}

dh.set_header(buf);

dh.set_subrecords(fd);

country_name_rec = dh.lookup_subrec(country_name_tag);
  admin_name_rec = dh.lookup_subrec(admin_name_tag);

if ( !country_name_rec || !admin_name_rec )  {

   cerr << "\n\n  lookups failed\n\n";

   exit ( 1 );

}

if ( lseek(fd, dh.pos_first_record, SEEK_SET) < 0 )  {

   cerr << "\n\n  " << program_name << ": lseek error\n\n";

   exit ( 1 );

}

for (j=0; j<(dh.n_records); ++j)  {

   bytes = dh.record_length;

   n_read = read(fd, buf, bytes);

   if ( n_read != bytes )  {

      cerr << "\n\n  " << program_name << ": read error ... n_read = " << n_read << "\n\n";

      exit ( 1 );

   }

   buf[dh.record_length] = 0;

   r = country_name_rec;

   substring((const char *) buf, junk, r->start_pos, r->start_pos + r->field_length - 1);

   s = junk;

   s.ws_strip();

   if ( strcmp(s.text(), target_country_name) != 0 )  continue;

   r = admin_name_rec;

   substring((const char *) buf, junk, r->start_pos, r->start_pos + r->field_length - 1);

   s = junk;

   s.ws_strip();

   if ( n_records > max_records )  {

      cerr << "\n\n  " << program_name << ": too many records!\n\n";

      exit ( 1 );

   }

   cout << '\"' << s << "\"\n";

   records[n_records].rec_num = j;

   records[n_records].admin = s;

   ++n_records;

}   //  for j

close(fd);   fd = -1;

   //
   //  write the new file
   //

ofstream f;
int index;
ShpFileHeader h;
ShpRecordHeader rh;
ShpPolygonRecord gr;

ConcatString output_filename = target_country_name;
output_filename << ".dat";

cout << "Writing: " <<  output_filename << "\n";

f.open(output_filename);

if ( !f )  {

   cerr << "\n\n  " << program_name << ": unable to open output file \""
        << output_filename << "\"\n\n";

   exit ( 1 );

}

if ( (fd = open(shp_filename, O_RDONLY)) < 0 )  {

   cerr << "\n\n  " << program_name << ": unable to open shp file \""
        << shp_filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  main header
   //

bytes = 100;

if ( (n_read = read(fd, buf, bytes)) != bytes )  {

   cerr << "\n\n  " << program_name << ": trouble reading main file header from input file "
        << shp_filename << "\"\n\n";

   exit ( 1 );

}

h.set(buf);

   //
   //  records
   //

while ( (n_read = read(fd, buf, 8)) == 8 )  {

   rh.set(buf);

   bytes = rh.content_length_bytes;

   if ( bytes >= buf_size )  {

      cerr << "\n\n  " << program_name << ": buffer too small ... increase to at least "
           << bytes << "\n\n";

      exit ( 1 );

   }

   if ( (n_read = read(fd, buf, bytes)) != bytes )  {

      cerr << "\n\n  " << program_name << ": trouble reading record data ... n_read = " << n_read << "\n\n";

      exit ( 1 );

   }

   gr.set(buf);

   if ( (index = get_index(rh.record_number_0)) < 0 )  continue;

   write_record(f, gr, records[index].rec_num, records[index].admin);


}   //  while










   //
   //  done
   //

close(fd);  fd = -1;

f.close();

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int get_index(const int recnum)

{

int j;

for (j=0; j<n_records; ++j)  {

   if ( recnum == records[j].rec_num )  return ( j );

}



return ( -1 );

}


////////////////////////////////////////////////////////////////////////


void write_record(ostream & out, const ShpPolygonRecord & gr, const int recnum, const ConcatString & admin)

{

int j;

for (j=0; j<(gr.n_parts); ++j)  {

   write_part(out, gr, recnum, admin, j);

}


return;

}


////////////////////////////////////////////////////////////////////////


void write_part(ostream & out, const ShpPolygonRecord & gr, const int recnum, const ConcatString & admin, const int part)

{

int j;
const int start_index = gr.start_index(part);
const int  stop_index = gr.stop_index(part);
const int n = stop_index - start_index + 1;
char junk[256];

   //
   //  first line
   //

out << recnum << ' ';

out << n << ' ';

out << (gr.y_min()) << ' ' << (gr.y_max()) << ' ';

out << -(gr.x_max()) << ' ' << -(gr.x_min()) << ' ';

out << '\'' << admin << " (" << target_country_name << ")\'";

out << '\n';

   //
   //  points
   //

for (j=start_index; j<=stop_index; ++j)  {

   sprintf(junk, " %.5f %.5f", gr.lat(j), gr.lon(j));

   out << junk << '\n';

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void usage()

{

cerr << "\n\n  usage:  " << program_name << " shp_filename dbf_filename target_country_name\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////
