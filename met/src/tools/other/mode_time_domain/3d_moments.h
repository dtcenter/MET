

////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_3D_MOMENTS_H__
#define  __MTD_3D_MOMENTS_H__


////////////////////////////////////////////////////////////////////////


class MtdMoments {

   public:

      void init_from_scratch();

      void assign(const MtdMoments &);

      int N;    //  0th order moments

      double Sx;    //  1st order moments
      double Sy;    // 
      double St;    // 

      double Sxx;   //  2nd order CENTRAL moments
      double Sxy;   //  
      double Sxt;   //  
      double Syy;   //  
      double Syt;   //  
      double Stt;   //  

      bool IsCentralized;   //  default: false

   public:

      MtdMoments();
     ~MtdMoments();
      MtdMoments(const MtdMoments &);
      MtdMoments & operator=(const MtdMoments &);

      void clear();

         //
         //  set stuff
         //


         //
         //  get stuff
         //

      int volume() const;


         //
         //  do stuff
         //

      void add(double x, double y, double t);

      void centralize();


      void calc_velocity(double & vx, double & vy) const;

      double calc_3D_axis_plane_angle() const;



};


////////////////////////////////////////////////////////////////////////


inline int MtdMoments::volume() const { return ( N ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_3D_MOMENTS_H__  */


////////////////////////////////////////////////////////////////////////


