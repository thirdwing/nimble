#ifndef __NIMARR
#define __NIMARR

#include "NimArrBase.h"
#include "Utils.h"

template<int ndim, class T>
  class NimArr;

// Here is the specialization for 1 dimensions (for any type, T = double, int or bool).

template<class T>
class NimArr<1, T> : public NimArrBase<T> {
public:
  int size1;
  int calculateIndex(int i){return(NimArrBase<T>::offset + NimArrBase<T>::stride1 * i);}
  T &operator()(int i) {return((*NimArrBase<T>::vPtr)[calculateIndex(i)]);} // could add asserts here

  ~NimArr<1, T>() {};

  template<class Tother>
  NimArr<1, T> &mapCopy(const NimArr<1, Tother> &other) {
    //   std::cout<<"In NimArr<1,T> mapCopy\n";
    if(size1 != other.size1) {
      std::cout<<"Error in mapCopy.  Sizes don't match: "<<size1<<" != "<<other.size1<<"\n";
    }
    typename vector<T>::iterator to(NimArrBase<T>::vPtr->begin() + NimArrBase<T>::offset);
    typename vector<Tother>::iterator from = other.vPtr->begin() + other.offset;
    int otherStride = other.stride1;
    for(int iii = 0 ; iii < size1 ; ++iii) {
	 *to = *from;
	 to += NimArrBase<T>::stride1;
	 from += otherStride;
    }
    //    std::cout<<"Leaving NimArr<1,T> mapCopy\n";
    return(*this);
  }

  template<class Tother>
  NimArr<1, T> &templateCopyOperator(const NimArr<1, Tother> &other) { // This makes the copied-to object contiguous-memory.  Use mapCopy to copy into an existing map
    //std::cout<<"Using operator= for a NimArr<1, T>\n";
    //std::cout<<"types "<<NimArrBase<T>::getNimType()<<" "<<other.getNimType()<<"\n";
    if(NimArrBase<T>::isMap()) {
      // std::cout<<"going to map copy\n";
     return(mapCopy(other));
    }
  
    NimArrBase<T>::NAdims = other.dim();
    size1 = NimArrBase<T>::NAdims[0];

    NimArrBase<T>::NAlength = other.size();
    //    NimArrBase<T>::myType = other.getNimType();
    NimArrBase<T>::setMyType();

    NimArrBase<T>::boolMap = false;
    NimArrBase<T>::offset = 0;
    // vPtr, NAstrides, offset, stride1, boolMap
    //   NimArrBase<T>::NAstrides.resize(1); 
    NimArrBase<T>::NAstrides[0] = NimArrBase<T>::stride1 = 1;   
    if(other.boolMap) {
      NimArrBase<T>::v.resize(size1);
       typename vector<T>::iterator to(NimArrBase<T>::v.begin());
       typename vector<T>::iterator toEnd = NimArrBase<T>::v.end();
       typename vector<Tother>::iterator from = (*other.getVptr()).begin() + other.offset;
       int otherStride = other.stride1;
       for( ; to != toEnd ; ++to ) {
	 //	 std::cout<<*from<<"\n";
	 *to = *from;
	 from += otherStride;
       }
    } else {
      NimArrBase<T>::v.resize(size1);
      std::copy(other.v.begin(), other.v.end(), NimArrBase<T>::v.begin());
      //      NimArrBase<T>::v = other.v;
    }
    NimArrBase<T>::setVptr();
    //    std::cout<<"Leaving operator= for a NimArr<1, T>\n";
    return(*this);
  }

  NimArr<1, T> &operator=(const NimArr<1, double> &other) {return(templateCopyOperator<double>(other));}
  NimArr<1, T> &operator=(const NimArr<1, int> &other) {return(templateCopyOperator<int>(other));}
  

  NimArr<1, T> (const NimArr<1, T> &other) :
  NimArrBase<T>(other)
  {
    //   std::cout<<"Using copy constructor for a NimArr<1, T>\n";
    NimArrBase<T>::NAdims = other.dim();
    size1 = NimArrBase<T>::NAdims[0];
    // vPtr, NAstrides, offset, stride1, boolMap
    NimArrBase<T>::NAstrides.resize(1); 
    NimArrBase<T>::NAstrides[0] = NimArrBase<T>::stride1 = 1;   
     if(other.boolMap) {
       NimArrBase<T>::v.resize(size1);
       typename vector<T>::iterator to(NimArrBase<T>::v.begin());
       typename vector<T>::iterator toEnd = NimArrBase<T>::v.end();
       typename vector<T>::iterator from = (*other.vPtr).begin() + other.offset;
       int otherStride = other.stride1;
       for( ; to != toEnd ; ++to ) {
	 *to = *from;
	 from += otherStride;
       }
    } else {
      NimArrBase<T>::v = other.v;
    }
    NimArrBase<T>::setVptr();

  }

