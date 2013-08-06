//=================================================================================================
/*!
//  \file blaze/math/intrinsics/Store.h
//  \brief Header file for the intrinsic aligned store functionality
//
//  Copyright (C) 2011 Klaus Iglberger - All Rights Reserved
//
//  This file is part of the Blaze library. This library is free software; you can redistribute
//  it and/or modify it under the terms of the GNU General Public License as published by the
//  Free Software Foundation; either version 3, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
//  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along with a special
//  exception for linking and compiling against the Blaze library, the so-called "runtime
//  exception"; see the file COPYING. If not, see http://www.gnu.org/licenses/.
*/
//=================================================================================================

#ifndef _BLAZE_MATH_INTRINSICS_STORE_H_
#define _BLAZE_MATH_INTRINSICS_STORE_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <blaze/math/intrinsics/BasicTypes.h>
#include <blaze/system/Vectorization.h>
#include <blaze/util/AlignmentCheck.h>
#include <blaze/util/Assert.h>
#include <blaze/util/Complex.h>
#include <blaze/util/constraints/Integral.h>
#include <blaze/util/EnableIf.h>
#include <blaze/util/StaticAssert.h>


namespace blaze {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Auxiliary helper struct for intrinsic aligned store operations.
// \ingroup intrinsics
//
// This helper structure provides the mapping between the size of an integral data type and the
// according intrinsic aligned store function. Note that the type \a T must be an integral data
// type. Instantiating the Store class with a non-integral data type results in a compilation
// error.
*/
template< typename T  // Type of the integral
        , size_t N >  // Size of the integral
struct Store;
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  SPECIALIZATIONS OF THE STORE CLASS TEMPLATE
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Specialization of the Store class template for 2-byte integral data types.
// \ingroup intrinsics
*/
template< typename T >  // Type of the integral
struct Store<T,2UL>
{
 public:
   //**Type definitions****************************************************************************
   typedef sse_int16_t  Type;
   //**********************************************************************************************

   //**Set function********************************************************************************
   static inline void store( T* address, const Type& value )
   {
      BLAZE_INTERNAL_ASSERT( checkAlignment( address ), "Invalid alignment detected" );

#if BLAZE_AVX2_MODE
      _mm256_store_si256( reinterpret_cast<__m256i*>( address ), value.value );
#elif BLAZE_SSE2_MODE
      _mm_store_si128( reinterpret_cast<__m128i*>( address ), value.value );
#else
      *address = value.value;
#endif
   }
   //**********************************************************************************************

 private:
   //**Compile time checks*************************************************************************
   BLAZE_CONSTRAINT_MUST_BE_INTEGRAL_TYPE( T );
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Specialization of the Store class template for 4-byte integral data types.
// \ingroup intrinsics
*/
template< typename T >  // Type of the integral
struct Store<T,4UL>
{
 public:
   //**Type definitions****************************************************************************
   typedef sse_int32_t  Type;
   //**********************************************************************************************

   //**Set function********************************************************************************
   static inline void store( T* address, const Type& value )
   {
      BLAZE_INTERNAL_ASSERT( checkAlignment( address ), "Invalid alignment detected" );

#if BLAZE_MIC_MODE
      _mm512_store_epi32( address, value.value );
#elif BLAZE_AVX2_MODE
      _mm256_store_si256( reinterpret_cast<__m256i*>( address ), value.value );
#elif BLAZE_SSE2_MODE
      _mm_store_si128( reinterpret_cast<__m128i*>( address ), value.value );
#else
      *address = value.value;
#endif
   }
   //**********************************************************************************************

 private:
   //**Compile time checks*************************************************************************
   BLAZE_CONSTRAINT_MUST_BE_INTEGRAL_TYPE( T );
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Specialization of the Store class template for 8-byte integral data types.
// \ingroup intrinsics
*/
template< typename T >  // Type of the integral
struct Store<T,8UL>
{
 public:
   //**Type definitions****************************************************************************
   typedef sse_int64_t  Type;
   //**********************************************************************************************

   //**Set function********************************************************************************
   static inline void store( T* address, const Type& value )
   {
      BLAZE_INTERNAL_ASSERT( checkAlignment( address ), "Invalid alignment detected" );

#if BLAZE_MIC_MODE
      _mm512_store_epi64( address, value.value );
#elif BLAZE_AVX2_MODE
      _mm256_store_si256( reinterpret_cast<__m256i*>( address ), value.value );
#elif BLAZE_SSE2_MODE
      _mm_store_si128( reinterpret_cast<__m128i*>( address ), value.value );
#else
      *address = value.value;
#endif
   }
   //**********************************************************************************************

