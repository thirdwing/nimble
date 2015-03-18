// various additional distributions functions needed by NIMBLE
// Author: Chris Paciorek 
// Date: Initial development in February 2014
// uses various BLAS routines and constants from R's C API
// compile as "R CMD SHLIB dists.cpp"

// FIXME: need full suite of error-checking, including types coming from R

//#include "Utils.h" // moved to dists.h
#include "nimble/dists.h"
#include <R_ext/Lapack.h>


double dwish_chol(double* x, double* chol, double df, int p, int scale_param, int give_log) {
  char uplo('U');
  char side('L');
  char diag('N');
  char transT('T');
  char transN('N');
  int info(0);
  double alpha(1.0);

  int i;

  double dens = -(df*p/2 * M_LN2 + p*(p-1)*M_LN_SQRT_PI/2);
  for(i = 0; i < p; i++)
    dens -= lgammafn((df - i) / 2);

  if(scale_param) {
    for(i = 0; i < p*p; i += p + 1) 
      dens -= df * log(chol[i]);
  } else {
    for(i = 0; i < p*p; i += p + 1) 
      dens += df * log(chol[i]);
  }

  // determinant of x using Cholesky:
  // dpotrf overwrites x so need to copy first since the trace calc below also overwrites
  double* tmp = new double[p*p];
  for(i = 0; i < p*p; i++) 
    tmp[i] = x[i];
  F77_CALL(dpotrf)(&uplo, &p, tmp, &p, &info);
  for(i = 0; i < p*p; i += p + 1) 
    dens += (df - p - 1) * log(tmp[i]);
  delete [] tmp;

  // do upper-triangular solves for scale parameterization
  // or upper-triangular multiplies for rate parameterization
  // dtr{m,s}m is a BLAS level-3 function
  double tmp_dens = 0.0;
  if(scale_param) {
    F77_CALL(dtrsm)(&side, &uplo, &transT, &diag, &p, &p, &alpha, 
           chol, &p, x, &p);
    F77_CALL(dtrsm)(&side, &uplo, &transN, &diag, &p, &p, &alpha, 
           chol, &p, x, &p);
    for(i = 0; i < p*p; i += p + 1) 
      tmp_dens += x[i];
    dens += -0.5 * tmp_dens;
  } else {
    // this could be improved by doing efficient U^T U multiply followed by direct product multiply
    F77_CALL(dtrmm)(&side, &uplo, &transN, &diag, &p, &p, &alpha, 
           chol, &p, x, &p);
    // at this point don't need to do all the multiplications so don't call dtrmm again
    // direct product of upper triangles
    for(int j = 0; j < p; j++) {
      for(i = 0; i <= j; i++) {
        tmp_dens += x[j*p+i] * chol[j*p+i];
      }
    }
    dens += -0.5 * tmp_dens;
  }

  return give_log ? dens : exp(dens);
}


SEXP C_dwish_chol(SEXP x, SEXP chol, SEXP df, SEXP scale_param, SEXP return_log) 
// calculates Wishart density given Cholesky of scale or rate matrix
// Cholesky matrix should be given as a numeric vector in column-major order
//   including all n x n elements; lower-triangular elements are ignored
{
  if(!isReal(x) || !isReal(chol) || !isReal(df) || !isLogical(scale_param) || !isLogical(return_log))
    RBREAK("Error (C_dwish_chol): invalid input type for one of the arguments.\n");
  int p = pow(LENGTH(chol), 0.5);
  int give_log = (int) LOGICAL(return_log)[0];
  int scale = (int) LOGICAL(scale_param)[0];

  double* c_x = REAL(x);
  double* c_chol = REAL(chol);
  double c_df = REAL(df)[0];
  
  if(c_df < p)
    RBREAK("Error (C_dwish_chol): inconsistent degrees of freedom and dimension.\n");

  SEXP ans;
  PROTECT(ans = allocVector(REALSXP, 1));  
  REAL(ans)[0] = dwish_chol(c_x, c_chol, c_df, p, scale, give_log);
  UNPROTECT(1);
  return ans;
}