  NimArr<1, T>() : NimArrBase<T>() {
    NimArrBase<T>::NAdims.resize(1);
    NimArrBase<T>::NAstrides.resize(1);
    setSize(0);
  }

  void setMap(NimArrBase<T> &source, int off, int str1, int is1) {
    NimArrBase<T>::boolMap = true;
    NimArrBase<T>::offset = off;
    NimArrBase<T>::vPtr = source.getVptr();
    NimArrBase<T>::NAdims[0] = NimArrBase<T>::NAlength = size1 = is1;
    NimArrBase<T>::NAstrides[0] = NimArrBase<T>::stride1 = str1;
  }

  NimArr<1, T>(vector<T> &vm, int off, int str1, int is1) : NimArrBase<T>(vm, off) {
    NimArrBase<T>::NAdims.resize(1);
    NimArrBase<T>::NAstrides.resize(1);
    NimArrBase<T>::NAdims[0] = NimArrBase<T>::NAlength = size1 = is1;
    NimArrBase<T>::NAstrides[0] = NimArrBase<T>::stride1 = str1;
  }

  NimArr<1, T>(int is1) : NimArrBase<T>() {
    NimArrBase<T>::NAdims.resize(1);
    NimArrBase<T>::NAstrides.resize(1);
    setSize(is1);
  }

  void setSize(int is1) {
    NimArrBase<T>::NAdims[0] = size1 = is1;
    NimArrBase<T>::NAstrides[0] = NimArrBase<T>::stride1 = 1;
    NimArrBase<T>::setLength(size1);
  }
  virtual void setSize(vector<int> sizeVec) {setSize(sizeVec[0]);}
  virtual int numDims() const {return(1);}
  virtual int dimSize(int i) const {
    switch(i) {
    case 0:
      return(size1); break;
    default:
      PRINTF("Error, incorrect dimension given to dimSize\n");
      return(0);
    }
  }
};



// Here is the specialization for 2 dimensions.

template<class T>
class NimArr<2, T> : public NimArrBase<T> {
public:
  int size1, size2, stride2;
  int calculateIndex(int i, int j) {return(NimArrBase<T>::offset + NimArrBase<T>::stride1 * i + stride2 * j);} // j * s1 + i);}
  T &operator()(int i, int j) {return((*NimArrBase<T>::vPtr)[calculateIndex(i, j)]);} // could add asserts here

  ~NimArr<2, T>() {};

  NimArr<2, T> &mapCopy(const NimArr<2, T> &other) {
    // std::cout<<"In NimArr<2, T> mapCopy\n";
    if(size1 != other.size1) {
      std::cout<<"Error in mapCopy.  Sizes 1 don't match: "<<size1<<" != "<<other.size1<<"\n";
    }
    if(size2 != other.size2) {
      std::cout<<"Error in mapCopy.  Sizes 2 don't match: "<<size2<<" != "<<other.size2<<"\n";
    }

    typename vector<T>::iterator to(NimArrBase<T>::vPtr->begin() + NimArrBase<T>::offset);
    typename vector<T>::iterator from = other.vPtr->begin() + other.offset;
    int otherStride1 = other.stride1;
    int otherStride2 = other.stride2;
    for(int iCol = 0; iCol < size2; ++iCol) {
      for(int iRow = 0; iRow < size1; ++iRow) {
	*to = *from;
	to += NimArrBase<T>::stride1;
	from += otherStride1;
      }
      from += (-size1 * otherStride1) + otherStride2;
      to += (-size1 * NimArrBase<T>::stride1) + stride2;
    }
    // std::cout<<"Leaving NimArr<2, T> mapCopy\n";
    return(*this);
  }

