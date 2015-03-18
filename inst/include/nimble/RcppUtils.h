#ifndef __RCPPUTILS
#define __RCPPUTILS

#include "NimArrBase.h"
#include "NimArr.h"
#include "R.h"
#include "Utils.h"
#include <string>
#include <vector>
#include <Rinternals.h>
using namespace std;
#define Inf R_PosInf
#define NA 0

void multivarTestCall(double *x, int n);

bool all(bool x);
bool any(bool x);
int t(int x);
double t(double x);
int prod(int x);
double prod(double x);

string STRSEXP_2_string(SEXP Ss, int i);

vector<double> SEXP_2_vectorDouble( SEXP Sn ); /* Sn can be numeric or integer from R*/
double SEXP_2_double(SEXP Sn, int i = 0); /* Ditto */
SEXP double_2_SEXP(double v);
SEXP vectorDouble_2_SEXP(const vector<double> &v);
SEXP vectorInt_2_SEXP(const vector<int> &v);

vector<int> SEXP_2_vectorInt(SEXP Sn, int offset = 0); /* Sn can be numeric or integer from R */
/* Offset is added to every value, so if the vectors are indices, offset = -1 is useful */
/* If Sn is numeric but not integer, a warning is issued if it contains non-integers */
int SEXP_2_int(SEXP Sn, int i = 0, int offset = 0);
SEXP int_2_SEXP(int i);
bool SEXP_2_bool(SEXP Sn, int i = 0);
SEXP bool_2_SEXP(bool ind);

SEXP cGetMVElementOneRow(NimVecType* typePtr, nimType vecType, int index);
SEXP cGetMVElementOneRow(NimVecType* typePtr, nimType vecType, int nrowCpp, int index);
void cSetMVElementSingle(NimVecType* typePtr, nimType vecType,  int index, SEXP Svalue);

bool checkString(SEXP Ss, int len);
bool checkNumeric(SEXP Sval, int len);

vector<int> getSEXPdims(SEXP Sx);

