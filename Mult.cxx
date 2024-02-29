#include <array>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <functional>

using namespace std::chrono;

// =============================================================================================================================================================
// template < int Size > struct Variable;
// template < int Size > std::ostream& operator<< ( std::ostream& aStr , const Variable< Size >& A );

struct T
{
  bool value;
};


template < int Size >
struct Variable
{ 
  Variable()
  {} 

  Variable( uint64_t aValue )
  { 
    if( Size > 64 ) throw std::runtime_error( "Variable size exceeded 64-bits" );
    #pragma GCC unroll 65534
    #pragma GCC ivdep     
    for( int i(0); i!=Size; ++i ){ 
      value[i] = new T{ bool( aValue & 0x1 ) };
      aValue >>= 1;
    }      
    if( aValue ) throw std::runtime_error( "Value does not fit size" );
  }

  Variable( const Variable<Size>& ) = default;

  operator uint64_t() const 
  {
    uint64_t lRet(0);
    for( int i(0); i!=Size; ++i ) if( value[i]->value ) lRet |= ( 1<<i );
    return lRet;
  }
  
  Variable< 1 > operator() ( const int& index ) const __attribute__((always_inline,flatten))
  {
    Variable< 1 > lRet;
    lRet.value[0] = value[index];
    return lRet;
  }

  template< int lo , int hi >
  Variable< hi - lo > _slice_ () const
  {
    Variable< hi - lo > lRet;
    for( auto src( value.begin() + lo ) , dest( lRet.value.begin() ); src != value.begin() + hi; ++src, ++dest ) *dest = *src;
    return lRet;
  }
  
  Variable< Size >& operator= ( uint64_t aValue ) __attribute__((always_inline,flatten))
  {
    if( Size > 64 ) throw std::runtime_error( "Variable size exceeded 64-bits" );
    #pragma GCC unroll 65534
    #pragma GCC ivdep     
    for( int i(0); i!=Size; ++i ){ 
      value[i]->value = bool( aValue & 0x1 );
      aValue >>= 1;
    }      
    if( aValue ) throw std::runtime_error( "Value does not fit size" );    
    return *this;
  }
  
  T* operator[] ( const std::size_t& i ) __attribute__((always_inline,flatten))
  { return value.at(i); }
  
  const T* operator[] ( const std::size_t& i ) const __attribute__((always_inline,flatten))
  { return value.at(i); }
  
  std::array< T* , Size > value;
};

#define slice(lo,hi) template _slice_<lo,hi>()

template < int Size >
std::ostream& operator<< ( std::ostream& aStr , const Variable< Size >& A )
{
  aStr << "0b";
  for( int i(Size-1); i>=0; --i ) std::cout << A[i]->value; 
  return aStr;
}

template < int SizeA , int SizeB >
Variable< SizeA + SizeB > operator& ( const Variable< SizeA >& A , const Variable< SizeB >& B )
{
  Variable< SizeA + SizeB > lRet( 0 );
  auto dest( lRet.value.begin() );
  for( auto src( A.value.begin() ); src != A.value.end(); ++src, ++dest ) *dest = *src;
  for( auto src( B.value.begin() ); src != B.value.end(); ++src, ++dest ) *dest = *src;
  return lRet;
};


// =============================================================================================================================================================


struct NonTrivial
{
  NonTrivial()
  {
    NonTrivial::instances.push_back( this );
  }
  
  virtual void Forward() = 0;
  
  static void Run()
  {
    for( auto& i : NonTrivial::instances ) (*i).Forward();
  }
  
  static std::vector< NonTrivial* > instances;
};


std::vector< NonTrivial* > NonTrivial::instances;

// =============================================================================================================================================================
template < int Size >
struct Not : public NonTrivial
{
  Not( const Variable< Size >&  aA ) : A( aA ) , P( 0 ) {}
  void Forward() __attribute__((always_inline,flatten)) { 
    #pragma GCC unroll 65534
    #pragma GCC ivdep 
    for( auto p(P.value.begin()) , a(A.value.begin()) ; p!=P.value.end() ; ++p , ++a ) (**p).value = not (**a).value; 
  }    
  Variable< Size > A , P;
};
// =============================================================================================================================================================

