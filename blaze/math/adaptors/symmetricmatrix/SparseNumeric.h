//=================================================================================================
/*!
//  \file blaze/math/adaptors/symmetricmatrix/SparseNumeric.h
//  \brief SymmetricMatrix specialization for sparse matrices with numeric element type
//
//  Copyright (C) 2013 Klaus Iglberger - All Rights Reserved
//
//  This file is part of the Blaze library. You can redistribute it and/or modify it under
//  the terms of the New (Revised) BSD License. Redistribution and use in source and binary
//  forms, with or without modification, are permitted provided that the following conditions
//  are met:
//
//  1. Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice, this list
//     of conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//  3. Neither the names of the Blaze development group nor the names of its contributors
//     may be used to endorse or promote products derived from this software without specific
//     prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
//  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
//  SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
//  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
//  DAMAGE.
*/
//=================================================================================================

#ifndef _BLAZE_MATH_ADAPTORS_SYMMETRICMATRIX_SPARSENUMERIC_H_
#define _BLAZE_MATH_ADAPTORS_SYMMETRICMATRIX_SPARSENUMERIC_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <algorithm>
#include <stdexcept>
#include <vector>
#include <blaze/math/adaptors/symmetricmatrix/BaseTemplate.h>
#include <blaze/math/adaptors/symmetricmatrix/NumericProxy.h>
#include <blaze/math/constraints/Expression.h>
#include <blaze/math/constraints/Lower.h>
#include <blaze/math/constraints/Resizable.h>
#include <blaze/math/constraints/SparseMatrix.h>
#include <blaze/math/constraints/Symmetric.h>
#include <blaze/math/constraints/Upper.h>
#include <blaze/math/expressions/SparseMatrix.h>
#include <blaze/math/shims/Clear.h>
#include <blaze/math/shims/IsDefault.h>
#include <blaze/math/sparse/SparseElement.h>
#include <blaze/math/sparse/SparseMatrix.h>
#include <blaze/math/typetraits/IsComputation.h>
#include <blaze/math/typetraits/IsLower.h>
#include <blaze/math/typetraits/IsResizable.h>
#include <blaze/math/typetraits/IsRowMajorMatrix.h>
#include <blaze/math/typetraits/IsSquare.h>
#include <blaze/math/typetraits/IsSymmetric.h>
#include <blaze/math/typetraits/IsUpper.h>
#include <blaze/util/Assert.h>
#include <blaze/util/constraints/Const.h>
#include <blaze/util/constraints/Numeric.h>
#include <blaze/util/constraints/Pointer.h>
#include <blaze/util/constraints/Reference.h>
#include <blaze/util/constraints/Volatile.h>
#include <blaze/util/DisableIf.h>
#include <blaze/util/EnableIf.h>
#include <blaze/util/typetraits/IsNumeric.h>
#include <blaze/util/Types.h>
#include <blaze/util/Unused.h>


