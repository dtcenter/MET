

////////////////////////////////////////////////////////////////////////


#ifndef  __INTEGER_CONVEX_HULL_H__
#define  __INTEGER_CONVEX_HULL_H__


////////////////////////////////////////////////////////////////////////


struct IntPoint {

   int x;

   int y;

   bool used;

   int orig_index;

};


////////////////////////////////////////////////////////////////////////


   //
   //  Note: "hull" has to be at least of size n_in + 1
   //

extern void ihull(const IntPoint * in, const int n_in, IntPoint * hull, int & n_hull);


////////////////////////////////////////////////////////////////////////


inline int calc_turn(const IntPoint & p0, const IntPoint & p1, const IntPoint & p2)

{

const int x1 = p1.x - p0.x;
const int y1 = p1.y - p0.y;

const int x2 = p2.x - p0.x;
const int y2 = p2.y - p0.y;

const int k = x1*y2 - x2*y1;

if ( k > 0 )  return ( 1 );

return ( (k < 0) ? -1 : 0 );

}


////////////////////////////////////////////////////////////////////////


#endif   /*  __INTEGER_CONVEX_HULL_H__  */


////////////////////////////////////////////////////////////////////////


