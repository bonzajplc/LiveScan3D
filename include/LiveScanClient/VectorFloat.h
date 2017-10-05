#pragma once
/**************************************************************************************************
* Pico Engine by Plastic Demoscene Group
* File name:				VectorFloat.h
* Key people:				uho (author)
**************************************************************************************************/
//#include "../picoDef.h"
//#include "../picoBits.h"
#include "TypesAndUtils.h"
#include <cmath>

#define PI 3.141592653589793f

#if _MSC_VER
#define PICO_VECF_INLINE __forceinline
#else
#define PICO_VECF_INLINE inline
#endif
#define PICO_VECF_NORMALIZATION_EPSILON	f32(1e-20f)	

//////////////////////////////////////////////////////////////////////////
// --- Vec2F
struct Vec2F
{
    f32 x, y;

    PICO_VECF_INLINE Vec2F() {}
    PICO_VECF_INLINE explicit Vec2F( f32 v ) : x( v ), y( v ) {}
    PICO_VECF_INLINE Vec2F( f32 vx, f32 vy)  : x( vx ), y( vy ) {}
    PICO_VECF_INLINE Vec2F( const Vec2F& v ) : x( v.x ), y( v.y ) {}
    PICO_VECF_INLINE Vec2F( const f32 v[2] ) : x( v[0] ), y( v[1] ) {}

    PICO_VECF_INLINE f32& operator[]( int index )
    {
        PICO_ASSERT( index >= 0 && index <= 2 );
        return reinterpret_cast<f32*>( this )[index];
    }
    PICO_VECF_INLINE const f32& operator[]( int index ) const
    {
        PICO_ASSERT( index >= 0 && index <= 2 );
        return reinterpret_cast<const f32*>( this )[index];
    }
    PICO_VECF_INLINE Vec2F&  operator = ( const Vec2F& p )			{ x = p.x; y = p.y; return *this; }
    PICO_VECF_INLINE Vec2F   operator - ()                 const    { return Vec2F( -x, -y ); }
    PICO_VECF_INLINE Vec2F   operator + ( const Vec2F& v ) const	{ return Vec2F( x + v.x, y + v.y ); }
    PICO_VECF_INLINE Vec2F   operator - ( const Vec2F& v ) const	{ return Vec2F( x - v.x, y - v.y ); }
    PICO_VECF_INLINE Vec2F   operator * ( f32 f )          const	{ return Vec2F( x * f, y * f ); }
    PICO_VECF_INLINE Vec2F   operator / ( f32 f )          const    { f = 1.0f / f; return Vec2F( x * f, y * f ); }

    PICO_VECF_INLINE Vec2F& operator +=( const Vec2F& v )
    {
        x += v.x;
        y += v.y;
        return *this;
    }

    PICO_VECF_INLINE Vec2F& operator -=( const Vec2F& v )
    {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    PICO_VECF_INLINE Vec2F& operator *=( f32 f )
    {
        x *= f;
        y *= f;
        return *this;
    }

    PICO_VECF_INLINE Vec2F& operator /=( f32 f )
    {
        f = 1.0f / f;
        x *= f;
        y *= f;
        return *this;
    }
};

//////////////////////////////////////////////////////////////////////////
// --- Vec3F
struct Vec3F
{
    f32 x, y, z;

    PICO_VECF_INLINE Vec3F() {}
    PICO_VECF_INLINE explicit Vec3F( f32 v ) : x( v ), y( v ), z( v ) {}
    PICO_VECF_INLINE Vec3F( f32 vx, f32 vy, f32 vz ) : x( vx ), y( vy ), z( vz ) {}
    PICO_VECF_INLINE Vec3F( const Vec3F& v ) : x( v.x ), y( v.y ), z( v.z ) {}
    PICO_VECF_INLINE Vec3F( const f32 v[3] ) : x( v[0] ), y( v[1] ), z( v[2] ) {}

    PICO_VECF_INLINE f32& operator[]( int index )
    {
        PICO_ASSERT( index >= 0 && index <= 2 );
        return reinterpret_cast<f32*>( this )[index];
    }
    PICO_VECF_INLINE const f32& operator[]( int index ) const
    {
        PICO_ASSERT( index >= 0 && index <= 2 );
        return reinterpret_cast<const f32*>( this )[index];
    }
    PICO_VECF_INLINE Vec3F&  operator = ( const Vec3F& p )			{ x = p.x; y = p.y; z = p.z;	return *this; }
    PICO_VECF_INLINE Vec3F   operator - ()                 const     { return Vec3F( -x, -y, -z ); }
    PICO_VECF_INLINE Vec3F   operator + ( const Vec3F& v ) const		{ return Vec3F( x + v.x, y + v.y, z + v.z ); }
    PICO_VECF_INLINE Vec3F   operator - ( const Vec3F& v ) const		{ return Vec3F( x - v.x, y - v.y, z - v.z ); }
    PICO_VECF_INLINE Vec3F   operator *( f32 f )           const		{ return Vec3F( x * f, y * f, z * f ); }
    PICO_VECF_INLINE Vec3F   operator /( f32 f )           const     { f = 1.0f / f; return Vec3F( x * f, y * f, z * f ); }