  NimArr<2, T> &operator=(const NimArr<2, T> &other) {
    // std::cout<<"Using operator= for a NimArr<2, T>\n";

    if(NimArrBase<T>::isMap()) {
      return(mapCopy(other));
    }

    NimArrBase<T>::NAdims = other.dim();
    size1 = NimArrBase<T>::NAdims[0];
    size2 = NimArrBase<T>::NAdims[1];

    NimArrBase<T>::NAlength = other.size();
    NimArrBase<T>::myType = other.getNimType();

    NimArrBase<T>::boolMap = false;
    NimArrBase<T>::offset = 0;
   
    // vPtr, NAstrides, offset, stride1, boolMap
    //    NimArrBase<T>::NAstrides.resize(2); 
    NimArrBase<T>::NAstrides[0] = NimArrBase<T>::stride1 = 1;   
    NimArrBase<T>::NAstrides[1] = stride2 = size1;   
    if(other.boolMap) {
      NimArrBase<T>::v.resize(NimArrBase<T>::NAlength);
      typename vector<T>::iterator to = NimArrBase<T>::v.begin();
      typename vector<T>::iterator from = (*other.vPtr).begin() + other.offset;
      int otherStride1 = other.stride1;
      int otherStride2 = other.stride2;
      for(int iCol = 0; iCol < size2; ++iCol) {
	for(int iRow = 0; iRow < size1; ++iRow) {
	  *to = *from;
	  ++to;
	  from += otherStride1;
	}
	from += (-size1 * otherStride1) + otherStride2;
      }
    } else {
      NimArrBase<T>::v = other.v;
    }
    NimArrBase<T>::setVptr();
    // std::cout<<"Leaving operator= for a NimArr<2, T>\n";
    return(*this);
  }

  NimArr<2, T> (const NimArr<2, T> &other) :
  NimArrBase<T>(other)
  {
    NimArrBase<T>::NAdims = other.dim();
    // std::cout<<"Using copy constructor for a NimArr<2, T>\n";
    size1 = NimArrBase<T>::NAdims[0];
    size2 = NimArrBase<T>::NAdims[1];

    NimArrBase<T>::NAstrides.resize(2); 
    NimArrBase<T>::NAstrides[0] = NimArrBase<T>::stride1 = 1;   
    NimArrBase<T>::NAstrides[1] = stride2 = size1;   
    if(other.boolMap) {
      NimArrBase<T>::v.resize(NimArrBase<T>::NAlength);
      typename vector<T>::iterator to = NimArrBase<T>::v.begin();
      typename vector<T>::iterator from = (*other.vPtr).begin() + other.offset;
      int otherStride1 = other.stride1;
      int otherStride2 = other.stride2;
      for(int iCol = 0; iCol < size2; ++iCol) {
	for(int iRow = 0; iRow < size1; ++iRow) {
	  *to = *from;
	  ++to;
	  from += otherStride1;
	}
	from += (-size1 * otherStride1) + otherStride2;
      }
    } else {
      NimArrBase<T>::v = other.v;
    }
    NimArrBase<T>::setVptr();
  }

  NimArr<2, T> () : NimArrBase<T>() {
    NimArrBase<T>::NAdims.resize(2);
    NimArrBase<T>::NAstrides.resize(2);
    setSize(0, 0);
  }

  void setMap(NimArrBase<T> &source, int off, int str1, int str2, int is1, int is2) {
    NimArrBase<T>::boolMap = true;
    NimArrBase<T>::offset = off;
    NimArrBase<T>::vPtr = source.getVptr();
    NimArrBase<T>::NAdims[0] =  size1 = is1;
    NimArrBase<T>::NAdims[1] = size2 = is2;
    NimArrBase<T>::NAlength = size1 * size2;
    NimArrBase<T>::NAstrides[0] = NimArrBase<T>::stride1 = str1;
    NimArrBase<T>::NAstrides[1] = stride2 = str2;
  }

  NimArr<2, T>(vector<T> &vm, int off, int str1, int str2, int is1, int is2) : NimArrBase<T>(vm, off) {
    NimArrBase<T>::NAdims.resize(2);
    NimArrBase<T>::NAdims[0] = size1 = is1;
    NimArrBase<T>::NAdims[1] = size2 = is2;
    NimArrBase<T>::NAlength = size1 * size2;
    // not setSize because it uses the allocated vm
    NimArrBase<T>::NAstrides.resize(2);
    NimArrBase<T>::NAstrides[0] = NimArrBase<T>::stride1 = str1;
    NimArrBase<T>::NAstrides[1] = stride2 = str2;
  }

  NimArr<2, T>(int is1, int is2) : NimArrBase<T>() {
    NimArrBase<T>::NAdims.resize(2); 
    NimArrBase<T>::NAstrides.resize(2);
    setSize(is1, is2);
  }

    void setSize(int is1, int is2) {
      NimArrBase<T>::NAdims[0] = size1 = is1; 
      NimArrBase<T>::NAdims[1] = size2 = is2;
      NimArrBase<T>::setLength(size1 * size2);
      NimArrBase<T>::NAstrides[0] = NimArrBase<T>::stride1 = 1;
      NimArrBase<T>::NAstrides[1] = stride2 = is1;
    }
    virtual void setSize(vector<int> sizeVec) {
      setSize(sizeVec[0], sizeVec[1]);
    }
    virtual int numDims() const {return(2);}
    virtual int dimSize(int i) const {
      switch(i) {
      case 0:
	return(size1); break;
      case 1:
	return(size2); break;
      default:
	PRINTF("Error, incorrect dimension given to dimSize\n");
	return(0);
      }
    }
};