void rwish_chol(double *Z, double* chol, double df, int p, int scale_param) {
  // Ok that this returns the array as return value? NIMBLE C code will need to free the memory
  char uplo('U');
  char sideL('L');
  //  char sideR('R');
  char diag('N');
  char transT('T');
  char transN('N');
  //  int info(0);
  double alpha(1.0);
  double beta(0.0);

  int i, j, uind, lind;

  //  double* Z = new double[p*p];

  // fill diags with sqrts of chi-squares and upper triangle (for scale_param) with std normals
  for(j = 0; j < p; j++) {
    // double *Z_j = &Z[j*p];
    //Z_j[j] = sqrt(rchisq(df - (double) j)); 
    Z[j*p + j] = sqrt(rchisq(df - (double) j)); 
    for(i = 0; i < j; i++) {
      uind = i + j * p, /* upper triangle index */
      lind = j + i * p; /* lower triangle index */
      Z[(scale_param ? uind : lind)] = norm_rand();
      Z[(scale_param ? lind : uind)] = 0;
    }
    /*
    for(i = 0; i < j; i++) 
      Z_j[i] = norm_rand();
    for (i = j + 1; i < p; i++)
      Z_j[i] = 0;
    */
  }
 
  // multiply Z*chol, both upper triangular or solve(chol, Z^T)
  // would be more efficient if make use of fact that right-most matrix is triangular
  if(scale_param) F77_CALL(dtrmm)(&sideL, &uplo, &transN, &diag, &p, &p, &alpha, Z, &p, chol, &p);
  else F77_CALL(dtrsm)(&sideL, &uplo, &transN, &diag, &p, &p, &alpha, chol, &p, Z, &p);

  // cp result to Z or chol so can be used as matrix to multiply against and overwrite
  if(scale_param) {
    for(j = 0; j < p*p; j++) 
      Z[j] = chol[j];
  } else {
    for(j = 0; j < p*p; j++) 
      chol[j] = Z[j]; // FIXME: check this case.  We don't want to overwite an input argument.
  }

  // do crossprod of result; again this would be more efficient use fact that t(input) is lower-tri
  if(scale_param) F77_CALL(dtrmm)(&sideL, &uplo, &transT, &diag, &p, &p, &alpha, chol, &p, Z, &p);
  else F77_CALL(dgemm)(&transN, &transT, &p, &p, &p, &alpha, chol, &p, chol, &p, &beta, Z, &p); 

}

SEXP C_rwish_chol(SEXP chol, SEXP df, SEXP scale_param) 
// generates single Wishart draw given Cholesky of scale or rate matrix
// Cholesky matrix should be given as a numeric vector in column-major order
//   including all n x n elements; lower-triangular elements are ignored
{
  if(!isReal(chol) || !isReal(df) || !isLogical(scale_param))
    RBREAK("Error (C_rwish_chol): invalid input type for one of the arguments.\n");
  int n_chol = LENGTH(chol);
  int p = pow(n_chol, 0.5);
  int scale = (int) LOGICAL(scale_param)[0];

  double* c_chol = REAL(chol);
  double c_df = REAL(df)[0];

  if(c_df < p)
    RBREAK("Error (C_rwish_chol): inconsistent degrees of freedom and dimension.\n");

  GetRNGstate(); 

  SEXP ans;
  PROTECT(ans = allocVector(REALSXP, n_chol));  
  rwish_chol(REAL(ans), c_chol, c_df, p, scale);
  
  PutRNGstate();
  UNPROTECT(1);
  return ans;
}



double ddirch(double* x, double* alpha, int K, int give_log) 
// scalar function that can be called directly by NIMBLE with same name as in R
{
  double sumAlpha(0.0);
  double sumX(0.0);
  double dens(0.0);
  for(int i = 0; i < K; i++) {
    dens += (alpha[i]-1) * log(x[i]) - lgammafn(alpha[i]) ;
    sumAlpha += alpha[i];
    sumX += x[i];
  }
  if(sumX > 1.0 + 10*DBL_EPSILON || sumX < 1.0 - 10*DBL_EPSILON) {
    return R_NegInf;
  }
  // should return error instead?

  dens += lgammafn(sumAlpha);
  return give_log ? dens : exp(dens);
}

void rdirch(double *ans, double* alpha, int K) 
// scalar function that can be called directly by NIMBLE with same name as in R
{
  //  double* ans = new double[K];
  double sum(0.0);
  for(int i = 0; i < K; i++) {
    ans[i] = rgamma(alpha[i], 1);
    sum += ans[i];
  }
  for(int i = 0; i < K; i++) {
    ans[i] /= sum;
  }
}