    PICO_VECF_INLINE Vec3F& operator +=( const Vec3F& v )
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    PICO_VECF_INLINE Vec3F& operator -=( const Vec3F& v )
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    PICO_VECF_INLINE Vec3F& operator *=( f32 f )
    {
        x *= f;
        y *= f;
        z *= f;
        return *this;
    }
    
    PICO_VECF_INLINE Vec3F& operator /=( f32 f )
    {
        f = 1.0f / f;
        x *= f;
        y *= f;
        z *= f;
        return *this;
    }

    static PICO_VECF_INLINE Vec3F xAxis() { return Vec3F( 1.f, 0.f, 0.f ); }
    static PICO_VECF_INLINE Vec3F yAxis() { return Vec3F( 0.f, 1.f, 0.f ); }
    static PICO_VECF_INLINE Vec3F zAxis() { return Vec3F( 0.f, 0.f, 1.f ); }

};

//////////////////////////////////////////////////////////////////////////
// --- Vec4F
struct Vec4F
{
    f32 x, y, z, w;

    PICO_VECF_INLINE Vec4F() {}
    PICO_VECF_INLINE explicit Vec4F( f32 v ) : x( v ), y( v ), z( v ), w( v ) {}
    PICO_VECF_INLINE Vec4F( f32 vx, f32 vy, f32 vz, f32 vw ) : x( vx ), y( vy ), z( vz ), w( vw ) {}
    PICO_VECF_INLINE Vec4F( const Vec4F& v ) : x( v.x ), y( v.y ), z( v.z ), w( v.w ) {}
    PICO_VECF_INLINE Vec4F( const Vec3F& xyz, f32 vw ) : x( xyz.x ), y( xyz.y ), z( xyz.z ), w( vw ) {}

    PICO_VECF_INLINE f32& operator[]( int index )
    {
        PICO_ASSERT( index >= 0 && index <= 3 );
        return reinterpret_cast<f32*>( this )[index];
    }
    PICO_VECF_INLINE const f32& operator[]( int index ) const
    {
        PICO_ASSERT( index >= 0 && index <= 3 );
        return reinterpret_cast<const f32*>( this )[index];
    }
    PICO_VECF_INLINE Vec4F&  operator = ( const Vec4F& p )			{ x = p.x; y = p.y; z = p.z; w = p.w; return *this; }
    PICO_VECF_INLINE Vec4F   operator - ()                 const     { return Vec4F( -x, -y, -z, -w ); }
    PICO_VECF_INLINE Vec4F   operator + ( const Vec4F& v ) const		{ return Vec4F( x + v.x, y + v.y, z + v.z, w + v.w ); }
    PICO_VECF_INLINE Vec4F   operator - ( const Vec4F& v ) const		{ return Vec4F( x - v.x, y - v.y, z - v.z, w - v.w ); }
    PICO_VECF_INLINE Vec4F   operator *( f32 f )           const		{ return Vec4F( x * f, y * f, z * f, w * f ); }
    PICO_VECF_INLINE Vec4F   operator /( f32 f )           const     { f = 1.0f / f; return Vec4F( x * f, y * f, z * f, w * f ); }

    PICO_VECF_INLINE Vec4F& operator +=( const Vec4F& v )
    {
        x += v.x;
        y += v.y;
        z += v.z;
        w += v.w;
        return *this;
    }

    PICO_VECF_INLINE Vec4F& operator -=( const Vec4F& v )
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        w -= v.w;
        return *this;
    }

    PICO_VECF_INLINE Vec4F& operator *=( f32 f )
    {
        x *= f;
        y *= f;
        z *= f;
        w *= f;
        return *this;
    }

    PICO_VECF_INLINE Vec4F& operator /=( f32 f )
    {
        f = 1.0f / f;
        x *= f;
        y *= f;
        z *= f;
        w *= f;
        return *this;
    }

    const Vec3F getXYZ() const { return Vec3F( x, y, z ); }
          Vec3F getXYZ()       { return Vec3F( x, y, z ); }

    void setXYZ( const Vec3F& v )
    {
        x = v.x;
        y = v.y;
        z = v.z;
    }

