#ifndef __DISTS
#define __DISTS

#include "Utils.h"

extern "C" {
// BLAS/LAPACK routines
  // not needed when #include "R_ext/lapack.h"
  /*
  int dgemm_(char*, char*, int*, int*, int*, double*, double*, int*, double*, int*, double*, double*, int*); 
  int dtrmv_(char*, char*, char*, int*, double*, int*, double*, int*);
  int dtrsv_(char*, char*, char*, int*, double*, int*, double*, int*);
  int dtrsm_(char*, char*, char*, char*, int*, int*, double*, double*, int*, double*, int*);
  int dtrmm_(char*, char*, char*, char*, int*, int*, double*, double*, int*, double*, int*);
  int dpotrf_(char*, int*, double*, int*, int*);
  */

  // NIMBLE C wrappers called from R
  SEXP C_dmnorm_chol(SEXP, SEXP, SEXP, SEXP, SEXP); 
  SEXP C_rmnorm_chol(SEXP, SEXP, SEXP); 
  SEXP C_dwish_chol(SEXP, SEXP, SEXP, SEXP, SEXP);
  SEXP C_rwish_chol(SEXP, SEXP, SEXP);
  SEXP C_dcat(SEXP, SEXP, SEXP);
  SEXP C_rcat(SEXP, SEXP);
  SEXP C_dmulti(SEXP, SEXP, SEXP, SEXP);
  SEXP C_rmulti(SEXP, SEXP);
  SEXP C_ddirch(SEXP, SEXP, SEXP);
  SEXP C_rdirch(SEXP);

}

// NOTE: R CMD SHLIB seems to handle C++ code without using wrapping the functions in 'extern "C"'; note that some of these functions have a bit of C++ syntax

// core scalar d/r functions provided by NIMBLE to extend R
double dcat(int, double*, int, int);
int rcat(double*, int);
double dmulti(int*, int, double*, int, int);
void rmulti(int *, int, double*, int);
double ddirch(double*, double*, int, int);
void rdirch(double*, double*, int);

double dmnorm_chol(double*, double*, double*, int, int, int);
void rmnorm_chol(double *, double*, double*, int, int);
double dwish_chol(double*, double*, double, int, int, int);
void rwish_chol(double*, double*, double, int, int);


// SHOULD BE IN nimDists.h

extern "C" {
  SEXP C_rt_nonstandard(SEXP, SEXP, SEXP, SEXP);
  SEXP C_dt_nonstandard(SEXP, SEXP, SEXP, SEXP, SEXP);
}

double dt_nonstandard(double, double, double, double, int);
double rt_nonstandard(double, double, double);



#endif
