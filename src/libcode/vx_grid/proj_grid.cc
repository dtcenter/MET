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

#include "vx_util.h"
#include "vx_log.h"

#include "proj_grid.h"
#include "find_grid_by_name.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ProjGrid
   //


////////////////////////////////////////////////////////////////////////


ProjGrid::ProjGrid()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ProjGrid::~ProjGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ProjGrid::ProjGrid(const ProjGrid & g)

{

init_from_scratch();

clear();

assign(g);

}


////////////////////////////////////////////////////////////////////////


ProjGrid & ProjGrid::operator=(const ProjGrid & g)

{

if ( this == &g )  return ( * this );

clear();

assign(g);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


ProjGrid::ProjGrid(const char * _name)

{

init_from_scratch();

set(_name);

}


////////////////////////////////////////////////////////////////////////


void ProjGrid::init_from_scratch()

{

info = make_shared<ProjInfo>();

Nx = Ny = 0;

return;

}


////////////////////////////////////////////////////////////////////////


ProjGrid::ProjGrid(const LambertData &data)

{

init_from_scratch();

set(data);

}


////////////////////////////////////////////////////////////////////////


ProjGrid::ProjGrid(const StereographicData &data)

{

init_from_scratch();

set(data);

}


////////////////////////////////////////////////////////////////////////


ProjGrid::ProjGrid(const LatLonData &data)

{

init_from_scratch();

set(data);

}


////////////////////////////////////////////////////////////////////////


ProjGrid::ProjGrid(const RotatedLatLonData &data)

{

init_from_scratch();

set(data);

}


////////////////////////////////////////////////////////////////////////


ProjGrid::ProjGrid(const MercatorData &data)

{

init_from_scratch();

set(data);

}


////////////////////////////////////////////////////////////////////////


ProjGrid::ProjGrid(const GaussianData &data)

{

init_from_scratch();

set(data);

}


////////////////////////////////////////////////////////////////////////


ProjGrid::ProjGrid(const LaeaData &data)

{

init_from_scratch();

set(data);

}


////////////////////////////////////////////////////////////////////////


void ProjGrid::clear()

{

info = nullptr;

Aff.clear();

Nx = Ny = 0;

Name.clear();


return;

}


////////////////////////////////////////////////////////////////////////


void ProjGrid::assign(const ProjGrid & p)

{

clear();

info = p.info;

Aff = p.Aff;

Nx = p.Nx;
Ny = p.Ny;

Name = p.Name;


return;

}


////////////////////////////////////////////////////////////////////////


void ProjGrid::dump(ostream & out, int depth) const

{

Indent prefix(depth);


out << prefix << "Name = ";

if ( Name.nonempty() )  out << '\"' << Name << "\"\n";
else                    out << "(nul)\n";

out << prefix << "(nx, ny) = (" << Nx << ", " << Ny << ")\n";

out << prefix << "Affine: \n";

Aff.dump(out, depth + 1);

out << prefix << "proj_set = ";

if ( Proj_Set.nonempty() )  out << '\"' << Proj_Set  << "\"\n";
else                        out << "(nul)\n";

out << prefix << "pj = " << (info->pj)  << '\n';

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void ProjGrid::set(const char * _name)

{

clear();

bool status = find_grid_by_name(_name, *this);

if ( !status )  {

   mlog << Error << "\nProjGrid::set(const char *) -> "
        << "grid lookup failed for name \""
        << _name << "\"\n\n";

   exit ( 1 );

}


return;

}


////////////////////////////////////////////////////////////////////////


void ProjGrid::set(const LambertData & data)

{

ConcatString cs;
double xx, yy;

Proj_Set = "+proj=lcc";   //  lcc means lambert conformal conic

   //   radius of earth

cs.format("+R=%.3f", data.r_km);

Proj_Set << ' ' << cs;

   //  the two secant latitudes for the lambert projection

cs.format("+lat_1=%.3f +lat_2=%.3f", data.scale_lat_1, data.scale_lat_2);

Proj_Set << ' ' << cs;

   //  orientation longitude
   //    note: PROJ calls it "Longitude of projection center"

cs.format("+lon_0=%.3f", -(data.lon_orient));   //  note minus sign

Proj_Set << ' ' << cs;

mlog << Debug(4) << "Lambert Conformal proj parameters: " << Proj_Set << "\n";

set_proj(Proj_Set.c_str());

   //  create the affine part of the transformation

const double s = 1.0/(data.d_km);   //  scale factor

Aff.set_mb(s, 0.0, 0.0, s, 0.0, 0.0);

latlon_to_xy(data.lat_pin, data.lon_pin, xx, yy);

Aff.set_b(data.x_pin - xx, data.y_pin - yy);

set_size(data.nx, data.ny);

set_name(data.name);

}


////////////////////////////////////////////////////////////////////////


void ProjGrid::set(const StereographicData & data)

{

mlog << Error << "\nvoid ProjGrid::set(const StereographicData & data) -> "
     << "not yet implemented\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


void ProjGrid::set(const LatLonData & data)

{

mlog << Error << "\nvoid ProjGrid::set(const LatLonData & data) -> "
     << "not yet implemented\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


void ProjGrid::set(const RotatedLatLonData & data)

{

mlog << Error << "\nvoid ProjGrid::set(const RotatedLatLonData & data) -> "
     << "not yet implemented\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


void ProjGrid::set(const MercatorData & data)

{

mlog << Error << "\nvoid ProjGrid::set(const MercatorData & data) -> "
     << "not yet implemented\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


void ProjGrid::set(const GaussianData & data)

{

mlog << Error << "\nvoid ProjGrid::set(const GaussianData & data) -> "
     << "not yet implemented\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


void ProjGrid::set(const LaeaData & data)

{

mlog << Error << "\nvoid ProjGrid::set(const LaeaData & data) -> "
     << "not yet implemented\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


void ProjGrid::set_proj(const char * s)

{

clear();

shared_ptr<ProjInfo> i = make_shared<ProjInfo>();


i->pj = proj_create (PJ_DEFAULT_CTX, s);

i->C = 0;

if ( ! (i->pj) )  {

   const int proj_errno = proj_context_errno(i->C);

   ConcatString err_str = proj_errno_string(proj_errno);

   mlog << Error << "\nProjGrid::set_proj() -> "
        << "Failed to create transformation with string \""
        << s << "\": " << err_str << "\n\n";

   exit ( 1 );

}

info = i;

return;

}


////////////////////////////////////////////////////////////////////////


void ProjGrid::latlon_to_xy (double lat, double lon, double & x, double & y) const

{

PJ_COORD a, b;
double x_proj, y_proj;
PJ * p = info->pj;


   //
   //  PROJ part
   //

a.lp.phi =  lat*rad_per_deg;
a.lp.lam = -lon*rad_per_deg;   //  note minus sign

b = proj_trans (p, PJ_FWD, a);

x_proj = b.xy.x;
y_proj = b.xy.y;

   //
   //  Affine part
   //

Aff.forward(x_proj, y_proj, x, y);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ProjGrid::xy_to_latlon (double x, double y, double & lat, double & lon) const

{

PJ_COORD a, b;
double x_proj, y_proj;
PJ * p = info->pj;


   //
   //  Affine part
   //

Aff.reverse(x, y, x_proj, y_proj);

   //
   //  PROJ part
   //

a.xy.x = x_proj;
a.xy.y = y_proj;

b = proj_trans (p, PJ_INV, a);

lat =  deg_per_rad*(b.lp.phi);
lon = -deg_per_rad*(b.lp.lam);    //  note minus sign

   //
   //  might need to do range reduction on lon here
   //

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////