    static PICO_VECF_INLINE Vec4F xAxis() { return Vec4F( 1.f, 0.f, 0.f, 0.f ); }
    static PICO_VECF_INLINE Vec4F yAxis() { return Vec4F( 0.f, 1.f, 0.f, 0.f ); }
    static PICO_VECF_INLINE Vec4F zAxis() { return Vec4F( 0.f, 0.f, 1.f, 0.f ); }

};

//////////////////////////////////////////////////////////////////////////
// --- Vec3S
struct Vec3S
{
	u16 x, y, z;

	PICO_VECF_INLINE Vec3S() {}
	PICO_VECF_INLINE explicit Vec3S( u16 v ): x( v ), y( v ), z( v ) {}
	PICO_VECF_INLINE Vec3S( u16 vx, u16 vy, u16 vz ) : x( vx ), y( vy ), z( vz ) {}
	PICO_VECF_INLINE Vec3S( const Vec3S& v ) : x( v.x ), y( v.y ), z( v.z ) {}
	PICO_VECF_INLINE Vec3S( const u16 v[3] ) : x( v[0] ), y( v[1] ), z( v[2] ) {}

	PICO_VECF_INLINE u16& operator[]( int index )
	{
		PICO_ASSERT( index >= 0 && index <= 2 );
		return reinterpret_cast<u16*>( this )[index];
	}
	PICO_VECF_INLINE const u16& operator[]( int index ) const
	{
		PICO_ASSERT( index >= 0 && index <= 2 );
		return reinterpret_cast<const u16*>( this )[index];
	}
	PICO_VECF_INLINE Vec3S&  operator = ( const Vec3S& p ) { x = p.x; y = p.y; z = p.z;	return *this; }
	PICO_VECF_INLINE Vec3S   operator - ()                 const { return Vec3S( -x, -y, -z ); }
	PICO_VECF_INLINE Vec3S   operator + ( const Vec3S& v ) const { return Vec3S( x + v.x, y + v.y, z + v.z ); }
	PICO_VECF_INLINE Vec3S   operator - ( const Vec3S& v ) const { return Vec3S( x - v.x, y - v.y, z - v.z ); }
	PICO_VECF_INLINE Vec3S   operator *( u16 f )           const { return Vec3S( x * f, y * f, z * f ); }
	PICO_VECF_INLINE Vec3S   operator /( u16 f )           const { return Vec3S( x / f, y / f, z / f ); }
	PICO_VECF_INLINE Vec3S   operator %( u16 f )           const { return Vec3S( x % f, y % f, z % f ); }

	PICO_VECF_INLINE Vec3S& operator +=( const Vec3S& v )
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	PICO_VECF_INLINE Vec3S& operator -=( const Vec3S& v )
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	PICO_VECF_INLINE Vec3S& operator *=( u16 f )
	{
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}

	PICO_VECF_INLINE Vec3S& operator /=( u16 f )
	{
		x /= f;
		y /= f;
		z /= f;
		return *this;
	}

	PICO_VECF_INLINE Vec3S& operator %=( u16 f )
	{
		x %= f;
		y %= f;
		z %= f;
		return *this;
	}
};

//////////////////////////////////////////////////////////////////////////
// --- QuatF

struct Mat33F;
struct QuatF
{
    f32 x, y, z, w;
    
    PICO_VECF_INLINE QuatF()	{}
    PICO_VECF_INLINE explicit QuatF( f32 r ) : x( 0.0f ), y( 0.0f ), z( 0.0f ), w( r ) {}
    PICO_VECF_INLINE explicit QuatF( f32 nx, f32 ny, f32 nz, f32 nw ) : x( nx ), y( ny ), z( nz ), w( nw ) {}
    
    PICO_VECF_INLINE QuatF( const QuatF& v ) : x( v.x ), y( v.y ), z( v.z ), w( v.w ) {}
    PICO_VECF_INLINE explicit QuatF( const Mat33F& m );
    PICO_VECF_INLINE QuatF&	operator=( const QuatF& p )			{ x = p.x; y = p.y; z = p.z; w = p.w;	return *this; }

    PICO_VECF_INLINE QuatF& operator*= ( const QuatF& q )
    {
        const f32 tx = w*q.x + q.w*x + y*q.z - q.y*z;
        const f32 ty = w*q.y + q.w*y + z*q.x - q.z*x;
        const f32 tz = w*q.z + q.w*z + x*q.y - q.x*y;

        w = w*q.w - q.x*x - y*q.y - q.z*z;
        x = tx;
        y = ty;
        z = tz;

        return *this;
    }