// =============================================================================================================================================================
template < int Size >
struct And : public NonTrivial
{
  And( const Variable< Size >&  aA , const Variable< Size >& aB ) : A( aA ) , B( aB ) , P( 0 ) {}
  void Forward() __attribute__((always_inline,flatten)) { 
    #pragma GCC unroll 65534
    #pragma GCC ivdep 
    for( auto p(P.value.begin()) , a(A.value.begin()) , b(B.value.begin()) ; p!=P.value.end() ; ++p , ++a , ++b ) (**p).value = (**a).value and (**b).value; 
  }    
  Variable< Size > A , B , P;
};
// =============================================================================================================================================================

// =============================================================================================================================================================
template < int Size >
struct Or : public NonTrivial
{
  Or( const Variable< Size >&  aA , const Variable< Size >& aB ) : A( aA ) , B( aB ) , P( 0 ) {}
  void Forward() __attribute__((always_inline,flatten)) { 
    #pragma GCC unroll 65534
    #pragma GCC ivdep 
    for( auto p(P.value.begin()) , a(A.value.begin()) , b(B.value.begin()) ; p!=P.value.end() ; ++p , ++a , ++b ) (**p).value = (**a).value or (**b).value; 
  }    
  Variable< Size > A , B , P;
};
// =============================================================================================================================================================

// =============================================================================================================================================================
template < int Size >
struct Xor : public NonTrivial
{
  Xor( const Variable< Size >&  aA , const Variable< Size >& aB ) : A( aA ) , B( aB ) , P( 0 ) {}
  void Forward() __attribute__((always_inline,flatten)) { 
    #pragma GCC unroll 65534
    #pragma GCC ivdep 
    for( auto p(P.value.begin()) , a(A.value.begin()) , b(B.value.begin()) ; p!=P.value.end() ; ++p , ++a , ++b ) (**p).value = (**a).value xor (**b).value; 
  }    
  Variable< Size > A , B , P;
};
// =============================================================================================================================================================

// =============================================================================================================================================================
template < int Size >
struct Xnor : public NonTrivial
{
  Xnor( const Variable< Size >&  aA , const Variable< Size >& aB ) : A( aA ) , B( aB ) , P( 0 ) {}
  void Forward() __attribute__((always_inline,flatten)) { 
    #pragma GCC unroll 65534
    #pragma GCC ivdep 
    for( auto p(P.value.begin()) , a(A.value.begin()) , b(B.value.begin()) ; p!=P.value.end() ; ++p , ++a , ++b ) (**p).value = not( (**a).value xor (**b).value ); 
  }    
  Variable< Size > A , B , P;
};
// =============================================================================================================================================================

// =============================================================================================================================================================
template < int Size >
struct Mux : public NonTrivial {
  Mux( const Variable< 1 >& x , const Variable< Size >& a , const Variable< Size >& b ) : X(x) , A(a) , B(b) , P( 0 ) {}
  void Forward() __attribute__((always_inline,flatten)) { 
    const bool lSwitch( X[0]->value );
    #pragma GCC unroll 65534
    #pragma GCC ivdep 
    for( auto p(P.value.begin()) , a(A.value.begin()) , b(B.value.begin()) ; p!=P.value.end() ; ++p , ++a , ++b ) (**p).value = lSwitch ? (**a).value : (**b).value; 
  }    
  Variable< 1 > X; Variable< Size > A , B , P;  
};
// =============================================================================================================================================================

// =============================================================================================================================================================
template < int Size >
struct Fanout : public NonTrivial {
  Fanout( const Variable< 1 >& x ) : X(x) , P( 0 ) {}  
  void Forward() __attribute__((always_inline,flatten)) { 
    const bool lValue( X[0]->value );
    #pragma GCC unroll 65534
    #pragma GCC ivdep 
    for( auto p(P.value.begin()) ; p!=P.value.end() ; ++p ) (**p).value = lValue; 
  }    
  Variable< 1 > X; Variable< Size > P;  
};
// =============================================================================================================================================================







// =============================================================================================================================================================
template < int Size >
struct Add
{
  static constexpr int SizeHi = Size/2;
  static constexpr int SizeLo = Size - SizeHi;
  static constexpr int OutputSize = Size + 1;

  Add( const Variable< Size >&  A , const Variable< Size >& B ) : 
    AdderLo( A.slice( 0 , SizeLo ) , B.slice( 0 , SizeLo ) ), AdderHi( A.slice( SizeLo , Size ) , B.slice( SizeLo , Size ) ),
    MuxP( AdderLo.P(SizeLo) , AdderHi.Q , AdderHi.P ), MuxQ( AdderLo.Q(SizeLo) , AdderHi.Q , AdderHi.P ),
    P( AdderLo.P.slice( 0 , SizeLo ) & MuxP.P ), Q( AdderLo.Q.slice( 0 , SizeLo ) & MuxQ.P )
  {}
  