SEXP C_ddirch(SEXP x, SEXP alpha, SEXP return_log) 
{
  if(!isReal(x) || !isReal(alpha) || !isLogical(return_log)) 
    RBREAK("Error (C_ddirch): invalid input type for one of the arguments.\n");
  int K = LENGTH(alpha);
  if(LENGTH(x) != K)
    RBREAK("Error (C_ddirch): length of x must equal length of alpha.\n")
  int give_log = (int) LOGICAL(return_log)[0];
  SEXP ans;
    
  if(K == 0) {
    return alpha;
  }

  double* c_x = REAL(x);
  double* c_alpha = REAL(alpha);

  PROTECT(ans = allocVector(REALSXP, 1));  
  REAL(ans)[0] = ddirch(c_x, c_alpha, K, give_log);

  UNPROTECT(1);
  return ans;
}
 

SEXP C_rdirch(SEXP alpha) {
  if(!isReal(alpha))
    RBREAK("Error (C_rdirch): invalid input type for the argument.\n");
  int K = LENGTH(alpha);

  SEXP ans;

  if(K == 0) {
    PROTECT(ans = allocVector(INTSXP, 0));
    UNPROTECT(1);
    return ans;
  }

  double* c_alpha = REAL(alpha);

  GetRNGstate(); 

  PROTECT(ans = allocVector(REALSXP, K));  
  rdirch(REAL(ans), c_alpha, K);
  PutRNGstate();
  UNPROTECT(1);
  return ans;
}



double dmulti(int* x, int size, double* prob, int K, int give_log) // Calling functions need to copy first arg to int if needed
// scalar function that can be called directly by NIMBLE with same name as in R
{
  double dens = lgammafn(size + 1);
  for(int i = 0; i < K; i++) {
   	if(x[i] != 0 & prob[i] != 0)
    dens += x[i]*log(prob[i]) - lgammafn(x[i] + 1);
  }
  return give_log ? dens : exp(dens);
}

void rmulti(int *ans, int size, double* prob, int K) // Calling functions need to copy first arg back and forth to double if needed
// scalar function that can be called directly by NIMBLE with same name as in R
// just call Rmath's rmultinom, which passes result by pointer
{
  rmultinom(size, prob, K, ans);
}

SEXP C_dmulti(SEXP x, SEXP size, SEXP prob, SEXP return_log) 
{
  if(!isInteger(x) || !isInteger(size) || !isReal(prob) || !isLogical(return_log)) 
    RBREAK("Error (C_dmulti): invalid input type for one of the arguments.\n");
  int K = LENGTH(prob);
  if(LENGTH(x) != K)
    RBREAK("Error (C_dmulti): length of x must equal size.\n")
  int give_log = (int) LOGICAL(return_log)[0];
  SEXP ans;
    
  int i;

  if(K == 0) {
    return prob;
  }

  int* c_x = INTEGER(x);
  double* c_prob = REAL(prob);
  int c_size = INTEGER(size)[0];

  double sum = 0.0;
  for(i = 0; i < K; i++) 
    sum += c_prob[i];
  if(sum > 1.0 + 10*DBL_EPSILON || sum < 1.0 - 10*DBL_EPSILON)
    RBREAK("Error (C_dmulti): sum of probabilities is not equal to 1.\n");

  PROTECT(ans = allocVector(REALSXP, 1));  
  REAL(ans)[0] = dmulti(c_x, c_size, c_prob, K, give_log);

  UNPROTECT(1);
  return ans;
}
 

SEXP C_rmulti(SEXP size, SEXP prob) {
  if(!isInteger(size) || !isReal(prob))
    RBREAK("Error (C_rmulti): invalid input type for one of the arguments.\n");
  int K = LENGTH(prob);

  SEXP ans;
  int i;

  if(K == 0) {
    PROTECT(ans = allocVector(INTSXP, 0));
    UNPROTECT(1);
    return ans;
  }

  double* c_prob = REAL(prob);
  int c_size = INTEGER(size)[0];

  double sum = 0.0;
  for(i = 0; i < K; i++) 
    sum += c_prob[i];
  if(sum > 1.0 + 10*DBL_EPSILON || sum < 1.0 - 10*DBL_EPSILON)
    RBREAK("Error (C_rmulti): sum of probabilities is not equal to 1.\n");

  GetRNGstate(); 

  PROTECT(ans = allocVector(INTSXP, K));  
  rmulti(INTEGER(ans), c_size, c_prob, K);
  PutRNGstate();
  UNPROTECT(1);
  return ans;
}


double dcat(int x, double* prob, int K, int give_log)
// scalar function that can be called directly by NIMBLE with same name as in R
{
  if(x > K || x < 1) return((give_log)? R_NegInf : 0.0);
  return give_log ? log(prob[x-1]) : prob[x-1];
}

