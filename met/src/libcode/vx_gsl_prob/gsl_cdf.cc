// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <cmath>

#include "gsl_cdf.h"

#include "gsl/gsl_randist.h"

////////////////////////////////////////////////////////////////////////

static double F_newton(double x, double y, double deg_freedom_1, double deg_freedom_2);

////////////////////////////////////////////////////////////////////////

double znorm(double x)

{

double y;

y = gsl_cdf_ugaussian_P(x);

return ( y );

}

////////////////////////////////////////////////////////////////////////

double zinv(double y)

{

double x;

x = gsl_cdf_ugaussian_Pinv(y);

return ( x );

}

////////////////////////////////////////////////////////////////////////

double dnorm(double x)

{

double y;

y = gsl_ran_ugaussian_pdf(x);

return ( y );

}

////////////////////////////////////////////////////////////////////////

double normal_cdf(double x, double mu, double sigma)

{

double y;

y = gsl_cdf_gaussian_P(x - mu, sigma);

return ( y );

}

////////////////////////////////////////////////////////////////////////

double normal_cdf_inv(double y, double mu, double sigma)

{

double x;

x = mu + gsl_cdf_gaussian_Pinv(y, sigma);

return ( x );

}

////////////////////////////////////////////////////////////////////////

double normal_pdf(double x, double sigma)

{

double y;

y = gsl_ran_gaussian_pdf(x, sigma);

return ( y );

}

////////////////////////////////////////////////////////////////////////

double chi2_cdf(double x, double deg_freedom)

{

double y;

y = gsl_cdf_chisq_P(x, deg_freedom);

return ( y );

}

////////////////////////////////////////////////////////////////////////

double chi2_cdf_inv(double y, double deg_freedom)

{

double x, cv_normal;
double large_n = 50.0;

//
// The following is a workaround for handling a GSL bug when the
// degrees of freedom are greater than 1,263,131.  As of GSL
// version 1.11, a bug in the gsl_cdf_chisq_Pinv routine causes the
// program to abort for large degrees of freedom.  Once the GSL bug
// is fixed, this check should be removed.  No warning is necessary
// since the difference in the result is out in the 5th decimal place.
//
if(deg_freedom > large_n) {

   cv_normal = normal_cdf_inv(y, 0.0, 1.0);
   x = deg_freedom*
       pow(1.0 - (2.0/(9.0*deg_freedom)) +
           cv_normal*sqrt(2.0/(9.0*deg_freedom)), 3.0);
}
else {
   x = gsl_cdf_chisq_Pinv(y, deg_freedom);
}

return ( x );

}

////////////////////////////////////////////////////////////////////////

double students_t_cdf(double x, double deg_freedom)

{

double y;

y = gsl_cdf_tdist_P(x, deg_freedom);

return ( y );

}

////////////////////////////////////////////////////////////////////////

double students_t_cdf_inv(double y, double deg_freedom)

{

double x;

x = gsl_cdf_tdist_Pinv(y, deg_freedom);

return ( x );

}

////////////////////////////////////////////////////////////////////////

double F_cdf(double x, double deg_freedom_1, double deg_freedom_2)

{

double y;

y = gsl_cdf_fdist_P(x, deg_freedom_1, deg_freedom_2);

return ( y );

}

////////////////////////////////////////////////////////////////////////

double F_cdf_inv(double y, double deg_freedom_1, double deg_freedom_2)

{

double x, x_new, cor;
const double tol = 1.0e-10;


x = 1.0;   //  starting value

do {

   x_new = F_newton(x, y, deg_freedom_1, deg_freedom_2);

   // mlog << Debug(1) << "x_new = " << x_new << "\n";

   cor = fabs(x - x_new);

   if ( x_new <= 0.0 )  x = x/2.0;
   else                 x = x_new;

}  while ( cor >= tol );


// mlog << Error << "\nF_cdf_inv() -> not yet implemented\n\n";
// 
// exit ( 1 );


return ( x );

}

////////////////////////////////////////////////////////////////////////
//
//  Code for misc functions
//
////////////////////////////////////////////////////////////////////////

double F_newton(double x, double y, double deg_freedom_1, double deg_freedom_2)   //  fig newton?

{

double x_new;
double top, bot;


top = F_cdf(x, deg_freedom_1, deg_freedom_2) - y;

bot = gsl_ran_fdist_pdf(x, deg_freedom_1, deg_freedom_2);

x_new = x - (top/bot);


return ( x_new );

}

////////////////////////////////////////////////////////////////////////
