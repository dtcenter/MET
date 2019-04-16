// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

using namespace std;

////////////////////////////////////////////////////////////////////////


#include <cstdio>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <cstdlib>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>

#include "vx_cal.h"
#include "vx_util.h"
#include "vx_log.h"

#include "grib_classes.h"
#include "grib_utils.h"


////////////////////////////////////////////////////////////////////////

inline int imin(int a, int b)  { return ( (a < b) ? a : b ); } 

static const char *separator = "===================================================";


////////////////////////////////////////////////////////////////////////


static long find_magic_cookie(int);

static int calc_lead_time(Section1_Header *);

static void ibm_to_ieee(const uint4 *, uint4 *);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class GribRecord
   //


////////////////////////////////////////////////////////////////////////


GribRecord::GribRecord()

{

is  = new Section0_Header;
pds = (unsigned char *) 0;
gds = new Section2_Header;
bms = new Section3_Header;
bds = new Section4_Header;

if ( !is || !gds || !bms || !bds )  {

   mlog << Error << "\nGribRecord::GribRecord() -> memory allocation error\n\n";

   exit ( 1 );
//   throw GribError(mem_alloc_error, __LINE__, __FILE__, "\n\n  GribRecord::GribRecord() -> memory allocation error\n\n");

}

memset(is,  0, sizeof(Section0_Header));
memset(gds, 0, sizeof(Section2_Header));
memset(bms, 0, sizeof(Section3_Header));
memset(bds, 0, sizeof(Section4_Header));

rec_num = pds_len = gds_flag = bms_flag = 0;

m_value = 0.0;
b_value = 0.0;

r_value = 0.0;

d_value = 0;
e_value = 0;

nx = 0;
ny = 0;

issue = 0;

lead = 0;

word_size = 0;

data_lseek_offset = record_lseek_offset = (off_t) 0;

// data   = (unsigned char *) 0;
// bitmap = (unsigned char *) 0;
//
//data_size = data_alloc = 0;
//bitmap_size = bitmap_alloc = 0;

// data = new vector<unsigned char>();
// bitmap = new vector<unsigned char>();

mask = 0L;

Sec0_offset_in_record = -1;
Sec1_offset_in_record = -1;
Sec2_offset_in_record = -1;
Sec3_offset_in_record = -1;
Sec4_offset_in_record = -1;

Sec0_offset_in_file = -1;
Sec1_offset_in_file = -1;
Sec2_offset_in_file = -1;
Sec3_offset_in_file = -1;
Sec4_offset_in_file = -1;

}


////////////////////////////////////////////////////////////////////////


GribRecord::~GribRecord()

{

if (  is )  { delete  is;    is = (Section0_Header *) 0; }
if ( pds )  { delete [] pds;   pds = (unsigned char *)   0; }
if ( gds )  { delete gds;   gds = (Section2_Header *) 0; }
if ( bms )  { delete bms;   bms = (Section3_Header *) 0; }
if ( bds )  { delete bds;   bds = (Section4_Header *) 0; }
//if ( data ) { delete data; data = (vector<unsigned char> *) 0; }
//if ( bitmap ) { delete bitmap; bitmap = (vector<unsigned char> *) 0; }
}


////////////////////////////////////////////////////////////////////////


GribRecord::GribRecord(const GribRecord &g)

{

is  = new Section0_Header;
pds = new unsigned char [g.pds_len];
gds = new Section2_Header;
bms = new Section3_Header;
bds = new Section4_Header;

if ( !is || !pds || !gds || !bms || !bds )  {

   mlog << Error << "\nGribRecord::GribRecord(const GribRecord &) -> memory allocation error\n\n";

   exit ( 1 );
//   throw GribError(mem_alloc_error, __LINE__, __FILE__, "\n\n  GribRecord::GribRecord(const GribRecord &) -> memory allocation error\n\n");

}

memcpy(is,  g.is,  sizeof(Section0_Header));
memcpy(pds, g.pds, sizeof(unsigned char)*g.pds_len);
memcpy(gds, g.gds, sizeof(Section2_Header));
memcpy(bms, g.bms, sizeof(Section3_Header));
memcpy(bds, g.bds, sizeof(Section4_Header));

rec_num             = g.rec_num;
pds_len             = g.pds_len;
gds_flag            = g.gds_flag;
bms_flag            = g.bms_flag;
m_value             = g.m_value;
b_value             = g.b_value;
r_value             = g.r_value;
d_value             = g.d_value;
e_value             = g.e_value;
issue               = g.issue;
lead                = g.lead;
word_size           = g.word_size;
record_lseek_offset = g.record_lseek_offset;
data_lseek_offset   = g.data_lseek_offset;
mask                = g.mask;

nx                  = g.nx;
ny                  = g.ny;

Sec0_offset_in_record = g.Sec0_offset_in_record;
Sec1_offset_in_record = g.Sec1_offset_in_record;
Sec2_offset_in_record = g.Sec2_offset_in_record;
Sec3_offset_in_record = g.Sec3_offset_in_record;
Sec4_offset_in_record = g.Sec4_offset_in_record;

Sec0_offset_in_file = g.Sec0_offset_in_file;
Sec1_offset_in_file = g.Sec1_offset_in_file;
Sec2_offset_in_file = g.Sec2_offset_in_file;
Sec3_offset_in_file = g.Sec3_offset_in_file;
Sec4_offset_in_file = g.Sec4_offset_in_file;

if ( g.data.size() )  {
    data = g.data;
}

if ( g.bitmap.size() )  {
    bitmap = g.bitmap;
}

TO = g.TO;

}


////////////////////////////////////////////////////////////////////////


GribRecord & GribRecord::operator=(const GribRecord &g)