    PICO_VECF_INLINE QuatF& operator+= ( const QuatF& q )
    {
        x += q.x;
        y += q.y;
        z += q.z;
        w += q.w;
        return *this;
    }

    PICO_VECF_INLINE QuatF& operator-= ( const QuatF& q )
    {
        x -= q.x;
        y -= q.y;
        z -= q.z;
        w -= q.w;
        return *this;
    }

    PICO_VECF_INLINE QuatF& operator*= ( const f32 s )
    {
        x *= s;
        y *= s;
        z *= s;
        w *= s;
        return *this;
    }

    PICO_VECF_INLINE QuatF operator*( const QuatF& q ) const
    {
        return QuatF( w*q.x + q.w*x + y*q.z - q.y*z,
                      w*q.y + q.w*y + z*q.x - q.z*x,
                      w*q.z + q.w*z + x*q.y - q.x*y,
                      w*q.w - x*q.x - y*q.y - z*q.z );
    }

    PICO_VECF_INLINE QuatF operator+( const QuatF& q ) const
    {
        return QuatF( x + q.x, y + q.y, z + q.z, w + q.w );
    }

    PICO_VECF_INLINE QuatF operator-() const
    {
        return QuatF( -x, -y, -z, -w );
    }


    PICO_VECF_INLINE QuatF operator-( const QuatF& q ) const
    {
        return QuatF( x - q.x, y - q.y, z - q.z, w - q.w );
    }

    PICO_VECF_INLINE QuatF operator*( f32 r ) const
    {
        return QuatF( x*r, y*r, z*r, w*r );
    }

    static PICO_VECF_INLINE QuatF identity() { return QuatF( 0.f, 0.f, 0.f, 1.f ); }
    static PICO_VECF_INLINE QuatF rotation( f32 angleRadians, const Vec3F& unitAxis );
};

//////////////////////////////////////////////////////////////////////////
// --- Mat33F

struct Mat33F
{
    Vec3F column0, column1, column2;

    PICO_VECF_INLINE Mat33F() {}

    PICO_VECF_INLINE Mat33F( const Vec3F& col0, const Vec3F& col1, const Vec3F& col2 )
        : column0( col0 ), column1( col1 ), column2( col2 )
    {}


    PICO_VECF_INLINE explicit Mat33F( f32 r )
        : column0( r, 0.0f, 0.0f ), column1( 0.0f, r, 0.0f ), column2( 0.0f, 0.0f, r )
    {}

    PICO_VECF_INLINE explicit Mat33F( f32 values[] ) :
        column0( values[0], values[1], values[2] ),
        column1( values[3], values[4], values[5] ),
        column2( values[6], values[7], values[8] )
    {
    }

    PICO_VECF_INLINE explicit Mat33F( const QuatF& q )
    {
        const f32 x = q.x;
        const f32 y = q.y;
        const f32 z = q.z;
        const f32 w = q.w;

        const f32 x2 = x + x;
        const f32 y2 = y + y;
        const f32 z2 = z + z;

        const f32 xx = x2*x;
        const f32 yy = y2*y;
        const f32 zz = z2*z;

        const f32 xy = x2*y;
        const f32 xz = x2*z;
        const f32 xw = x2*w;

        const f32 yz = y2*z;
        const f32 yw = y2*w;
        const f32 zw = z2*w;

        column0 = Vec3F( 1.0f - yy - zz, xy + zw, xz - yw );
        column1 = Vec3F( xy - zw, 1.0f - xx - zz, yz + xw );
        column2 = Vec3F( xz + yw, yz - xw, 1.0f - xx - yy );
    }

    PICO_VECF_INLINE Mat33F( const Mat33F& other )
        : column0( other.column0 ), column1( other.column1 ), column2( other.column2 )
    {}

    PICO_VECF_INLINE Mat33F& operator=( const Mat33F& other )
    {
        column0 = other.column0;
        column1 = other.column1;
        column2 = other.column2;
        return *this;
    }

    PICO_VECF_INLINE static Mat33F identity()
    {
        return Mat33F( 1.f );
    }

    PICO_VECF_INLINE Mat33F operator-() const
    {
        return Mat33F( -column0, -column1, -column2 );
    }

    PICO_VECF_INLINE Mat33F operator+( const Mat33F& other ) const
    {
        return Mat33F( column0 + other.column0,
            column1 + other.column1,
            column2 + other.column2 );
    }

    PICO_VECF_INLINE Mat33F operator-( const Mat33F& other ) const
    {
        return Mat33F( column0 - other.column0,
            column1 - other.column1,
            column2 - other.column2 );
    }

    PICO_VECF_INLINE Mat33F operator*( f32 scalar ) const
    {
        return Mat33F( column0*scalar, column1*scalar, column2*scalar );
    }