int rcat(double* prob, int K)
// scalar function that can be called directly by NIMBLE with same name as in R
// problem is no apparent way to call this w/o also passing the number of categories
// relying on sum to 1 risks accessing beyond storage
// we'll need to figure out how to inject the number of categories w/in NIMBLE
{
  double u = unif_rand();
  double prob_cum = prob[0];
  int value = 1;
  while(u > prob_cum && value < K) {
    prob_cum += prob[value];
    value++;
  }

  if(prob_cum > 1.0 + 10*DBL_EPSILON)
    // tolerance of ~ 1 x 10^-15; this is not based on any deep thought
    // this will only catch the issue if u puts us in the last bin
    error("(rcat): sum of probabilities is greater than 1.");
  return value;
}
 
SEXP C_dcat(SEXP x, SEXP prob, SEXP return_log) 
{
  // this will call NIMBLE's dcat() for computation on scalars
  // p must be a single vector of probs adding to one, but x can be a vector

  if(!isInteger(x) || !isReal(prob) || !isLogical(return_log)) 
    RBREAK("Error (C_dcat): invalid input type for one of the arguments.\n");
  int n_x = LENGTH(x);
  int K = LENGTH(prob);
  int give_log = (int) LOGICAL(return_log)[0];
  SEXP ans;
    
  int i;

  if(n_x == 0) {
    return x;
  }

  int* c_x = INTEGER(x);
  double* c_prob = REAL(prob);

  double sum = 0.0;
  for(i = 0; i < K; i++) 
    sum += c_prob[i];
  if(sum > 1.0 + 10*DBL_EPSILON || sum < 1.0 - 10*DBL_EPSILON)
    RBREAK("Error (C_dcat): sum of probabilities is not equal to 1.\n");

  PROTECT(ans = allocVector(REALSXP, n_x));  
  for(i = 0; i < n_x; i++) {
    REAL(ans)[i] = dcat(c_x[i], c_prob, K, give_log);
  }

  UNPROTECT(1);
  return ans;
}
  
SEXP C_rcat(SEXP n, SEXP prob) {
  if(!isInteger(n) || !isReal(prob))
    RBREAK("Error (C_rcat): invalid input type for one of the arguments.\n");
  int n_values = INTEGER(n)[0];
  int K = LENGTH(prob);

  SEXP ans;
  int i;

  if(n_values == 0) {
    PROTECT(ans = allocVector(INTSXP, 0));
    UNPROTECT(1);
    return ans;
  }
  if(n_values < 0)
    RBREAK("Error (C_rcat): n must be non-negative.\n");

  double* c_prob = REAL(prob);

  double sum = 0.0;
  for(i = 0; i < K; i++) 
    sum += c_prob[i];
  if(sum > 1.0 + 10*DBL_EPSILON || sum < 1.0 - 10*DBL_EPSILON)
    RBREAK("Error (C_rcat): sum of probabilities is not equal to 1.\n");

  GetRNGstate(); 
  PROTECT(ans = allocVector(INTSXP, n_values));  

  for(i = 0; i < n_values; i++) 
    INTEGER(ans)[i] = rcat(c_prob, K);

  PutRNGstate();
  UNPROTECT(1);
  return ans;

}
  
double dmnorm_chol(double* x, double* mean, double* chol, int n, int prec_param, int give_log) {
  char uplo('U');
  char transPrec('N');
  char transCov('T');
  char diag('N');
  int lda(n);
  int incx(1);

  double dens = -n * M_LN_SQRT_2PI;
  int i;
  // add diagonals of Cholesky

  if(prec_param) {
    for(i = 0; i < n*n; i += n + 1) 
      dens += log(chol[i]);
  } else {
    for(i = 0; i < n*n; i += n + 1) 
      dens -= log(chol[i]);
  }
  for(i = 0; i < n; i++) 
    x[i] -= mean[i];

  // do matrix-vector multiply with upper-triangular matrix stored column-wise as full n x n matrix (prec parameterization)
  // or upper-triangular (transpose) solve (cov parameterization)
  // dtr{m,s}v is a BLAS level-2 function
  if(prec_param) F77_CALL(dtrmv)(&uplo, &transPrec, &diag, &n, chol, &lda, x, &incx);
  else F77_CALL(dtrsv)(&uplo, &transCov, &diag, &n, chol, &lda, x, &incx);

  // sum of squares to calculate quadratic form
  double tmp = 0.0;
  for(i = 0; i < n; i++)
    tmp += x[i] * x[i];

  dens += -0.5 * tmp;

  return give_log ? dens : exp(dens);
}

