// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

#ifndef  __MET_GSL_RANDIST_H__
#define  __MET_GSL_RANDIST_H__

////////////////////////////////////////////////////////////////////////

#include "vx_util.h"
#include "gsl/gsl_randist.h"

////////////////////////////////////////////////////////////////////////
//
// Setup and free a random number generator
//
////////////////////////////////////////////////////////////////////////

extern void rng_set(gsl_rng *&r, const char *, const char *);
extern void rng_free(gsl_rng *r);

////////////////////////////////////////////////////////////////////////
//
// Shuffling and sampling from of a set of objects
//
////////////////////////////////////////////////////////////////////////

extern void ran_shuffle(const gsl_rng *r, double *, int);
extern void ran_shuffle(const gsl_rng *r, NumArray &);

extern void ran_choose(const gsl_rng *r, double *, int, double *, int);
extern void ran_choose(const gsl_rng *r, NumArray &, NumArray &, int);

extern void ran_sample(const gsl_rng *r, double *, int, double *, int);
extern void ran_sample(const gsl_rng *r, NumArray &, NumArray &, int);

////////////////////////////////////////////////////////////////////////

#endif   /*  __MET_GSL_RANDIST_H__  */

////////////////////////////////////////////////////////////////////////

