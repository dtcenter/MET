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
#include "shx_file.h"
#include "dbf_file.h"
#include "shp_poly_record.h"
#include "int_array.h"


////////////////////////////////////////////////////////////////////////


static ConcatString country_name_tag;
static ConcatString admin_name_tag;


////////////////////////////////////////////////////////////////////////


   //
   //  Default values for command-line switches
   //

static ConcatString output_directory;

static bool do_separate_files = false;


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static CommandLine cline;

static const int buf_size = 1200000;

static unsigned char buf[buf_size];

struct RecordInfo {

   ConcatString country;

   ConcatString admin;   //  might be empty

   int offset;   //  bytes

   int length;   //  bytes

};


static RecordInfo * records = 0;

static int n_records = 0;

static const char sq = '\'';   //  single quote


////////////////////////////////////////////////////////////////////////


static void usage();

static void set_outdir         (const StringArray &);
static void set_separate_files (const StringArray &);

static void get_record_infos(const char * shx_filename, const char * dbf_filename);

static void write_record (ostream &, const ShpPolyRecord &, int & recnum, const ConcatString & anno);
static void write_part   (ostream &, const ShpPolyRecord &, int & recnum, const ConcatString & anno, const int part);

static ConcatString create_anno_string(const RecordInfo &);

static void make_one_file       (const char * shp_filename);
static void make_separate_files (const char * shp_filename);

static void process_array(int fd, const ConcatString & output_filename, const IntArray & i);

static void patch(ConcatString &);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

cline.set(argc, argv);

cline.set_usage(usage);

cline.add(set_outdir,         "-outdir",         1);

cline.add(set_separate_files, "-separate_files", 0);

cline.parse();

if ( cline.n() != 5 )  usage();

const ConcatString shp_filename = cline[0];
const ConcatString shx_filename = cline[1];
const ConcatString dbf_filename = cline[2];
country_name_tag                = cline[3];
admin_name_tag                  = cline[4];


   //
   //  get record info
   //

get_record_infos(shx_filename.c_str(), dbf_filename.c_str());

cout << "There are " << n_records << " records.\n\n";


for (int j=0; j<n_records; ++j)  {

   cout << "Record " << j << " of " << n_records << "...\n";

   cout << "   country = " << records[j].country.contents() << '\n';
   cout << "   admin   = " << records[j].admin.contents()   << '\n';
   cout << "   offset  = " << records[j].offset             << '\n';
   cout << "   length  = " << records[j].length             << '\n';

   if ( records[j].length > buf_size)  {

      cerr << "\n  " << program_name << ": buffer size (" << buf_size
           << ") is too small. Increase to at least " << records[j].length
           << ".\n\n";

      exit ( 1 );
   }
}


   //
   //  output
   //

if ( do_separate_files )   {

    make_separate_files(shp_filename.c_str());

} else {

   make_one_file(shp_filename.c_str());

}


   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

cerr << "\nusage: " << program_name << ' '
     << "[-outdir path] [-separate_files] "
     << "shp_file shx_file dbf_file country_field admin_field\n\n";

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


void set_separate_files(const StringArray &)

{

do_separate_files = true;

return;

}


////////////////////////////////////////////////////////////////////////


void write_record(ostream & out, const ShpPolyRecord & gr, int & recnum, const ConcatString & anno)

{

int j;

for (j=0; j<(gr.n_parts); ++j)  {

   write_part(out, gr, recnum, anno, j);

}


return;

}


////////////////////////////////////////////////////////////////////////


void write_part(ostream & out, const ShpPolyRecord & gr, int & recnum, const ConcatString & anno, const int part)

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

out << sq << anno << sq;

out << '\n';

   //
   //  points
   //

for (j=start_index; j<=stop_index; ++j)  {

   snprintf(junk, sizeof(junk), " %.5f %.5f", gr.lat(j), gr.lon(j));

   out << junk << '\n';

}

   //
   //  done
   //

++recnum;

return;

}


////////////////////////////////////////////////////////////////////////


void get_record_infos(const char * shx_filename, const char * dbf_filename)