// Here is the specialization for 3 dimensions.

template<class T>
class NimArr<3, T> : public NimArrBase<T> {
 public:
  int size1, size2, size3, stride2, stride3;//s1s2;
  int calculateIndex(int i, int j, int k) {return(NimArrBase<T>::offset + NimArrBase<T>::stride1 * i + stride2 * j + stride3 * k);} //k * s1s2 + j*s1 + i);}
  T &operator()(int i, int j, int k) {return((*NimArrBase<T>::vPtr)[calculateIndex(i, j, k)]);} // could add asserts here

  ~NimArr<3, T>() {};

  NimArr<3, T> &mapCopy(const NimArr<3, T> &other) {
    // std::cout<<"In NimArr<3, T> mapCopy\n";
    if(size1 != other.size1) {
      std::cout<<"Error in mapCopy.  Sizes 1 don't match: "<<size1<<" != "<<other.size1<<"\n";
    }
    if(size2 != other.size2) {
      std::cout<<"Error in mapCopy.  Sizes 2 don't match: "<<size2<<" != "<<other.size2<<"\n";
    }
    if(size3 != other.size3) {
      std::cout<<"Error in mapCopy.  Sizes 3 don't match: "<<size3<<" != "<<other.size3<<"\n";
    }

    typename vector<T>::iterator to(NimArrBase<T>::vPtr->begin() + NimArrBase<T>::offset);
    typename vector<T>::iterator from = other.vPtr->begin() + other.offset;
    int otherStride1 = other.stride1;
    int otherStride2 = other.stride2;
    int otherStride3 = other.stride3;
    for(int i3rd = 0; i3rd < size3; ++i3rd) {
      for(int iCol = 0; iCol < size2; ++iCol) {
	for(int iRow = 0; iRow < size1; ++iRow) {
	  *to = *from;
	  to += NimArrBase<T>::stride1;
	  from += otherStride1;
	}
	from += (-size1 * otherStride1) + otherStride2;
	to += (-size1 * NimArrBase<T>::stride1) + stride2;
      }
      from += (-size2 * otherStride2) + otherStride3;
      to += (-size2 * stride2 + stride2);
    }
    // std::cout<<"Leaving NimArr<3, T> mapCopy\n";
    return(*this);
  }

  NimArr<3, T> &operator=(const NimArr<3, T> &other) {
    // std::cout<<"Using operator= for a NimArr<3, T>\n";

    if(NimArrBase<T>::isMap()) {
      return(mapCopy(other));
    }

    NimArrBase<T>::NAdims = other.dim();
    size1 = NimArrBase<T>::NAdims[0];
    size2 = NimArrBase<T>::NAdims[1];
    size3 = NimArrBase<T>::NAdims[2];

    NimArrBase<T>::NAlength = other.size();
    NimArrBase<T>::myType = other.getNimType();

    NimArrBase<T>::boolMap = false;
    NimArrBase<T>::offset = 0;
   
    // vPtr, NAstrides, offset, stride1, boolMap
    //    NimArrBase<T>::NAstrides.resize(2); 
    NimArrBase<T>::NAstrides[0] = NimArrBase<T>::stride1 = 1;   
    NimArrBase<T>::NAstrides[1] = stride2 = size1;   
    NimArrBase<T>::NAstrides[2] = stride3 = size1 * size2;   
    if(other.boolMap) {
      NimArrBase<T>::v.resize(NimArrBase<T>::NAlength);
      typename vector<T>::iterator to = NimArrBase<T>::v.begin();
      typename vector<T>::iterator from = (*other.vPtr).begin() + other.offset;
      int otherStride1 = other.stride1;
      int otherStride2 = other.stride2;
      int otherStride3 = other.stride3;
      for(int i3rd = 0; i3rd < size3; ++i3rd) {
	for(int iCol = 0; iCol < size2; ++iCol) {
	  for(int iRow = 0; iRow < size1; ++iRow) {
	    *to = *from;
	    ++to;
	    from += otherStride1;
	}
	  from += (-size1 * otherStride1) + otherStride2;
	}
	from += (-size2 * otherStride2) + otherStride3;
      }
    } else {
      NimArrBase<T>::v = other.v;
    }
    NimArrBase<T>::setVptr();
    // std::cout<<"Leaving operator= for a NimArr<3, T>\n";
    return(*this);
  }