namespace blaze {

//=================================================================================================
//
//  CLASS TEMPLATE SPECIALIZATION FOR SPARSE MATRICES WITH NUMERIC ELEMENT TYPE
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Specialization of SymmetricMatrix for sparse matrices with numeric element type.
// \ingroup symmetric_matrix
//
// This specialization of SymmetricMatrix adapts the class template to the requirements of sparse
// matrices with numeric element type.
*/
template< typename MT >  // Type of the adapted sparse matrix
class SymmetricMatrix<MT,false,true>
   : public SparseMatrix< SymmetricMatrix<MT,false,true>, IsColumnMajorMatrix<MT>::value >
{
 private:
   //**Type definitions****************************************************************************
   typedef typename MT::OppositeType   OT;  //!< Opposite type of the sparse matrix.
   typedef typename MT::TransposeType  TT;  //!< Transpose type of the sparse matrix.
   typedef typename MT::ElementType    ET;  //!< Element type of the sparse matrix.
   //**********************************************************************************************

   //**Enumerations********************************************************************************
   //! Compile time flag indicating whether the given matrix type is a row-major matrix.
   enum { rmm = IsRowMajorMatrix<MT>::value };
   //**********************************************************************************************

 public:
   //**Type definitions****************************************************************************
   typedef SymmetricMatrix<MT,false,true>  This;            //!< Type of this SymmetricMatrix instance.
   typedef This                            ResultType;      //!< Result type for expression template evaluations.
   typedef SymmetricMatrix<OT,false,true>  OppositeType;    //!< Result type with opposite storage order for expression template evaluations.
   typedef SymmetricMatrix<TT,false,true>  TransposeType;   //!< Transpose type for expression template evaluations.
   typedef ET                              ElementType;     //!< Type of the matrix elements.
   typedef typename MT::ReturnType         ReturnType;      //!< Return type for expression template evaluations.
   typedef const This&                     CompositeType;   //!< Data type for composite expression templates.
   typedef NumericProxy<MT>                Reference;       //!< Reference to a non-constant matrix value.
   typedef typename MT::ConstReference     ConstReference;  //!< Reference to a constant matrix value.
   typedef typename MT::ConstIterator      ConstIterator;   //!< Iterator over constant elements.
   //**********************************************************************************************

   //**Rebind struct definition********************************************************************
   /*!\brief Rebind mechanism to obtain a SymmetricMatrix with different data/element type.
   */
   template< typename ET >  // Data type of the other matrix
   struct Rebind {
      //! The type of the other SymmetricMatrix.
      typedef SymmetricMatrix< typename MT::template Rebind<ET>::Other >  Other;
   };
   //**********************************************************************************************

   //**SymmetricValue class definition*************************************************************
   /*!\brief Representation of two synchronized values within the sparse symmetric matrix.
   */
   class SymmetricValue
   {
    private:
      //**Type trait generation********************************************************************
      BLAZE_CREATE_GET_TYPE_MEMBER_TYPE_TRAIT( GetValueType, value_type, INVALID_TYPE );
      //*******************************************************************************************

    public:
      //**Type definitions*************************************************************************
      //! Value type of the represented complex element.
      typedef typename GetValueType<ElementType>::Type  ValueType;
      //*******************************************************************************************

      //**Constructor******************************************************************************
      /*!\brief Constructor for the SymmetricValue class.
      //
      // \param v1 Reference to the first represented value.
      // \param v2 Reference to the second represented value.
      */
      inline SymmetricValue( ElementType& v1, ElementType& v2 )
         : v1_( &v1 )  // The first represented value.
         , v2_( &v2 )  // The second represented value.
      {}
      //*******************************************************************************************

      //*******************************************************************************************
      /*!\brief Copy assignment operator for SymmetricValue.
      //
      // \param sv The symmetric value to be copied.
      // \return Reference to the assigned symmetric value.
      */
      inline SymmetricValue& operator=( const SymmetricValue& sv ) {
         *v1_ = *sv.v1_;
         if( v1_ != v2_ )
            v2_ = *sv.v2_;
         return *this;
      }
      //*******************************************************************************************

      //*******************************************************************************************
      /*!\brief Assignment to the symmetric value.
      //
      // \param v The new value of the symmetric value.
      // \return Reference to the assigned symmetric value.
      */
      template< typename T > inline SymmetricValue& operator=( const T& v ) {
         *v1_ = v;
         if( v1_ != v2_ )
            *v2_ = v;
         return *this;
      }
      //*******************************************************************************************

      //*******************************************************************************************
      /*!\brief Addition assignment to the symmetric value.
      //
      // \param v The right-hand side value to be added to the symmetric value.
      // \return Reference to the assigned symmetric value.
      */
      template< typename T > inline SymmetricValue& operator+=( const T& v ) {
         *v1_ += v;
         if( v1_ != v2_ )
            *v2_ += v;
         return *this;
      }
      //*******************************************************************************************

      //*******************************************************************************************
      /*!\brief Subtraction assignment to the symmetric value.
      //
      // \param v The right-hand side value to be subtracted from the symmetric value.
      // \return Reference to the assigned symmetric value.
      */
      template< typename T > inline SymmetricValue& operator-=( const T& v ) {
         *v1_ -= v;
         if( v1_ != v2_ )
            *v2_ -= v;
         return *this;
      }
      //*******************************************************************************************

      //*******************************************************************************************
      /*!\brief Multiplication assignment to the symmetric value.
      //
      // \param v The right-hand side value for the multiplication.
      // \return Reference to the assigned symmetric value.
      */
      template< typename T > inline SymmetricValue& operator*=( const T& v ) {
         *v1_ *= v;
         if( v1_ != v2_ )
            *v2_ *= v;
         return *this;
      }
      //*******************************************************************************************

      //*******************************************************************************************
      /*!\brief Division assignment to the symmetric value.
      //
      // \param v The right-hand side value for the division.
      // \return Reference to the assigned symmetric value.
      */
      template< typename T > inline SymmetricValue& operator/=( const T& v ) {
         *v1_ /= v;
         if( v1_ != v2_ )
            *v2_ /= v;
         return *this;
      }
      //*******************************************************************************************

      //*******************************************************************************************
      /*!\brief Conversion to the represented value.
      //
      // \return Copy of the represented value.
      */
      inline operator ElementType() const {
         return *v1_;
      }
      //*******************************************************************************************

      //*******************************************************************************************
      /*!\brief Returns the real part of the represented complex number.
      //
      // \return The current real part of the represented complex number.
      //
      // In case the proxy represents a complex number, this function returns the current value
      // of its real part.
      */
      inline ValueType real() const {
         return v1_->real();
      }
      //*******************************************************************************************

      //*******************************************************************************************
      /*!\brief Setting the real part of the represented complex number.
      //
      // \param value The new value for the real part.
      // \return void
      //
      // In case the proxy represents a complex number, this function sets a new value to its
      // real part.
      */
      inline void real( ValueType value ) const {
         v1_->real( value );
         if( v1_ != v2_ )
            v2_->real( value );
      }
      //*******************************************************************************************

      //*******************************************************************************************
      /*!\brief Returns the imaginary part of the represented complex number.
      //
      // \return The current imaginary part of the represented complex number.
      //
      // In case the proxy represents a complex number, this function returns the current value of its
      // imaginary part.
      */
      inline ValueType imag() const {
         return v1_->imag();
      }
      //*******************************************************************************************

      //*******************************************************************************************
      /*!\brief Setting the imaginary part of the represented complex number.
      //
      // \param value The new value for the imaginary part.
      // \return void
      //
      // In case the proxy represents a complex number, this function sets a new value to its
      // imaginary part.
      */
      inline void imag( ValueType value ) const {
         v1_->imag( value );
         if( v1_ != v2_ )
            v2_->imag( value );
      }
      //*******************************************************************************************

    private:
      //**Member variables*************************************************************************
      ElementType* v1_;  //!< The first represented value.
      ElementType* v2_;  //!< The second represented value.
      //*******************************************************************************************
   };
   //**********************************************************************************************

   //**SymmetricElement class definition***********************************************************
   /*!\brief Representation of two synchronized elements within the sparse symmetric matrix.
   */
   class SymmetricElement : private SparseElement
   {
    private:
      //**Type definitions*************************************************************************
      typedef typename MT::Iterator  IteratorType;  //!< Type of the underlying sparse matrix iterators.
      //*******************************************************************************************

    public:
      //**Type definitions*************************************************************************
      typedef SymmetricValue        ValueType;       //!< The value type of the value-index-pair.
      typedef size_t                IndexType;       //!< The index type of the value-index-pair.
      typedef SymmetricValue        Reference;       //!< Reference return type.
      typedef const SymmetricValue  ConstReference;  //!< Reference-to-const return type.
      typedef SymmetricElement*     Pointer;         //!< Pointer return type.
      //*******************************************************************************************

      //**Constructor******************************************************************************
      /*!\brief Constructor for the SymmetricElement class.
      //
      // \param e1 Iterator to the first represented sparse element.
      // \param e2 Iterator to the second represented sparse element.
      */
      inline SymmetricElement( IteratorType e1, IteratorType e2 )
         : e1_( e1 )  // The first represented sparse element
         , e2_( e2 )  // The second represented sparse element
      {}
      //*******************************************************************************************

      //**Assignment operator**********************************************************************
      /*!\brief Assignment to the symmetric element.
      //
      // \param v The new v of the symmetric element.
      // \return Reference to the assigned symmetric element.
      */
      template< typename T > inline SymmetricElement& operator=( const T& v ) {
         *e1_ = v;
         if( e1_ != e2_ )
            *e2_ = v;
         return *this;
      }
      //*******************************************************************************************

      //**Addition assignment operator*************************************************************
      /*!\brief Addition assignment to the symmetric element.
      //
      // \param v The right-hand side value for the addition.
      // \return Reference to the assigned symmetric element.
      */
      template< typename T > inline SymmetricElement& operator+=( const T& v ) {
         *e1_ += v;
         if( e1_ != e2_ )
            *e2_ += v;
         return *this;
      }
      //*******************************************************************************************

      //**Subtraction assignment operator**********************************************************
      /*!\brief Subtraction assignment to the symmetric element.
      //
      // \param v The right-hand side value for the subtraction.
      // \return Reference to the assigned symmetric element.
      */
      template< typename T > inline SymmetricElement& operator-=( const T& v ) {
         *e1_ -= v;
         if( e1_ != e2_ )
            *e2_ -= v;
         return *this;
      }
      //*******************************************************************************************

      //**Multiplication assignment operator*******************************************************
      /*!\brief Multiplication assignment to the symmetric element.
      //
      // \param v The right-hand side value for the multiplication.
      // \return Reference to the assigned symmetric element.
      */
      template< typename T > inline SymmetricElement& operator*=( const T& v ) {
         *e1_ *= v;
         if( e1_ != e2_ )
            *e2_ *= v;
         return *this;
      }
      //*******************************************************************************************

      //**Division assignment operator*************************************************************
      /*!\brief Division assignment to the symmetric element.
      //
      // \param v The right-hand side value for the division.
      // \return Reference to the assigned symmetric element.
      */
      template< typename T > inline SymmetricElement& operator/=( const T& v ) {
         *e1_ /= v;
         if( e1_ != e2_ )
            *e2_ /= v;
         return *this;
      }
      //*******************************************************************************************

      //**Element access operator******************************************************************
      /*!\brief Direct access to the symmetric element.
      //
      // \return Reference to the value of the symmetric element.
      */
      inline Pointer operator->() {
         return this;
      }
      //*******************************************************************************************

      //**Value function***************************************************************************
      /*!\brief Access to the current value of the symmetric element.
      //
      // \return The current value of the symmetric element.
      */
      inline Reference value() const {
         return Reference( e1_->value(), e2_->value() );
      }
      //*******************************************************************************************

      //**Index function***************************************************************************
      /*!\brief Access to the current index of the symmetric element.
      //
      // \return The current index of the symmetric element.
      */
      inline IndexType index() const {
         return e1_->index();
      }
      //*******************************************************************************************

    private:
      //**Member variables*************************************************************************
      IteratorType e1_;  //!< The first represented sparse element.
      IteratorType e2_;  //!< The second represented sparse element.
      //*******************************************************************************************
   };
   //**********************************************************************************************

   //**Iterator class definition*******************************************************************
   /*!\brief Iterator over the elements of the sparse symmetric matrix.
   */
   class Iterator
   {
    public:
      //**Type definitions*************************************************************************
      typedef typename MT::Iterator  IteratorType;  //!< Type of the underlying sparse matrix iterators.

      typedef std::forward_iterator_tag  IteratorCategory;  //!< The iterator category.
      typedef SymmetricElement           ValueType;         //!< Type of the underlying elements.
      typedef ValueType                  PointerType;       //!< Pointer return type.
      typedef ValueType                  ReferenceType;     //!< Reference return type.
      typedef ptrdiff_t                  DifferenceType;    //!< Difference between two iterators.

      // STL iterator requirements
      typedef IteratorCategory  iterator_category;  //!< The iterator category.
      typedef ValueType         value_type;         //!< Type of the underlying elements.
      typedef PointerType       pointer;            //!< Pointer return type.
      typedef ReferenceType     reference;          //!< Reference return type.
      typedef DifferenceType    difference_type;    //!< Difference between two iterators.
      //*******************************************************************************************

      //**Default constructor**********************************************************************
      /*!\brief Default constructor for the Iterator class.
      */
      inline Iterator()
         : pos_   (      )  // Iterator to the current sparse symmetric matrix element
         , matrix_( NULL )  // The sparse matrix containing the iterator
         , index_ ( 0UL  )  // The row/column index of the iterator
      {}
      //*******************************************************************************************

      //**Constructor******************************************************************************
      /*!\brief Constructor for the Iterator class.
      //
      // \param pos The initial position of the iterator.
      // \param matrix The sparse matrix containing the iterator.
      // \param index The row/column index of the iterator.
      */
      inline Iterator( IteratorType pos, MT& matrix, size_t index )
         : pos_   ( pos     )  // Iterator to the current sparse symmetric matrix element
         , matrix_( &matrix )  // The sparse matrix containing the iterator
         , index_ ( index   )  // The row/column index of the iterator
      {}
      //*******************************************************************************************

      //**Prefix increment operator****************************************************************
      /*!\brief Pre-increment operator.
      //
      // \return Reference to the incremented iterator.
      */
      inline Iterator& operator++() {
         ++pos_;
         return *this;
      }
      //*******************************************************************************************

      //**Postfix increment operator***************************************************************
      /*!\brief Post-increment operator.
      //
      // \return The previous position of the iterator.
      */
      inline const Iterator operator++( int ) {
         const Iterator tmp( *this );
         ++(*this);
         return tmp;
      }
      //*******************************************************************************************

      //**Element access operator******************************************************************
      /*!\brief Direct access to the current sparse matrix element.
      //
      // \return Reference to the current sparse matrix element.
      */
      inline ReferenceType operator*() const {
         const IteratorType pos2( rmm ? matrix_->find( pos_->index(), index_ )
                                      : matrix_->find( index_, pos_->index() ) );
         BLAZE_INTERNAL_ASSERT( pos2 != matrix_->end( pos_->index() ), "Missing matrix element detected" );
         return ReferenceType( pos_, pos2 );
      }
      //*******************************************************************************************

      //**Element access operator******************************************************************
      /*!\brief Direct access to the current sparse matrix element.
      //
      // \return Pointer to the current sparse matrix element.
      */
      inline PointerType operator->() const {
         const IteratorType pos2( rmm ? matrix_->find( pos_->index(), index_ )
                                      : matrix_->find( index_, pos_->index() ) );
         BLAZE_INTERNAL_ASSERT( pos2 != matrix_->end( pos_->index() ), "Missing matrix element detected" );
         return PointerType( pos_, pos2 );
      }
      //*******************************************************************************************

      //**Conversion operator**********************************************************************
      /*!\brief Conversion to an iterator over constant elements.
      //
      // \return An iterator over constant elements.
      */
      inline operator ConstIterator() const {
         return pos_;
      }
      //*******************************************************************************************

      //**Equality operator************************************************************************
      /*!\brief Equality comparison between two Iterator objects.
      //
      // \param rhs The right-hand side matrix iterator.
      // \return \a true if the iterators refer to the same element, \a false if not.
      */
      inline bool operator==( const Iterator& rhs ) const {
         return pos_ == rhs.pos_;
      }
      //*******************************************************************************************

      //**Inequality operator**********************************************************************
      /*!\brief Inequality comparison between two Iterator objects.
      //
      // \param rhs The right-hand side matrix iterator.
      // \return \a true if the iterators don't refer to the same element, \a false if they do.
      */
      inline bool operator!=( const Iterator& rhs ) const {
         return !( *this == rhs );
      }
      //*******************************************************************************************

      //**Subtraction operator*********************************************************************
      /*!\brief Calculating the number of elements between two matrix iterators.
      //
      // \param rhs The right-hand side matrix iterator.
      // \return The number of elements between the two matrix iterators.
      */
      inline DifferenceType operator-( const Iterator& rhs ) const {
         return pos_ - rhs.pos_;
      }
      //*******************************************************************************************

      //**Base function****************************************************************************
      /*!\brief Access to the current position of the matrix iterator.
      //
      // \return The current position of the matrix iterator.
      */
      inline IteratorType base() const {
         return pos_;
      }
      //*******************************************************************************************

    private:
      //**Member variables*************************************************************************
      IteratorType pos_;     //!< Iterator to the current sparse symmetric matrix element.
      MT*          matrix_;  //!< The sparse matrix containing the iterator.
      size_t       index_;   //!< The row/column index of the iterator.
      //*******************************************************************************************
   };
   //**********************************************************************************************

   //**Compilation flags***************************************************************************
   //! Compilation switch for the expression template assignment strategy.
   enum { smpAssignable = 0 };
   //**********************************************************************************************

   //**Constructors********************************************************************************
   /*!\name Constructors */
   //@{
   explicit inline SymmetricMatrix();
   explicit inline SymmetricMatrix( size_t n );
   explicit inline SymmetricMatrix( size_t n, size_t nonzeros );
   explicit inline SymmetricMatrix( size_t n, const std::vector<size_t>& nonzeros );

   inline SymmetricMatrix( const SymmetricMatrix& m );

   template< typename MT2, bool SO >
   inline SymmetricMatrix( const Matrix<MT2,SO>& m );
   //@}
   //**********************************************************************************************

   //**Destructor**********************************************************************************
   // No explicitly declared destructor.
   //**********************************************************************************************

   //**Data access functions***********************************************************************
   /*!\name Data access functions */
   //@{
   inline Reference      operator()( size_t i, size_t j );
   inline ConstReference operator()( size_t i, size_t j ) const;
   inline Iterator       begin ( size_t i );
   inline ConstIterator  begin ( size_t i ) const;
   inline ConstIterator  cbegin( size_t i ) const;
   inline Iterator       end   ( size_t i );
   inline ConstIterator  end   ( size_t i ) const;
   inline ConstIterator  cend  ( size_t i ) const;
   //@}
   //**********************************************************************************************

   //**Assignment operators************************************************************************
   /*!\name Assignment operators */
   //@{
   inline SymmetricMatrix& operator=( const SymmetricMatrix& rhs );

   template< typename MT2, bool SO >
   inline typename DisableIf< IsComputation<MT2>, SymmetricMatrix& >::Type
      operator=( const Matrix<MT2,SO>& rhs );

   template< typename MT2, bool SO >
   inline typename EnableIf< IsComputation<MT2>, SymmetricMatrix& >::Type
      operator=( const Matrix<MT2,SO>& rhs );

   template< typename MT2, bool SO >
   inline typename DisableIf< IsComputation<MT2>, SymmetricMatrix& >::Type
      operator+=( const Matrix<MT2,SO>& rhs );

   template< typename MT2, bool SO >
   inline typename EnableIf< IsComputation<MT2>, SymmetricMatrix& >::Type
      operator+=( const Matrix<MT2,SO>& rhs );

   template< typename MT2, bool SO >
   inline typename DisableIf< IsComputation<MT2>, SymmetricMatrix& >::Type
      operator-=( const Matrix<MT2,SO>& rhs );

   template< typename MT2, bool SO >
   inline typename EnableIf< IsComputation<MT2>, SymmetricMatrix& >::Type
      operator-=( const Matrix<MT2,SO>& rhs );

   template< typename MT2, bool SO >
   inline SymmetricMatrix& operator*=( const Matrix<MT2,SO>& rhs );

   template< typename Other >
   inline typename EnableIf< IsNumeric<Other>, SymmetricMatrix >::Type&
      operator*=( Other rhs );

   template< typename Other >
   inline typename EnableIf< IsNumeric<Other>, SymmetricMatrix >::Type&
      operator/=( Other rhs );
   //@}
   //**********************************************************************************************

   //**Utility functions***************************************************************************
   /*!\name Utility functions */
   //@{
                              inline size_t           rows() const;
                              inline size_t           columns() const;
                              inline size_t           capacity() const;
                              inline size_t           capacity( size_t i ) const;
                              inline size_t           nonZeros() const;
                              inline size_t           nonZeros( size_t i ) const;
                              inline void             reset();
                              inline void             reset( size_t i );
                              inline void             clear();
                              inline Iterator         set( size_t i, size_t j, const ElementType& value );
                              inline Iterator         insert( size_t i, size_t j, const ElementType& value );
                              inline void             erase( size_t i, size_t j );
                              inline Iterator         erase( size_t i, Iterator pos );
                              inline Iterator         erase( size_t i, Iterator first, Iterator last );
                              inline void             resize ( size_t n, bool preserve=true );
                              inline void             reserve( size_t nonzeros );
                              inline void             reserve( size_t i, size_t nonzeros );
                              inline void             trim();
                              inline void             trim( size_t i );
                              inline SymmetricMatrix& transpose();
   template< typename Other > inline SymmetricMatrix& scale( const Other& scalar );
   template< typename Other > inline SymmetricMatrix& scaleDiagonal( Other scale );
                              inline void             swap( SymmetricMatrix& m ) /* throw() */;
   //@}
   //**********************************************************************************************

   //**Lookup functions****************************************************************************
   /*!\name Lookup functions */
   //@{
   inline Iterator      find      ( size_t i, size_t j );
   inline ConstIterator find      ( size_t i, size_t j ) const;
   inline Iterator      lowerBound( size_t i, size_t j );
   inline ConstIterator lowerBound( size_t i, size_t j ) const;
   inline Iterator      upperBound( size_t i, size_t j );
   inline ConstIterator upperBound( size_t i, size_t j ) const;
   //@}
   //**********************************************************************************************

   //**Low-level utility functions*****************************************************************
   /*!\name Low-level utility functions */
   //@{
   inline void append  ( size_t i, size_t j, const ElementType& value, bool check=false );
   inline void finalize( size_t i );
   //@}
   //**********************************************************************************************

   //**Expression template evaluation functions****************************************************
   /*!\name Expression template evaluation functions */
   //@{
   template< typename Other > inline bool canAlias ( const Other* alias ) const;
   template< typename Other > inline bool isAliased( const Other* alias ) const;

   inline bool canSMPAssign() const;
   //@}
   //**********************************************************************************************

 private:
   //**Member variables****************************************************************************
   /*!\name Member variables */
   //@{
   MT matrix_;  //!< The adapted sparse matrix.
   //@}
   //**********************************************************************************************

   //**Friend declarations*************************************************************************
   template< typename MT2, bool DF2, bool NF2 >
   friend bool isDefault( const SymmetricMatrix<MT2,DF2,NF2>& m );
   //**********************************************************************************************

   //**Compile time checks*************************************************************************
   BLAZE_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE       ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_REFERENCE_TYPE       ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_POINTER_TYPE         ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_CONST                ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_VOLATILE             ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_EXPRESSION_TYPE      ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_LOWER_MATRIX_TYPE    ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_UPPER_MATRIX_TYPE    ( MT );
   BLAZE_CONSTRAINT_MUST_BE_NUMERIC_TYPE             ( ElementType );
   BLAZE_STATIC_ASSERT( IsResizable<MT>::value || IsSquare<MT>::value );
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  CONSTRUCTORS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief The default constructor for SymmetricMatrix.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline SymmetricMatrix<MT,false,true>::SymmetricMatrix()
   : matrix_()  // The adapted sparse matrix
{
   BLAZE_INTERNAL_ASSERT( isSquare( matrix_ ), "Non-square symmetric matrix detected" );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Constructor for a matrix of size \f$ n \times n \f$.
//
// \param n The number of rows and columns of the matrix.
//
// The matrix is initialized to the zero matrix and has no free capacity.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline SymmetricMatrix<MT,false,true>::SymmetricMatrix( size_t n )
   : matrix_( n, n )  // The adapted sparse matrix
{
   BLAZE_CONSTRAINT_MUST_BE_RESIZABLE( MT );

   BLAZE_INTERNAL_ASSERT( isSquare( matrix_ ), "Non-square symmetric matrix detected" );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Constructor for a matrix of size \f$ n \times n \f$.
//
// \param n The number of rows and columns of the matrix.
// \param nonzeros The number of expected non-zero elements.
//
// The matrix is initialized to the zero matrix.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline SymmetricMatrix<MT,false,true>::SymmetricMatrix( size_t n, size_t nonzeros )
   : matrix_( n, n, nonzeros )  // The adapted sparse matrix
{
   BLAZE_CONSTRAINT_MUST_BE_RESIZABLE( MT );

   BLAZE_INTERNAL_ASSERT( isSquare( matrix_ ), "Non-square symmetric matrix detected" );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Constructor for a matrix of size \f$ n \times n \f$.
//
// \param n The number of rows and columns of the matrix.
// \param nonzeros The expected number of non-zero elements in each row.
//
// The matrix is initialized to the zero matrix.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline SymmetricMatrix<MT,false,true>::SymmetricMatrix( size_t n, const std::vector<size_t>& nonzeros )
   : matrix_( n, n, nonzeros )  // The adapted sparse matrix
{
   BLAZE_CONSTRAINT_MUST_BE_RESIZABLE( MT );

   BLAZE_INTERNAL_ASSERT( isSquare( matrix_ ), "Non-square symmetric matrix detected" );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief The copy constructor for SymmetricMatrix.
//
// \param m The symmetric matrix to be copied.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline SymmetricMatrix<MT,false,true>::SymmetricMatrix( const SymmetricMatrix& m )
   : matrix_( m.matrix_ )  // The adapted sparse matrix
{
   BLAZE_INTERNAL_ASSERT( isSquare( matrix_ ), "Non-square symmetric matrix detected" );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Conversion constructor from different matrices.
//
// \param m Matrix to be copied.
// \exception std::invalid_argument Invalid setup of symmetric matrix.
//
// This constructor initializes the symmetric matrix as a copy of the given matrix. In case the
// given matrix is not a symmetric matrix, a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Type of the adapted sparse matrix
template< typename MT2   // Type of the foreign matrix
        , bool SO >      // Storage order of the foreign matrix
inline SymmetricMatrix<MT,false,true>::SymmetricMatrix( const Matrix<MT2,SO>& m )
   : matrix_( ~m )  // The adapted sparse matrix
{
   if( IsLower<MT2>::value || IsUpper<MT2>::value ||
       ( !IsSymmetric<MT2>::value && !isSymmetric( matrix_ ) ) )
      throw std::invalid_argument( "Invalid setup of symmetric matrix" );

   BLAZE_INTERNAL_ASSERT( isSquare( matrix_ ), "Non-square symmetric matrix detected" );
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  DATA ACCESS FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief 2D-access to the matrix elements.
//
// \param i Access index for the row. The index has to be in the range \f$[0..N-1]\f$.
// \param j Access index for the column. The index has to be in the range \f$[0..N-1]\f$.
// \return Reference to the accessed value.
//
// The function call operator provides access to both elements \f$ a_{ij} \f$ and \f$ a_{ji} \f$.
// In order to preserve the symmetry of the matrix, any modification to one of the elements will
// also be applied to the other element.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline typename SymmetricMatrix<MT,false,true>::Reference
   SymmetricMatrix<MT,false,true>::operator()( size_t i, size_t j )
{
   BLAZE_USER_ASSERT( i<rows()   , "Invalid row access index"    );
   BLAZE_USER_ASSERT( j<columns(), "Invalid column access index" );

   return Reference( matrix_, i, j );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief 2D-access to the matrix elements.
//
// \param i Access index for the row. The index has to be in the range \f$[0..N-1]\f$.
// \param j Access index for the column. The index has to be in the range \f$[0..N-1]\f$.
// \return Reference to the accessed value.
//
// The function call operator provides access to both elements \f$ a_{ij} \f$ and \f$ a_{ji} \f$.
// In order to preserve the symmetry of the matrix, any modification to one of the elements will
// also be applied to the other element.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline typename SymmetricMatrix<MT,false,true>::ConstReference
   SymmetricMatrix<MT,false,true>::operator()( size_t i, size_t j ) const
{
   BLAZE_USER_ASSERT( i<rows()   , "Invalid row access index"    );
   BLAZE_USER_ASSERT( j<columns(), "Invalid column access index" );

   return matrix_(i,j);
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator to the first element of row/column \a i.
//
// \param i The row/column index.
// \return Iterator to the first element of row/column \a i.
//
// This function returns a row/column iterator to the first element of row/column \a i. In case
// the storage order is set to \a rowMajor the function returns an iterator to the first element
// of row \a i, in case the storage flag is set to \a columnMajor the function returns an iterator
// to the first element of column \a i.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline typename SymmetricMatrix<MT,false,true>::Iterator
   SymmetricMatrix<MT,false,true>::begin( size_t i )
{
   return Iterator( matrix_.begin(i), matrix_, i );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator to the first element of row/column \a i.
//
// \param i The row/column index.
// \return Iterator to the first element of row/column \a i.
//
// This function returns a row/column iterator to the first element of row/column \a i. In case
// the symmetric matrix adapts a \a rowMajor sparse matrix the function returns an iterator to
// the first element of row \a i, in case it adapts a \a columnMajor sparse matrix the function
// returns an iterator to the first element of column \a i.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline typename SymmetricMatrix<MT,false,true>::ConstIterator
   SymmetricMatrix<MT,false,true>::begin( size_t i ) const
{
   return matrix_.begin(i);
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator to the first element of row/column \a i.
//
// \param i The row/column index.
// \return Iterator to the first element of row/column \a i.
//
// This function returns a row/column iterator to the first element of row/column \a i. In case
// the symmetric matrix adapts a \a rowMajor sparse matrix the function returns an iterator to
// the first element of row \a i, in case it adapts a \a columnMajor sparse matrix the function
// returns an iterator to the first element of column \a i.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline typename SymmetricMatrix<MT,false,true>::ConstIterator
   SymmetricMatrix<MT,false,true>::cbegin( size_t i ) const
{
   return matrix_.cbegin(i);
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator just past the last element of row/column \a i.
//
// \param i The row/column index.
// \return Iterator just past the last element of row/column \a i.
//
// This function returns an row/column iterator just past the last element of row/column \a i.
// In case the storage order is set to \a rowMajor the function returns an iterator just past
// the last element of row \a i, in case the storage flag is set to \a columnMajor the function
// returns an iterator just past the last element of column \a i.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline typename SymmetricMatrix<MT,false,true>::Iterator
   SymmetricMatrix<MT,false,true>::end( size_t i )
{
   return Iterator( matrix_.end(i), matrix_, i );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator just past the last element of row/column \a i.
//
// \param i The row/column index.
// \return Iterator just past the last element of row/column \a i.
//
// This function returns an row/column iterator just past the last element of row/column \a i.
// In case the symmetric matrix adapts a \a rowMajor sparse matrix the function returns an iterator
// just past the last element of row \a i, in case it adapts a \a columnMajor sparse matrix the
// function returns an iterator just past the last element of column \a i.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline typename SymmetricMatrix<MT,false,true>::ConstIterator
   SymmetricMatrix<MT,false,true>::end( size_t i ) const
{
   return matrix_.end(i);
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator just past the last element of row/column \a i.
//
// \param i The row/column index.
// \return Iterator just past the last element of row/column \a i.
//
// This function returns an row/column iterator just past the last element of row/column \a i.
// In case the symmetric matrix adapts a \a rowMajor sparse matrix the function returns an iterator
// just past the last element of row \a i, in case it adapts a \a columnMajor sparse matrix the
// function returns an iterator just past the last element of column \a i.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline typename SymmetricMatrix<MT,false,true>::ConstIterator
   SymmetricMatrix<MT,false,true>::cend( size_t i ) const
{
   return matrix_.cend(i);
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ASSIGNMENT OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Copy assignment operator for SymmetricMatrix.
//
// \param rhs Matrix to be copied.
// \return Reference to the assigned matrix.
//
// If possible and necessary, the matrix is resized according to the given \f$ N \times N \f$
// matrix and initialized as a copy of this matrix.
*/
template< typename MT >  // Type of the adapted dense matrix
inline SymmetricMatrix<MT,false,true>&
   SymmetricMatrix<MT,false,true>::operator=( const SymmetricMatrix& rhs )
{
   matrix_ = rhs.matrix_;

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Assignment operator for plain matrices.
//
// \param rhs The plain matrix to be copied.
// \return Reference to the assigned matrix.
// \exception std::invalid_argument Invalid assignment to symmetric matrix.
//
// If possible and necessary, the matrix is resized according to the given \f$ N \times N \f$
// matrix and initialized as a copy of this matrix. If the matrix cannot be resized accordingly,
// a \a std::invalid_argument exception is thrown. Also note that the given matrix must be a
// symmetric matrix. Otherwise, a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Type of the adapted dense matrix
template< typename MT2   // Type of the right-hand side matrix
        , bool SO >      // Storage order of the right-hand side matrix
inline typename DisableIf< IsComputation<MT2>, SymmetricMatrix<MT,false,true>& >::Type
   SymmetricMatrix<MT,false,true>::operator=( const Matrix<MT2,SO>& rhs )
{
   if( IsLower<MT2>::value || IsUpper<MT2>::value ||
       ( !IsSymmetric<MT2>::value && !isSymmetric( ~rhs ) ) )
      throw std::invalid_argument( "Invalid assignment to symmetric matrix" );

   matrix_ = ~rhs;

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Assignment operator for matrix computations.
//
// \param rhs The matrix computation to be copied.
// \return Reference to the assigned matrix.
// \exception std::invalid_argument Invalid assignment to symmetric matrix.
//
// If possible and necessary, the matrix is resized according to the given \f$ N \times N \f$
// matrix and initialized as a copy of this matrix. If the matrix cannot be resized accordingly,
// a \a std::invalid_argument exception is thrown. Also note that the given matrix must be a
// symmetric matrix. Otherwise, a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Type of the adapted dense matrix
template< typename MT2   // Type of the right-hand side matrix
        , bool SO >      // Storage order of the right-hand side matrix
inline typename EnableIf< IsComputation<MT2>, SymmetricMatrix<MT,false,true>& >::Type
   SymmetricMatrix<MT,false,true>::operator=( const Matrix<MT2,SO>& rhs )
{
   using std::swap;

   if( IsLower<MT2>::value || IsUpper<MT2>::value )
      throw std::invalid_argument( "Invalid assignment to symmetric matrix" );

   if( IsSymmetric<MT2>::value ) {
      matrix_ = ~rhs;
   }
   else {
      MT tmp( ~rhs );

      if( !isSymmetric( tmp ) )
         throw std::invalid_argument( "Invalid assignment to symmetric matrix" );

      move( matrix_, tmp );
   }

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Addition assignment operator for the addition of a plain matrix (\f$ A+=B \f$).
//
// \param rhs The right-hand side plain matrix to be added.
// \return Reference to the matrix.
// \exception std::invalid_argument Invalid assignment to symmetric matrix.
//
// In case the current sizes of the two matrices don't match, a \a std::invalid_argument exception
// is thrown. Also note that the result of the addition operation must be a symmetric matrix, i.e.
// the given matrix must be a symmetric matrix. In case the result is not a symmetric matrix, a
// \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Type of the adapted dense matrix
template< typename MT2   // Type of the right-hand side matrix
        , bool SO >      // Storage order of the right-hand side matrix
inline typename DisableIf< IsComputation<MT2>, SymmetricMatrix<MT,false,true>& >::Type
   SymmetricMatrix<MT,false,true>::operator+=( const Matrix<MT2,SO>& rhs )
{
   if( IsLower<MT2>::value || IsUpper<MT2>::value ||
       ( !IsSymmetric<MT2>::value && !isSymmetric( ~rhs ) ) )
      throw std::invalid_argument( "Invalid assignment to symmetric matrix" );

   matrix_ += ~rhs;

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Addition assignment operator for the addition of a matrix computation (\f$ A+=B \f$).
//
// \param rhs The right-hand side matrix computation to be added.
// \return Reference to the matrix.
// \exception std::invalid_argument Invalid assignment to symmetric matrix.
//
// In case the current sizes of the two matrices don't match, a \a std::invalid_argument exception
// is thrown. Also note that the result of the addition operation must be a symmetric matrix, i.e.
// the given matrix must be a symmetric matrix. In case the result is not a symmetric matrix, a
// \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Type of the adapted dense matrix
template< typename MT2   // Type of the right-hand side matrix
        , bool SO >      // Storage order of the right-hand side matrix
inline typename EnableIf< IsComputation<MT2>, SymmetricMatrix<MT,false,true>& >::Type
   SymmetricMatrix<MT,false,true>::operator+=( const Matrix<MT2,SO>& rhs )
{
   if( IsLower<MT2>::value || IsUpper<MT2>::value )
      throw std::invalid_argument( "Invalid assignment to symmetric matrix" );

   if( IsSymmetric<MT2>::value ) {
      matrix_ += ~rhs;
   }
   else {
      typename MT2::ResultType tmp( ~rhs );

      if( !isSymmetric( tmp ) )
         throw std::invalid_argument( "Invalid assignment to symmetric matrix" );

      matrix_ += tmp;
   }

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Subtraction assignment operator for the subtraction of a plain matrix (\f$ A-=B \f$).
//
// \param rhs The right-hand side plain matrix to be subtracted.
// \return Reference to the matrix.
// \exception std::invalid_argument Invalid assignment to symmetric matrix.
//
// In case the current sizes of the two matrices don't match, a \a std::invalid_argument exception
// is thrown. Also note that the result of the subtraction operation must be a symmetric matrix,
// i.e. the given matrix must be a symmetric matrix. In case the result is not a symmetric matrix,
// a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Type of the adapted dense matrix
template< typename MT2   // Type of the right-hand side matrix
        , bool SO >      // Storage order of the right-hand side matrix
inline typename DisableIf< IsComputation<MT2>, SymmetricMatrix<MT,false,true>& >::Type
   SymmetricMatrix<MT,false,true>::operator-=( const Matrix<MT2,SO>& rhs )
{
   if( IsLower<MT2>::value || IsUpper<MT2>::value ||
       ( !IsSymmetric<MT2>::value && !isSymmetric( ~rhs ) ) )
      throw std::invalid_argument( "Invalid assignment to symmetric matrix" );

   matrix_ -= ~rhs;

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Subtraction assignment operator for the subtraction of a matrix computation (\f$ A-=B \f$).
//
// \param rhs The right-hand side matrix computation to be subtracted.
// \return Reference to the matrix.
// \exception std::invalid_argument Invalid assignment to symmetric matrix.
//
// In case the current sizes of the two matrices don't match, a \a std::invalid_argument exception
// is thrown. Also note that the result of the subtraction operation must be a symmetric matrix,
// i.e. the given matrix must be a symmetric matrix. In case the result is not a symmetric matrix,
// a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Type of the adapted dense matrix
template< typename MT2   // Type of the right-hand side matrix
        , bool SO >      // Storage order of the right-hand side matrix
inline typename EnableIf< IsComputation<MT2>, SymmetricMatrix<MT,false,true>& >::Type
   SymmetricMatrix<MT,false,true>::operator-=( const Matrix<MT2,SO>& rhs )
{
   if( IsLower<MT2>::value || IsUpper<MT2>::value )
      throw std::invalid_argument( "Invalid assignment to symmetric matrix" );

   if( IsSymmetric<MT2>::value ) {
      matrix_ -= ~rhs;
   }
   else {
      typename MT2::ResultType tmp( ~rhs );

      if( !isSymmetric( tmp ) )
         throw std::invalid_argument( "Invalid assignment to symmetric matrix" );

      matrix_ -= tmp;
   }

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Multiplication assignment operator for the multiplication of a matrix (\f$ A*=B \f$).
//
// \param rhs The right-hand side matrix for the multiplication.
// \return Reference to the matrix.
// \exception std::invalid_argument Matrix sizes do not match.
//
// In case the current sizes of the two matrices don't match, a \a std::invalid_argument exception
// is thrown. Also note that the result of the multiplication operation must be a symmetric matrix.
// In case it is not, a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Type of the adapted dense matrix
template< typename MT2   // Type of the right-hand side matrix
        , bool SO >      // Storage order of the right-hand side matrix
inline SymmetricMatrix<MT,false,true>&
   SymmetricMatrix<MT,false,true>::operator*=( const Matrix<MT2,SO>& rhs )
{
   using std::swap;

   MT tmp( matrix_ * ~rhs );

   if( !isSymmetric( tmp ) )
      throw std::invalid_argument( "Invalid assignment to static matrix" );

   move( matrix_, tmp );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Multiplication assignment operator for the multiplication between a matrix and
//        a scalar value (\f$ A*=s \f$).
//
// \param rhs The right-hand side scalar value for the multiplication.
// \return Reference to the matrix.
*/
template< typename MT >     // Type of the adapted dense matrix
template< typename Other >  // Data type of the right-hand side scalar
inline typename EnableIf< IsNumeric<Other>, SymmetricMatrix<MT,false,true> >::Type&
   SymmetricMatrix<MT,false,true>::operator*=( Other rhs )
{
   matrix_ *= rhs;
   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Division assignment operator for the division of a matrix by a scalar value
//        (\f$ A/=s \f$).
//
// \param rhs The right-hand side scalar value for the division.
// \return Reference to the matrix.
*/
template< typename MT >     // Type of the adapted dense matrix
template< typename Other >  // Data type of the right-hand side scalar
inline typename EnableIf< IsNumeric<Other>, SymmetricMatrix<MT,false,true> >::Type&
   SymmetricMatrix<MT,false,true>::operator/=( Other rhs )
{
   BLAZE_USER_ASSERT( rhs != Other(0), "Division by zero detected" );

   matrix_ /= rhs;
   return *this;
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns the current number of rows of the matrix.
//
// \return The number of rows of the matrix.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline size_t SymmetricMatrix<MT,false,true>::rows() const
{
   return matrix_.rows();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns the current number of columns of the matrix.
//
// \return The number of columns of the matrix.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline size_t SymmetricMatrix<MT,false,true>::columns() const
{
   return matrix_.columns();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns the maximum capacity of the matrix.
//
// \return The capacity of the matrix.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline size_t SymmetricMatrix<MT,false,true>::capacity() const
{
   return matrix_.capacity();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns the current capacity of the specified row/column.
//
// \param i The index of the row/column.
// \return The current capacity of row/column \a i.
//
// This function returns the current capacity of the specified row/column. In case the symmetric
// matrix adapts a \a rowMajor sparse matrix the function returns the capacity of row \a i, in
// case it adapts a \a columnMajor sparse matrix the function returns the capacity of column \a i.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline size_t SymmetricMatrix<MT,false,true>::capacity( size_t i ) const
{
   return matrix_.capacity(i);
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns the total number of non-zero elements in the matrix
//
// \return The number of non-zero elements in the symmetric matrix.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline size_t SymmetricMatrix<MT,false,true>::nonZeros() const
{
   return matrix_.nonZeros();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns the number of non-zero elements in the specified row/column.
//
// \param i The index of the row/column.
// \return The number of non-zero elements of row/column \a i.
//
// This function returns the current number of non-zero elements in the specified row/column. In
// case the symmetric matrix adapts a \a rowMajor sparse matrix the function returns the number of
// non-zero elements in row \a i, in case it adapts a to \a columnMajor sparse matrix the function
// returns the number of non-zero elements in column \a i.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline size_t SymmetricMatrix<MT,false,true>::nonZeros( size_t i ) const
{
   return matrix_.nonZeros(i);
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Reset to the default initial values.
//
// \return void
*/
template< typename MT >  // Type of the adapted sparse matrix
inline void SymmetricMatrix<MT,false,true>::reset()
{
   matrix_.reset();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Reset the specified row \b and column to the default initial values.
//
// \param i The index of the row/column.
// \return void
// \exception std::invalid_argument Invalid row/column access index.
//
// This function resets the values in the specified row \b and column to their default value.
// The following example demonstrates this by means of a \f$ 5 \times 5 \f$ symmetric matrix:

   \code
   blaze::SymmetricMatrix< blaze::CompressedMatrix<int> > A;

   // Initializing the symmetric matrix A to
   //
   //      (  0  2  5 -4  0 )
   //      (  2  1 -3  7  0 )
   //  A = (  5 -3  8 -1 -2 )
   //      ( -4  7 -1  0 -6 )
   //      (  0  0 -2 -6  1 )
   // ...

   // Resetting the 1st row/column results in the matrix
   //
   //      (  0  0  5 -4  0 )
   //      (  0  0  0  0  0 )
   //  A = (  5  0  8 -1 -2 )
   //      ( -4  0 -1  0 -6 )
   //      (  0  0 -2 -6  1 )
   //
   A.reset( 1UL );
   \endcode

// Note that the reset() function has no impact on the capacity of the matrix or row/column.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline void SymmetricMatrix<MT,false,true>::reset( size_t i )
{
   for( typename MT::Iterator it=matrix_.begin(i); it!=matrix_.end(i); ++it )
   {
      const size_t j( it->index() );

      if( i == j )
         continue;

      if( IsRowMajorMatrix<MT>::value ) {
         const typename MT::Iterator pos( matrix_.find( j, i ) );
         BLAZE_INTERNAL_ASSERT( pos != matrix_.end( j ), "Missing element detected" );
         matrix_.erase( j, pos );
      }
      else {
         const typename MT::Iterator pos( matrix_.find( i, j ) );
         BLAZE_INTERNAL_ASSERT( pos != matrix_.end( j ), "Missing element detected" );
         matrix_.erase( j, pos );
      }
   }

   matrix_.reset( i );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Clearing the symmetric matrix.
//
// \return void
//
// This function clears the symmetric matrix and returns it to its default state.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline void SymmetricMatrix<MT,false,true>::clear()
{
   using blaze::clear;

   clear( matrix_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Setting elements of the symmetric matrix.
//
// \param i The row index of the new element. The index has to be in the range \f$[0..N-1]\f$.
// \param j The column index of the new element. The index has to be in the range \f$[0..N-1]\f$.
// \param value The value of the element to be set.
// \return Iterator to the set element.
//
// This function sets the value of both the elements \f$ a_{ij} \f$ and \f$ a_{ji} \f$ of the
// symmetric matrix and returns an iterator to the successfully set element \f$ a_{ij} \f$. In
// case the symmetric matrix already contains the two elements with index \a i and \a j their
// values are modified, else two new elements with the given \a value are inserted.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline typename SymmetricMatrix<MT,false,true>::Iterator
   SymmetricMatrix<MT,false,true>::set( size_t i, size_t j, const ElementType& value )
{
   if( i != j )
      matrix_.set( j, i, value );
   return Iterator( matrix_.set( i, j, value ), matrix_, ( rmm ? i : j ) );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Inserting elements into the symmetric matrix.
//
// \param i The row index of the new element. The index has to be in the range \f$[0..N-1]\f$.
// \param j The column index of the new element. The index has to be in the range \f$[0..N-1]\f$.
// \param value The value of the element to be inserted.
// \return Iterator to the newly inserted element.
// \exception std::invalid_argument Invalid sparse matrix access index.
//
// This function inserts both the elements \f$ a_{ij} \f$ and \f$ a_{ji} \f$ into the symmetric
// matrix and returns an iterator to the successfully inserted element \f$ a_{ij} \f$. However,
// duplicate elements are not allowed. In case the symmetric matrix an element with row index
// \a i and column index \a j, a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline typename SymmetricMatrix<MT,false,true>::Iterator
   SymmetricMatrix<MT,false,true>::insert( size_t i, size_t j, const ElementType& value )
{
   if( i != j )
      matrix_.insert( j, i, value );
   return Iterator( matrix_.insert( i, j, value ), matrix_, ( rmm ? i : j ) );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Erasing elements from the symmetric matrix.
//
// \param i The row index of the element to be erased. The index has to be in the range \f$[0..N-1]\f$.
// \param j The column index of the element to be erased. The index has to be in the range \f$[0..N-1]\f$.
// \return void
//
// This function erases both elements \f$ a_{ij} \f$ and \f$ a_{ji} \f$ from the symmetric matrix.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline void SymmetricMatrix<MT,false,true>::erase( size_t i, size_t j )
{
   matrix_.erase( i, j );
   if( i != j )
      matrix_.erase( j, i );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Erasing elements from the symmetric matrix.
//
// \param i The row/column index of the element to be erased. The index has to be in the range \f$[0..N-1]\f$.
// \param pos Iterator to the element to be erased.
// \return Iterator to the element after the erased element.
//
// This function erases both the specified element and its according symmetric counterpart from
// the symmetric matrix. In case the storage order is set to \a rowMajor the given index \a i
// refers to a row, in case the storage flag is set to \a columnMajor \a i refers to a column.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline typename SymmetricMatrix<MT,false,true>::Iterator
   SymmetricMatrix<MT,false,true>::erase( size_t i, Iterator pos )
{
   const typename MT::Iterator base( pos.base() );

   if( base == matrix_.end( i ) )
      return pos;

   const size_t j( base->index() );

   if( i == j ) {
      BLAZE_INTERNAL_ASSERT( matrix_.find( i, i ) != matrix_.end( i ), "Missing element detected" );
      return Iterator( matrix_.erase( i, base ), matrix_, i );
   }

   if( IsRowMajorMatrix<MT>::value ) {
      BLAZE_INTERNAL_ASSERT( matrix_.find( j, i ) != matrix_.end( j ), "Missing element detected" );
      matrix_.erase( j, matrix_.find( j, i ) );
      return Iterator( matrix_.erase( i, base ), matrix_, i );
   }
   else {
      BLAZE_INTERNAL_ASSERT( matrix_.find( i, j ) != matrix_.end( j ), "Missing element detected" );
      matrix_.erase( j, matrix_.find( i, j ) );
      return Iterator( matrix_.erase( i, base ), matrix_, i );
   }
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Erasing a range of elements from the symmetric matrix.
//
// \param i The row/column index of the element to be erased. The index has to be in the range \f$[0..N-1]\f$.
// \param first Iterator to first element to be erased.
// \param last Iterator just past the last element to be erased.
// \return Iterator to the element after the erased element.
//
// This function erases both the range of elements specified by the iterator pair \a first and
// \a last and their according symmetric counterparts from the symmetric matrix. In case the
// storage order is set to \a rowMajor the given index \a i refers to a row, in case the storage
// flag is set to \a columnMajor \a i refers to a column.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline typename SymmetricMatrix<MT,false,true>::Iterator
   SymmetricMatrix<MT,false,true>::erase( size_t i, Iterator first, Iterator last )
{
   for( typename MT::Iterator it=first.base(); it!=last.base(); ++it )
   {
      const size_t j( it->index() );

      if( i == j )
         continue;

      if( IsRowMajorMatrix<MT>::value ) {
         BLAZE_INTERNAL_ASSERT( matrix_.find( j, i ) != matrix_.end( j ), "Missing element detected" );
         matrix_.erase( j, i );
      }
      else {
         BLAZE_INTERNAL_ASSERT( matrix_.find( i, j ) != matrix_.end( j ), "Missing element detected" );
         matrix_.erase( i, j );
      }
   }

   return Iterator( matrix_.erase( i, first.base(), last.base() ), matrix_, i );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Changing the size of the symmetric matrix.
//
// \param n The new number of rows and columns of the matrix.
// \param preserve \a true if the old values of the matrix should be preserved, \a false if not.
// \return void
//
// This function resizes the matrix using the given size to \f$ m \times n \f$. During this
// operation, new dynamic memory may be allocated in case the capacity of the matrix is too
// small. Note that this function may invalidate all existing views (submatrices, rows, columns,
// ...) on the matrix if it is used to shrink the matrix. Additionally, the resize operation
// potentially changes all matrix elements. In order to preserve the old matrix values, the
// \a preserve flag can be set to \a true.
*/
template< typename MT >  // Type of the adapted sparse matrix
void SymmetricMatrix<MT,false,true>::resize( size_t n, bool preserve )
{
   BLAZE_CONSTRAINT_MUST_BE_RESIZABLE( MT );

   UNUSED_PARAMETER( preserve );

   BLAZE_INTERNAL_ASSERT( isSquare( matrix_ ), "Non-square symmetric matrix detected" );

   matrix_.resize( n, n, true );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Setting the minimum capacity of the symmetric matrix.
//
// \param nonzeros The new minimum capacity of the symmetric matrix.
// \return void
//
// This function increases the capacity of the symmetric matrix to at least \a nonzeros elements.
// The current values of the matrix elements and the individual capacities of the matrix rows
// are preserved.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline void SymmetricMatrix<MT,false,true>::reserve( size_t nonzeros )
{
   matrix_.reserve( nonzeros );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Setting the minimum capacity of a specific row/column of the symmetric matrix.
//
// \param i The row/column index \f$[0..N-1]\f$.
// \param nonzeros The new minimum capacity of the specified row/column.
// \return void
//
// This function increases the capacity of row/column \a i of the symmetric matrix to at least
// \a nonzeros elements. The current values of the symmetric matrix and all other individual
// row/column capacities are preserved. In case the storage order is set to \a rowMajor, the
// function reserves capacity for row \a i. In case the storage order is set to \a columnMajor,
// the function reserves capacity for column \a i. The index has to be in the range \f$[0..N-1]\f$.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline void SymmetricMatrix<MT,false,true>::reserve( size_t i, size_t nonzeros )
{
   matrix_.reserve( i, nonzeros );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Removing all excessive capacity from all rows/columns.
//
// \return void
//
// The trim() function can be used to reverse the effect of all row/column-specific reserve()
// calls. The function removes all excessive capacity from all rows (in case of a rowMajor
// matrix) or columns (in case of a columnMajor matrix). Note that this function does not
// remove the overall capacity but only reduces the capacity per row/column.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline void SymmetricMatrix<MT,false,true>::trim()
{
   matrix_.trim();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Removing all excessive capacity of a specific row/column of the symmetric matrix.
//
// \param i The index of the row/column to be trimmed \f$[0..N-1]\f$.
// \return void
//
// This function can be used to reverse the effect of a row/column-specific reserve() call.
// It removes all excessive capacity from the specified row (in case of a rowMajor matrix)
// or column (in case of a columnMajor matrix). The excessive capacity is assigned to the
// subsequent row/column.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline void SymmetricMatrix<MT,false,true>::trim( size_t i )
{
   matrix_.trim( i );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Transposing the symmetric matrix.
//
// \return Reference to the transposed matrix.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline SymmetricMatrix<MT,false,true>& SymmetricMatrix<MT,false,true>::transpose()
{
   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Scaling of the matrix by the scalar value \a scalar (\f$ A=B*s \f$).
//
// \param scalar The scalar value for the matrix scaling.
// \return Reference to the matrix.
*/
template< typename MT >     // Type of the adapted sparse matrix
template< typename Other >  // Data type of the scalar value
inline SymmetricMatrix<MT,false,true>& SymmetricMatrix<MT,false,true>::scale( const Other& scalar )
{
   matrix_.scale( scalar );
   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Scaling the diagonal of the symmetric matrix by the scalar value \a scalar.
//
// \param scalar The scalar value for the diagonal scaling.
// \return Reference to the symmetric matrix.
*/
template< typename MT >     // Type of the adapted sparse matrix
template< typename Other >  // Data type of the scalar value
inline SymmetricMatrix<MT,false,true>& SymmetricMatrix<MT,false,true>::scaleDiagonal( Other scalar )
{
   matrix_.scaleDiagonal( scalar );
   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Swapping the contents of two matrices.
//
// \param m The matrix to be swapped.
// \return void
// \exception no-throw guarantee.
*/
template< typename MT >  // Type of the adapted sparse matrix
inline void SymmetricMatrix<MT,false,true>::swap( SymmetricMatrix& m ) /* throw() */
{
   using std::swap;

   swap( matrix_, m.matrix_ );
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  LOOKUP FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Searches for a specific matrix element.
//
// \param i The row index of the search element. The index has to be in the range \f$[0..N-1]\f$.
// \param j The column index of the search element. The index has to be in the range \f$[0..N-1]\f$.
// \return Iterator to the element in case the index is found, end() iterator otherwise.
//
// This function can be used to check whether a specific element is contained in the symmetric
// matrix. It specifically searches for the element with row index \a i and column index \a j.
// In case the element is found, the function returns an row/column iterator to the element.
// Otherwise an iterator just past the last non-zero element of row \a i or column \a j (the
// end() iterator) is returned. Note that the returned symmetric matrix iterator is subject
// to invalidation due to inserting operations via the function call operator or the insert()
// function!
*/
template< typename MT >  // Type of the adapted sparse matrix
inline typename SymmetricMatrix<MT,false,true>::Iterator
   SymmetricMatrix<MT,false,true>::find( size_t i, size_t j )
{
   return Iterator( matrix_.find( i, j ), matrix_, ( rmm ? i : j ) );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Searches for a specific matrix element.
//
// \param i The row index of the search element. The index has to be in the range \f$[0..N-1]\f$.
// \param j The column index of the search element. The index has to be in the range \f$[0..N-1]\f$.
// \return Iterator to the element in case the index is found, end() iterator otherwise.
//
// This function can be used to check whether a specific element is contained in the symmetric
// matrix. It specifically searches for the element with row index \a i and column index \a j.
// In case the element is found, the function returns an row/column iterator to the element.
// Otherwise an iterator just past the last non-zero element of row \a i or column \a j (the
// end() iterator) is returned. Note that the returned symmetric matrix iterator is subject
// to invalidation due to inserting operations via the function call operator or the insert()
// function!
*/
template< typename MT >  // Type of the adapted sparse matrix
inline typename SymmetricMatrix<MT,false,true>::ConstIterator
   SymmetricMatrix<MT,false,true>::find( size_t i, size_t j ) const
{
   return matrix_.find( i, j );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator to the first index not less then the given index.
//
// \param i The row index of the search element. The index has to be in the range \f$[0..N-1]\f$.
// \param j The column index of the search element. The index has to be in the range \f$[0..N-1]\f$.
// \return Iterator to the first index not less then the given index, end() iterator otherwise.
//
// In case of a row-major matrix, this function returns a row iterator to the first element with
// an index not less then the given column index. In case of a column-major matrix, the function
// returns a column iterator to the first element with an index not less then the given row
// index. In combination with the upperBound() function this function can be used to create a
// pair of iterators specifying a range of indices. Note that the returned symmetric matrix
// iterator is subject to invalidation due to inserting operations via the function call operator
// or the insert() function!
*/
template< typename MT >  // Type of the adapted sparse matrix
inline typename SymmetricMatrix<MT,false,true>::Iterator
   SymmetricMatrix<MT,false,true>::lowerBound( size_t i, size_t j )
{
   return Iterator( matrix_.lowerBound( i, j ), matrix_, ( rmm ? i : j ) );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator to the first index not less then the given index.
//
// \param i The row index of the search element. The index has to be in the range \f$[0..N-1]\f$.
// \param j The column index of the search element. The index has to be in the range \f$[0..N-1]\f$.
// \return Iterator to the first index not less then the given index, end() iterator otherwise.
//
// In case of a row-major matrix, this function returns a row iterator to the first element with
// an index not less then the given column index. In case of a column-major matrix, the function
// returns a column iterator to the first element with an index not less then the given row
// index. In combination with the upperBound() function this function can be used to create a
// pair of iterators specifying a range of indices. Note that the returned symmetric matrix
// iterator is subject to invalidation due to inserting operations via the function call operator
// or the insert() function!
*/
template< typename MT >  // Type of the adapted sparse matrix
inline typename SymmetricMatrix<MT,false,true>::ConstIterator
   SymmetricMatrix<MT,false,true>::lowerBound( size_t i, size_t j ) const
{
   return matrix_.lowerBound( i, j );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator to the first index greater then the given index.
//
// \param i The row index of the search element. The index has to be in the range \f$[0..N-1]\f$.
// \param j The column index of the search element. The index has to be in the range \f$[0..N-1]\f$.
// \return Iterator to the first index greater then the given index, end() iterator otherwise.
//
// In case of a row-major matrix, this function returns a row iterator to the first element with
// an index greater then the given column index. In case of a column-major matrix, the function
// returns a column iterator to the first element with an index greater then the given row
// index. In combination with the upperBound() function this function can be used to create a
// pair of iterators specifying a range of indices. Note that the returned symmetric matrix
// iterator is subject to invalidation due to inserting operations via the function call operator
// or the insert() function!
*/
template< typename MT >  // Type of the adapted sparse matrix
inline typename SymmetricMatrix<MT,false,true>::Iterator
   SymmetricMatrix<MT,false,true>::upperBound( size_t i, size_t j )
{
   return Iterator( matrix_.upperBound( i, j ), matrix_, ( rmm ? i : j ) );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator to the first index greater then the given index.
//
// \param i The row index of the search element. The index has to be in the range \f$[0..N-1]\f$.
// \param j The column index of the search element. The index has to be in the range \f$[0..N-1]\f$.
// \return Iterator to the first index greater then the given index, end() iterator otherwise.
//
// In case of a row-major matrix, this function returns a row iterator to the first element with
// an index greater then the given column index. In case of a column-major matrix, the function
// returns a column iterator to the first element with an index greater then the given row
// index. In combination with the upperBound() function this function can be used to create a
// pair of iterators specifying a range of indices. Note that the returned symmetric matrix
// iterator is subject to invalidation due to inserting operations via the function call operator
// or the insert() function!
*/
template< typename MT >  // Type of the adapted sparse matrix
inline typename SymmetricMatrix<MT,false,true>::ConstIterator
   SymmetricMatrix<MT,false,true>::upperBound( size_t i, size_t j ) const
{
   return matrix_.upperBound( i, j );
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  LOW-LEVEL UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Appending elements to the specified row/column of the symmetric matrix.
//
// \param i The row index of the new element. The index has to be in the range \f$[0..N-1]\f$.
// \param j The column index of the new element. The index has to be in the range \f$[0..N-1]\f$.
// \param value The value of the element to be appended.
// \param check \a true if the new value should be checked for default values, \a false if not.
// \return void
//
// This function both appends the element \f$ a_{ij} \f$ to the specified row/column and inserts
// its according counterpart \f$ a_{ji} \f$ into the symmetric matrix. Since element \f$ a_{ij} \f$
// is appended without any additional memory allocation, it is strictly necessary to keep the
// following preconditions in mind:
//
//  - the index of the new element must be strictly larger than the largest index of non-zero
//    elements in the specified row/column of the sparse matrix
//  - the current number of non-zero elements in the matrix must be smaller than the capacity
//    of the matrix
//
// Ignoring these preconditions might result in undefined behavior! The optional \a check
// parameter specifies whether the new value should be tested for a default value. If the new
// value is a default value (for instance 0 in case of an integral element type) the value is
// not appended. Per default the values are not tested.
//
// Although in addition to element \f$ a_{ij} \f$ a second element \f$ a_{ji} \f$ is inserted into
// the matrix, this function still provides the most efficient way to fill a symmetric matrix with
// values. However, in order to achieve maximum efficiency, the matrix has to be specifically
// prepared with reserve() calls:

   \code
   using blaze::CompressedMatrix;
   using blaze::SymmetricMatrix;
   using blaze::rowMajor;

   // Setup of the symmetric matrix
   //
   //       ( 0 1 3 )
   //   A = ( 1 2 0 )
   //       ( 3 0 0 )

   SymmetricMatrix< CompressedMatrix<double,rowMajor> > A( 3 );

   A.reserve( 5 );         // Reserving enough space for 5 non-zero elements
   A.reserve( 0, 2 );      // Reserving two non-zero elements in the first row
   A.reserve( 1, 2 );      // Reserving two non-zero elements in the second row
   A.reserve( 2, 1 );      // Reserving a single non-zero element in the third row
   A.append( 0, 1, 1.0 );  // Appending the value 1 at position (0,1) and (1,0)
   A.append( 1, 1, 2.0 );  // Appending the value 2 at position (1,1)
   A.append( 2, 0, 3.0 );  // Appending the value 3 at position (2,0) and (0,2)
   \endcode

// \b Note: Although append() does not allocate new memory, it still invalidates all iterators
// returned by the end() functions!
*/
template< typename MT >  // Type of the adapted sparse matrix
inline void SymmetricMatrix<MT,false,true>::append( size_t i, size_t j, const ElementType& value, bool check )
{
   matrix_.append( i, j, value, check );
   if( i != j && ( !check || !isDefault( value ) ) )
      matrix_.insert( j, i, value );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Finalizing the element insertion of a row/column.
//
// \param i The index of the row/column to be finalized \f$[0..N-1]\f$.
// \return void
//
// This function is part of the low-level interface to efficiently fill a matrix with elements.
// After completion of row/column \a i via the append() function, this function can be called to
// finalize row/column \a i and prepare the next row/column for insertion process via append().
//
// \b Note: Although finalize() does not allocate new memory, it still invalidates all iterators
// returned by the end() functions!
*/
template< typename MT >  // Type of the adapted sparse matrix
inline void SymmetricMatrix<MT,false,true>::finalize( size_t i )
{
   matrix_.trim( i );
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  EXPRESSION TEMPLATE EVALUATION FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns whether the matrix can alias with the given address \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this matrix, \a false if not.
//
// This function returns whether the given address can alias with the matrix. In contrast
// to the isAliased() function this function is allowed to use compile time expressions
// to optimize the evaluation.
*/
template< typename MT >     // Type of the adapted sparse matrix
template< typename Other >  // Data type of the foreign expression
inline bool SymmetricMatrix<MT,false,true>::canAlias( const Other* alias ) const
{
   return matrix_.canAlias( alias );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns whether the matrix is aliased with the given address \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this matrix, \a false if not.
//
// This function returns whether the given address is aliased with the matrix. In contrast
// to the conAlias() function this function is not allowed to use compile time expressions
// to optimize the evaluation.
*/
template< typename MT >     // Type of the adapted sparse matrix
template< typename Other >  // Data type of the foreign expression
inline bool SymmetricMatrix<MT,false,true>::isAliased( const Other* alias ) const
{
   return matrix_.isAliased( alias );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns whether the matrix can be used in SMP assignments.
//
// \return \a true in case the matrix can be used in SMP assignments, \a false if not.
//
// This function returns whether the matrix can be used in SMP assignments. In contrast to the
// \a smpAssignable member enumeration, which is based solely on compile time information, this
// function additionally provides runtime information (as for instance the current number of
// rows and/or columns of the matrix).
*/
template< typename MT >  // Type of the adapted sparse matrix
inline bool SymmetricMatrix<MT,false,true>::canSMPAssign() const
{
   return matrix_.canSMPAssign();
}
/*! \endcond */
//*************************************************************************************************

} // namespace blaze

#endif