    PICO_VECF_INLINE Vec3F operator*( const Vec3F& vec ) const
    {
        return column0*vec.x + column1*vec.y + column2*vec.z;
    }

    PICO_VECF_INLINE Mat33F operator*( const Mat33F& other ) const
    {
        //Rows from this <dot> columns from other
        //column0 = transform(other.column0) etc
        return Mat33F( (*this) * other.column0,
                       (*this) * other.column1,
                       (*this) * other.column2 );
    }

    PICO_VECF_INLINE Mat33F& operator+=( const Mat33F& other )
    {
        column0 += other.column0;
        column1 += other.column1;
        column2 += other.column2;
        return *this;
    }

    PICO_VECF_INLINE Mat33F& operator-=( const Mat33F& other )
    {
        column0 -= other.column0;
        column1 -= other.column1;
        column2 -= other.column2;
        return *this;
    }

    PICO_VECF_INLINE Mat33F& operator*=( f32 scalar )
    {
        column0 *= scalar;
        column1 *= scalar;
        column2 *= scalar;
        return *this;
    }

    PICO_VECF_INLINE Mat33F& operator*=( const Mat33F &other )
    {
        *this = *this * other;
        return *this;
    }

    PICO_VECF_INLINE       Vec3F& operator[]( unsigned int num )       { return ( &column0 )[num]; }
    PICO_VECF_INLINE const Vec3F& operator[]( unsigned int num ) const { return ( &column0 )[num]; }

    
    PICO_VECF_INLINE f32 operator()( unsigned int row, unsigned int col ) const
    {
        return ( *this )[col][row];
    }

    PICO_VECF_INLINE f32& operator()( unsigned int row, unsigned int col )
    {
        return ( *this )[col][row];
    }
};

//////////////////////////////////////////////////////////////////////////
// --- Vec2F functions
PICO_VECF_INLINE f32 lengthSqr( const Vec2F& v )
{
	return v.x * v.x + v.y * v.y;
}
PICO_VECF_INLINE f32 length( const Vec2F& v )
{
	return ::sqrt( lengthSqr( v ) );
}
PICO_VECF_INLINE Vec2F normalize( const Vec2F& v )
{
	return v / length( v );
}
PICO_VECF_INLINE Vec2F lerp( float t, const Vec2F& a, const Vec2F& b )
{
    return a + ( b - a )*t;
}
PICO_VECF_INLINE Vec2F mulPerElem( const Vec2F& a, const Vec2F& b )
{
	return Vec2F( a.x * b.x, a.y * b.y );
}
PICO_VECF_INLINE Vec2F divPerElem( const Vec2F& a, const Vec2F& b )
{
	return Vec2F( a.x / b.x, a.y / b.y );
}

//////////////////////////////////////////////////////////////////////////
// --- Vec3F functions
PICO_VECF_INLINE const Vec3F operator *( float scalar, const Vec3F &vec )
{
	return vec * scalar;
}

PICO_VECF_INLINE f32 lengthSqr( const Vec3F& v )
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}
PICO_VECF_INLINE f32 length( const Vec3F& v )
{
    return ::sqrt( lengthSqr( v ) );
}
/**
\brief returns the scalar product of this and other.
*/
PICO_VECF_INLINE f32 dot( const Vec3F& a, const Vec3F& b )
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

/**
\brief cross product
*/
PICO_VECF_INLINE Vec3F cross( const Vec3F& a, const Vec3F& b )
{
    return Vec3F( a.y * b.z - a.z * b.y,
                  a.z * b.x - a.x * b.z,
                  a.x * b.y - a.y * b.x );
}

PICO_VECF_INLINE Vec3F mulPerElem( const Vec3F& a, const Vec3F& b )
{
    return Vec3F( a.x * b.x, a.y * b.y, a.z*b.z );
}
PICO_VECF_INLINE Vec3F divPerElem( const Vec3F& a, const Vec3F& b )
{
    return Vec3F( a.x / b.x, a.y / b.y, a.z / b.z );
}

PICO_VECF_INLINE Vec3F normalize( const Vec3F& v )
{
    const f32 m = length(v);
    PICO_ASSERT( m >= PICO_VECF_NORMALIZATION_EPSILON );
    return v / m;
}

PICO_VECF_INLINE Vec3F normalizeSafe( const Vec3F& v )
{
    f32 mag = lengthSqr(v);
    if( mag < PICO_VECF_NORMALIZATION_EPSILON )
        return Vec3F( 0.0f );
    
    mag = ::sqrt( mag );
    return v / mag;
}