  NimArr<3, T> (const NimArr<3, T> &other) :
  NimArrBase<T>(other)
  {
    NimArrBase<T>::NAdims = other.dim();
    // std::cout<<"Using copy constructor for a NimArr<3, T>\n";
    size1 = NimArrBase<T>::NAdims[0];
    size2 = NimArrBase<T>::NAdims[1];
    size3 = NimArrBase<T>::NAdims[2];

    NimArrBase<T>::NAstrides.resize(3); 
    NimArrBase<T>::NAstrides[0] = NimArrBase<T>::stride1 = 1;   
    NimArrBase<T>::NAstrides[1] = stride2 = size1;   
    NimArrBase<T>::NAstrides[2] = stride3 = size1 * size2;   
    if(other.boolMap) {
      NimArrBase<T>::v.resize(NimArrBase<T>::NAlength);
      typename vector<T>::iterator to = NimArrBase<T>::v.begin();
      typename vector<T>::iterator from = (*other.vPtr).begin() + other.offset;
      int otherStride1 = other.stride1;
      int otherStride2 = other.stride2;
      int otherStride3 = other.stride3;
      for(int i3rd = 0; i3rd < size3; ++i3rd) {
	for(int iCol = 0; iCol < size2; ++iCol) {
	  for(int iRow = 0; iRow < size1; ++iRow) {
	    *to = *from;
	    ++to;
	    from += otherStride1;
	  }
	  from += (-size1 * otherStride1) + otherStride2;
	}
	from += (-size2 * otherStride2) + otherStride3;
      }
    } else {
      NimArrBase<T>::v = other.v;
    }
    NimArrBase<T>::setVptr();
  }

  NimArr<3, T> () : NimArrBase<T>() {
    NimArrBase<T>::NAdims.resize(3); 
    NimArrBase<T>::NAstrides.resize(3); 
    setSize(0, 0, 0);
  }

  void setMap(NimArrBase<T> &source, int off, int str1, int str2, int str3, int is1, int is2, int is3) {
    NimArrBase<T>::boolMap = true;
    NimArrBase<T>::offset = off;
    NimArrBase<T>::vPtr = source.getVptr();
    NimArrBase<T>::NAdims[0] =  size1 = is1;
    NimArrBase<T>::NAdims[1] = size2 = is2;
    NimArrBase<T>::NAdims[2] = size3 = is3;

    NimArrBase<T>::NAlength = size1 * size2 * size3;
    NimArrBase<T>::NAstrides[0] = NimArrBase<T>::stride1 = str1;
    NimArrBase<T>::NAstrides[1] = stride2 = str2;
  //  NimArrBase<T>::NAstrides[1] = stride2 = str3;   This has to be a bug -Cliff
  	NimArrBase<T>::NAstrides[2] = stride3 = str3;
  }


  NimArr<3, T>(vector<T> &vm, int off, int str1, int str2, int str3, int is1, int is2, int is3) : NimArrBase<T>(vm, off) {
    NimArrBase<T>::NAdims.resize(3);
    NimArrBase<T>::NAdims[0] = size1 = is1;
    NimArrBase<T>::NAdims[1] = size2 = is2;
    NimArrBase<T>::NAdims[2] = size3 = is3;

    // not setSize because it uses the allocated vm
    NimArrBase<T>::NAstrides.resize(3);
    NimArrBase<T>::NAstrides[0] = NimArrBase<T>::stride1 = str1;
    NimArrBase<T>::NAstrides[1] = stride2 = str2;
    NimArrBase<T>::NAstrides[2] = stride3 = str3;

    NimArrBase<T>::NAlength = size1 * size2 * size3;
  }  

  NimArr<3, T>(int is1, int is2, int is3) : NimArrBase<T>() {
    NimArrBase<T>::NAdims.resize(3);
    NimArrBase<T>::NAstrides.resize(3);
    setSize(is1, is2, is3);
  }

  void setSize(int is1, int is2, int is3) {
    NimArrBase<T>::NAdims[0] = size1 = is1;
    NimArrBase<T>::NAdims[1] = size2 = is2;
    NimArrBase<T>::NAdims[2] = size3 = is3;
    NimArrBase<T>::NAstrides[0] = NimArrBase<T>::stride1 = 1;
    NimArrBase<T>::NAstrides[1] = stride2 = is1;    
    NimArrBase<T>::NAstrides[2] = stride3 = is1*is2;
    NimArrBase<T>::setLength(stride3 * size3);
  }

  
  virtual void setSize(vector<int> sizeVec) {setSize(sizeVec[0], sizeVec[1], sizeVec[2]);}
  virtual int numDims() const {return(3);}
  virtual int dimSize(int i) const {
    switch(i) {
    case 0:
      return(size1); break;
    case 1:
      return(size2); break;
    case 2:
      return(size3); break;
    default:
      PRINTF("Error, incorrect dimension given to dimSize\n");
      return(0);
    }
  }
};