extern "C" {
    SEXP setVec(SEXP Sextptr, SEXP Svalue);
    SEXP getVec(SEXP Sextptr);
    SEXP getVec_Integer(SEXP Sextptr);

    SEXP addBlankModelValueRows(SEXP Sextptr, SEXP numAdded);
    SEXP getNRow(SEXP Sextptr);
    SEXP copyModelValuesElements(SEXP SextptrFrom, SEXP SextptrTo, SEXP rowsFrom, SEXP rowsTo);
    SEXP getMVElement(SEXP Sextptr, SEXP Sindex);
    SEXP getMVsize(SEXP Sextptr);

    SEXP getMVElementAsList(SEXP SextPtr, SEXP Sindices);
    SEXP setMVElementFromList(SEXP vNimPtr, SEXP rList, SEXP Sindices);

    SEXP fastMatrixInsert(SEXP matrixInto, SEXP matrix, SEXP rowStart, SEXP colStart);
    SEXP matrix2VecNimArr(SEXP RvecNimPtr, SEXP matrix, SEXP rowStart, SEXP rowEnd);
    SEXP matrix2ListDouble(SEXP matrix, SEXP list, SEXP listStartIndex, SEXP RnRows,  SEXP dims);
    SEXP matrix2ListInt(SEXP matrix, SEXP list, SEXP listStartIndex, SEXP RnRows,  SEXP dims);

    SEXP printMVElement(SEXP Sextptr, SEXP Sindex);
    SEXP setMVElement(SEXP Sextptr, SEXP Sindex, SEXP Svalue);

    SEXP resizeNumListRow(SEXP Sextptr, SEXP Sindex, SEXP dims); 	// resizes a particular row of a numericlist

//  SEXP setNumList(SEXP Sextptr, SEXP Sindex, SEXP Svalue);   automatically resizes. Might want to use later
    SEXP setNumListRows(SEXP Sextptr, SEXP nRows, SEXP setSize2row1);		// this sets the number of rows in a numericList (really, any VecNimArr)


    SEXP setVarPointer(SEXP SextptrModelVar, SEXP SextptrStorageVar, SEXP Srownum);
    SEXP makeNumericList(SEXP nDims, SEXP type, SEXP nRows);

    SEXP newSampObject();	//  Creates our new object from sampleClass (will be generated automatically later)
    //	Just for use in demos
    // 	To get a pointer to an element from sampleClass, use
    //	getModelObjectPtr (from the NamedObjects.cpp file)

    SEXP Nim_2_SEXP(SEXP rPtr, SEXP NumRefers);	//	Returns SEXP object with correct data type and dimensions. NumRefers
    //  should be an integer with the number of dereferencing required for rPtr
    //	So if rPtr is a pointer to a NimArr, NumRefers = 1
    //	If rPtr is a pointer to a pointer to a NimArr, NumRefers = 2
    //	Currently only NumRefers = 1 and 2 are allowed, but easily updated
    //	by extending "getNimTypePtr" function

    SEXP SEXP_2_Nim(SEXP rPtr, SEXP NumRefers, SEXP rValues, SEXP allowResize); //	Copies values from rValues to NimArr. Same behavior
    // 	with NumRefers as above. Also, type checking is done
    // 	by R.internals functions INTEGER and REAL

    SEXP Nim_2_Nim(SEXP rPtrFrom, SEXP numRefFrom, SEXP rPtrTo, SEXP numRefTo);
    //  Copies from one NimArr to another. Type checks
    //	For now, both NimArr's must be either double or int. We can add other
    //  types or allow conversion by extending Nim_2_Nim and the cNim_2_Nim options

    SEXP SEXP_2_double(SEXP rPtr, SEXP refNum, SEXP rScalar);
    SEXP double_2_SEXP(SEXP rPtr, SEXP refNum);
    SEXP SEXP_2_int(SEXP rPtr, SEXP refNum, SEXP rScalar);
    SEXP SEXP_2_bool(SEXP rPtr, SEXP refNum, SEXP rScalar);
    SEXP bool_2_SEXP(SEXP rPtr, SEXP refNum);
    SEXP SEXP_2_int(SEXP rPtr, SEXP refNum, SEXP rScalar);
    SEXP int_2_SEXP(SEXP rPtr, SEXP refNum);


    SEXP setPtrVectorOfPtrs(SEXP SaccessorPtr, SEXP ScontentsPtr, SEXP Ssize);
    SEXP setOnePtrVectorOfPtrs(SEXP SaccessorPtr, SEXP Si, SEXP ScontentsPtr);
    SEXP getOnePtrVectorOfPtrs(SEXP SaccessorPtr, SEXP Si);

    SEXP rankSample(SEXP p, SEXP n);


    SEXP getEnvVar_Sindex(SEXP sString, SEXP sEnv, SEXP sIndex);// This is a utility for looking up a field of an environment
    // sString is a character vector with the field name we want
    // sEnv is the environment
    // sIndex is the index of the sString that contains the name
    // we actually want to use.
    //	Look up by sString[sIndex] is done to allow for easy looping
    //Important Note: sIndex = 1 looks up the first name (i.e. use R indexing, not C)
    SEXP getEnvVar(SEXP sString, SEXP sEnv);	// Same as above, but uses sIndex = 1 (i.e. sString is a single character string)

    SEXP setEnvVar_Sindex(SEXP sString, SEXP sEnv, SEXP sVal, SEXP sIndex);	//Same as getEnvVar_Sindex, but this function sets rather than gets
    SEXP setEnvVar(SEXP sString, SEXP sEnv, SEXP sVal);						//Same as above but uses sIndex = 1
}

class vectorOfPtrsAccessBase {
  public:
    virtual void setTheVec(void *tV, int size)=0;
    virtual void setVecPtr(int i, void *v)=0;
    virtual void *getVecPtr(int i)=0;
};

template<class T>
class vectorOfPtrsAccess : public vectorOfPtrsAccessBase {
  public:
    vector<T*> *theVec;
    void setTheVec(void* v, int size) {
        theVec = static_cast<vector<T*>*>(v);
        theVec->resize(size);
    }
    void setVecPtr(int i, void *v) {
        (*theVec)[i] = static_cast<T*>(v);
    }
    void *getVecPtr(int i) {
        return(static_cast<void *>( (*theVec)[i] ) );
    }
};


