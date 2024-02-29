#include <array>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <iostream>

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
  
  Variable< 1 > operator() ( const int& index ) const __attribute__((always_inline))
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
  
  // Variable< Size >& operator= ( const Variable< Size >& aOther ) __attribute__((always_inline))
  // {
    // for ( auto lDest( value.begin() ) , lSrc( aOther.value.begin() ) ; lDest != value.end() ; ++lDest , ++lSrc ) **lDest = **lSrc;   
    // return *this;
  // }
  
  T* operator[] ( const std::size_t& i ) __attribute__((always_inline))
  { return value.at(i); }
  
  const T* operator[] ( const std::size_t& i ) const __attribute__((always_inline))
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
  void Forward() __attribute__((always_inline)) { 
    #pragma GCC unroll 65534
    #pragma GCC ivdep 
    for( int i(0); i!=Size; ++i ) P[i]->value = not A[i]->value; 
  }    
  Variable< Size > A , P;
};
// =============================================================================================================================================================

// =============================================================================================================================================================
template < int Size >
struct And : public NonTrivial
{
  And( const Variable< Size >&  aA , const Variable< Size >& aB ) : A( aA ) , B( aB ) , P( 0 ) {}
  void Forward() __attribute__((always_inline)) { 
    #pragma GCC unroll 65534
    #pragma GCC ivdep 
    for( int i(0); i!=Size; ++i ) P[i]->value = A[i]->value and B[i]->value; 
  }    
  Variable< Size > A , B , P;
};
// =============================================================================================================================================================

// =============================================================================================================================================================
template < int Size >
struct Or : public NonTrivial
{
  Or( const Variable< Size >&  aA , const Variable< Size >& aB ) : A( aA ) , B( aB ) , P( 0 ) {}
  void Forward() __attribute__((always_inline)) { 
    #pragma GCC unroll 65534
    #pragma GCC ivdep 
    for( int i(0); i!=Size; ++i ) P[i]->value = A[i]->value or B[i]->value; 
  }    
  Variable< Size > A , B , P;
};
// =============================================================================================================================================================

// =============================================================================================================================================================
template < int Size >
struct Xor : public NonTrivial
{
  Xor( const Variable< Size >&  aA , const Variable< Size >& aB ) : A( aA ) , B( aB ) , P( 0 ) {}
  void Forward() __attribute__((always_inline)) { 
    #pragma GCC unroll 65534
    #pragma GCC ivdep 
    for( int i(0); i!=Size; ++i ) P[i]->value = A[i]->value xor B[i]->value; 
  }    
  Variable< Size > A , B , P;
};
// =============================================================================================================================================================

// =============================================================================================================================================================
template < int Size >
struct Xnor : public NonTrivial
{
  Xnor( const Variable< Size >&  aA , const Variable< Size >& aB ) : A( aA ) , B( aB ) , P( 0 ) {}
  void Forward() __attribute__((always_inline)) { 
    #pragma GCC unroll 65534
    #pragma GCC ivdep 
    for( int i(0); i!=Size; ++i ) P[i]->value = not( A[i]->value xor B[i]->value ); 
  }    
  Variable< Size > A , B , P;
};
// =============================================================================================================================================================

// =============================================================================================================================================================
template < int Size >
struct Mux : public NonTrivial {
  Mux( const Variable< 1 >& x , const Variable< Size >& a , const Variable< Size >& b ) : X(x) , A(a) , B(b) , P( 0 ) {}
  void Forward() __attribute__((always_inline)) { 
    #pragma GCC unroll 65534
    #pragma GCC ivdep 
    for( int i(0); i!=Size; ++i ) P[i]->value = X[0]->value ? A[i]->value : B[i]->value; 
  }    
  Variable< 1 > X; Variable< Size > A , B , P;  
};
// =============================================================================================================================================================

// =============================================================================================================================================================
template < int Size >
struct Fanout : public NonTrivial {
  Fanout( const Variable< 1 >& x ) : X(x) , P( 0 ) {}  
  void Forward() __attribute__((always_inline)) { 
    #pragma GCC unroll 65534
    #pragma GCC ivdep 
    for( int i(0); i!=Size; ++i ) P[i]->value = X[0]->value; 
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
  
  // void Forward() __attribute__((always_inline))
  // {   
    // AdderLo.Forward(), AdderHi.Forward(), MuxP.Forward(), MuxQ.Forward();  
  // }
    
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
  
  // void Forward() __attribute__((always_inline))
  // {
    // mFanoutA.Forward(), mFanoutB.Forward(), mXor.Forward(), mAnd.Forward(), mXnor.Forward(), mOr.Forward();    
  // }

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
  
  // void Forward() __attribute__((always_inline))
  // {   
    // MultLo.Forward(), MultHi.Forward(), Adder.Forward();
  // }
  
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


  Variable< 8 > A( 0 );
  Variable< 8 > B( 0 );
  // Add< 8 > lAdder( A , B );
  Mult< 8,8 > lMult( A , B );


  for( uint64_t i(0); i!=255; ++i )
  {
    for( uint64_t j(0); j!=255; ++j )
    {

      #pragma GCC unroll 65534
      #pragma GCC ivdep 
      for( int x(0); x!=8; ++x ){ 
        A.value[x]->value = bool( (i>>x) & 0x1 );
        B.value[x]->value = bool( (j>>x) & 0x1 );
      }      
      
      NonTrivial::Run();
      // lAdder.Forward();  
      // std::cout << i << " " << j << " " << uint64_t(lAdder.P) << std::endl;
      // if( (i+j) != int64_t(lAdder.P) ) throw std::runtime_error( "Value mismatch" );

      // lMult.Forward();  
      // // std::cout << i << " " << j << " " << uint64_t(lMult.P)  << " | " << (i*j) << "\n" << std::endl;
      if( (i*j) != int64_t(lMult.P) ) throw std::runtime_error( "Value mismatch" );
    }
  }
  


  // Variable< 1 > A( 1 );
  // Variable< 1 > B( 0 );
  
  // Add< 1 > lAdder( A , B );
  // lAdder.Forward();  

  // // std::cout << lAdder.P << std::endl;

  
  // Variable< 2 > Q( 2 );
  // for( int i(0); i!=2; ++i ) lAdder.P[i].gradient = int( Q[i].value ) - int( lAdder.P[i].value );
  // // std::cout << lAdder.P << std::endl;

  // // std::cout << A << " | " << B << std::endl;

  // // lAdder.Backward();
  
  // std::cout << "=> " << A << " | " << B << std::endl;
  
  
  // // Gradient< 5 > D = Q - lAdder.P;

  // // std::cout << "Target = " << Q << std::endl;
  // // std::cout << "Value  = " << lAdder.P << std::endl;
  // // // std::cout << "Delta  = " << D << std::endl;
  

  // // Mult< 4,4 > lMult( A , B );
  // // lMult.Forward();  
  // // std::cout << i << " * " << j << " = " << (i*j) << " | " << int64_t(lMult.P) << std::endl;
  // // if( (i*j) != int64_t(lMult.P) ) throw std::runtime_error( "Value mismatch" );
      

  
};
// =============================================================================================================================================================