  void Forward() __attribute__((always_inline,flatten))
  {   
    AdderLo.Forward(), AdderHi.Forward(), MuxP.Forward(), MuxQ.Forward();  
  }
    
  Add< SizeLo > AdderLo;
  Add< SizeHi > AdderHi;
  Mux< SizeHi+1 > MuxP , MuxQ;
  Variable< OutputSize > P , Q;
};


template <>
struct Add< 1 >
{ 
  static constexpr int OutputSize = 2;

  Add( const Variable< 1 >&  aA , const Variable< 1 >& aB ) : 
    mFanoutA( aA ) , mFanoutB( aB ), 
    mXor( mFanoutA.P(0) , mFanoutB.P(0) ), mAnd( mFanoutA.P(1) , mFanoutB.P(1) ), mXnor( mFanoutA.P(2) , mFanoutB.P(2) ), mOr( mFanoutA.P(3) , mFanoutB.P(3) ),
    P( mXor.P & mAnd.P ) , Q( mXnor.P & mOr.P )
  {}
  
  void Forward() __attribute__((always_inline,flatten))
  {
    mFanoutA.Forward(), mFanoutB.Forward(), mXor.Forward(), mAnd.Forward(), mXnor.Forward(), mOr.Forward();    
  }

  Fanout<4> mFanoutA , mFanoutB;
  Xor<1> mXor;
  And<1> mAnd;
  Xnor<1> mXnor;
  Or<1> mOr;
  Variable< OutputSize > P , Q; 
};
// =============================================================================================================================================================



// =============================================================================================================================================================
template < int SizeA , int SizeB >
struct Mult
{
  static constexpr int SizeAHi = SizeA/2;
  static constexpr int SizeALo = SizeA - SizeAHi;
  static constexpr int SumSize = SizeB + SizeAHi;
  static constexpr int OutputSize = SizeA + SizeB;

  Mult( const Variable< SizeA >& A , const Variable< SizeB >& B ) :
    MultLo( B , A.slice( 0 , SizeALo ) ) , MultHi( B , A.slice( SizeALo , SizeA ) ) ,
    Adder( MultLo.P.slice(SizeALo,SizeALo+SizeB) & Variable<SizeAHi>(0) , MultHi.P ),
    P( MultLo.P.slice(0,SizeALo) & Adder.P.slice(0,SumSize) )
  {}    
  
  void Forward() __attribute__((always_inline,flatten))
  {   
    MultLo.Forward(), MultHi.Forward(), Adder.Forward();
  }
  
  Mult< SizeB , SizeALo > MultLo;
  Mult< SizeB , SizeAHi > MultHi;  
  Add< SumSize > Adder;  
  Variable< OutputSize > P;
};


template <>
struct Mult< 1 , 1 > : public And<1>
{ 
  static constexpr int OutputSize = 2;

  Mult( const Variable< 1 >&  A , const Variable< 1 >& B ) : And( A , B ) , P( And::P & Variable<1>(0) )
  {}

  Variable< OutputSize > P;
};
// =============================================================================================================================================================


// =============================================================================================================================================================
int main()
{ 
  Variable< 8 > A( 0 ) , B( 0 );
  Mult< 8,8 > lMult( A , B );

  { auto start = high_resolution_clock::now(); \
    for( uint64_t x(0); x!=20; ++x ) for( uint64_t i(0); i!=255; ++i ) for( uint64_t j(0); j!=255; ++j ) {
      A = i; B = j;
      NonTrivial::Run();
    }
    std::cout << "Using global Run     : " << duration_cast<microseconds>( high_resolution_clock::now() - start).count()/1000000.0 << "s" << std::endl;
  }

  // { auto start = high_resolution_clock::now(); \
    // for( uint64_t x(0); x!=20; ++x ) for( uint64_t i(0); i!=255; ++i ) for( uint64_t j(0); j!=255; ++j ) {
      // A = i; B = j;
      // lMult.Forward();
    // }
    // std::cout << "Using recursive calls: " << duration_cast<microseconds>( high_resolution_clock::now() - start).count()/1000000.0 << "s" << std::endl;
  // }

};
// =============================================================================================================================================================