/*
  Apparently partial specialization of function templates is not allowed.
  So these are witten for doubles, and when we get to integers and logicals we can
  use overlaoding or different names.
 */
template<int ndim>
void SEXP_2_NimArr(SEXP Sn, NimArr<ndim, double> &ans );
template<int ndim>
void SEXP_2_NimArr(SEXP Sn, NimArr<ndim, int> &ans );

template<int ndim>
SEXP NimArr_2_SEXP(const NimArr<ndim, double> &val);
template<int ndim>
SEXP NimArr_2_SEXP(const NimArr<ndim, int> &val);

template<>
void SEXP_2_NimArr<1>(SEXP Sn, NimArr<1, double> &ans);
template<>
void SEXP_2_NimArr<1>(SEXP Sn, NimArr<1, int> &ans);

template<int ndim>
void SEXP_2_NimArr(SEXP Sn, NimArr<ndim, double> &ans) {
    if(!(isNumeric(Sn) || isLogical(Sn))) PRINTF("Error: SEXP_2_NimArr<ndim> called for SEXP that is not a numeric or logica!\n");
    vector<int> inputDims(getSEXPdims(Sn));
    if(inputDims.size() != ndim) PRINTF("Error: Wrong number of input dimensions in SEXP_2_NimArr<ndim, double> called for SEXP that is not a numeric!\n");
    // if(ans.size() != 0) PRINTF("Error: trying to reset a NimArr that was already sized\n");
    ans.setSize(inputDims);
    int nn = LENGTH(Sn);
    if(isReal(Sn)) {
        std::copy(REAL(Sn), REAL(Sn) + nn, ans.getPtr() );
    } else {
        if(isInteger(Sn) || isLogical(Sn)) {
            int *iSn = isInteger(Sn) ? INTEGER(Sn) : LOGICAL(Sn);
            std::copy(iSn, iSn + nn, ans.getPtr()); //v);
        } else {
            PRINTF("Error: could not handle input type to SEXP_2_NimArr\n");
        }
    }
}

// ACTUALLY THIS IS IDENTICAL CODE TO ABOVE, SO THEY COULD BE COMBINED WITHOUT TEMPLATE SPECIALIZATION
template<int ndim>
void SEXP_2_NimArr(SEXP Sn, NimArr<ndim, int> &ans) {
    if(!(isNumeric(Sn) || isLogical(Sn))) PRINTF("Error: SEXP_2_NimArr<ndim> called for SEXP that is not a numeric or logica!\n");
    vector<int> inputDims(getSEXPdims(Sn));
    if(inputDims.size() != ndim) PRINTF("Error: Wrong number of input dimensions in SEXP_2_NimArr<ndim, double> called for SEXP that is not a numeric!\n");
    // if(ans.size() != 0) PRINTF("Error: trying to reset a NimArr that was already sized\n");
    ans.setSize(inputDims);
    int nn = LENGTH(Sn);
    if(isReal(Sn)) {
        std::copy(REAL(Sn), REAL(Sn) + nn, ans.getPtr() );
    } else {
        if(isInteger(Sn) || isLogical(Sn)) {
            int *iSn = isInteger(Sn) ? INTEGER(Sn) : LOGICAL(Sn);
            std::copy(iSn, iSn + nn, ans.getPtr()); //v);
        } else {
            PRINTF("Error: could not handle input type to SEXP_2_NimArr\n");
        }
    }
}

template<int ndim>
void SEXP_2_NimArr(SEXP Sn, NimArr<ndim, bool> &ans) {
    if(!(isNumeric(Sn) || isLogical(Sn))) PRINTF("Error: SEXP_2_NimArr<ndim> called for SEXP that is not a numeric or logica!\n");
    vector<int> inputDims(getSEXPdims(Sn));
    if(inputDims.size() != ndim) PRINTF("Error: Wrong number of input dimensions in SEXP_2_NimArr<ndim, double> called for SEXP that is not a numeric!\n");
    // if(ans.size() != 0) PRINTF("Error: trying to reset a NimArr that was already sized\n");
    ans.setSize(inputDims);
    int nn = LENGTH(Sn);
    if(isReal(Sn)) {
        std::copy(REAL(Sn), REAL(Sn) + nn, ans.getPtr() );
    } else {
        if(isInteger(Sn) || isLogical(Sn)) {
            int *iSn = isInteger(Sn) ? INTEGER(Sn) : LOGICAL(Sn);
            std::copy(iSn, iSn + nn, ans.getPtr()); //v);
        } else {
            PRINTF("Error: could not handle input type to SEXP_2_NimArr\n");
        }
    }
}