PICO_VECF_INLINE Vec3F minPerElem( const Vec3F& a, const Vec3F& b )
{
    return Vec3F( minOfPair( a.x, b.x ), minOfPair( a.y, b.y ), minOfPair( a.z, b.z ) );
}

PICO_VECF_INLINE f32 minElem( const Vec3F& v )
{
    return minOfPair( v.x, minOfPair( v.y, v.z ) );
}

PICO_VECF_INLINE Vec3F maxPerElem( const Vec3F& a, const Vec3F& b )
{
    return Vec3F( maxOfPair( a.x, b.x ), maxOfPair( a.y, b.y ), maxOfPair( a.z, b.z ) );
}
PICO_VECF_INLINE Vec3F clampPerElem( const Vec3F& v, const Vec3F& minValue, const Vec3F& maxValue )
{
	return maxPerElem( minValue, minPerElem( v, maxValue ) );
}
PICO_VECF_INLINE f32 maxElem( const Vec3F& v )
{
    return maxOfPair( v.x, maxOfPair( v.y, v.z ) );
}

PICO_VECF_INLINE Vec3F absPerElem( const Vec3F& v )
{
    return Vec3F( ::fabs( v.x ), ::fabs( v.y ), ::fabs( v.z ) );
}
PICO_VECF_INLINE Vec3F lerp( f32 t, const Vec3F& a, const Vec3F& b )
{
    return a + ( b - a ) * t;
}
PICO_VECF_INLINE Vec3F fractPerElem( const Vec3F& v )
{
	float intPart;
	return Vec3F(
		modf( v.x, &intPart ),
		modf( v.y, &intPart ),
		modf( v.z, &intPart )
	);
}
//////////////////////////////////////////////////////////////////////////
// --- Vec4F functions
PICO_VECF_INLINE f32 dot( const Vec4F& a, const Vec4F& b )
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w*b.w;
}

PICO_VECF_INLINE Vec4F mulPerElem( const Vec4F& a, const Vec4F& b )
{
    return Vec4F( a.x * b.x, a.y * b.y, a.z*b.z, a.w * b.w );
}
PICO_VECF_INLINE Vec4F makePlane( const Vec3F& normal, const Vec3F& pointOnPlane )
{
    return Vec4F( normal, -dot( normal, pointOnPlane ) );
}
PICO_VECF_INLINE Vec3F projectVectorOnPlane( const Vec3F& vec, const Vec4F& plane )
{
    const Vec3F& n = plane.getXYZ();
    const Vec3F& V = vec;
    const Vec3F W = V - (n * dot( V, n ));
    return W;
}
PICO_VECF_INLINE Vec3F projectPointOnPlane( const Vec3F& point, const Vec4F& plane )
{
    const Vec3F& n = plane.getXYZ();
    const Vec3F& Q = point;

    const Vec3F Qp = Q - n * ( dot( Q, n ) + plane.w );
    return Qp;
}

//////////////////////////////////////////////////////////////////////////
// --- Quat functions
PICO_VECF_INLINE QuatF::QuatF( const Mat33F& m )
{
    f32 tr = m( 0, 0 ) + m( 1, 1 ) + m( 2, 2 ), h;
    if( tr >= 0 )
    {
        h = ::sqrt( tr + 1 );
        w = 0.5f * h;
        h = 0.5f / h;

        x = ( m( 2, 1 ) - m( 1, 2 ) ) * h;
        y = ( m( 0, 2 ) - m( 2, 0 ) ) * h;
        z = ( m( 1, 0 ) - m( 0, 1 ) ) * h;
    }
    else
    {
        unsigned int i = 0;
        if( m( 1, 1 ) > m( 0, 0 ) )
            i = 1;
        if( m( 2, 2 ) > m( i, i ) )
            i = 2;
        switch( i )
        {
        case 0:
            h = ::sqrt( ( m( 0, 0 ) - ( m( 1, 1 ) + m( 2, 2 ) ) ) + 1 );
            x = 0.5f * h;
            h = 0.5f / h;

            y = ( m( 0, 1 ) + m( 1, 0 ) ) * h;
            z = ( m( 2, 0 ) + m( 0, 2 ) ) * h;
            w = ( m( 2, 1 ) - m( 1, 2 ) ) * h;
            break;
        case 1:
            h = ::sqrt( ( m( 1, 1 ) - ( m( 2, 2 ) + m( 0, 0 ) ) ) + 1 );
            y = 0.5f * h;
            h = 0.5f / h;

            z = ( m( 1, 2 ) + m( 2, 1 ) ) * h;
            x = ( m( 0, 1 ) + m( 1, 0 ) ) * h;
            w = ( m( 0, 2 ) - m( 2, 0 ) ) * h;
            break;
        case 2:
            h = ::sqrt( ( m( 2, 2 ) - ( m( 0, 0 ) + m( 1, 1 ) ) ) + 1 );
            z = 0.5f * h;
            h = 0.5f / h;

            x = ( m( 2, 0 ) + m( 0, 2 ) ) * h;
            y = ( m( 1, 2 ) + m( 2, 1 ) ) * h;
            w = ( m( 1, 0 ) - m( 0, 1 ) ) * h;
            break;
        default: // Make compiler happy
            x = y = z = w = 0;
            break;
        }
    }
}