// Here is the specialization for 4 dimensions.

template<class T>
class NimArr<4, T> : public NimArrBase<T> {
 public:
  int size1, size2, size3, size4, stride2, stride3, stride4;//s1s2;
  int calculateIndex(int i, int j, int k, int l) {return(NimArrBase<T>::offset + NimArrBase<T>::stride1 * i + stride2 * j + stride3 * k + stride4 * l );} //k * s1s2 + j*s1 + i);}
  T &operator()(int i, int j, int k, int l) {return((*NimArrBase<T>::vPtr)[calculateIndex(i, j, k, l)]);} // could add asserts here

  ~NimArr<4, T>() {};

  NimArr<4, T> &mapCopy(const NimArr<4, T> &other) {
    // std::cout<<"In NimArr<3, T> mapCopy\n";
    if(size1 != other.size1) {
      std::cout<<"Error in mapCopy.  Sizes 1 don't match: "<<size1<<" != "<<other.size1<<"\n";
    }
    if(size2 != other.size2) {
      std::cout<<"Error in mapCopy.  Sizes 2 don't match: "<<size2<<" != "<<other.size2<<"\n";
    }
    if(size3 != other.size3) {
      std::cout<<"Error in mapCopy.  Sizes 3 don't match: "<<size3<<" != "<<other.size3<<"\n";
    }
	if(size4 != other.size4){
		std::cout << "Error in mapCopy.  Sizes 4 don't match: "<<size4<<" != "<<other.size4<<"\n";
	}
    typename vector<T>::iterator to(NimArrBase<T>::vPtr->begin() + NimArrBase<T>::offset);
    typename vector<T>::iterator from = other.vPtr->begin() + other.offset;
    int otherStride1 = other.stride1;
    int otherStride2 = other.stride2;
    int otherStride3 = other.stride3;
    int otherStride4 = other.stride4;
    for(int i4th = 0; i4th < size4; ++i4th){
		for(int i3rd = 0; i3rd < size3; ++i3rd) {
	    	for(int iCol = 0; iCol < size2; ++iCol) {
				for(int iRow = 0; iRow < size1; ++iRow) {
			  		*to = *from;
	  		   		to += NimArrBase<T>::stride1;
	  		   		from += otherStride1;
				}
				from += (-size1 * otherStride1) + otherStride2;
				to += (-size1 * NimArrBase<T>::stride1) + stride2;
      		}
	    	from += (-size2 * otherStride2) + otherStride3;
      		to += (-size2 * stride2 + stride2);
    	}
    	from += (-size3 *otherStride3) + otherStride4;
    	to += (-size3 * stride3 + stride3);
    }
    // std::cout<<"Leaving NimArr<3, T> mapCopy\n";
    return(*this);
  }

  NimArr<4, T> &operator=(const NimArr<3, T> &other) {
    // std::cout<<"Using operator= for a NimArr<3, T>\n";

    if(NimArrBase<T>::isMap()) {
      return(mapCopy(other));
    }

    NimArrBase<T>::NAdims = other.dim();
    size1 = NimArrBase<T>::NAdims[0];
    size2 = NimArrBase<T>::NAdims[1];
    size3 = NimArrBase<T>::NAdims[2];
	size4 = NimArrBase<T>::NAdims[3];

    NimArrBase<T>::NAlength = other.size();
    NimArrBase<T>::myType = other.getNimType();

    NimArrBase<T>::boolMap = false;
    NimArrBase<T>::offset = 0;
   
    // vPtr, NAstrides, offset, stride1, boolMap
    //    NimArrBase<T>::NAstrides.resize(2); 
    NimArrBase<T>::NAstrides[0] = NimArrBase<T>::stride1 = 1;   
    NimArrBase<T>::NAstrides[1] = stride2 = size1;   
    NimArrBase<T>::NAstrides[2] = stride3 = size1 * size2;   
    NimArrBase<T>::NAstrides[3] = stride4 = size1 * size2 * size3;
    if(other.boolMap) {
    	NimArrBase<T>::v.resize(NimArrBase<T>::NAlength);
    	typename vector<T>::iterator to = NimArrBase<T>::v.begin();
    	typename vector<T>::iterator from = (*other.vPtr).begin() + other.offset;
      	int otherStride1 = other.stride1;
      	int otherStride2 = other.stride2;
      	int otherStride3 = other.stride3;
      	int otherStride4 = other.stride4;
      	for(int i4th = 0; i4th < size4; ++i4th){
	  		for(int i3rd = 0; i3rd < size3; ++i3rd) {
				for(int iCol = 0; iCol < size2; ++iCol) {
		  			for(int iRow = 0; iRow < size1; ++iRow) {
		    			*to = *from;
		    			++to;
		    			from += otherStride1;
					}
		  			from += (-size1 * otherStride1) + otherStride2;
				}
				from += (-size2 * otherStride2) + otherStride3;
    	  	}
    	  	from += (-size3 * otherStride3) + otherStride4;
    	}	
    } else {
      NimArrBase<T>::v = other.v;
    }
    NimArrBase<T>::setVptr();
    // std::cout<<"Leaving operator= for a NimArr<3, T>\n";
    return(*this);
  }

