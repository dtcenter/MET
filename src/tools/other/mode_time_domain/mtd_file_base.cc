// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include <netcdf>

#include "mtd_file.h"
#include "mtd_partition.h"
#include "mtd_nc_defs.h"
#include "mtdfiletype_to_string.h"
#include "get_met_grid.h"

#include "vx_math.h"
#include "vx_nc_util.h"

using namespace std;
using namespace netCDF;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class MtdFileBase
   //


////////////////////////////////////////////////////////////////////////


MtdFileBase::MtdFileBase()

{

base_init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


MtdFileBase::~MtdFileBase()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::base_init_from_scratch()

{

G = (Grid *) nullptr;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::clear()

{

if ( G )  { delete G;  G = (Grid *) nullptr; }

Nx = Ny = Nt = 0;
 
StartValidTime = (unixtime) 0;

ActualValidTimes.clear();

DeltaT = 0;

Filename.clear();

FileType = no_mtd_file_type;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::base_assign(const MtdFileBase & f)

{

clear();

Nx         = f.Nx;
Ny         = f.Ny;
Nt         = f.Nt;

StartValidTime  = f.StartValidTime;

Lead_Times = f.Lead_Times;

DeltaT     = f.DeltaT;

Filename   = f.Filename;

FileType   = f.FileType;

if ( f.G )  set_grid ( *(f.G) );


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::dump(ostream & out, int depth) const

{

Indent prefix(depth);
int month, day, year, hour, minute, second;
char junk[256];

out << prefix << "Filename          = ";

if ( Filename.nonempty() )  out << '\"' << Filename << "\"\n";
else                        out << "(nul)\n";

unix_to_mdyhms(StartValidTime, month, day, year, hour, minute, second);

snprintf(junk, sizeof(junk), "%s %d, %d   %02d:%02d:%02d", short_month_name[month], day, year, hour, minute, second);


out << prefix << "StartValidTime    = " << StartValidTime << " ... (" << junk << ")\n";
out << prefix << "DeltaT            = " << DeltaT    << '\n';
out << prefix << "FileType          = " << mtdfiletype_to_string(FileType) << '\n';

out << prefix << "(Nx, Ny, Nt)      = (" << Nx << ", " << Ny << ", " << Nt << ")\n";

if ( G )  {

   out << prefix << "Grid ...\n";

   G->dump(out, depth + 1);

} else {

   out << prefix << "Grid              = 0\n";

}


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::set_filetype(MtdFileType t)

{

FileType = t;

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::set_grid(const Grid & g)

{

if ( G )  { delete G;  G = (Grid *) nullptr; }

G = new Grid ( g );

return;

}


////////////////////////////////////////////////////////////////////////


Grid MtdFileBase::grid() const

{

if ( !G )  {

   mlog << Error << "\nMtdFileBase::grid() const -> "
        << "no grid!\n\n";

   exit ( 1 );

}


return *G;

}


////////////////////////////////////////////////////////////////////////


const Grid * MtdFileBase::grid_p() const

{

return G;

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::latlon_to_xy(double lat, double lon, double & x, double & y) const

{

if ( !G )  {

   mlog << Error << "\nMtdFileBase::latlon_to_xy() -> "
        << "no grid!\n\n";

   exit ( 1 );

}

G->latlon_to_xy(lat, lon, x, y);

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::set_start_valid_time(unixtime t)

{

StartValidTime = t;

return;

}

////////////////////////////////////////////////////////////////////////


void MtdFileBase::set_delta_t(int seconds)

{

DeltaT = seconds;

return;

}

////////////////////////////////////////////////////////////////////////


void MtdFileBase::init_actual_valid_times(const vector<unixtime> &validTimes)

{

ActualValidTimes = validTimes;

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::xy_to_latlon(double x, double y, double & lat, double & lon) const

{

if ( !G )  {

   mlog << Error << "\nMtdFileBase::xy_to_latlon() -> "
        << "no grid!\n\n";

   exit ( 1 );

}

G->xy_to_latlon(x, y, lat, lon);

return;

}


////////////////////////////////////////////////////////////////////////


unixtime MtdFileBase::valid_time(int t) const

{

if ( (t < 0) || ( t >= Nt) )  {

   mlog << Error << "\nMtdFileBase::valid_time(int t) -> "
        << "range check error\n\n";

   exit ( 1 );

}


return ( StartValidTime + t*DeltaT );

}

////////////////////////////////////////////////////////////////////////


unixtime MtdFileBase::actual_valid_time(int t) const

{

if ( (t < 0) || ( t >= (int)ActualValidTimes.size()) )  {

   mlog << Error << "\nMtdFileBase::valid_time(int t) -> "
        << "range check error\n\n";

   exit ( 1 );

}

return ActualValidTimes[t];

}


////////////////////////////////////////////////////////////////////////


int MtdFileBase::lead_time(int index) const

{

if ( (index < 0) || ( index >= Nt) )  {

   mlog << Error << "\nMtdFileBase::lead_time(int t) -> "
        << "range check error\n\n";

   exit ( 1 );

}

return Lead_Times[index];

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::read(NcFile & f)

{

NcDim dim;

   //  Nx, Ny, Nt

dim = get_nc_dim(&f, nx_dim_name);
Nx  = GET_NC_SIZE(dim);

dim = get_nc_dim(&f, ny_dim_name);
Ny  = GET_NC_SIZE(dim);

dim = get_nc_dim(&f, nt_dim_name);
Nt  = GET_NC_SIZE(dim);

   //  Grid

G = new Grid;

read_netcdf_grid(&f, *G);

   //  timestamp info

ConcatString s;

get_att_value_string(&f, start_time_att_name, s);

StartValidTime = timestring_to_unix(s.text());

DeltaT = get_att_value_int(&f, delta_t_att_name);

   //  FileType

bool status = false;

get_att_value_string(&f, filetype_att_name, s);

status = string_to_mtdfiletype(s.text(), FileType);

if ( ! status )  {

   mlog << Error << "\nMtdFileBase::read(NcFile &) -> "
        << "unable to parse filetype string \""
        << s << "\"\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::write(NcFile & f) const

{

char junk[256];
ConcatString s;

   //  Add the time dimension

add_dim(&f, nt_dim_name, Nt);

   //  Grid

NcDim ny_dim;
NcDim nx_dim;

write_netcdf_proj(&f, *G, ny_dim, nx_dim);

   //  timestamp info

s = unix_to_yyyymmdd_hhmmss(StartValidTime);

add_att(&f, start_time_att_name, s.text());


snprintf(junk, sizeof(junk), "%d", DeltaT);

add_att(&f, delta_t_att_name, junk);

   //  FileType

s = mtdfiletype_to_string(FileType);

add_att(&f, filetype_att_name, s.text());


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::set_lead_time(int index, int value)

{

if ( (index < 0) || (index >= Nt) )  {

   mlog << Error << "MtdFileBase::set_lead_time(int index, int value) -> "
        << "range check error on index ... "
        << index << "\n\n";

   exit ( 1 );

}

Lead_Times.set(index, value);

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