{

if ( this == &g )  return ( *this );   //  check for a = a

pds = new unsigned char [g.pds_len];

memcpy(is,  g.is,  sizeof(Section0_Header));
memcpy(pds, g.pds, sizeof(unsigned char)*g.pds_len);
memcpy(gds, g.gds, sizeof(Section2_Header));
memcpy(bms, g.bms, sizeof(Section3_Header));
memcpy(bds, g.bds, sizeof(Section4_Header));

rec_num             = g.rec_num;
pds_len             = g.pds_len;
gds_flag            = g.gds_flag;
bms_flag            = g.bms_flag;
m_value             = g.m_value;
b_value             = g.b_value;
r_value             = g.r_value;
d_value             = g.d_value;
e_value             = g.e_value;
issue               = g.issue;
lead                = g.lead;
word_size           = g.word_size;
record_lseek_offset = g.record_lseek_offset;
data_lseek_offset   = g.data_lseek_offset;
mask                = g.mask;

nx                  = g.nx;
ny                  = g.ny;

Sec0_offset_in_record = g.Sec0_offset_in_record;
Sec1_offset_in_record = g.Sec1_offset_in_record;
Sec2_offset_in_record = g.Sec2_offset_in_record;
Sec3_offset_in_record = g.Sec3_offset_in_record;
Sec4_offset_in_record = g.Sec4_offset_in_record;

Sec0_offset_in_file = g.Sec0_offset_in_file;
Sec1_offset_in_file = g.Sec1_offset_in_file;
Sec2_offset_in_file = g.Sec2_offset_in_file;
Sec3_offset_in_file = g.Sec3_offset_in_file;
Sec4_offset_in_file = g.Sec4_offset_in_file;

if ( g.data.size() ) {
    data = g.data;
}

if ( g.bitmap.size() ) {
    bitmap = g.bitmap;
}

TO = g.TO;

return ( *this );

}


////////////////////////////////////////////////////////////////////////


void GribRecord::extend_data(int n)
{
    if (data.capacity() >= n)
        return;
        
    data.reserve(n);
}


////////////////////////////////////////////////////////////////////////


void GribRecord::extend_bitmap(int n)
{
    if (bitmap.capacity() >= n)
        return;
        
    bitmap.reserve(n);
}


////////////////////////////////////////////////////////////////////////


uint4 GribRecord::long_data_value(int n) const

{

   //
   //  n starts at zero here
   //

int k, byte, shift;
uint4 value, u;
unsigned char *c1 = (unsigned char *) 0;
const unsigned char *c2 = (unsigned char *) 0;

k = n*word_size;

byte = k/8;

c1 = (unsigned char *) (&u);

c2 = data.data() + byte;

   //
   //  we might go past the end of the data
   //  buffer here, but that's OK since we're
   //  only reading, not writing, and the
   //  garbage will get masked out
   //

if(native_endian == big_endian) {
   c1[0] = c2[0];
   c1[1] = c2[1];
   c1[2] = c2[2];
   c1[3] = c2[3];
} else {
   c1[3] = c2[0];
   c1[2] = c2[1];
   c1[1] = c2[2];
   c1[0] = c2[3];
}

shift = 32 - word_size - (k%8);

value = (u >> shift) & mask;

return ( value );

}


////////////////////////////////////////////////////////////////////////


double GribRecord::data_value(int n) const

{

   //
   //  n starts at zero here
   //

uint4 u;
double y;

u = long_data_value(n);

y = m_value*u + b_value;

return ( y );

}


////////////////////////////////////////////////////////////////////////


void GribRecord::reset()

{

if ( is )   memset(is,  0, sizeof(Section0_Header));
if ( pds )  memset(pds, 0, sizeof(unsigned char)*pds_len);
if ( gds )  memset(gds, 0, sizeof(Section2_Header));
if ( bms )  memset(bms, 0, sizeof(Section3_Header));
if ( bds )  memset(bds, 0, sizeof(Section4_Header));

if ( data.size() ) data.clear();

if ( bitmap.size() ) bitmap.clear();

rec_num = pds_len = gds_flag = bms_flag = d_value = e_value = issue = lead = word_size = 0;

m_value = b_value = r_value = 0.0;

record_lseek_offset = data_lseek_offset = (off_t) 0;

mask = (uint4) 0;

nx = 0;
ny = 0;

Sec0_offset_in_record = -1;
Sec1_offset_in_record = -1;
Sec2_offset_in_record = -1;
Sec3_offset_in_record = -1;
Sec4_offset_in_record = -1;

Sec0_offset_in_file = -1;
Sec1_offset_in_file = -1;
Sec2_offset_in_file = -1;
Sec3_offset_in_file = -1;
Sec4_offset_in_file = -1;

return;

}


////////////////////////////////////////////////////////////////////////


int GribRecord::bms_bit(int n) const

{

if ( (n < 0) || (gds_flag && (nx > 0) && (ny > 0) && (n >= nx*ny)) )  {

   mlog << Error << "\nGribRecord::bms_bit(int) -> range check error ... n = " << n << "\n\n";

   exit ( 1 );
//   char temp_str[max_temp_str_length];

//   snprintf(temp_str, sizeof(temp_str), "\n\n  GribRecord::bms_bit(int) -> range check error ... n = %d\n\n", n);

//   throw GribError(range_chk_error, __LINE__, __FILE__, temp_str);

}

int byte, bit;
int a;
unsigned char uc_mask;

byte = n/8;

bit = n%8;

uc_mask = ((unsigned char) 1) << (7 - bit);

a = 0;

if ( bitmap[byte] & uc_mask )  a = 1;

return ( a );

}


////////////////////////////////////////////////////////////////////////


int GribRecord::gribcode() const

{

int j;
Section1_Header *pds_ptr = (Section1_Header *) pds;

j = (int) (pds_ptr->grib_code);

return ( j );

}


////////////////////////////////////////////////////////////////////////


void GribRecord::set_TO()

