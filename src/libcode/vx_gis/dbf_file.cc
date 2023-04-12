// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>

#include "vx_util.h"
#include "vx_cal.h"

#include "dbf_file.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class DbfHeader
   //


////////////////////////////////////////////////////////////////////////


DbfHeader::DbfHeader()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


DbfHeader::~DbfHeader()

{

clear();

}


////////////////////////////////////////////////////////////////////////


DbfHeader::DbfHeader(const DbfHeader & h)

{

init_from_scratch();

assign(h);

}


////////////////////////////////////////////////////////////////////////


DbfHeader & DbfHeader::operator=(const DbfHeader & h)

{

if ( this == &h )  return ( * this );

assign(h);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void DbfHeader::init_from_scratch()

{

subrec = 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void DbfHeader::clear()

{

if ( subrec )  { delete [] subrec;  subrec = 0; }

type = 0;

last_update_mjd = 0;

n_records = 0;

pos_first_record = 0;

record_length = 0;

table_flag = 0;

code_page_mark = 0;

n_subrecs = 0;


return;

}


////////////////////////////////////////////////////////////////////////


void DbfHeader::assign(const DbfHeader & h)

{

clear();

type             = h.type;
last_update_mjd  = h.last_update_mjd;
n_records        = h.n_records;
pos_first_record = h.pos_first_record;
record_length    = h.record_length;
table_flag       = h.table_flag;
code_page_mark   = h.code_page_mark;
n_subrecs        = h.n_subrecs;

if ( h.subrec )  {

   int j;

   subrec = new DbfSubRecord [h.n_subrecs];

   for (j=0; j<(h.n_subrecs); ++j)  subrec[j] = h.subrec[j];

}   //  if 


return;

}


////////////////////////////////////////////////////////////////////////


void DbfHeader::dump(ostream & out, int depth) const

{

Indent p0(depth);
Indent p1(depth + 1);
int j, pos;
int m, d, y;

   //
   //  header
   //

out << p0 << "Header ...\n";

out << p1 << "type             = " << type             << '\n';

mjd_to_date(last_update_mjd, m, d, y);

out << p1 << "last_update_mjd  = " << last_update_mjd << "   ("
          << short_month_name[m] << ' ' << d << ", " << y << ")\n";

out << p1 << "n_records        = " << n_records        << '\n';
out << p1 << "pos_first_record = " << pos_first_record << '\n';
out << p1 << "record_length    = " << record_length    << '\n';
out << p1 << "table_flag       = " << table_flag       << '\n';
out << p1 << "code_page_mark   = " << code_page_mark   << '\n';
out << p1 << "n_subrecs        = " << n_subrecs        << "   (inferred)\n";

pos = 0;

   //
   //  subrecords (if any)
   //

if ( subrec )  {

   out << p0 << "\n";
   out << p0 << "SubRecords ... \n";

   for (j=0; j<n_subrecs; ++j)  {

      // out << "cumulative length = " << pos << '\n';

      out << p1 << "SubRecord " << j << "\n";

      subrec[j].dump(out, p1.depth + 1);

      pos += subrec[j].field_length;

   }

} else {

   out << p0 << "No SubRecords\n";

}

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void DbfHeader::set_header(unsigned char * buf)

{

clear();

int m, d, y;
short s;

type = (int) (buf[0]);

y = (int) (buf[1]);
m = (int) (buf[2]);
d = (int) (buf[3]);

y += 1900;

last_update_mjd = date_to_mjd(m, d, y);

memcpy(&n_records, buf + 4, 4);

memcpy(&s, buf + 8, 2);

pos_first_record = (int) s;

memcpy(&s, buf + 10, 2);

record_length = (int) s;

table_flag     = (int) (buf[28]);
code_page_mark = (int) (buf[29]);

   //

n_subrecs = (pos_first_record - 1 - 32)/32;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void DbfHeader::set_subrecords(int fd)

{

int j;
int bytes, n_read;
int pos;


subrec = new DbfSubRecord [n_subrecs];

unsigned char buf[32];

   //
   //  seek to the first record
   //

pos = 32;

if ( ::lseek(fd, pos, SEEK_SET) < 0 )  {

   mlog << Error << "\nDbfHeader::set_subrecords(int) -> "
        << "lseek error ... " << strerror(errno) << "\n\n";

   exit ( 1 );

}

   //
   //  read the records
   //

bytes = 32;

pos = 1;   //  ignore first (ie, delete flag) byte

for (j=0; j<n_subrecs; ++j)  {

   n_read = ::read(fd, buf, bytes);

   if ( n_read != bytes )  {

      mlog << Error << "\nDbfHeader::set_subrecords(int) -> "
           << "read error ... n_read = " << n_read << "\n\n";

      exit ( 1 );

   }

   subrec[j].set(buf);

   subrec[j].start_pos = pos;

   pos += subrec[j].field_length;

}   //  for j

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


DbfSubRecord * DbfHeader::lookup_subrec(const char * text) const

{

if ( empty(text) )  {

   mlog << Error << "\nDbfHeader::set_subrecords(const char *) -> "
        << "empty string!\n\n";

   exit ( 1 );

}

int j;

for (j=0; j<n_subrecs; ++j)  {

   if ( text == subrec[j].field_name )  return ( subrec + j );

}


return ( 0 );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class DbfSubRecord
   //


////////////////////////////////////////////////////////////////////////


DbfSubRecord::DbfSubRecord()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


DbfSubRecord::~DbfSubRecord()

{

clear();

}


////////////////////////////////////////////////////////////////////////


DbfSubRecord::DbfSubRecord(const DbfSubRecord & r)

{

init_from_scratch();

assign(r);

}


////////////////////////////////////////////////////////////////////////


DbfSubRecord & DbfSubRecord::operator=(const DbfSubRecord & r)

{

if ( this == &r )  return ( * this );

assign(r);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void DbfSubRecord::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void DbfSubRecord::clear()

{

field_name = "";

field_type = 0;

displacement = 0;

field_length = 0;

dp = 0;

field_flags = 0;

autoinc_next_value = 0;
autoinc_step_value = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void DbfSubRecord::assign(const DbfSubRecord & r)

{

clear();

field_name = r.field_name;

field_type         = r.field_type;
displacement       = r.displacement;
field_length       = r.field_length;
dp                 = r.dp;
field_flags        = r.field_flags;
autoinc_next_value = r.autoinc_next_value;
autoinc_step_value = r.autoinc_step_value;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void DbfSubRecord::dump(ostream & out, int depth) const

{

Indent prefix(depth);

if ( field_name.length() > 0 )  {

   out << prefix << "field_name         = \"" << field_name << "\"\n";

} else {

   out << prefix << "field_name         = (nul)\n";

}

out << prefix << "start_pos          = " << start_pos           << '\n';
out << prefix << "field_type         = " << field_type          << '\n';
out << prefix << "displacement       = " << displacement        << '\n';
out << prefix << "field_length       = " << field_length        << '\n';
out << prefix << "dp                 = " << dp                  << '\n';
out << prefix << "field_flags        = " << ((int) field_flags) << '\n';
out << prefix << "autoinc_next_value = " << autoinc_next_value  << '\n';
out << prefix << "autoinc_step_value = " << autoinc_step_value  << '\n';

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void DbfSubRecord::set(unsigned char * buf)

{

clear();

string str_buf((char *) buf);

field_name = str_buf.substr(0, 10);

field_type = buf[11];

memcpy(&displacement, buf + 12, 4);

field_length = (int) (buf[16]);

dp = (int) (buf[17]);

field_flags = buf[18];

memcpy(&autoinc_next_value, buf + 19, 4);

autoinc_step_value = (int) (buf[23]);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


void dump_record(ostream & out, const int depth, const unsigned char * buf, const DbfHeader & h)

{

int j, k;
int pos, len, k_max;
int max_name_len;
Indent p0 (depth);
const char * line = (const char *) buf;
const DbfSubRecord * s = h.subrec;

max_name_len = 0;

for (j=0; j<(h.n_subrecs); ++j)  {

  len = s[j].field_name.length();

   if ( len > max_name_len )  max_name_len = len;

}

pos = 1;   //  skip first byte

for (j=0; j<(h.n_subrecs); ++j)  {

   len = s[j].field_length;

   out << p0 << s[j].field_name;

   for (k=s[j].field_name.length(); k<max_name_len; ++k)  out << ' ';

   out << " = \"";

   for (k_max=(pos + len - 1); k_max>=pos; --k_max)  {

      if ( line[k_max] != ' ' )  break;  

   }

   for (k=pos; k<=k_max; ++k)  out << line[k];

   out << "\"\n" << flush;

   pos += len;

}   //  for j

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class DbfFile
   //


////////////////////////////////////////////////////////////////////////


DbfFile::DbfFile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


DbfFile::~DbfFile()

{

close();

}


////////////////////////////////////////////////////////////////////////


DbfFile::DbfFile(const DbfFile &)

{

mlog << Error << "\nDbfFile::DbfFile(const DbfFile &) -> "
     << "should never be called!\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


DbfFile & DbfFile::operator=(const DbfFile &)

{

mlog << Error << "\nDbfFile::operator=(const DbfFile &) -> "
     << "should never be called!\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


void DbfFile::init_from_scratch()

{

fd = -1;

close();

return;

}


////////////////////////////////////////////////////////////////////////


void DbfFile::close()

{

if ( fd >= 0 )  ::close(fd);

fd = -1;

Header.clear();

Filename.clear();

At_Eof = false;


return;

}


////////////////////////////////////////////////////////////////////////


const char * DbfFile::filename() const

{

if ( Filename.empty() )  return ( (const char *) 0 );

return ( Filename.text() );

}


////////////////////////////////////////////////////////////////////////


bool DbfFile::open(const char * path)

{

size_t n_read, bytes;
int j;

close();

if ( (fd = met_open(path, O_RDONLY)) < 0 )  {

   fd = -1;

   return ( false );

}

Filename = path;

   //
   //  read the file header
   //

const size_t  buf_size = 65536;
unsigned char buf[buf_size];

bytes = 32;

if((n_read = ::read(fd, buf, bytes)) != bytes) {
   mlog << Warning << "\nDbfFile::open(const char * path) -> "
        << "trouble reading file header from dbf file \""
        << path << "\"\n\n";
   close();

   return(false);
}

Header.set_header(buf);

   //
   //  subrecords
   //

Header.set_subrecords(fd);

   //
   //  done
   //

return ( true );

}

////////////////////////////////////////////////////////////////////////


int DbfFile::position() const

{

if ( ! is_open() )  {

   mlog << Error << "\nDbfFile::position() -> "
        << "no file open!\n\n";

   exit ( 1 );

}

const int pos = ::lseek(fd, 0, SEEK_CUR);

if ( pos < 0 )  {

   mlog << Error << "\nDbfFile::position() const -> "
        << "lseek error ... " << strerror(errno) << "\n\n";

   exit ( 1 );

}


return ( pos );

}


////////////////////////////////////////////////////////////////////////


void DbfFile::lseek(int offset, int whence)

{


if ( ! is_open() )  {

   mlog << Error << "\nDbfFile::lseek() -> "
        << "no file open!\n\n";

   exit ( 1 );

}


if ( ::lseek(fd, offset, whence) < 0 )  {

   mlog << Error << "\nDbfFile::lseek() -> "
        << "lseek(2) failed! ... " << strerror(errno) << "\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////


bool DbfFile::read(unsigned char * buf, int n_bytes)

{

if ( ! is_open() )  {

   mlog << Error << "\nDbfFile::read() -> "
        << "no file open!\n\n";

   exit ( 1 );

}

int n_read;

n_read = ::read(fd, buf, n_bytes);

if ( n_read < 0 )  {   //  some kind of error

   mlog << Error << "\nDbfFile::read() -> "
        << "read error on dbf file \"" << Filename << "\"\n\n";

   exit ( 1 );

}

if ( n_read == 0 )  return ( false );

if ( n_read == n_bytes )  return ( true );

return ( false );   //  gotta return something

}


////////////////////////////////////////////////////////////////////////


StringArray DbfFile::subrecord_names()

{

StringArray sa;

   //
   //  subrecords (if any)
   //

if ( Header.subrec )  {

   for (int j=0; j<Header.n_subrecs; ++j)  {

      sa.add(Header.subrec[j].field_name);

   }

}

return ( sa );

}


////////////////////////////////////////////////////////////////////////


StringArray DbfFile::subrecord_values(int i_rec)

{

size_t n_read, bytes, pos;
const size_t buf_size = 65536;
unsigned char buf[buf_size];
ConcatString cs;
StringArray sa;
int j;

   //
   //  check range
   //

if ( i_rec < 0 || i_rec > (Header.n_records) )  {

   mlog << Error << "\nDbfFile::subrecord_values(int i_rec) -> "
        << "requested record index (" << i_rec << ") out of range (0,"
        << Header.n_records << ")!\n\n";

   exit ( 1 );

}

bytes = Header.record_length;

if ( ::lseek(fd, Header.pos_first_record + i_rec * bytes, SEEK_SET) < 0 )  {

   mlog << Error << "\nDbfFile::subrecord_values(int i_rec) -> "
        << "lseek error\n\n";

   exit ( 1 );

}

   //
   //  read the requested record into the buffer
   //

n_read = ::read(fd, buf, bytes < buf_size ? bytes : buf_size);

if ( n_read != bytes )  {

   mlog << Error << "\nDbfFile::subrecord_values(int i_rec) -> "
        << "read error ... n_read = " << n_read << "\n\n";

   exit ( 1 );

}

if ( Header.record_length < buf_size)  buf[Header.record_length] = 0;

std::string s = (const char *) buf+1; //  skip first byte

   //
   //  parse each subrecord value
   //

for (j=0,pos=0; j<(Header.n_subrecs); ++j)  {

   cs = s.substr(pos, Header.subrec[j].field_length);
   cs.ws_strip();
   sa.add(cs);

   pos += Header.subrec[j].field_length;

}

return ( sa );

}


////////////////////////////////////////////////////////////////////////