template<int ndim>
SEXP NimArr_2_SEXP(NimArr<ndim, double> &val) {
    SEXP Sans;
    int outputLength = val.size();
    PROTECT(Sans = allocVector(REALSXP, outputLength));
    double *ans = REAL(Sans);

    std::copy(val.getPtr(), val.getPtr() + outputLength, ans);
    if(val.numDims() > 1) {
        SEXP Sdim;
        PROTECT(Sdim = allocVector(INTSXP, val.numDims() ) );
        for(int idim = 0; idim < val.numDims(); ++idim) INTEGER(Sdim)[idim] = val.dimSize(idim);
        setAttrib(Sans, R_DimSymbol, Sdim);
        UNPROTECT(2);
    } else {
        UNPROTECT(1);
    }
    return(Sans);
}

template<int ndim>
SEXP NimArr_2_SEXP(NimArr<ndim, int> &val) {
    SEXP Sans;
    int outputLength = val.size();
    PROTECT(Sans = allocVector(INTSXP, outputLength));
    int *ans = INTEGER(Sans);

    std::copy(val.getPtr(), val.getPtr() + outputLength, ans);
    if(val.numDims() > 1) {
        SEXP Sdim;
        PROTECT(Sdim = allocVector(INTSXP, val.numDims() ) );
        for(int idim = 0; idim < val.numDims(); ++idim) INTEGER(Sdim)[idim] = val.dimSize(idim);
        setAttrib(Sans, R_DimSymbol, Sdim);
        UNPROTECT(2);
    } else {
        UNPROTECT(1);
    }
    return(Sans);
}

template<int ndim>
SEXP NimArr_2_SEXP(NimArr<ndim, bool> &val) {
    SEXP Sans;
    int outputLength = val.size();
    PROTECT(Sans = allocVector(LGLSXP, outputLength));
    int *ans = LOGICAL(Sans);

    std::copy(val.getPtr(), val.getPtr() + outputLength, ans);
    if(val.numDims() > 1) {
        SEXP Sdim;
        PROTECT(Sdim = allocVector(LGLSXP, val.numDims() ) );
        for(int idim = 0; idim < val.numDims(); ++idim) LOGICAL(Sdim)[idim] = val.dimSize(idim);
        setAttrib(Sans, R_DimSymbol, Sdim);
        UNPROTECT(2);
    } else {
        UNPROTECT(1);
    }
    return(Sans);
}



void row2NimArr(SEXP &matrix, NimArrBase<double> &nimPtr, int startPoint, int len, int nRows);
void row2NimArr(SEXP &matrix, NimArrBase<int> &nimPtr, int startPoint, int len, int nRows);

template <class T>
void cNimArr_2_NimArr(NimArrBase<T> &nimFrom, NimArrBase<T> &nimTo);

template <class T1, class T2>
void cNimArr_2_NimArr_Index(NimArrBase<T1> &nimFrom, int fromBegin, NimArrBase<T2> &nimTo, int toBegin, int length);

//void trashNamedObjectsPtr(SEXP rPtr);
//void trashElementPtr(SEXP rPtr);
void sampleFinalizer(SEXP ptr);
template<int nDim, class T>
void VecNimArrFinalizer(SEXP ptr);
void dontDeleteFinalizer(SEXP ptr);

//static void ProbSampleReplace(int n, double *p, int *perm, int nans, int *ans);

double nimMod(double a, double b);

template<class T>
int length(vector<T> vec) {
    return(vec.size());
}

class orderedPair {	//simple class which is used to be sorted by value, but remember what the original order was. used in rawSample
  public:
    double value;
    int rank;
};

bool compareOrderedPair(orderedPair a, orderedPair b);			//function called for sort

void rawSample(double* p, int c_samps, int N, int* ans, bool unsort);
void rankSample(NimArr<1, double>& weights, int& n, NimArr<1, int>& output);
#endif
