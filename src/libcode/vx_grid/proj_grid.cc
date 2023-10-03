

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>

#include "vx_util.h"

#include "proj_grid.h"


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


void ProjGrid::init_from_scratch()

{

// C = 0;
// 
// pj = 0;

info = make_shared<ProjInfo>();

Nx = Ny = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void ProjGrid::clear()

{

// info.   //  ?

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

//Aff.dump(out, depth + 1);

out << prefix << "proj_set = ";

if ( Proj_Set.nonempty() )  out << '\"' << Proj_Set  << "\"\n";
else                        out << "(nul)\n";



   //
   //  how to dump out the PROJ stuff?
   //
   //    for now, we'll just write out the pointer value;
   //

out << prefix << "pj = " << (info->pj)  << '\n';


   //
   //  done
   //

out.flush();

return;

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