 private:
   //**Compile time checks*************************************************************************
   BLAZE_CONSTRAINT_MUST_BE_INTEGRAL_TYPE( T );
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  INTRINSIC STORE FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Aligned store of a vector of integral values.
// \ingroup intrinsics
//
// \param address The target address.
// \param value The integral vector to be stored.
// \return void
//
// This function stores a vector of integral values. The given address must be aligned according
// to the enabled instruction set (16-byte alignment in case of SSE, 32-byte alignment in case
// of AVX, and 64-byte alignment in case of MIC.
*/
template< typename T >  // Type of the integral value
inline typename EnableIf< IsIntegral<T> >::Type
   store( T* address, const typename Store<T,sizeof(T)>::Type& value )
{
   Store<T,sizeof(T)>::store( address, value );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Aligned store of a vector of 'float' values.
// \ingroup intrinsics
//
// \param address The target address.
// \param value The 'float' vector to be stored.
// \return void
//
// This function stores a vector of 'float' values. The given address must be aligned according
// to the enabled instruction set (16-byte alignment in case of SSE, 32-byte alignment in case of
// AVX, and 64-byte alignment in case of MIC.
*/
inline void store( float* address, const sse_float_t& value )
{
   BLAZE_INTERNAL_ASSERT( checkAlignment( address ), "Invalid alignment detected" );

#if BLAZE_MIC_MODE
   _mm512_store_ps( address, value.value );
#elif BLAZE_AVX_MODE
   _mm256_store_ps( address, value.value );
#elif BLAZE_SSE_MODE
   _mm_store_ps( address, value.value );
#else
   *address = value.value;
#endif
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Aligned store of a vector of 'double' values.
// \ingroup intrinsics
//
// \param address The target address.
// \param value The 'double' vector to be stored.
// \return void
//
// This function stores a vector of 'double' values. The given address must be aligned according
// to the enabled instruction set (16-byte alignment in case of SSE, 32-byte alignment in case of
// AVX, and 64-byte alignment in case of MIC.
*/
inline void store( double* address, const sse_double_t& value )
{
   BLAZE_INTERNAL_ASSERT( checkAlignment( address ), "Invalid alignment detected" );

#if BLAZE_MIC_MODE
   _mm512_store_pd( address, value.value );
#elif BLAZE_AVX_MODE
   _mm256_store_pd( address, value.value );
#elif BLAZE_SSE2_MODE
   _mm_store_pd( address, value.value );
#else
   *address = value.value;
#endif
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Aligned store of a vector of 'complex<float>' values.
// \ingroup intrinsics
//
// \param address The target address.
// \param value The 'complex<float>' vector to be stored.
// \return void
//
// This function stores a vector of 'complex<float>' values. The given address must be aligned
// according to the enabled instruction set (16-byte alignment in case of SSE, 32-byte alignment
// in case of AVX, and 64-byte alignment in case of MIC.
*/
inline void store( complex<float>* address, const sse_cfloat_t& value )
{
   BLAZE_STATIC_ASSERT  ( sizeof( complex<float> ) == 2UL*sizeof( float ) );
   BLAZE_INTERNAL_ASSERT( checkAlignment( address ), "Invalid alignment detected" );

#if BLAZE_AVX_MODE
   _mm256_store_ps( reinterpret_cast<float*>( address ), value.value );
#elif BLAZE_SSE_MODE
   _mm_store_ps( reinterpret_cast<float*>( address ), value.value );
#else
   *address = value.value;
#endif
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Aligned store of a vector of 'complex<double>' values.
// \ingroup intrinsics
//
// \param address The target address.
// \param value The 'complex<double>' vector to be stored.
// \return void
//
// This function stores a vector of 'complex<double>' values. The given address must be aligned
// according to the enabled instruction set (16-byte alignment in case of SSE, 32-byte alignment
// in case of AVX, and 64-byte alignment in case of MIC.
*/
inline void store( complex<double>* address, const sse_cdouble_t& value )
{
   BLAZE_STATIC_ASSERT  ( sizeof( complex<double> ) == 2UL*sizeof( double ) );
   BLAZE_INTERNAL_ASSERT( checkAlignment( address ), "Invalid alignment detected" );

#if BLAZE_AVX_MODE
   _mm256_store_pd( reinterpret_cast<double*>( address ), value.value );
#elif BLAZE_SSE2_MODE
   _mm_store_pd( reinterpret_cast<double*>( address ), value.value );
#else
   *address = value.value;
#endif
}
//*************************************************************************************************

} // namespace blaze

#endif