SEXP C_dmnorm_chol(SEXP x, SEXP mean, SEXP chol, SEXP prec_param, SEXP return_log) 
// calculates mv normal density given Cholesky of precision matrix or covariance matrix
// Cholesky matrix should be given as a numeric vector in column-major order
//   including all n x n elements; lower-triangular elements are ignored
{
  if(!isReal(x) || !isReal(mean) || !isReal(chol) || !isLogical(prec_param) || !isLogical(return_log))
    RBREAK("Error (C_dnorm_chol): invalid input type for one of the arguments.\n");
  int n_x = LENGTH(x);
  int n_mean = LENGTH(mean);
  int give_log = (int) LOGICAL(return_log)[0];
  int prec = (int) LOGICAL(prec_param)[0];

  double* c_x = REAL(x);
  double* c_mean = REAL(mean);
  double* c_chol = REAL(chol);

  double* xcopy = new double[n_x];
  for(int i = 0; i < n_x; ++i) 
    xcopy[i] = c_x[i];

  double* full_mean;
  if(n_mean < n_x) {
    full_mean = new double[n_x];
    int i_mean = 0;
    for(int i = 0; i < n_x; i++) {
      full_mean[i] = c_mean[i_mean++];
      if(i_mean == n_mean) i_mean = 0;
    }
  } else full_mean = c_mean;
  
  SEXP ans;
  PROTECT(ans = allocVector(REALSXP, 1));  
  REAL(ans)[0] = dmnorm_chol(xcopy, full_mean, c_chol, n_x, prec, give_log);
  if(n_mean < n_x)
    delete [] full_mean;
  delete [] xcopy;
  UNPROTECT(1);
  return ans;
}

void rmnorm_chol(double *ans, double* mean, double* chol, int n, int prec_param) {
  // Ok that this returns the array as return value? NIMBLE C code will need to free the memory
  char uplo('U');
  char transPrec('N');
  char transCov('T');
  char diag('N');
  int lda(n);
  int incx(1);
  
  int i;

  double* devs = new double[n];
  //  double* ans = new double[n];

  for(i = 0; i < n; i++) 
    devs[i] = norm_rand();


  // do upper-triangular solve or (transpose) multiply
  // dtr{s,m}v is a BLAS level-2 function
  if(prec_param) F77_CALL(dtrsv)(&uplo, &transPrec, &diag, &n, chol, &lda, devs, &incx);
  else F77_CALL(dtrmv)(&uplo, &transCov, &diag, &n, chol, &lda, devs, &incx);

  for(i = 0; i < n; i++) 
    ans[i] = mean[i] + devs[i];

  delete [] devs;
}

SEXP C_rmnorm_chol(SEXP mean, SEXP chol, SEXP prec_param) 
// generates single mv normal draw given Cholesky of precision matrix or covariance matrix
// Cholesky matrix should be given as a numeric vector in column-major order
//   including all n x n elements; lower-triangular elements are ignored
{
  if(!isReal(mean) || !isReal(chol) || !isLogical(prec_param))
    RBREAK("Error (C_rmnorm_chol): invalid input type for one of the arguments.\n");
  int n_mean = LENGTH(mean);
  int n_chol = LENGTH(chol);
  int n_values = pow(n_chol, 0.5);
  int prec = (int) LOGICAL(prec_param)[0];

  int i;

  double* c_mean = REAL(mean);
  double* c_chol = REAL(chol);
  double* full_mean; 

  if(n_mean < n_values) {
    full_mean = new double[n_values];
    int i_mean = 0;
    for(i = 0; i < n_values; i++) {
      full_mean[i] = c_mean[i_mean++];
      if(i_mean == n_mean) i_mean = 0;
    }
  } else full_mean = c_mean;

  GetRNGstate(); 

  SEXP ans;
  PROTECT(ans = allocVector(REALSXP, n_values));  
  rmnorm_chol(REAL(ans), full_mean, c_chol, n_values, prec);

  PutRNGstate();
  if(n_mean < n_values) 
    delete [] full_mean;
  UNPROTECT(1);
  return ans;
}



