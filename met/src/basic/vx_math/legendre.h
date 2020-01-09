

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __VX_LEGENDRE_H__
#define  __VX_LEGENDRE_H__


////////////////////////////////////////////////////////////////////////


class Legendre {

   private:

      void init_from_scratch();

      void assign(const Legendre &);




      int MaxDegree;

      double X;   //  last x value

      double * P;   //  allocated

      double * PP;   //  allocated

   public:

      Legendre();
     ~Legendre();
      Legendre(const Legendre &);
      Legendre & operator=(const Legendre &);

      void clear();

         //
         //  set stuff
         //

      void set_max_degree(int);

         //
         //  get stuff
         //

      double last_x() const;

      double d_and_r_root(int);
      double  lether_root(int);

      void d_and_r_root_weight(int, double & r, double & w);
      void  lether_root_weight(int, double & r, double & w);

         //
         //  do stuff
         //

      void calc(double x);


      double value          () const;   //  assumes values already calculated
      double value     (int n) const;   //  assumes values already calculated

      double der_value      () const;   //  assumes values already calculated
      double der_value (int n) const;   //  assumes values already calculated
      

      

};


////////////////////////////////////////////////////////////////////////


inline double Legendre::value() const { return ( P[MaxDegree] ); }

inline double Legendre::last_x() const { return ( X ); }

inline double Legendre::value(int __n__) const { return ( P[__n__] ); }

inline double Legendre::der_value() const { return ( PP[MaxDegree] ); }

inline double Legendre::der_value(int __n__) const { return ( PP[__n__] ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_LEGENDRE_H__  */


////////////////////////////////////////////////////////////////////////