PICO_VECF_INLINE Vec3F getXYZ( const QuatF& q )
{
    return Vec3F( q.x, q.y, q.z );
}

PICO_VECF_INLINE void toRadiansAndUnitAxis( f32& angle, Vec3F& axis, const QuatF& q )
{
    const f32 quatEpsilon = 1.0e-8f;
    const f32 s2 = q.x*q.x + q.y*q.y + q.z*q.z;
    if( s2<quatEpsilon*quatEpsilon )  // can't extract a sensible axis
    {
        angle = 0.0f;
        axis = Vec3F( 1.0f, 0.0f, 0.0f );
    }
    else
    {
        const f32 s = 1.0f / ::sqrtf( s2 );
        axis = Vec3F( q.x, q.y, q.z ) * s;
        angle = ::fabs( q.w ) < quatEpsilon ? PI : ::atan2( s2*s, q.w ) * 2.0f;
    }
}
PICO_VECF_INLINE QuatF QuatF::rotation( f32 angleRadians, const Vec3F& unitAxis )
{
    PICO_ASSERT( ::abs( 1.0f - length( unitAxis ) ) < 1e-3f );
    const f32 a = angleRadians * 0.5f;
    const f32 s = ::sin( a );

    QuatF q;
    q.w = ::cos( a );
    q.x = unitAxis.x * s;
    q.y = unitAxis.y * s;
    q.z = unitAxis.z * s;
    return q;
}

PICO_VECF_INLINE f32 getAngle( const QuatF& q )
{
    return ::acos( q.w ) * 2.0f;
}
PICO_VECF_INLINE f32 dot( const QuatF& a, const QuatF& b )
{
    return a.x * b.x + 
           a.y * b.y + 
           a.z * b.z + 
           a.w * b.w;
}
PICO_VECF_INLINE f32 lengthSqr( const QuatF& q )
{
    return dot( q, q );
}

PICO_VECF_INLINE float length( const QuatF& q )
{
    return ::sqrt( lengthSqr( q ) );
}

PICO_VECF_INLINE QuatF normalize( const QuatF& q )
{
    const f32 mag = length( q );
    PICO_ASSERT( mag >= PICO_VECF_NORMALIZATION_EPSILON );

    const f32 s = 1.0f / mag;
    return QuatF( q.x*s, q.y*s, q.z*s, q.w*s );
}

PICO_VECF_INLINE QuatF normalizeSafe( const QuatF& q )
{
    const f32 mag = length( q );
    if( mag >= PICO_VECF_NORMALIZATION_EPSILON )
    {
        const f32 imag = 1.0f / mag;
        return QuatF(
            q.x * imag,
            q.y * imag,
            q.z * imag,
            q.w * imag );
    }
    return q;
}

PICO_VECF_INLINE QuatF conj( const QuatF& q )
{
    return QuatF( -q.x, -q.y, -q.z, q.w );
}



PICO_VECF_INLINE Vec3F getBasisVector0( const QuatF& q )
{
    const f32 x2 = q.x*2.0f;
    const f32 w2 = q.w*2.0f;
    return Vec3F( ( q.w * w2 )  - 1.0f + q.x*x2,
                  ( q.z * w2 )  + q.y*x2,
                  ( -q.y * w2 ) + q.z*x2 );
}

PICO_VECF_INLINE Vec3F getBasisVector1( const QuatF& q )
{
    const f32 y2 = q.y*2.0f;
    const f32 w2 = q.w*2.0f;
    return Vec3F( ( -q.z * w2 ) + q.x*y2,
                  ( q.w * w2 ) - 1.0f + q.y*y2,
                  ( q.x * w2 ) + q.z*y2 );
}

PICO_VECF_INLINE Vec3F getBasisVector2( const QuatF& q )
{
    const f32 z2 = q.z*2.0f;
    const f32 w2 = q.w*2.0f;
    return Vec3F( ( q.y * w2 ) + q.x*z2,
        ( -q.x * w2 ) + q.y*z2,
        (  q.w * w2 ) - 1.0f + q.z*z2 );
}