double dt_nonstandard(double x, double df, double mu, double sigma, int give_log)
// scalar function that can be called directly by NIMBLE with same name as in R
// 'n' is name of 'df' in R's C dt
{
  // standardize and multiply by Jacobian of transformation
  if(sigma <= 0) {
    if(sigma < 0) return(R_NaN);
    else return give_log ? R_NegInf : 0.0;
  }         
  if(give_log) return dt( (x - mu)/sigma, df, give_log) - log(sigma);
  else return dt( (x - mu)/sigma, df, give_log) / sigma;
}

double rt_nonstandard(double df, double mu, double sigma)
// scalar function that can be called directly by NIMBLE with same name as in R
{
  if(sigma < 0) return R_NaN;
  return mu + sigma * rt(df);
}


SEXP C_dt_nonstandard(SEXP x, SEXP df, SEXP mu, SEXP sigma, SEXP return_log) {
  if(!isReal(x) || !isReal(df) || !isReal(mu) || !isReal(sigma) || !isLogical(return_log)) 
    RBREAK("Error (C_dt_nonstandard): invalid input type for one of the arguments.");
  int n_x = LENGTH(x);
  int n_mu = LENGTH(mu);
  int n_sigma = LENGTH(sigma);
  int n_df = LENGTH(df);
  int give_log = (int) LOGICAL(return_log)[0];
  SEXP ans;
    
  if(n_x == 0) {
    return x;
  }
    
  PROTECT(ans = allocVector(REALSXP, n_x));  
  double* c_x = REAL(x);
  double* c_mu = REAL(mu);
  double* c_sigma = REAL(sigma);
  double* c_df = REAL(df);

  // FIXME: abstract the recycling as a function
  if(n_mu == 1 && n_sigma == 1 && n_df == 1) {
    // if no parameter vectors, more efficient not to deal with multiple indices
    for(int i = 0; i < n_x; i++) 
      REAL(ans)[i] = dt_nonstandard(c_x[i], *c_df, *c_mu, *c_sigma, give_log);
  } else {
    int i_mu = 0;
    int i_sigma = 0;
    int i_df = 0;
    for(int i = 0; i < n_x; i++) {
      REAL(ans)[i] = dt_nonstandard(c_x[i], c_df[i_df++], c_mu[i_mu++], c_sigma[i_sigma++], give_log);
      //c_mu[i_mu++] + c_sigma[i_sigma++] * rt(c_df[i_df++]);
      // implement recycling:
      if(i_mu == n_mu) i_mu = 0;
      if(i_sigma == n_sigma) i_sigma = 0;
      if(i_df == n_df) i_df = 0;
    }
  }
    
  UNPROTECT(1);
  return ans;
}
  
SEXP C_rt_nonstandard(SEXP n, SEXP df, SEXP mu, SEXP sigma) {
  // this will call R's rt() for computation on scalars
  if(!isInteger(n) || !isReal(df) || !isReal(mu) || !isReal(sigma))
    RBREAK("Error (C_rt_nonstandard): invalid input type for one of the arguments.");
  int n_mu = LENGTH(mu);
  int n_sigma = LENGTH(sigma);
  int n_df = LENGTH(df);
  int n_values = INTEGER(n)[0];
  SEXP ans;
    
  if(n_values == 0) {
    PROTECT(ans = allocVector(REALSXP, 0));
    UNPROTECT(1);
    return ans;
  }
  if(n_values < 0)
    // should formalize using R's C error-handling API
    RBREAK("Error (C_rt_nonstandard): n must be non-negative.\n");
    
  GetRNGstate(); 
    
  PROTECT(ans = allocVector(REALSXP, n_values));  
  double* c_mu = REAL(mu);
  double* c_sigma = REAL(sigma);
  double* c_df = REAL(df);
  if(n_mu == 1 && n_sigma == 1 && n_df == 1) {
    // if no parameter vectors, more efficient not to deal with multiple indices
    for(int i = 0; i < n_values; i++) 
      REAL(ans)[i] = rt_nonstandard(*c_df, *c_mu, *c_sigma);
  } else {
    int i_mu = 0;
    int i_sigma = 0;
    int i_df = 0;
    for(int i = 0; i < n_values; i++) {
      REAL(ans)[i] = rt_nonstandard(c_df[i_df++], c_mu[i_mu++], c_sigma[i_sigma++]);
      //c_mu[i_mu++] + c_sigma[i_sigma++] * rt(c_df[i_df++]);
      // implement recycling:
      if(i_mu == n_mu) i_mu = 0;
      if(i_sigma == n_sigma) i_sigma = 0;
      if(i_df == n_df) i_df = 0;
    }
  }
    
  PutRNGstate();
  UNPROTECT(1);
  return ans;
}
  


