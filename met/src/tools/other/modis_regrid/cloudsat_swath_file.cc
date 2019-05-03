// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
#include <string.h>
#include <cmath>

#include "vx_util.h"

#include "cloudsat_swath_file.h"
#include "sat_utils.h"


////////////////////////////////////////////////////////////////////////


static const bool do_shuffle = false;

static const int buf_size = 65536;
// static const int buf_size = 15000000;

static unsigned char buf[buf_size];

static const int max_dims = 100;


////////////////////////////////////////////////////////////////////////


static void clear_buf();
static void clear_buf(void *, int nbytes);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class SatDimension
   //


////////////////////////////////////////////////////////////////////////


SatDimension::SatDimension()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


SatDimension::~SatDimension()

{

clear();

}


////////////////////////////////////////////////////////////////////////


SatDimension::SatDimension(const SatDimension & d)

{

init_from_scratch();

assign(d);

}


////////////////////////////////////////////////////////////////////////


SatDimension & SatDimension::operator=(const SatDimension & d)

{

if ( this == &d )  return ( * this );

assign(d);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void SatDimension::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void SatDimension::clear()

{

Name.clear();

Size = -1;

return;

}


////////////////////////////////////////////////////////////////////////


void SatDimension::assign(const SatDimension & d)

{

clear();

Name = d.Name;

Size = d.Size;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void SatDimension::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Name = ";

if ( Name.empty() )  out << "(nul)\n";
else                 out << '\"' << Name << "\"\n";

out << prefix << "Size = " << Size << "\n";


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void SatDimension::set_name(const char * text)

{

Name = text;

return;

}


////////////////////////////////////////////////////////////////////////


void SatDimension::set_size(int k)

{

Size = k;

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class SatAttribute
   //


////////////////////////////////////////////////////////////////////////


SatAttribute::SatAttribute()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


SatAttribute::~SatAttribute()

{

clear();

}


////////////////////////////////////////////////////////////////////////


SatAttribute::SatAttribute(const SatAttribute & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


SatAttribute & SatAttribute::operator=(const SatAttribute & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void SatAttribute::init_from_scratch()

{

Ival = (int *) 0;

Dval = (double *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void SatAttribute::clear()

{

Name.clear();

Numbertype = -1;

Bytes = -1;

Nvalues = 0;

Sval.clear();

if ( Ival )  { delete [] Ival;  Ival = (int *) 0; }
if ( Dval )  { delete [] Dval;  Dval = (double *) 0; }

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void SatAttribute::assign(const SatAttribute & a)

{

clear();


Name = a.Name;

Numbertype = a.Numbertype;

Bytes = a.Bytes;


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void SatAttribute::dump(ostream & out, int depth) const

{

int j;
Indent prefix(depth);

out << prefix << "Name       = ";

if ( Name.empty() )  out << "(nul)\n";
else                 out << '\"' << Name << "\"\n";

out << prefix << "Numbertype = " << numbertype_to_string(Numbertype) << "\n";

out << prefix << "Bytes      = " << Bytes << "\n";

out << prefix << "Nvalues    = " << Nvalues << "\n";


if ( Nvalues > 1 )   out << prefix << "Values     = ";
else                 out << prefix << "Value      = ";

if ( Nvalues > 1 )  out << "[ ";

for (j=0; j<Nvalues; ++j)  {


   switch ( Numbertype )  {

      case nt_char_8:
         if ( Sval.empty() )  out << "(nul)";
         else                 out << '\"' << Sval << '\"';
         break;

      case nt_int_8:    //  fall through
      case nt_int_16:   //  fall through
      case nt_int_32:   //  fall through
         out << Ival[j];
         break;

      case nt_float_32:   //  fall through
      case nt_float_64:   //  fall through
         out << Dval[j];
         break;

      default:
         mlog << Error
              << "\n\n  SatAttribute::dump() -> bad number type ... "
              << numbertype_to_string(Numbertype) << "\n\n";
         exit ( 1 );
         break;

   }   //  switch

   if ( j < (Nvalues - 1) )  out << ", ";

}   //  for j

if ( Nvalues > 1 )  out << " ]";

out << "\n";


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void SatAttribute::set_name(const char * text)

{

Name = text;

return;

}


////////////////////////////////////////////////////////////////////////


void SatAttribute::set_number_type(int k)

{

Numbertype = k;

return;

}


////////////////////////////////////////////////////////////////////////


void SatAttribute::set_bytes(int k)

{

Bytes = k;

return;

}


////////////////////////////////////////////////////////////////////////


void SatAttribute::set_value(int nt, unsigned char * b, int n)

{

int j;
int bytes_per_sample;
float f[2];
double d[2];
short s[2];
char c[2];


Sval.clear();
if ( Dval )  { delete [] Dval;  Dval = (double *) 0; }
if ( Ival )  { delete [] Ival;  Ival = (int *) 0; }

Numbertype = nt;

Bytes = n;

Nvalues = 0;

bytes_per_sample = 0;

switch ( Numbertype )  {

   case nt_char_8:
      Nvalues = 1;
      for (j=0; j<Bytes; ++j)  { Sval.add((char) b[j]); }
      break;

   /////////////////////////////////////////////////////////////////////

   case nt_int_8:
      bytes_per_sample = 1;
      // if ( Bytes%bytes_per_sample )  {
      //    mlog << Error
      //         << "\n\n  SatAttribute::set_value() -> bad short size ... " << Bytes << "\n\n";
      //    exit ( 1 ); 
      // }
      Nvalues = Bytes/bytes_per_sample;
      Ival = new int [Nvalues];
      for (j=0; j<Nvalues; ++j)  {
         memcpy(c, b + j*bytes_per_sample, bytes_per_sample);
         // if ( do_shuffle )  shuffle_1(c);
         Ival[j] = (int) (c[0]);
      }
      break;

   /////////////////////////////////////////////////////////////////////

   case nt_int_16:
      bytes_per_sample = 2;
      if ( Bytes%bytes_per_sample )  {
         mlog << Error
              << "\n\n  SatAttribute::set_value() -> bad short size ... " << Bytes << "\n\n";
         exit ( 1 ); 
      }
      Nvalues = Bytes/bytes_per_sample;
      Ival = new int [Nvalues];
      for (j=0; j<Nvalues; ++j)  {
         memcpy(s, b + j*bytes_per_sample, bytes_per_sample);
         if ( do_shuffle )  shuffle_2(s);
         Ival[j] = (int) (s[0]);
      }
      break;

   /////////////////////////////////////////////////////////////////////

   case nt_float_32:
      bytes_per_sample = 4;
      if ( Bytes%bytes_per_sample )  {
         mlog << Error
              << "\n\n  SatAttribute::set_value() -> bad float size ... " << Bytes << "\n\n";
         exit ( 1 ); 
      }
      Nvalues = Bytes/bytes_per_sample;
      Dval = new double [Nvalues];
      for (j=0; j<Nvalues; ++j)  {
         memcpy(f, b + j*bytes_per_sample, bytes_per_sample);
         if ( do_shuffle )  shuffle_4(f);
         Dval[j] = (double) (f[0]);
      }
      break;

   /////////////////////////////////////////////////////////////////////

   case nt_float_64:
      bytes_per_sample = 8;
      if ( Bytes%bytes_per_sample )  {
         mlog << Error
              << "\n\n  SatAttribute::set_value() -> bad double size ... " << Bytes << "\n\n";
         exit ( 1 ); 
      }
      Nvalues = Bytes/bytes_per_sample;
      Dval = new double [Nvalues];
      for (j=0; j<Nvalues; ++j)  {
         memcpy(d, b + bytes_per_sample*j, bytes_per_sample);
         if ( do_shuffle )  shuffle_8(d);
         Dval[j] = d[0];
      }
      break;

   /////////////////////////////////////////////////////////////////////

   default:
      mlog << Error
           << "\n\n  SatAttribute::set_value() -> bad numbertype ... "
           << numbertype_to_string(Numbertype) << "\n\n";
      exit ( 1 );
      break;

}   //  switch




   //
   //  done
   //

return;

}



////////////////////////////////////////////////////////////////////////


   //
   //  Code for class SwathDataField
   //


////////////////////////////////////////////////////////////////////////


SwathDataField::SwathDataField()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


SwathDataField::~SwathDataField()

{

clear();

}


////////////////////////////////////////////////////////////////////////


SwathDataField::SwathDataField(const SwathDataField & df)

{

init_from_scratch();

assign(df);

}


////////////////////////////////////////////////////////////////////////


SwathDataField & SwathDataField::operator=(const SwathDataField & df)

{

if ( this == &df )  return ( * this );

assign(df);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void SwathDataField::init_from_scratch()

{

Dimensions = (SatDimension **) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void SwathDataField::clear()

{

Name.clear();

Rank = -1;

Numbertype = -1;

Ndimensions = 0;

if ( Dimensions )  { delete [] Dimensions;  Dimensions = (SatDimension **) 0; }

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void SwathDataField::assign(const SwathDataField & df)

{

clear();

Name = df.Name;

Rank = df.Rank;

Numbertype = df.Numbertype;

Ndimensions = df.Ndimensions;

if ( df.Dimensions )  {

   int j;

   Dimensions = new SatDimension * [Ndimensions];

   for (j=0; j<Ndimensions; ++j)  Dimensions[j] = df.Dimensions[j];

}


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void SwathDataField::dump(ostream & out, int depth) const

{

int j;
Indent prefix(depth);
ConcatString s;


out << prefix << "Name        = ";

if ( Name.empty() )  out << "(nul)\n";
else                 out << '\"' << Name << "\"\n";

out << prefix << "Rank        = " << Rank << "\n";

s = numbertype_to_string(Numbertype);

out << prefix << "Numbertype  = " << s << "\n";

out << prefix << "Ndimensions = " << Ndimensions << "\n";

for (j=0; j<Ndimensions; ++j)  {

   out << prefix << "Dimension # " << j << " ...\n";

   Dimensions[j]->dump(out, depth + 1);

}

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void SwathDataField::set_name(const char * s)

{

Name = s;

return;

}


////////////////////////////////////////////////////////////////////////


void SwathDataField::set_rank(int k)

{

Rank = k;

return;

}


////////////////////////////////////////////////////////////////////////


void SwathDataField::set_numbertype(int k)

{

Numbertype = k;

return;

}


////////////////////////////////////////////////////////////////////////


void SwathDataField::set_n_dimensions(int k)

{

if ( k <= 0 )  {

   mlog << Error
        << "\n\n  SwathDataField::set_n_dimensions(int) -> bad value ... " << k << "\n\n";

   exit ( 1 );

}

if ( Dimensions )  { delete [] Dimensions;  Dimensions = (SatDimension **) 0; }

Dimensions = new SatDimension * [k];

int j;

for (j=0; j<k; ++j)  Dimensions[j] = (SatDimension *) 0;

Ndimensions = k;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void SwathDataField::set_dimension(int k, SatDimension * d)

{

if ( (k < 0) || (k >= Ndimensions) )  {

   mlog << Error
        << "\n\n  SwathDataField::set_dimensions(int, SatDimension *) -> range check error\n\n";

   exit ( 1 );

}

Dimensions[k] = d;

return;

}


////////////////////////////////////////////////////////////////////////


int SwathDataField::dimension_size(int k) const

{

if ( (k < 0) || (k >= Ndimensions) )  {

   mlog << Error
        << "\n\n  SwathDataField::dimensions_size(int) const -> range check error\n\n";

   exit ( 1 );

}

int m = Dimensions[k]->size();

return ( m );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class CloudsatSwath
   //


////////////////////////////////////////////////////////////////////////


CloudsatSwath::CloudsatSwath()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


CloudsatSwath::~CloudsatSwath()

{

clear();

}


////////////////////////////////////////////////////////////////////////


CloudsatSwath::CloudsatSwath(const CloudsatSwath & s)

{

init_from_scratch();

assign(s);

}


////////////////////////////////////////////////////////////////////////


CloudsatSwath & CloudsatSwath::operator=(const CloudsatSwath & s)

{

if ( this == &s )  return ( * this );

assign(s);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void CloudsatSwath::init_from_scratch()

{

DataField = (SwathDataField *) 0;

Attribute = (SatAttribute *) 0;

GeoField  = (SwathDataField *) 0;

Dimension = (SatDimension *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void CloudsatSwath::clear()

{

Name.clear();

SwathId = -1;

if ( DataField )  { delete [] DataField;  DataField = (SwathDataField *) 0; }

if ( Attribute )  { delete [] Attribute;  Attribute = (SatAttribute *) 0; }

if (  GeoField )  { delete []  GeoField;   GeoField = (SwathDataField *) 0; }

if ( Dimension )  { delete [] Dimension;  Dimension = (SatDimension *) 0; }

Ndatafields = 0;

Nattributes = 0;

Ngeofields  = 0;

Ndimensions = 0;

Latitude     = (SwathDataField *) 0;
Longitude    = (SwathDataField *) 0;

Height       = (SwathDataField *) 0;

Reflectivity = (SwathDataField *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void CloudsatSwath::assign(const CloudsatSwath & s)

{

clear();

Name = s.Name;

SwathId = s.SwathId;

mlog << Error
     << "\n\n  CloudsatSwath::assign(const CloudsatSwath &) -> not finished yet!\n\n";

exit ( 1 );

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void CloudsatSwath::dump(ostream & out, int depth) const

{

int j;
Indent prefix(depth);


out << prefix << "Name        = ";

if ( Name.empty() )  out << "(nul)\n";
else                 out << '\"' << Name << "\"\n";

out << prefix << "SwathId     = " << SwathId << "\n";

out << prefix << "\n";

out << prefix << "Ndatafields = " << Ndatafields << "\n";

for (j=0; j<Ndatafields; ++j)  {

   out << prefix << "\n";

   out << prefix << "DataField # " << j << " ...\n";

   DataField[j].dump(out, depth + 1);

}

out << prefix << "\n";

out << prefix << "Ngeofields  = " << Ngeofields << "\n";

for (j=0; j<Ngeofields; ++j)  {

   out << prefix << "\n";

   out << prefix << "GeoField # " << j << " ...\n";

   GeoField[j].dump(out, depth + 1);

}


out << prefix << "\n";

out << prefix << "Ndimensions = " << Ndimensions << "\n";

for (j=0; j<Ndimensions; ++j)  {

   out << prefix << "\n";

   out << prefix << "Dimension # " << j << " ...\n";

   Dimension[j].dump(out, depth + 1);

}

out << prefix << "\n";

out << prefix << "Nattributes = " << Nattributes << "\n";

for (j=0; j<Nattributes; ++j)  {

   out << prefix << "\n";

   out << prefix << "Attribute # " << j << " ...\n";

   Attribute[j].dump(out, depth + 1);

}

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void CloudsatSwath::set_name(const char * text)

{

Name = text;

return;

}


////////////////////////////////////////////////////////////////////////


void CloudsatSwath::set_swath_id(int n)

{

SwathId = n;

return;

}


////////////////////////////////////////////////////////////////////////


void CloudsatSwath::get_data_fields()

{

int j, k;
StringArray a;
const char * c = (const char *) 0;
ConcatString s;
SatDimension * d = (SatDimension *) 0;
int n_dims;
int32 * rank       = (int32 *) 0;
int32 * numbertype = (int32 *) 0;
int32 dims[max_dims];
int32 r, nt;


clear_buf();

if ( SWinqdatafields(SwathId, (char *) buf, 0, 0) < 0 )  {

   mlog << Error
        << "\n\n  CloudsatSwath::get_data_fields() -> error (1)\n\n";

   exit ( 1 );

}

parse_csl(buf, a);

Ndatafields = a.n_elements();

DataField = new SwathDataField [Ndatafields];

for (j=0; j<Ndatafields; ++j)  {

   DataField[j].set_name(a[j].c_str());

}

rank = new int32 [Ndatafields];
numbertype = new int32 [Ndatafields];

clear_buf();

if ( SWinqdatafields(SwathId, (char *) buf, rank, numbertype) < 0 )  {

   mlog << Error
        << "\n\n  CloudsatSwath::get_data_fields() -> error (2)\n\n";

   exit ( 1 );

}

for (j=0; j<Ndatafields; ++j)  {

   DataField[j].set_rank       (rank[j]);
   DataField[j].set_numbertype (numbertype[j]);

}

   //
   //  get dimensions for each data field
   //

for (j=0; j<Ndatafields; ++j)  {

   s = DataField[j].name();

   c = s.c_str();

   if ( SWfieldinfo(SwathId, (char *) c, &r, dims, &nt, (char *) buf) < 0 )  {

      mlog << Error
           << "\n\n  CloudsatSwath::get_data_fields() -> error (3)\n\n";

      exit ( 1 );

   }

   parse_csl(buf, a);

   n_dims = a.n_elements();

   DataField[j].set_n_dimensions(n_dims);

   for (k=0; k<n_dims; ++k)  {

      d = dimension(a[k].c_str());

      if ( !d )  {

         mlog << Error
              << "\n\n  CloudsatSwath::get_data_fields() -> error (4)\n\n";

         exit ( 1 );

      }

      DataField[j].set_dimension(k, d);

   }

}   //  for j



   //
   //  done
   //

if ( rank )        { delete [] rank;  rank = (int32 *) 0; }
if ( numbertype )  { delete [] numbertype;  numbertype = (int32 *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////


void CloudsatSwath::get_attributes()

{

int j;
int32 nt, att_buf_size, att_size, retval;
StringArray a;


Nattributes = 0;

if ( (retval = SWinqattrs(SwathId, NULL, &att_buf_size)) < 0 )  {

   mlog << Error
        << "\n\n  CloudsatSwath::get_attributes() -> can't get attribute buffer size\n\n";

   exit ( 1 );

}

if ( retval == 0 )  return;

char * att_buf = new char [att_buf_size];

if ( (retval = SWinqattrs(SwathId, att_buf, &att_buf_size)) < 0 )  {

   mlog << Error
        << "\n\n  CloudsatSwath::get_attributes() -> can't get attribute names\n\n";
   if ( att_buf )  { delete [] att_buf;   att_buf = (char *) 0; }

   exit ( 1 );

}

 if ( retval == 0 ) {
   if ( att_buf )  { delete [] att_buf;   att_buf = (char *) 0; }
   return;
 }
parse_csl(att_buf, a);

Nattributes = a.n_elements();

Attribute = new SatAttribute [Nattributes];

for (j=0; j<Nattributes; ++j)  {

   Attribute[j].set_name(a[j].c_str());

   if ( SWattrinfo(SwathId, (char*)a[j].c_str(), &nt, &att_size) < 0 )  {

      mlog << Error
           << "\n\n  CloudsatSwath::get_attributes() -> can't get info on attribute \"" << (a[j]) << "\"\n\n";

      if ( att_buf )  { delete [] att_buf;   att_buf = (char *) 0; }
      exit ( 1 );

   }

   // Attribute[j].set_number_type ((int) nt);
   // Attribute[j].set_bytes       ((int) att_size);

   clear_buf(att_buf, att_buf_size);

   if ( SWreadattr(SwathId, (char*)a[j].c_str(), att_buf) < 0 )  {

      mlog << Error
           << "\n\n  CloudsatSwath::get_attributes() -> can't get value for attribute \"" << (a[j]) << "\"\n\n";
      if ( att_buf )  { delete [] att_buf;   att_buf = (char *) 0; }
      exit ( 1 );

   }

   Attribute[j].set_value((int) nt, (unsigned char *) att_buf, (int) att_size);

}   //  for j



   //
   //  done
   //

if ( att_buf )  { delete [] att_buf;   att_buf = (char *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////


void CloudsatSwath::get_geo_fields()

{

int j, k;
int n_dims;
const char * c = (const char *) 0;
ConcatString s;
SatDimension * d = (SatDimension *) 0;
StringArray a;
int32 * rank       = (int32 *) 0;
int32 * numbertype = (int32 *) 0;
int32 dims[max_dims];
int32 r, nt;


clear_buf();

if ( SWinqgeofields(SwathId, (char *) buf, 0, 0) < 0 )  {

   mlog << Error
        << "\n\n  CloudsatSwath::get_geo_fields() -> error (1)\n\n";

   exit ( 1 );

}

parse_csl(buf, a);

Ngeofields = a.n_elements();

GeoField = new SwathDataField [Ngeofields];

for (j=0; j<Ngeofields; ++j)  {

   GeoField[j].set_name(a[j].c_str());

}

rank = new int32 [Ngeofields];
numbertype = new int32 [Ngeofields];

clear_buf();

if ( SWinqgeofields(SwathId, (char *) buf, rank, numbertype) < 0 )  {

   mlog << Error
        << "\n\n  CloudsatSwath::get_geo_fields() -> error (2)\n\n";

   exit ( 1 );

}

for (j=0; j<Ngeofields; ++j)  {

   GeoField[j].set_rank       (rank[j]);
   GeoField[j].set_numbertype (numbertype[j]);

}


   //
   //  get dimensions for each data field
   //

for (j=0; j<Ngeofields; ++j)  {

   s = GeoField[j].name();

   c = s.c_str();

   if ( SWfieldinfo(SwathId, (char *) c, &r, dims, &nt, (char *) buf) < 0 )  {

      mlog << Error
           << "\n\n  CloudsatSwath::get_geo_fields() -> error (3)\n\n";

      exit ( 1 );

   }

   parse_csl(buf, a);

   n_dims = a.n_elements();

   GeoField[j].set_n_dimensions(n_dims);

   for (k=0; k<n_dims; ++k)  {

      d = dimension(a[k].c_str());

      if ( !d )  {

         mlog << Error
              << "\n\n  CloudsatSwath::get_data_fields() -> error (4)\n\n";

         exit ( 1 );

      }

      GeoField[j].set_dimension(k, d);

   }

}   //  for j


   //
   //  done
   //

if ( rank )  { delete [] rank;  rank = (int32 *) 0; }
if ( numbertype )  { delete [] numbertype;  numbertype = (int32 *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////


void CloudsatSwath::get_dimensions()

{

int j, k;
StringArray a;


clear_buf();

if ( SWinqdims(SwathId, (char *) buf, 0) < 0 )  {

   mlog << Error
        << "\n\n  CloudsatSwath::get_dimensions() -> error (1)\n\n";

   exit ( 1 );

}

parse_csl(buf, a);

Ndimensions = a.n_elements();

Dimension = new SatDimension [Ndimensions];

for (j=0; j<Ndimensions; ++j)  {

   Dimension[j].set_name(a[j].c_str());

   if ( (k = SWdiminfo(SwathId, (char*)a[j].c_str())) < 0 )  {

      mlog << Error
           << "\n\n  CloudsatSwath::get_dimensions() ->can't get size for dimension \"" << a[j] << "\"\n\n";

      exit ( 1 );

   }

   Dimension[j].set_size(k);

}   //  for j

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


SwathDataField * CloudsatSwath::get_field(const char * _name) const

{

int j;

for (j=0; j<Ndatafields; ++j)  {

   if ( DataField[j].name() == _name )  return ( DataField + j );

}


return ( (SwathDataField *) 0 );

}


////////////////////////////////////////////////////////////////////////


SwathDataField * CloudsatSwath::get_field(int k) const

{

if ( (k < 0) || (k >= Ndatafields) )  {

   mlog << Error
        << "\n\n  CloudsatSwath::get_field(int) const -> range check error\n\n";

   exit ( 1 );

}


return ( DataField + k );

}


////////////////////////////////////////////////////////////////////////


SwathDataField * CloudsatSwath::get_geo_field(const char * _name) const

{

int j;

for (j=0; j<Ngeofields; ++j)  {

   if ( GeoField[j].name() == _name )  return ( GeoField + j );

}


return ( (SwathDataField *) 0 );

}


////////////////////////////////////////////////////////////////////////


SwathDataField * CloudsatSwath::get_geo_field(int k) const

{

if ( (k < 0) || (k >= Ngeofields) )  {

   mlog << Error
        << "\n\n  CloudsatSwath::get_geo_field(int) const -> range check error\n\n";

   exit ( 1 );

}


return ( GeoField + k );

}


////////////////////////////////////////////////////////////////////////


SwathDataField * CloudsatSwath::get_data_field(const char * _name) const

{

int j;

for (j=0; j<Ndatafields; ++j)  {

   if ( DataField[j].name() == _name )  return ( DataField + j );

}


return ( (SwathDataField *) 0 );

}


////////////////////////////////////////////////////////////////////////


SwathDataField * CloudsatSwath::get_data_field(int k) const

{

if ( (k < 0) || (k >= Ndatafields) )  {

   mlog << Error
        << "\n\n  CloudsatSwath::get_data_field(int) const -> range check error\n\n";

   exit ( 1 );

}


return ( DataField + k );

}


////////////////////////////////////////////////////////////////////////


SatDimension * CloudsatSwath::dimension(int k) const

{

if ( (k < 0) || (k >= Ndimensions) )  {

   mlog << Error
        << "\n\n  CloudsatSwath::dimension(int) const -> range check error\n\n";

   exit ( 1 );

}


return ( Dimension + k );

}


////////////////////////////////////////////////////////////////////////


SatDimension * CloudsatSwath::dimension(const char * _name) const

{

int j;
SatDimension * d = (SatDimension *) 0;

for (j=0; j<Ndimensions; ++j)  {

   if ( Dimension[j].name() == _name )  return ( Dimension + j );

}

return ( d );

}


////////////////////////////////////////////////////////////////////////


void CloudsatSwath::setup_geo_pointers()

{

Latitude     = get_geo_field("Latitude");
Longitude    = get_geo_field("Longitude");

Height       = get_geo_field("Height");

Reflectivity = get_data_field("Radar_Reflectivity");


if ( !Latitude || !Longitude || !Height || !Reflectivity )  {

   mlog << Error
        << "\n\n  CloudsatSwath::setup_geo_pointers() -> some geofields not found!\n\n";

   exit ( 1 );

}


return;

}


////////////////////////////////////////////////////////////////////////


double CloudsatSwath::lat(int k)

{

if ( !Latitude )  setup_geo_pointers();

double x;
int n;
int32 start;
int32 edge = 1;
float * F = (float *) buf;

n = Latitude->dimension_size(0);

if ( (k < 0) || (k >= n) )  {

   mlog << Error
        << "\n\n  CloudsatSwath::lat(int) const -> range check error\n\n";

   exit ( 1 );

}

start = (int32) k;

if ( SWreadfield(SwathId, (char *) "Latitude", &start, 0, &edge, buf) < 0 )  {

   mlog << Error
        << "\n\n  CloudsatSwath::lat(int) const -> bad SWreadfield status\n\n";

   exit ( 1 );

}

x = (double) (F[0]);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double CloudsatSwath::lon(int k)

{

if ( !Longitude )  setup_geo_pointers();

double x;
int n;
int32 start;
int32 edge = 1;
float * F = (float *) buf;

n = Longitude->dimension_size(0);

if ( (k < 0) || (k >= n) )  {

   mlog << Error
        << "\n\n  CloudsatSwath::lon(int) const -> range check error\n\n";

   exit ( 1 );

}

start = (int32) k;

if ( SWreadfield(SwathId, (char *) "Longitude", &start, 0, &edge, buf) < 0 )  {

   mlog << Error
        << "\n\n  CloudsatSwath::lon(int) const -> bad SWreadfield status\n\n";

   exit ( 1 );

}

x = (double) (F[0]);

x = -x;   //  west longitude postitve

return ( x );

}


////////////////////////////////////////////////////////////////////////


double CloudsatSwath::height_m(int ray, int bin)

{

if ( !Height )  setup_geo_pointers();

double x;
int n1, n2;
int32 start[2];
int32 edge[2];
short * S = (short *) buf;

n1 = Height->dimension_size(0);
n2 = Height->dimension_size(1);

if ( (ray < 0) || (ray >= n1) || (bin < 0) || (bin >= n2) )  {

   mlog << Error
        << "\n\n  CloudsatSwath::height_m(int) const -> range check error\n\n";

   exit ( 1 );

}

start[0] = (int32) ray;
start[1] = (int32) bin;

edge[0] = edge[1] = 1;

if ( SWreadfield(SwathId, (char *) "Height", start, 0, edge, buf) < 0 )  {

   mlog << Error
        << "\n\n  CloudsatSwath::height_m(int) const -> bad SWreadfield status\n\n";

   exit ( 1 );

}

x = (double) (S[0]);

return ( x );

}



////////////////////////////////////////////////////////////////////////


double CloudsatSwath::reflectivity(int ray, int bin)

{

if ( !Reflectivity )  setup_geo_pointers();

double x;
int n1, n2;
int32 start[2];
int32 edge[2];
short * S = (short *) buf;

n1 = Reflectivity->dimension_size(0);
n2 = Reflectivity->dimension_size(1);

if ( (ray < 0) || (ray >= n1) || (bin < 0) || (bin >= n2) )  {

   mlog << Error
        << "\n\n  CloudsatSwath::reflectivity(int, int) const -> range check error\n\n";

   exit ( 1 );

}

start[0] = (int32) ray;
start[1] = (int32) bin;

edge[0] = edge[1] = 1;

if ( SWreadfield(SwathId, (char *) "Radar_Reflectivity", start, 0, edge, buf) < 0 )  {

   mlog << Error
        << "\n\n  CloudsatSwath::reflectivity(int, int) const -> bad SWreadfield status\n\n";

   exit ( 1 );

}

x = (double) (S[0]);

x *= 0.001;

return ( x );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class CloudsatSwathFile
   //


////////////////////////////////////////////////////////////////////////


CloudsatSwathFile::CloudsatSwathFile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


CloudsatSwathFile::~CloudsatSwathFile()

{

close();

}


////////////////////////////////////////////////////////////////////////


CloudsatSwathFile::CloudsatSwathFile(const CloudsatSwathFile &)

{

// init_from_scratch();
// 
// assign(f);

mlog << Error
     << "\n\n  CloudsatSwathFile::CloudsatSwathFile(const CloudsatSwathFile &) -> should never be called!\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


CloudsatSwathFile & CloudsatSwathFile::operator=(const CloudsatSwathFile &)

{

// if ( this == &f )  return ( * this );
// 
// assign(f);


mlog << Error
     << "\n\n  CloudsatSwathFile::operator=(const CloudsatSwathFile &) -> should never be called!\n\n";

exit ( 1 );

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void CloudsatSwathFile::init_from_scratch()

{

FileId = -1;

Swath  = (CloudsatSwath *) 0;

close();

return;

}


////////////////////////////////////////////////////////////////////////


void CloudsatSwathFile::dump(ostream & out, int depth) const

{

int j;
Indent prefix(depth);

out << prefix << "Filename = ";

if ( Filename.empty() )  out << "(nul)\n";
else {

   out << '\"' << short_name() << "\"\n";

}

out << prefix << "\n";

out << prefix << "Nswaths = " << Nswaths << "\n";

for (j=0; j<Nswaths; ++j)  {

   out << prefix << "Swath # " << j << " ...\n";

   Swath[j].dump(out, depth + 1);

}


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


bool CloudsatSwathFile::open(const char * _filename)

{

int j, k;
int32 size;
StringArray a;


close();

Filename = _filename;

   //
   //  open file
   //

if ( (FileId = SWopen((char *) _filename, DFACC_READ)) < 0 )  {

   mlog << Error
        << "\n\n  CloudsatSwathFile::open(const char *) -> unable to open input file \"" << _filename << "\"\n\n";

   close();

   return ( false );

}

   //
   //  get swaths
   //

clear_buf();

if ( SWinqswath((char *) _filename, (char *) buf, &size) < 0 )  {

   mlog << Error
        << "\n\n  CloudsatSwathFile::open(const char *) -> unable to get number of swaths from file \"" << _filename << "\"\n\n";

   close();

   return ( false );

}

parse_csl(buf, a);

Nswaths = a.n_elements();

if ( Nswaths > 0 )  Swath = new CloudsatSwath [Nswaths];

for (j=0; j<Nswaths; ++j)  {

   Swath[j].set_name(a[j].c_str());

   if ( (k = SWattach(FileId, (char *) a[j].c_str())) < 0 )  {

      mlog << Error
           << "\n\n  CloudsatSwathFile::open(const char *) -> to attach swath # " << j << " in file \"" << _filename << "\"\n\n";

      close();

      return ( false );

   }

   Swath[j].set_swath_id(k);

   Swath[j].get_dimensions();

   Swath[j].get_data_fields();

   Swath[j].get_attributes();

   Swath[j].get_geo_fields();


}   //  for j


   //
   //  done
   //

return ( true );

}



////////////////////////////////////////////////////////////////////////


void CloudsatSwathFile::close()

{

if ( (FileId >= 0) && (SWclose(FileId) < 0) )  {

   mlog << Error
        << "\n\n  CloudsatSwathFile::close() -> trouble closing file\n\n";

   exit ( 1 );

}

FileId = -1;

Filename.clear();

if ( Swath )  { delete [] Swath;  Swath = (CloudsatSwath *) 0; }

Nswaths = 0;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString CloudsatSwathFile::short_name() const

{

ConcatString s;

if ( Filename.nonempty() )  s = get_short_name(Filename.c_str());

return ( s );

}


////////////////////////////////////////////////////////////////////////


CloudsatSwath * CloudsatSwathFile::swath(int k) const

{

if ( (k < 0) || (k >= Nswaths) )  {

   mlog << Error
        << "\n\n  CloudsatSwathFile::swath(int) const -> range check error\n\n";

   exit ( 1 );

}

return ( Swath + k );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


void clear_buf()

{

memset(buf, 0, buf_size);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void clear_buf(void * p, int nbytes)

{

memset(p, 0, nbytes);

return;

}


////////////////////////////////////////////////////////////////////////