{

int xdir, ydir, order;

gds_to_order(*gds, xdir, ydir, order);

TO.set(xdir, ydir, order);

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class GribFileRep
   //


////////////////////////////////////////////////////////////////////////


GribFileRep::GribFileRep()

{

fd = -1;

file_start = (long) -1;

name = (char *) 0;

referenceCount = buf_size = n_alloc = n_records = issue = lead = 0;

buf = (unsigned char *) 0;

record_info = (RecordInfo *) 0;

}


////////////////////////////////////////////////////////////////////////


GribFileRep::~GribFileRep()

{

if ( fd >= 0 )  { ::close(fd);  fd = -1; }

if ( buf )  { delete [] buf;  buf = (unsigned char *) 0; }

if ( name )  { delete [] name;  name = (char *) 0; }

if ( n_alloc ) { delete [] record_info;  record_info = (RecordInfo *) 0; }


}

////////////////////////////////////////////////////////////////////////


void GribFileRep::record_extend(int n)

{

if ( n_alloc > n )  return;

int j;
RecordInfo *r  = (RecordInfo *) 0;


++n;

n_alloc = (n + 99)/100;

n_alloc *= 100;

r = new RecordInfo [n_alloc];

if ( !r )  {

   mlog << Error << "\nvoid GribFileRep::record_extend(int) -> memory allocation error\n\n";

   exit ( 1 );
//   throw GribError(mem_alloc_error, __LINE__, __FILE__, "\n\n  void GribFileRep::record_extend(int) -> memory allocation error\n\n");

}

for (j=0; j<n_records; ++j)  {

   r[j].lseek_offset = record_info[j].lseek_offset;

   r[j].gribcode = record_info[j].gribcode;

}

for (j=n_records; j<n_alloc; ++j)  {

   r[j].lseek_offset = -1;

   r[j].gribcode = -1;

}

delete [] record_info;  record_info = (RecordInfo *) 0;

record_info = r;  r = (RecordInfo *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void GribFileRep::realloc_buf(size_t n_bytes)

{

if ( buf )  { delete [] buf;  buf = 0;  buf_size = 0; }

buf = new unsigned char [n_bytes];

if ( !buf )  {

   mlog << Error << "\n\n  GribFileRep::realloc_buf(long long) -> memory allocation error\n\n";

   exit ( 1 );

}

buf_size = n_bytes;

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class GribFile
   //


////////////////////////////////////////////////////////////////////////


GribFile::GribFile()

{

rep = (GribFileRep *) 0;

}


////////////////////////////////////////////////////////////////////////


GribFile::GribFile(const char *filename)

{

rep = (GribFileRep *) 0;

open(filename);

}


////////////////////////////////////////////////////////////////////////


bool GribFile::open(const char *filename)

{

int j;
off_t pos;

close();

if ( !(rep = new GribFileRep) )  {

   mlog << Error << "\nGribFile::open(char *) -> memory allocation error\n\n";

   exit ( 1 );
//   throw GribError(mem_alloc_error, __LINE__, __FILE__, "\n\n  GribFile::open(char *) -> memory allocation error\n\n");

}

rep->referenceCount = 1;

   //
   //  Strip off leading path component
   //

j = strlen(filename) - 1;

while ( (j >= 0) && (filename[j] != '/') )   --j;

++j;

if ( !(rep->name = new char [1 + strlen(filename + j)]) )  {

   mlog << Error << "\nGribFile::open(const char *) -> memory allocation error 1\n\n";

   exit ( 1 );
//   throw GribError(mem_alloc_error, __LINE__, __FILE__, "\n\n  GribFile::open(const char *) -> memory allocation error 1\n\n");

}

strcpy(rep->name, filename + j);

rep->issue = rep->lead = 0;

if ( (rep->fd = met_open(filename, O_RDONLY)) < 0 )  {

   mlog << Error << "\nGribFile::open(const char *) -> unable to open grib file " << filename << "\n\n";

   exit ( 1 );
//   char temp_str[max_temp_str_length];

//   snprintf(temp_str, sizeof(temp_str), "\n\n  GribFile::open(const char *) -> unable to open grib file %s\n\n", filename);

//   throw GribError(open_error, __LINE__, __FILE__, temp_str);
   return( false );
}

if ( !(rep->buf = new unsigned char [default_gribfile_buf_size]) )  {

   mlog << Error << "\nGribFile::open(const char *) -> memory allocation error 2\n\n";

   exit ( 1 );
//   throw GribError(mem_alloc_error, __LINE__, __FILE__, "\n\n  GribFile::open(const char *) -> memory allocation error 2\n\n");

}

rep->buf_size = default_gribfile_buf_size;

if(!skip_header()) return ( false );

index_records();

pos = lseek(rep->fd, rep->file_start, SEEK_SET);

return ( true );

}


////////////////////////////////////////////////////////////////////////


GribFile::GribFile(const GribFile &g)

{

rep = g.rep;

if ( rep )  ++rep->referenceCount;

}


////////////////////////////////////////////////////////////////////////


GribFile & GribFile::operator=(const GribFile &g)

{

if ( this == &g )  return ( *this );

close();

rep = g.rep;

if ( rep )  ++rep->referenceCount;

return ( *this );

}


////////////////////////////////////////////////////////////////////////


GribFile::~GribFile()

{

close();

}


////////////////////////////////////////////////////////////////////////


int GribFile::operator>>(GribRecord &g)

{

int j = read_record(g);

if ( j < 0 )  return ( -1 );

if ( j > 0 )  return ( 1 );

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int GribFile::read_record(GribRecord & g)

{

size_t len, bytes, n_read, s;
int j;
int m, d, y, hh, mm;
int D, E;
off_t file_pos, res;
off_t bytes_processed;
unsigned char *c = (unsigned char *) 0, c3[3];
double t;
float r[4];
uint4 ibm;


g.reset();

file_pos = find_magic_cookie(rep->fd);

res = lseek(rep->fd, file_pos, SEEK_SET);

bytes_processed = 0;

   //
   //  Infer the record number by the current file position
   //

for (g.rec_num=0,j=0; j<(rep->n_records); ++j)  {

   if ( file_pos == rep->record_info[j].lseek_offset )  {
      g.rec_num = j+1;
      break;
   }

}

   //
   //  Process section 0
   //

g.Sec0_offset_in_file   = file_pos + bytes_processed;
g.Sec0_offset_in_record = bytes_processed;

bytes = sizeof(Section0_Header);

if ( (n_read = read(rep->buf, bytes)) == 0 ) return ( 0 );

memcpy(g.is, rep->buf, 8);

if ( (n_read < 0) || (n_read != bytes) )  {

   mlog << Error << "\nGribFile::read_record() -> error reading section 0 header ... nread = " << n_read << "\n\n";

   return ( -1 );

}

if ( strncmp(g.is->grib_name, "GRIB", 4) != 0 )  {

   off_t pos2 = lseek(rep->fd, 0L, SEEK_END);

   mlog << Error << "\nGribFile::read_record(GribRecord &) -> \"GRIB\" magic cookie not found in grib record\n\n"
        << "     reading record number " << (rep->n_records) << ", bytes left in file = "
        << (pos2 - file_pos) << "\n\n";

   return ( 0 );

   // exit ( 1 );

}

s = char3_to_int(g.is->length);

g.record_lseek_offset = file_pos;

if ( s > (rep->buf_size) )  rep->realloc_buf(s);

// if ( s > (rep->buf_size) )  {
// 
//    mlog << Error << "\nGribFile::read_record(GribRecord &) -> "
//         << "found a grib record larger than the buffer size.\n\n"
//         << "  Increase the buffer to at least " << s << " bytes.\n\n\n";
// 
//    exit ( 1 );

//   char temp_str[max_temp_str_length];

//   snprintf(temp_str, sizeof(temp_str), "\n\n  GribFile::read_record(GribRecord &) -> found a grib record larger than the buffer size.\n\n  Increase the buffer to at least %d bytes.\n\n\n", s);

//   throw GribError(record_size_error, __LINE__, __FILE__, temp_str);

// }

if ( read(8, s - 8) == 0 )  return ( 0 );

if ( strncmp((char *) (rep->buf + (s - 4)), "7777", 4) != 0 )  {

   mlog << Error << "\nGribFile::read_record(GribRecord &) -> trailing \"7777\" not found in grib record\n\n";

//   exit ( 1 );
//   throw GribError(missing_trail_7777_error, __LINE__, __FILE__, "\n\n  GribFile::read_record(GribRecord &) -> trailing \"7777\" not found in grib record\n\n");
   return ( 0 );
}

bytes_processed += 8;

   //
   //  Process section 1
   //

g.Sec1_offset_in_file   = file_pos + bytes_processed;
g.Sec1_offset_in_record = bytes_processed;

   //
   //  Extract the PDS length
   //
c3[0] = *(rep->buf + bytes_processed);
c3[1] = *(rep->buf + bytes_processed + 1);
c3[2] = *(rep->buf + bytes_processed + 2);
len = char3_to_int(c3);

g.pds = new unsigned char [len];

memcpy(g.pds, rep->buf + bytes_processed, sizeof(unsigned char)*len);

Section1_Header *pds_ptr = (Section1_Header *) g.pds;

c = (unsigned char *) (&D);
D = 0;

if(native_endian == big_endian) {
   c[2] = pds_ptr->d_value[0] & 127;
   c[3] = pds_ptr->d_value[1];
} else {
   c[0] = pds_ptr->d_value[1];
   c[1] = pds_ptr->d_value[0] & 127;
}

if ( pds_ptr->d_value[0] & 128 )  D = -D;
g.d_value = D;

m  = pds_ptr->month;
d  = pds_ptr->day;
y  = 100*(pds_ptr->century - 1) + (pds_ptr->year);
hh = pds_ptr->hour;
mm = pds_ptr->minute;

g.issue = mdyhms_to_unix(m, d, y, hh, mm, 0);

g.lead = calc_lead_time(pds_ptr);

bytes_processed += char3_to_int(pds_ptr->length);

   //
   //  Process section 2
   //


if ( (pds_ptr->flag) & 128 )  {

   g.Sec2_offset_in_file   = file_pos + bytes_processed;
   g.Sec2_offset_in_record = bytes_processed;

   g.gds_flag = 1;

//
//  ????? Need to change this?????
//
   memcpy(g.gds, rep->buf + bytes_processed, sizeof(Section2_Header));

   g.nx = char2_to_int(g.gds->nx);
   g.ny = char2_to_int(g.gds->ny);

   g.set_TO();

   bytes_processed += char3_to_int(g.gds->length);

} else {

   g.gds_flag = 0;

}

   //
   //  Process section 3
   //

if ( pds_ptr->flag & 64 )  {

   g.Sec3_offset_in_file   = file_pos + bytes_processed;
   g.Sec3_offset_in_record = bytes_processed;

   g.bms_flag = 1;

   memcpy(g.bms, rep->buf + bytes_processed, sizeof(Section3_Header));

   s = char3_to_int(g.bms->length) - 6;

   g.extend_bitmap(s);

   g.bitmap.clear();
   unsigned char *begin = rep->buf + bytes_processed + 6;
   g.bitmap.assign(begin, begin+s);
   
   // mlog << Debug(1) << "\n\n  reading " << s << " bytes into bitmap at file location " << (bytes_processed + 6) << "\n\n";

   bytes_processed += char3_to_int(g.bms->length);

} else {

   g.bms_flag = 0;

}

   //
   //  Process section 4
   //

g.Sec4_offset_in_file   = file_pos + bytes_processed;
g.Sec4_offset_in_record = bytes_processed;

memcpy(g.bds, rep->buf + bytes_processed, sizeof(Section4_Header));

c = (unsigned char *) (&E);
E = 0;

if(native_endian == big_endian) {
   c[2] = (g.bds->e_value[0]) & 127;
   c[3] = g.bds->e_value[1];
} else {
   c[0] = g.bds->e_value[1];
   c[1] = (g.bds->e_value[0]) & 127;
}

if ( g.bds->e_value[0] & 128 )  E = -E;
g.e_value = E;

c = (unsigned char *) (&ibm);

if(native_endian == big_endian) {
   c[0] = g.bds->r_value[0];
   c[1] = g.bds->r_value[1];
   c[2] = g.bds->r_value[2];
   c[3] = g.bds->r_value[3];
} else {
   c[3] = g.bds->r_value[0];
   c[2] = g.bds->r_value[1];
   c[1] = g.bds->r_value[2];
   c[0] = g.bds->r_value[3];
}

ibm_to_ieee(&ibm, (uint4 *) r);

g.r_value = (double) (r[0]);

t = pow(10.0, (double) (-D));

g.m_value = t*pow(2.0, ((double) E));
g.b_value = t*(g.r_value);

if ( (g.bds->flag) & 128 )  {

   mlog << Error << "\nGribFile::read_record(GribRecord &) -> Spherical Harmonic data not implemented.\n\n";

   exit ( 1 );
//   throw GribError(spher_harm_not_impl_error, __LINE__, __FILE__, "\n\n  GribFile::read_record(GribRecord &) -> Spherical Harmonic data not implemented.\n\n");

}

if ( (g.bds->flag) & 64 )  {

   mlog << Error << "\nGribFile::read_record(GribRecord &) -> Second order packing not implemented.\n\n";

   exit ( 1 );
//   throw GribError(second_ord_pkg_not_impl_error, __LINE__, __FILE__, "\n\n  GribFile::read_record(GribRecord &) -> Second order packing not implemented.\n\n");

}

g.word_size = (int) (g.bds->size);

if ( g.word_size > 32 )  {

   mlog << Error << "\nGribFile::read_record(GribRecord &) -> Binary data word size of "
        << g.word_size << " found\n\n"
        << "   Binary data word sizes > 32 bits are not implemented.\n\n";

   exit ( 1 );
//   char temp_str[max_temp_str_length];

//   snprintf(temp_str, sizeof(temp_str), "\n\n  GribFile::read_record(GribRecord &) -> Binary data word size of %d found\n\n   Binary data word sizes > 32 bits are not implemented.\n\n", g.word_size);

//   throw GribError(word_size_error, __LINE__, __FILE__, temp_str);

}

g.data_lseek_offset = (long) (g.record_lseek_offset + bytes_processed + 11);

bytes = char3_to_int(g.bds->length) - 11;

g.extend_data(bytes);
g.data.clear();

unsigned char *begin = rep->buf + bytes_processed + 11;
g.data.assign(begin, begin+bytes);

g.mask = 0L;

for (j=0; j<(g.word_size); ++j)   g.mask |= (1 << j);

   //
   //  Done
   //

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int GribFile::skip_header()

{

size_t j, n_read;
off_t pos;

bool found = false;

pos = lseek(rep->fd, 0L, SEEK_SET);

pos = lseek(rep->fd, 0L, SEEK_CUR);

pos -= 4;

if ( pos < 0 )  pos = 0L;

lseek(rep->fd, pos, SEEK_SET);

n_read = ::read(rep->fd, (char *) (rep->buf), rep->buf_size);

if ( n_read == 0 )  {

   mlog << Error << "\nGribFile::skip_header() -> "
        << "\"GRIB\" magic cookie not found in grib file!!\n\n";

   return ( 0 );

//   throw GribError(missing_magic_cookie_error, __LINE__, __FILE__, "\n\n  GribFile::skip_header() -> \"GRIB\" magic cookie not found in grib file!!\n\n");
}

if ( n_read < 0 )  {

//   mlog << Error << "\nGribFile::skip_header() -> "
//        << "file read error\n\n";

   return ( 0 );

//   throw GribError(file_read_error, __LINE__, __FILE__, "\n\n  GribFile::skip_header() -> file read error\n\n");
}

for (j=0; j<=min(grib_search_bytes, (n_read - 4)); ++j)  {

   if ( strncmp((char *) (rep->buf + j), "GRIB", 4) == 0 )  {
      found = true;
      break;
   }
}//  for j

if ( !found ) {

   mlog << Error << "\nGribFile::skip_header() -> "
        << "can't find \"GRIB\" magic cookie\n\n";

   return ( 0 );

//   throw GribError(file_read_error, __LINE__, __FILE__, "\n\n  GribFile::skip_header() -> can't find \"GRIB\" magic cookie\n\n");
}

rep->file_start = (long) (pos + j);

pos = lseek(rep->fd, rep->file_start, SEEK_SET);

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


void GribFile::index_records()

{

GribRecord *g = new GribRecord();
Section1_Header *pds_ptr = (Section1_Header *) 0;

rep->record_extend(1);

rep->n_records = 0;

while ( read_record(*g) )  {

   rep->record_info[rep->n_records].lseek_offset = g->record_lseek_offset;

   pds_ptr = (Section1_Header *) g->pds;

   rep->record_info[rep->n_records].gribcode = pds_ptr->grib_code;

   ++rep->n_records;

   if ( rep->n_records >= rep->n_alloc )  rep->record_extend(rep->n_alloc + 1);

}

delete g;

}


////////////////////////////////////////////////////////////////////////


size_t GribFile::read()

{

size_t n_read;

if ( (n_read = ::read(rep->fd, (char *) rep->buf, rep->buf_size)) < 0 )  {

   mlog << Error << "\nGribFile::read() -> file read error\n\n";

   exit ( 1 );
//   throw GribError(file_read_error, __LINE__, __FILE__, "\n\n  GribFile::read() -> file read error\n\n");

}

return ( n_read );

}


////////////////////////////////////////////////////////////////////////


size_t GribFile::read(size_t bytes)

{

size_t n_read;

if ( bytes > rep->buf_size )  {

   mlog << Error << "\nGribFile::read(int) -> can't read " << bytes
        << " bytes into a " << (rep->buf_size) << " byte buffer\n\n";

   exit ( 1 );
//   char temp_str[max_temp_str_length];

//   snprintf(temp_str, sizeof(temp_str), "\n\n  GribFile::read(int) -> can't read %d bytes into a %d byte buffer\n\n", bytes, rep->buf_size);

//   throw GribError(buffer_size_error, __LINE__, __FILE__, temp_str);

}

if ( (n_read = ::read(rep->fd, (char *) rep->buf, bytes)) < 0 )  {

   mlog << Error << "\nGribFile::read() -> file read error\n\n";

   exit ( 1 );
//   throw GribError(file_read_error, __LINE__, __FILE__, "\n\n  GribFile::read() -> file read error\n\n");

}

return ( n_read );

}


////////////////////////////////////////////////////////////////////////


size_t GribFile::read(off_t buffer_offset, size_t bytes)

{

size_t n_read;
int B;

if ( (buffer_offset + bytes) > (rep->buf_size) )  {

   mlog << Error << "\nGribFile::read(int, int) -> requested read would overflow buffer\n\n";

   exit ( 1 );
//   throw GribError(read_overflow_error, __LINE__, __FILE__, "\n\n  GribFile::read(int, int) -> requested read would overflow buffer\n\n");

}

B = min<int>(bytes, rep->buf_size - buffer_offset);   //  to please fortify

n_read = ::read(rep->fd, (rep->buf + buffer_offset), B);

if ( n_read != B )  {

   mlog << Error << "\nGribFile::read() -> file read error ... requested " << B << " bytes, got " << n_read << "\n\n";

   exit ( 1 );
//   char temp_str[max_temp_str_length];

//   snprintf(temp_str, sizeof(temp_str), "\n\n  GribFile::read() -> file read error ... requested %d bytes, got %d\n\n", bytes, n_read);

//   throw GribError(file_read_error, __LINE__, __FILE__, temp_str);

}

return ( n_read );

}


////////////////////////////////////////////////////////////////////////


int GribFile::read(void *c, int bytes)

{

int n_read;

if ( (n_read = ::read(rep->fd, c, bytes)) < 0 )  {

   mlog << Error << "\nGribFile::read() -> file read error\n\n";

   exit ( 1 );
//   throw GribError(file_read_error, __LINE__, __FILE__, "\n\n  GribFile::read() -> file read error\n\n");

}

return ( n_read );

}


////////////////////////////////////////////////////////////////////////


void GribFile::close()

{

if ( rep && (--rep->referenceCount == 0) )  delete rep;

rep = (GribFileRep *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


unsigned int GribFile::n_records()

{

return ( rep ? (rep->n_records) : 0 );

}


////////////////////////////////////////////////////////////////////////


off_t GribFile::record_offset(int n)

{

if ( (n < 0) || (n >= (rep->n_records)) )  {

   mlog << Error << "\nGribFile::record_offset(int) -> range check error\n\n";

   exit ( 1 );
//   throw GribError(range_chk_error, __LINE__, __FILE__, "\n\n  GribFile::record_offset(int) -> range check error\n\n");

}

return ( rep->record_info[n].lseek_offset );

}


////////////////////////////////////////////////////////////////////////


int GribFile::gribcode(int n)

{

if ( (n < 0) || (n >= (rep->n_records)) )  {

   mlog << Error << "\nGribFile::gribcode(int) -> range check error\n\n";

   exit ( 1 );
//   throw GribError(range_chk_error, __LINE__, __FILE__, "\n\n  GribFile::gribcode(int) -> range check error\n\n");

}

return ( rep->record_info[n].gribcode );

}


////////////////////////////////////////////////////////////////////////


int GribFile::issue()

{

return ( rep ? (rep->issue) : 0 );

}


////////////////////////////////////////////////////////////////////////


int GribFile::lead()

{

return ( rep ? (rep->lead) : 0 );

}


////////////////////////////////////////////////////////////////////////


const char * GribFile::name()

{

if ( !rep ) return ( (char *) 0 );

return ( rep->name );

}


////////////////////////////////////////////////////////////////////////


void GribFile::seek_record(int n)

{
off_t offset;

if ( (n < 0) || (n >= (rep->n_records)) )  {

   mlog << Error << "\nGribFile::seek_record(int) -> range check error\n\n";

   exit ( 1 );
//   throw GribError(range_chk_error, __LINE__, __FILE__, "\n\n  GribFile::seek_record(int) -> range check error\n\n");

}

offset = lseek(rep->fd, rep->record_info[n].lseek_offset, SEEK_SET);

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


double char4_to_dbl(const unsigned char *c)

{
   int positive, power;
   unsigned int abspower;
   long int mant;
   double value, exp;

   mant = (c[1] << 16) + (c[2] << 8) + c[3];
   if (mant == 0) return 0.0;

   positive = (c[0] & 0x80) == 0;
   power = (int) (c[0] & 0x7f) - 64;
   abspower = (unsigned int) (abs(power));


   /* calc exp */
   exp = 16.0;
   value = 1.0;
   while (abspower) {
      if (abspower & 1) {
         value *= exp;
      }
      exp = exp * exp;
      abspower >>= 1;
   }

   if (power < 0) value = 1.0 / value;
   value = value * mant / 16777216.0;
   if (positive == 0) value = -value;

   return(value);
}


////////////////////////////////////////////////////////////////////////


unsigned int char3_to_int(const unsigned char *c)

{

unsigned int i, j;

i = 0;

for (j=0; j<3; ++j)  i = i*256 + c[j];

return ( i );

}


////////////////////////////////////////////////////////////////////////


unsigned int char2_to_int(const unsigned char *c)

{

unsigned int i, j;

i = 0;

for (j=0; j<2; ++j)  i = i*256 + c[j];

return ( i );

}


////////////////////////////////////////////////////////////////////////


void ibm_to_ieee(const uint4 *ibm, uint4 *ieee)

{

int j;
uint4 a, b;

// const uint4 m31    = 2147483648;
const uint4 m31    = ((uint4) 1) << 31;
const uint4 m23    =    8388608;
const uint4 m0_23  =   16777215;
const uint4 m24_30 = 2130706432;

*ieee = 0;

b = (*ibm) & m0_23;

if ( b == 0 )   return;

if ( (*ibm) & m31 )  (*ieee) |= m31;

a = ( (*ibm) & m24_30 ) >> 24;

j = 0;

while ( !(b & m23) )  { ++j;  b <<= 1; }

a = (4*a - 130 - j)*m23;

b -= m23;

(*ieee) |= (a | b);

return;

}


////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream &file, const GribRecord &g)

{

file.setf(ios::fixed);

file << separator << "\n\n";
file << *(g.is);
file << separator << "\n\n";
file << *(g.pds);
file << separator << "\n\n";

if ( g.gds_flag )   { file << *(g.gds);  file << separator << "\n\n"; }
if ( g.bms_flag )   { file << *(g.bms);  file << separator << "\n\n"; }

file << *(g.bds);
file << separator << "\n\n";

file << "Grib Record:\n\n";

file << "    rec_num:            " << g.rec_num  << "\n";
file << "    pds_len:            " << g.pds_len  << "\n";
file << "   gds_flag:            " << g.gds_flag << "\n";
file << "   bms_flag:            " << g.bms_flag << "\n";

file << "   nx:                  " << g.nx << "\n";
file << "   ny:                  " << g.ny << "\n";

file << "   m_value:             ";    file.width(10);   file.precision(5);   file << g.m_value << "\n";
file << "   b_value:             ";    file.width(10);   file.precision(5);   file << g.b_value << "\n";
file << "   r_value:             ";    file.width(10);   file.precision(5);   file << g.r_value << "\n";

file << "   d_value:             " << g.d_value << "\n";
file << "   e_value:             " << g.e_value << "\n";

file << "   issue:               " << g.issue << "\n";
file << "   lead:                " << g.lead  << "\n";

file << "   word_size:           " << g.word_size  << "\n";

file << "   record_lseek_offset: " << g.record_lseek_offset  << "\n";
file << "   data_lseek_offset:   " << g.data_lseek_offset    << "\n";

file << "   mask:                " << g.mask << "\n\n";

file << "   Sec 0  (IS)  offset in file " << g.Sec0_offset_in_file << ",   offset in record " << g.Sec0_offset_in_record << "\n\n";

file << "   Sec 1 (PDS)  offset in file " << g.Sec1_offset_in_file << ",   offset in record " << g.Sec1_offset_in_record << "\n\n";

if ( g.gds_flag )
   file << "   Sec 2 (GDS)  offset in file " << g.Sec2_offset_in_file << ",   offset in record " << g.Sec2_offset_in_record << "\n\n";

if ( g.bms_flag )
   file << "   Sec 3 (BMS)  offset in file " << g.Sec3_offset_in_file << ",   offset in record " << g.Sec3_offset_in_record << "\n\n";

file << "   Sec 4 (BDS)  offset in file " << g.Sec4_offset_in_file << ",   offset in record " << g.Sec4_offset_in_record << "\n\n";

file << separator << "\n";

return ( file );

}


////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream &file, const Section0_Header &h)

{

int j;
const unsigned char *u = (const unsigned char *) (&h);

file.setf(ios::fixed);

file << "Section 0 (IS):\n\n";

for (j=0; j<8; ++j)  {

   file << "   octet ";

   file.width(2);   file << (j + 1) << ":   ";

   file.width(3);   file << ((int) u[j]) << "\n";

   if ( (j%5) == 4 )  file << "\n";

}

file << "\n\n";

file << "   grib_name:   " << h.grib_name[0] << h.grib_name[1] << h.grib_name[2] << h.grib_name[3] << "\n";

file << "   length:      " << char3_to_int(h.length) << "\n";

file << "   ed_num:      " << (int) (h.ed_num) << "\n\n";

return ( file );

}


////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream &file, const Section1_Header &h)

{

int j;
const unsigned char *u = (const unsigned char *) (&h);

file.setf(ios::fixed);

file << "Section 1 (PDS):\n\n";

for (j=0; j<28; ++j)  {

   file << "   octet ";

   file.width(2);   file << (j + 1) << ":   ";

   file.width(3);   file << ((int) u[j]) << "\n";

   if ( (j%5) == 4 )  file << "\n";

}

file << "\n\n";

file << "   length:      " << char3_to_int(h.length) << "\n";

file << "   ptv:         " << (int) h.ptv        << "\n";
file << "   center_id:   " << (int) h.center_id  << "\n";
file << "   process_id:  " << (int) h.process_id << "\n";
file << "   grid_id:     " << (int) h.grid_id    << "\n";
file << "   flag:        " << (int) h.flag       << "\n";
file << "   grib_code:   " << (int) h.grib_code  << "\n";
file << "   type:        " << (int) h.type       << "\n";

file << "   level_info:  " << (int) h.level_info[0] << " " << (int) h.level_info[1] << "\n";

file << "   year:        " << (int) h.year       << "\n";
file << "   month:       " << (int) h.month      << "\n";
file << "   day:         " << (int) h.day        << "\n";
file << "   hour:        " << (int) h.hour       << "\n";
file << "   minute:      " << (int) h.minute     << "\n";

file << "   fcst_unit:   " << (int) h.fcst_unit  << "\n";
file << "   p1:          " << (int) h.p1         << "\n";
file << "   p2:          " << (int) h.p2         << "\n";
file << "   tri:         " << (int) h.tri        << "\n";

file << "   nia:         " << char2_to_int(h.nia) << "\n";

file << "   nma:         " << (int) h.nma        << "\n";
file << "   century:     " << (int) h.century    << "\n";
file << "   sub_center:  " << (int) h.sub_center << "\n";

file << "   ens_application:  " << (int) (h.ens_application)  << "\n";
file << "   ens_type:         " << (int) (h.ens_type)         << "\n";
file << "   ens_number:       " << (int) (h.ens_number)       << "\n";

file << "   d_value:     " << char2_to_int(h.d_value) << "\n\n";

return ( file );

}

////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream &file, const Section2_Header &h)

{

int j;
const unsigned char *u = (const unsigned char *) (&h);

file.setf(ios::fixed);

file << "Section 2 (GDS):\n\n";

for (j=0; j<42; ++j)  {

   file << "   octet ";

   file.width(2);   file << (j + 1) << ":   ";

   file.width(3);   file << ((int) u[j]) << "\n";

   if ( (j%5) == 4 )  file << "\n";

}

file << "\n\n";

file << "   length:     " << char3_to_int(h.length) << "\n";

file << "   nv:         " << (int) h.nv             << "\n";
file << "   pvpl:       " << (int) h.pvpl           << "\n";
file << "   type:       " << (int) h.type           << "\n";

file << "   nx:         " << char2_to_int(h.nx)     << "\n";
file << "   ny:         " << char2_to_int(h.ny)     << "\n\n";

if ((h.type == 0) || (h.type == 2) || (h.type == 4))
{

   file << "   lat1:      " << char3_to_int(h.grid_type.latlon_grid.lat1)   << "\n";
   file << "   lon1:      " << char3_to_int(h.grid_type.latlon_grid.lon1)   << "\n";

   file << "   res_flag:  " << (int) h.grid_type.latlon_grid.res_flag       << "\n";

   file << "   lat2:      " << char3_to_int(h.grid_type.latlon_grid.lat2)   << "\n";
   file << "   lon2:      " << char3_to_int(h.grid_type.latlon_grid.lon2)   << "\n";

   file << "   di:        " << char2_to_int(h.grid_type.latlon_grid.di)     << "\n";
   file << "   dj:        " << char2_to_int(h.grid_type.latlon_grid.dj)     << "\n";

   file << "   scan_flag: " << (int) h.grid_type.latlon_grid.scan_flag      << "\n\n";
}
else if ((h.type == 3) || (h.type == 13))
{

   file << "   lat1:       " << char3_to_int(h.grid_type.lambert_conf.lat1)   << "\n";
   file << "   lon1:       " << char3_to_int(h.grid_type.lambert_conf.lon1)   << "\n";

   file << "   res_flag:   " << (int) h.grid_type.lambert_conf.res_flag       << "\n";

   file << "   lov:        " << char3_to_int(h.grid_type.lambert_conf.lov)    << "\n";

   file << "   dx:         " << char3_to_int(h.grid_type.lambert_conf.dx)     << "\n";
   file << "   dy:         " << char3_to_int(h.grid_type.lambert_conf.dy)     << "\n";

   file << "   pc_flag:    " << (int) h.grid_type.lambert_conf.pc_flag        << "\n";

   file << "   scan_flag:  " << (int) h.grid_type.lambert_conf.scan_flag      << "\n";

   file << "   latin1:     " << char3_to_int(h.grid_type.lambert_conf.latin1) << "\n";
   file << "   latin2:     " << char3_to_int(h.grid_type.lambert_conf.latin2) << "\n";

   file << "   lat_sp:     " << char3_to_int(h.grid_type.lambert_conf.lat_sp) << "\n";
   file << "   lon_sp:     " << char3_to_int(h.grid_type.lambert_conf.lon_sp) << "\n\n";
}
else if ((h.type == 5))
{

   file << "   lat1:       " << char3_to_int(h.grid_type.stereographic.lat1)   << "\n";
   file << "   lon1:       " << char3_to_int(h.grid_type.stereographic.lon1)   << "\n";

   file << "   res_flag:   " << (int) h.grid_type.stereographic.res_flag       << "\n";

   file << "   lov:        " << char3_to_int(h.grid_type.stereographic.lov)    << "\n";

   file << "   dx:         " << char3_to_int(h.grid_type.stereographic.dx)     << "\n";
   file << "   dy:         " << char3_to_int(h.grid_type.stereographic.dy)     << "\n";

   file << "   pc_flag:    " << (int) h.grid_type.stereographic.pc_flag        << "\n";

   file << "   scan_flag:  " << (int) h.grid_type.stereographic.scan_flag      << "\n\n";
}

return ( file );

}


////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream &file, const Section3_Header &h)

{

int j;
const unsigned char *u = (const unsigned char *) (&h);

file.setf(ios::fixed);

file << "Section 3 (BMS):\n\n";

for (j=0; j<6; ++j)  {

   file << "   octet ";

   file.width(2);   file << (j + 1) << ":   ";

   file.width(3);   file << ((int) u[j]) << "\n";

   if ( (j%5) == 4 )  file << "\n";

}

file << "\n\n";

file << "   length: " << char3_to_int(h.length) << "\n";

file << "   num:    " << (int) h.num << "\n";

file << "   flag:   " << char2_to_int(h.flag) << "\n\n";

return ( file );

}


////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream &file, const Section4_Header &h)

{

int j;
const unsigned char *u = (const unsigned char *) (&h);

file.setf(ios::fixed);

file << "Section 4 (BDS):\n\n";

for (j=0; j<12; ++j)  {

   file << "   octet ";

   file.width(2);   file << (j + 1) << ":   ";

   file.width(3);   file << ((int) u[j]) << "\n";

   if ( (j%5) == 4 )  file << "\n";

}

file << "\n\n";

file << "   length:     " << char3_to_int(h.length) << "\n";

file << "   flag:       " << (int) h.flag << "\n";

file << "   e_value:    " << char2_to_int(h.e_value) << "\n";

file << "   r_value:    " << (int) h.r_value[0] << " "
                          << (int) h.r_value[1] << " "
                          << (int) h.r_value[2] << " "
                          << (int) h.r_value[3] << "\n";

file << "   size:       " << (int) h.size << "\n";

file << "   data_start: " << (int) h.data_start << "\n\n";

return ( file );

}


////////////////////////////////////////////////////////////////////////


long find_magic_cookie(int fd)

{

int j;
size_t n_read;
long pos = 0;
char buf[100];
off_t offset;

while ( (n_read = read(fd, buf, sizeof(buf))) > 0 )  {

   pos = lseek(fd, 0L, SEEK_CUR) - n_read;

   for (j=0; j<=(n_read - 4); ++j)  {

      if ( (buf[j] == 'G') && (buf[j + 1] == 'R') && (buf[j + 2] == 'I') && (buf[j + 3] == 'B') )  {

         pos += j;

         offset = lseek(fd, pos + 2, SEEK_SET);

         return ( pos );

      }

   }   //  for j

   if ( n_read < 10 )  return ( -1 );

   pos += n_read - 5;

   offset = lseek(fd, pos, SEEK_SET);

}   //  while

if ( n_read == 0 )  return ( -1 );

if ( n_read < 0 )  {

   mlog << Error << "\nfind_magic_cookie(int) -> trouble reading file\n\n";

   exit ( 1 );
//   throw GribError(file_read_error, __LINE__, __FILE__, "\n\n  find_magic_cookie(int) -> trouble reading file\n\n");

}

return ( pos );

}


////////////////////////////////////////////////////////////////////////


int calc_lead_time(Section1_Header *pds)

{

int multiplier = 0;

switch ( pds->fcst_unit )  {

   case 0:   multiplier =        60;  break;   //  minute
   case 1:   multiplier =      3600;  break;   //  hour
   case 2:   multiplier =     86400;  break;   //  day
   case 3:   multiplier =   2592000;  break;   //  month (30 days)
   case 4:   multiplier =  31536000;  break;   //  year
   case 10:  multiplier =     10800;  break;   //  3 hours
   case 11:  multiplier =     21600;  break;   //  6 hours
   case 12:  multiplier =     43200;  break;   //  12 hours
   case 13:  multiplier =       900;  break;   //  15 minutes
   case 14:  multiplier =      1800;  break;   //  30 minutes
   case 254: multiplier =         1;  break;   //  seconds

   default:
      mlog << Error << "\ncalc_lead_time(Section1_Header *) -> bad value for fcst unit: " << (pds->fcst_unit) << "\n\n";
      exit ( 1 );
//      char temp_str[max_temp_str_length];

//      snprintf(temp_str, sizeof(temp_str), "\n\n  calc_lead_time(Section1_Header *) -> bad value for fcst unit: %d\n\n", pds->fcst_unit);

//      throw GribError(bad_fcst_unit_val_error, __LINE__, __FILE__, temp_str);

}

int pp1;

pp1 = pds->p1;

return ( multiplier*pp1 );

}


////////////////////////////////////////////////////////////////////////