PICO_VECF_INLINE const Vec3F rotate( const QuatF& q, const Vec3F& v )
{
    const f32 vx = 2.0f*v.x;
    const f32 vy = 2.0f*v.y;
    const f32 vz = 2.0f*v.z;
    const f32 w2 = q.w*q.w - 0.5f;
    const f32 dot2 = ( q.x*vx + q.y*vy + q.z*vz );
    return Vec3F
    (
        ( vx*w2 + ( q.y * vz - q.z * vy )*q.w + q.x*dot2 ),
        ( vy*w2 + ( q.z * vx - q.x * vz )*q.w + q.y*dot2 ),
        ( vz*w2 + ( q.x * vy - q.y * vx )*q.w + q.z*dot2 )
    );
}

PICO_VECF_INLINE const Vec3F rotateInv( const QuatF& q, const Vec3F& v )
{
    const f32 vx = 2.0f*v.x;
    const f32 vy = 2.0f*v.y;
    const f32 vz = 2.0f*v.z;
    const f32 w2 = q.w*q.w - 0.5f;
    const f32 dot2 = ( q.x*vx + q.y*vy + q.z*vz );
    return Vec3F
    (
        ( vx*w2 - ( q.y * vz - q.z * vy )*q.w + q.x*dot2 ),
        ( vy*w2 - ( q.z * vx - q.x * vz )*q.w + q.y*dot2 ),
        ( vz*w2 - ( q.x * vy - q.y * vx )*q.w + q.z*dot2 )
    );
}
inline QuatF slerp( f32 t, const QuatF& left, const QuatF& right )
{
    const f32 quatEpsilon = ( f32( 1.0e-8f ) );

    f32 cosine = dot( left, right );
    f32 sign = 1.f;
    if( cosine < 0 )
    {
        cosine = -cosine;
        sign = -1.f;
    }

    f32 sine = 1.f - cosine*cosine;

    if( sine >= quatEpsilon*quatEpsilon )
    {
        sine = ::sqrt( sine );
        const f32 angle = ::atan2f( sine, cosine );
        const f32 i_sin_angle = 1.f / sine;

        const f32 leftw = ::sinf( angle*( 1.f - t ) ) * i_sin_angle;
        const f32 rightw = ::sinf( angle * t ) * i_sin_angle * sign;

        return left * leftw + right * rightw;
    }

    return left;
}
//////////////////////////////////////////////////////////////////////////
// --- Mat33F functions
//! Get transposed matrix
PICO_VECF_INLINE Mat33F transpose( const Mat33F& m )
{
    const Vec3F v0( m.column0.x, m.column1.x, m.column2.x );
    const Vec3F v1( m.column0.y, m.column1.y, m.column2.y );
    const Vec3F v2( m.column0.z, m.column1.z, m.column2.z );

    return Mat33F( v0, v1, v2 );
}

PICO_VECF_INLINE Vec3F transformTranspose( const Mat33F& m, const Vec3F& other )
{
    return Vec3F( 
        dot( m.column0, other ),
        dot( m.column1, other ),
        dot( m.column2, other ) 
    );
}
//! Get determinant
PICO_VECF_INLINE f32 getDeterminant( const Mat33F& m )
{
    return dot( m.column0, cross( m.column1, m.column2 ) );
}

//! Get the real inverse
PICO_VECF_INLINE Mat33F inverse( const Mat33F& m )
{
    const f32 det = getDeterminant( m );
    Mat33F inv;

    if( det != 0 )
    {
        const f32 invDet = 1.0f / det;

        inv.column0[0] = invDet * ( m.column1[1] * m.column2[2] - m.column2[1] * m.column1[2] );
        inv.column0[1] = invDet *-( m.column0[1] * m.column2[2] - m.column2[1] * m.column0[2] );
        inv.column0[2] = invDet * ( m.column0[1] * m.column1[2] - m.column0[2] * m.column1[1] );

        inv.column1[0] = invDet *-( m.column1[0] * m.column2[2] - m.column1[2] * m.column2[0] );
        inv.column1[1] = invDet * ( m.column0[0] * m.column2[2] - m.column0[2] * m.column2[0] );
        inv.column1[2] = invDet *-( m.column0[0] * m.column1[2] - m.column0[2] * m.column1[0] );

        inv.column2[0] = invDet * ( m.column1[0] * m.column2[1] - m.column1[1] * m.column2[0] );
        inv.column2[1] = invDet *-( m.column0[0] * m.column2[1] - m.column0[1] * m.column2[0] );
        inv.column2[2] = invDet * ( m.column0[0] * m.column1[1] - m.column1[0] * m.column0[1] );

        return inv;
    }
    else
    {
        return Mat33F::identity();
    }
}