{

int fd = -1;
int j;
int n_read, bytes;
ConcatString s;
char junk[512];
DbfHeader hd;
DbfSubRecord rd;
DbfSubRecord * country_name_rec = 0;
DbfSubRecord *   admin_name_rec = 0;
DbfSubRecord * r                = 0;

   //
   //  get the country names (and admin names, if they're there) from the dbf file
   //

if ( (fd = met_open(dbf_filename, O_RDONLY)) < 0 )  {

   cerr << "\n  " << program_name << ": get_record_infos() -> unable to open dbf file \""
        << dbf_filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  main header
   //

bytes = 32;

if ( (n_read = read(fd, buf, bytes)) != bytes )  {

   cerr << "\n  " << program_name << ": get_record_infos() -> trouble reading header from dbf file \""
        << dbf_filename << "\"\n\n";

   exit ( 1 );

}

hd.set_header(buf);

   //
   //  dbf subrecords
   //

hd.set_subrecords(fd);

   //
   //  country name should be in there, but the admin name might not be
   //

country_name_rec = hd.lookup_subrec(country_name_tag.c_str());
  admin_name_rec = hd.lookup_subrec(admin_name_tag.c_str());

if ( ! country_name_rec )  {

   cout << "\n  WARNING: " << program_name << ": get_record_infos() -> "
        << "unable to get country name subrecord from dbf file \""
        << dbf_filename << "\". Processing all available records.\n\n";

}

   //
   //  allocate record info array
   //

n_records = hd.n_records;

records = new RecordInfo [n_records];

   //
   //  dbf records
   //

if ( lseek(fd, hd.pos_first_record, SEEK_SET) < 0 )  {

   cerr << "\n  " << program_name << ": lseek error\n\n";

   exit ( 1 );

}

for (j=0; j<(hd.n_records); ++j)  {

   if ( (hd.record_length + 1) > buf_size) {

      cerr << "\n  " << program_name << ": buffer size (" << buf_size
           << ") is too small. Increase to at least " << bytes
           << ".\n\n";

      exit ( 1 );

   }

   bytes = hd.record_length;

   n_read = read(fd, buf, bytes);

   if ( n_read != bytes )  {

      cerr << "\n  " << program_name << ": read error ... n_read = " << n_read << "\n\n";

      exit ( 1 );

   }

   buf[hd.record_length] = 0;

   if( country_name_rec )  {

      r = country_name_rec;

      substring((const char *) buf, junk, r->start_pos, r->start_pos + r->field_length - 1);

      s = junk;

      s.ws_strip();
   }
   else  {
      s << cs_erase << "REC" << j+1;
   }

   records[j].country = s;

   if ( admin_name_rec )  {

      r = admin_name_rec;

      substring((const char *) buf, junk, r->start_pos, r->start_pos + r->field_length - 1);

      s = junk;

      s.ws_strip();

      records[j].admin = s;

   } else records[j].admin.clear();

}   //  for j

   //
   //  done with the dbf file
   //

close(fd);

   ///////////////////////////////////////////

ShxFileHeader hx;
ShxRecord rx;

   //
   //  get the offsets and record lengths from the shx file
   //

if ( (fd = met_open(shx_filename, O_RDONLY)) < 0 )  {

   cerr << "\n  " << program_name << ": get_record_infos() -> unable to open shx file \""
        << shx_filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  shx main header
   //

bytes = 100;

if ( (n_read = read(fd, buf, bytes)) != bytes )  {

   cerr << "\n  " << program_name << ": get_record_infos() -> trouble reading main file header from shx file \""
        << shx_filename << "\"\n\n";

   exit ( 1 );

}

hx.set(buf);

   //
   //  shx records
   //

j = 0;

while ( (n_read = read(fd, buf, 8)) == 8 )  {

   rx.set(buf);

   records[j].offset = rx.offset_bytes;

   records[j].length = rx.content_length_bytes;

   ++j;

}   //  while

   //
   //  done with shx file
   //

close(fd);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString create_anno_string(const RecordInfo & r)

{

ConcatString s;

if ( r.admin.nonempty() )  {

   s << r.admin << " (" << r.country << ')';

} else {

   s = r.country;

}

return ( s );

}


////////////////////////////////////////////////////////////////////////


void make_one_file(const char * shp_filename)

{

int fd = -1;
int j;
int pos, n_read, bytes;
int rec_num;
ConcatString output_filename;
ConcatString s;
ConcatString anno;
// ShpRecordHeader rh;
ShpPolyRecord gr;
ofstream f;

   //
   //  construct output filename
   //

if ( output_directory.nonempty() )  output_filename << output_directory << '/';

s = get_short_name(shp_filename);

s.chomp(".shp");

output_filename << s << "_data";

   //
   //  open output file
   //

met_open(f, output_filename.c_str());

if ( ! f )  {

   cerr << "\n  " << program_name << ": make_one_file() -> unable to open output file \""
        << output_filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  open shp file
   //

if ( (fd = met_open(shp_filename, O_RDONLY)) < 0 )  {

   cerr << "\n  " << program_name << ": make_one_file() -> unable to open shp file \""
        << shp_filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  process the records
   //

rec_num = 0;

for (j=0; j<n_records; ++j)  {

   pos = records[j].offset + 8;   //  skip the record header

   if ( lseek(fd, pos, SEEK_SET) < 0 )  {

      cerr << "\n  " << program_name << ": make_one_file() -> lseek error\n\n";

      exit ( 1 );

   }

   bytes = records[j].length;   //  subtract 8?

   if ( (n_read = read(fd, buf, bytes)) != bytes )  {

      cerr << "\n  " << program_name << ": trouble reading record data ... n_read = " << n_read << "\n\n";

      exit ( 1 );

   }

   gr.set(buf);

   anno.erase();

   anno = create_anno_string(records[j]);

   write_record(f, gr, rec_num, anno);

}   //  for j




   //
   //  done
   //

f.close();

close(fd);

return;

}


////////////////////////////////////////////////////////////////////////


void make_separate_files (const char * shp_filename)

{

int fd = -1;
int j, k;
StringArray names;
ConcatString output_filename;
ConcatString s, ss;
IntArray i;

   //
   //  The thing that makes this tricky is that we can't assume that
   //    all of the data for a given country is contiguous in the shapefile
   //

   //
   //  get country names
   //

for (j=0; j<n_records; ++j)  {

   if ( !names.has(records[j].country) )  names.add(records[j].country);

}

   //
   //  open shp file
   //

if ( (fd = met_open(shp_filename, O_RDONLY)) < 0 )  {

   cerr << "\n  " << program_name << ": make_separate_files() -> unable to open shp file \""
        << shp_filename << "\"\n\n";

   exit ( 1 );

}

   //
   //   loop through the names, loading the shp record numbers into an array,
   //
   //      and then write those records to an output file
   //

for (k=0; k<(names.n()); ++k)  {

   i.clear();

   for (j=0; j<n_records; ++j)  {

      if ( names[k].c_str() == records[j].country )  i.add(j);

   }

   output_filename.erase();

   if ( output_directory.nonempty() )  output_filename << output_directory << '/';

   s = get_short_name(names[k].c_str());

   patch(s);

   ss = get_short_name(shp_filename);

   ss.chomp(".shp");

   output_filename << ss << '_' << s << "_data";

   process_array(fd, output_filename, i);

}   //  for k


   //
   //  done
   //

close(fd);

return;

}


////////////////////////////////////////////////////////////////////////


void process_array(int fd, const ConcatString & output_filename, const IntArray & i)

{

int j, k;
int pos, rec_num;
int n_read, bytes;
ShpPolyRecord gr;
ConcatString anno;
ofstream f;

   //
   //  open output file
   //

met_open(f, output_filename.c_str());

if ( ! f )  {

   cerr << "\n  process_array() -> unable to open output file \"" << output_filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  loop through the records
   //

rec_num = 0;

for (k=0; k<(i.n_elements()); ++k)  {

   j = i[k];

   pos = records[j].offset + 8;   //  skip record header

   if ( lseek(fd, pos, SEEK_SET) < 0 )  {

      cerr << "\n  process_array() -> lseek error on file \"" << output_filename << "\"\n\n";

      exit ( 1 );

   }

   bytes = records[j].length;   //  subtract 8?

   if ( (n_read = read(fd, buf, bytes)) != bytes )  {

      cerr << "\n  " << program_name << ": trouble reading record data ... n_read = " << n_read << "\n\n";

      exit ( 1 );

   }

   gr.set(buf);

   anno.erase();

   anno = create_anno_string(records[j]);

   write_record(f, gr, rec_num, anno);

}   //  for k




   //
   //  done
   //

f.close();

return;

}


////////////////////////////////////////////////////////////////////////


void patch(ConcatString & s)

{

int j;
ConcatString a = s;
char c;

s.erase();

for (j=0; j<(a.length()); ++j)  {

   c = a[j];

   switch ( c )  {

      case ' ':
         s.add('_');
         break;

      case '&':   //  fall thru
      case '.':   //  fall thru
         break;

      default:
         s.add(c);
         break;

   }

}



return;

}


////////////////////////////////////////////////////////////////////////