  NimArr<4, T> (const NimArr<4, T> &other) :
  NimArrBase<T>(other)
  {
    NimArrBase<T>::NAdims = other.dim();
    // std::cout<<"Using copy constructor for a NimArr<3, T>\n";
    size1 = NimArrBase<T>::NAdims[0];
    size2 = NimArrBase<T>::NAdims[1];
    size3 = NimArrBase<T>::NAdims[2];
	size4 = NimArrBase<T>::NAdims[3];

    NimArrBase<T>::NAstrides.resize(4); 
    NimArrBase<T>::NAstrides[0] = NimArrBase<T>::stride1 = 1;   
    NimArrBase<T>::NAstrides[1] = stride2 = size1;   
    NimArrBase<T>::NAstrides[2] = stride3 = size1 * size2;   
	NimArrBase<T>::NAstrides[3] = stride4 = size1 * size2 * size3;
    if(other.boolMap) {
      NimArrBase<T>::v.resize(NimArrBase<T>::NAlength);
      typename vector<T>::iterator to = NimArrBase<T>::v.begin();
      typename vector<T>::iterator from = (*other.vPtr).begin() + other.offset;
      int otherStride1 = other.stride1;
      int otherStride2 = other.stride2;
      int otherStride3 = other.stride3;
      int otherStride4 = other.stride4;
      for(int i4th = 0; i4th <size4; ++i4th){
	      for(int i3rd = 0; i3rd < size3; ++i3rd) {
			for(int iCol = 0; iCol < size2; ++iCol) {
				for(int iRow = 0; iRow < size1; ++iRow) {
	    			*to = *from;
	    			++to;
	    			from += otherStride1;
	  			}
	  			from += (-size1 * otherStride1) + otherStride2;
			}
			from += (-size2 * otherStride2) + otherStride3;
      	}
      	from += (-size3 * otherStride3) + otherStride4;
    }
    
  }else {
      NimArrBase<T>::v = other.v;
    }
    NimArrBase<T>::setVptr();
  }

  NimArr<4, T> () : NimArrBase<T>() {
    NimArrBase<T>::NAdims.resize(4); 
    NimArrBase<T>::NAstrides.resize(4); 
    setSize(0, 0, 0, 0);
  }

  void setMap(NimArrBase<T> &source, int off, int str1, int str2, int str3, int str4, int is1, int is2, int is3, int is4) {
    NimArrBase<T>::boolMap = true;
    NimArrBase<T>::offset = off;
    NimArrBase<T>::vPtr = source.getVptr();
    NimArrBase<T>::NAdims[0] =  size1 = is1;
    NimArrBase<T>::NAdims[1] = size2 = is2;
    NimArrBase<T>::NAdims[2] = size3 = is3;
	NimArrBase<T>::NAdims[3] = size4 = is4;

    NimArrBase<T>::NAlength = size1 * size2 * size3 * size4;
    NimArrBase<T>::NAstrides[0] = NimArrBase<T>::stride1 = str1;
    NimArrBase<T>::NAstrides[1] = stride2 = str2;
    NimArrBase<T>::NAstrides[2] = stride3 = str3;
    NimArrBase<T>::NAstrides[3] = stride4 = str4;
  }


  NimArr<4, T>(vector<T> &vm, int off, int str1, int str2, int str3, int str4, int is1, int is2, int is3, int is4) : NimArrBase<T>(vm, off) {
    NimArrBase<T>::NAdims.resize(4);
    NimArrBase<T>::NAdims[0] = size1 = is1;
    NimArrBase<T>::NAdims[1] = size2 = is2;
    NimArrBase<T>::NAdims[2] = size3 = is3;
	NimArrBase<T>::NAdims[3] = size4 = is4;
    // not setSize because it uses the allocated vm
    NimArrBase<T>::NAstrides.resize(4);
    NimArrBase<T>::NAstrides[0] = NimArrBase<T>::stride1 = str1;
    NimArrBase<T>::NAstrides[1] = stride2 = str2;
    NimArrBase<T>::NAstrides[2] = stride3 = str3;
	NimArrBase<T>::NAstrides[3] = stride4 = str4;

    NimArrBase<T>::NAlength = size1 * size2 * size3 * size4;
  }  

  NimArr<4, T>(int is1, int is2, int is3, int is4) : NimArrBase<T>() {
    NimArrBase<T>::NAdims.resize(4);
    NimArrBase<T>::NAstrides.resize(4);
    setSize(is1, is2, is3, is4);
  }

  void setSize(int is1, int is2, int is3, int is4) {
    NimArrBase<T>::NAdims[0] = size1 = is1;
    NimArrBase<T>::NAdims[1] = size2 = is2;
    NimArrBase<T>::NAdims[2] = size3 = is3;
    NimArrBase<T>::NAdims[3] = size4 = is4;
    NimArrBase<T>::NAstrides[0] = NimArrBase<T>::stride1 = 1;
    NimArrBase<T>::NAstrides[1] = stride2 = is1;    
    NimArrBase<T>::NAstrides[2] = stride3 = is1*is2;
    NimArrBase<T>::NAstrides[3] = stride4 = is1 * is2 * is3;
    NimArrBase<T>::setLength(stride4 * size4);
  }

  
  virtual void setSize(vector<int> sizeVec) {setSize(sizeVec[0], sizeVec[1], sizeVec[2], sizeVec[3]);}
  virtual int numDims() const {return(4);}
  virtual int dimSize(int i) const {
    switch(i) {
    case 0:
      return(size1); break;
    case 1:
      return(size2); break;
    case 2:
      return(size3); break;
    case 3:
      return(size4); break;
    default:
      PRINTF("Error, incorrect dimension given to dimSize\n");
      return(0);
    }
  }
};
























template<int ndim, class T>
class VecNimArr : public VecNimArrBase<T>  {
 public:
  ~VecNimArr<ndim, T>() {};
  std::vector< NimArr<ndim, T> > values;
  NimArr<ndim, T> &operator[](unsigned int i) {
    if(i >= values.size()) {
      std::cout<<"Error accessing a VecNimArr element: requested element "<<i<<" but values.size() is only "<<values.size()<<"\n";
      std::cout<<"Returning the first element if possible to avoid a segfault crash\n";
      return(values[0]);
    }
    return(values[i]);
  }
  virtual void resize(int i) {values.resize(i);}
  void resizeNoPtr(int i){values.resize(i);}
  int getsizeNoPtr(){return(values.size());}
  virtual NimArrBase<T>* getBasePtr(int i) {return(&(values[i]));}
  virtual int size() {return(values.size());}
  
  virtual void setRowDims(int row, vector<int> dims){
  	if(dims.size() != ndim){
  		PRINTF("Error: number of dimensions incorrect in resize of numericList\n");
  		return;
  		}
  	NimArrBase<T>* nimBasePtr = VecNimArr<ndim, T>::getBasePtr(row);
  	(*nimBasePtr).setSize(dims);
  	return;
  }
  
  virtual vector<int> getRowDims(int row){
  	NimArrBase<T>* nimBasePtr = VecNimArr<ndim, T>::getBasePtr(row);
  	int nRowDims = (*nimBasePtr).numDims();
  	vector<int> rowDims(nRowDims);
  	for(int i = 0; i < nRowDims; i++)
  		rowDims[i] = (*nimBasePtr).dimSize(i);
  	return(rowDims);
  }

  
  
  void setSize(int row, int d1){
  	if(ndim != 1){
  		PRINTF("Error: number of dimensions incorrect in resize of numericList\n");
  		return;
  	}
  	vector<int> dims;
  	dims.resize(1);
  	dims[0] = d1;
  	NimArrBase<T>* nimBasePtr = VecNimArr<ndim, T>::getBasePtr(row);
  	(*nimBasePtr).setSize(dims);
  	return;  	
  }

  void setSize(int row, int d1, int d2){
  	if(ndim != 2){
  		PRINTF("Error: number of dimensions incorrect in resize of numericList\n");
  		return;
  	}
  	vector<int> dims;
  	dims.resize(2);
  	dims[0] = d1;
  	dims[1] = d2;
  	NimArrBase<T>* nimBasePtr = VecNimArr<ndim, T>::getBasePtr(row);
  	(*nimBasePtr).setSize(dims);
  	return;  	
  }

  void setSize(int row, int d1, int d2, int d3){
  	if(ndim != 3){
  		PRINTF("Error: number of dimensions incorrect in resize of numericList\n");
  		return;
  	}
  	vector<int> dims;
  	dims.resize(3);
  	dims[0] = d1;
  	dims[1] = d2;
  	dims[2] = d3;
  	NimArrBase<T>* nimBasePtr = VecNimArr<ndim, T>::getBasePtr(row);
  	(*nimBasePtr).setSize(dims);
  	return;  	
  }


};
#endif
