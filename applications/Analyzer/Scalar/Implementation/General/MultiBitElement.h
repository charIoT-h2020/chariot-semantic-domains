/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : implementation of general scalar elements
// File      : MultiBitElement.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a multibit template class providing all the operations on multibits.
//

#ifndef Analyzer_Scalar_Implementation_MultiBitElementH
#define Analyzer_Scalar_Implementation_MultiBitElementH

#include "Analyzer/Scalar/Implementation/ImplementationElement.h"
#include "Numerics/Integer.h"
#include "Numerics/Floating.h"

namespace Analyzer { namespace Scalar {

namespace Floating { namespace Implementation {

template <class TypeMultiTraits>
class TMultiFloatElement;
class MultiFloatElement;
   
namespace Details {

template <class TypeMultiTraits>
class TMultiFloatElement;
class MultiFloatElement;

}}} // end of namespace Floating::Implementation::Details

namespace MultiBit { namespace Implementation { namespace Details {

template <class TypeBase>
class TMultiBitImplementOperation : public TypeBase {
  private:
   typedef TypeBase inherited;
   typedef TMultiBitImplementOperation<TypeBase> thisType;

  public:
   TMultiBitImplementOperation() {}
   TMultiBitImplementOperation(int size) { inherited::assertSize(size); }
   TMultiBitImplementOperation(thisType&& source) = default;
   TMultiBitImplementOperation(const thisType& source) = default;
   int asInt() const { return inherited::getValue(); }

   void setSize(int newSize) { inherited::setSize(newSize); }
   int getSize() const { return inherited::getSize(); }
   virtual bool isMultiBit() const { return true; }

   thisType& clear() { inherited::clear(); return *this; }
   
   thisType& operator=(const thisType& source) = default;
   thisType& operator=(unsigned int value)
      {  return (thisType&) TypeBase::operator=(value); }

   ComparisonResult compareSigned(const thisType& source) const;
   ComparisonResult compareUnsigned(const thisType& source) const { return inherited::_compare(source); }
   ComparisonResult compare(const thisType& source) const { return compareUnsigned(source); }

   bool apply(const CastMultiBitOperation& operation, EvaluationEnvironment& env);
   bool apply(const ExtendOperation& operation, EvaluationEnvironment& env);
   bool apply(const ReduceOperation& operation, EvaluationEnvironment& env);
   bool apply(const BitSetOperation& operation, const thisType& source, EvaluationEnvironment& env);
   bool apply(const ConcatOperation&, const thisType& source, EvaluationEnvironment& env);

   bool apply(const PrevSignedAssignOperation&, EvaluationEnvironment& env);
   bool apply(const PrevUnsignedAssignOperation&, EvaluationEnvironment& env);
   bool apply(const NextSignedAssignOperation&, EvaluationEnvironment& env);
   bool apply(const NextUnsignedAssignOperation&, EvaluationEnvironment& env);
   bool apply(const PlusSignedAssignOperation&, const thisType& source, EvaluationEnvironment& env);
   bool apply(const PlusUnsignedAssignOperation&, const thisType& source, EvaluationEnvironment& env);
   bool apply(const PlusUnsignedWithSignedAssignOperation&, const thisType& source, EvaluationEnvironment& env);
   bool apply(const MinusSignedAssignOperation&, const thisType& source, EvaluationEnvironment& env);
   bool apply(const MinusUnsignedAssignOperation&, const thisType& source, EvaluationEnvironment& env);
   bool apply(const MinSignedAssignOperation&, const thisType& source, EvaluationEnvironment& env);
   bool apply(const MinUnsignedAssignOperation&, const thisType& source, EvaluationEnvironment& env);
   bool apply(const MaxSignedAssignOperation&, const thisType& source, EvaluationEnvironment& env);
   bool apply(const MaxUnsignedAssignOperation&, const thisType& source, EvaluationEnvironment& env);
   bool apply(const TimesSignedAssignOperation&, const thisType& source, EvaluationEnvironment& env);
   bool apply(const TimesUnsignedAssignOperation&, const thisType& source, EvaluationEnvironment& env);
   bool apply(const DivideSignedAssignOperation&, const thisType& source, EvaluationEnvironment& env);
   bool apply(const DivideUnsignedAssignOperation&, const thisType& source, EvaluationEnvironment& env);
   bool apply(const OppositeSignedAssignOperation&, EvaluationEnvironment& env);
   bool apply(const ModuloSignedAssignOperation&, const thisType& source, EvaluationEnvironment& env);
   bool apply(const ModuloUnsignedAssignOperation&, const thisType& source, EvaluationEnvironment& env);
   bool apply(const BitOrAssignOperation&, const thisType& source, EvaluationEnvironment& env);
   bool apply(const BitAndAssignOperation&, const thisType& source, EvaluationEnvironment& env);
   bool apply(const BitExclusiveOrAssignOperation&, const thisType& source, EvaluationEnvironment& env);
   bool apply(const BitNegateAssignOperation&, EvaluationEnvironment& env);
   bool apply(const LeftShiftAssignOperation&, const thisType& source, EvaluationEnvironment& env);
   bool apply(const LogicalRightShiftAssignOperation&, const thisType& source, EvaluationEnvironment& env);
   bool apply(const ArithmeticRightShiftAssignOperation&, const thisType& source, EvaluationEnvironment& env);
   bool apply(const LeftRotateAssignOperation&, int rotate, EvaluationEnvironment& env);
   bool apply(const RightRotateAssignOperation&, int rotate, EvaluationEnvironment& env);
};

template <class TypeBase>
inline ComparisonResult
TMultiBitImplementOperation<TypeBase>::compareSigned(const thisType& source) const {
   bool isNegative = inherited::cbitArray(inherited::getSize()-1);
   bool isSourceNegative = source.cbitArray(inherited::getSize()-1);
   ComparisonResult result = CREqual;
   if (isNegative) {
      if (isSourceNegative) {
         result = inherited::_compare(source);
         if (result != CRNonComparable)
            result = (ComparisonResult) -result;
      }
      else
         result = CRLess;
   }
   else if (isSourceNegative)
      result = CRGreater;
   else
      result = inherited::_compare(source);
   return result;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const ExtendOperation& operation, EvaluationEnvironment& env) {
   int oldSize = getSize();
   inherited::setSize(oldSize + operation.getExtension());
   if ((operation.getType() == Operation::TExtendWithSign) && inherited::bitArray(oldSize-1)) {
      inherited merge;
      ((thisType&) merge).setSize(getSize());
      merge.neg(operation.getExtension());
      merge <<= oldSize;
      inherited::operator|=(merge);
   };
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const ReduceOperation& operation, EvaluationEnvironment& env) {
   if (operation.getLowBit() > 0)
      inherited::operator>>=(operation.getLowBit());
   inherited intersect;
   ((thisType&) intersect).setSize(getSize());
   intersect.neg(operation.getHighBit() - operation.getLowBit()+1);
   inherited::operator&=(intersect);
   inherited::setSize(operation.getHighBit() - operation.getLowBit()+1);
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const BitSetOperation& operation, const thisType& source, EvaluationEnvironment& env) {
   inherited clearMask, mergeMask(source);
   ((thisType&) clearMask).setSize(getSize());
   ((thisType&) mergeMask).setSize(getSize());
   clearMask.neg(operation.getHighBit()-operation.getLowBit()+1);
   if (operation.getLowBit() > 0) {
      mergeMask <<= operation.getLowBit();
      clearMask <<= operation.getLowBit();
   };
   mergeMask &= clearMask;
   clearMask.neg();
   inherited::operator&=(clearMask);
   inherited::operator|=(mergeMask);
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const ConcatOperation&, const thisType& source, EvaluationEnvironment& env) {
   inherited::assertSize(getSize()+source.getSize());
   inherited::operator<<=(source.getSize());
   inherited::operator|=(source);
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const MinSignedAssignOperation&, const thisType& source, EvaluationEnvironment& env) {
   ComparisonResult result = compareSigned(source);
   if (result == CRGreater)
      inherited::_assign(source);
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const MinUnsignedAssignOperation&, const thisType& source, EvaluationEnvironment& env) {
   ComparisonResult result = compareUnsigned(source);
   if (result == CRGreater)
      inherited::_assign(source);
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const MaxSignedAssignOperation&, const thisType& source, EvaluationEnvironment& env) {
   ComparisonResult result = compareSigned(source);
   if (result == CRLess)
      inherited::_assign(source);
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const MaxUnsignedAssignOperation&, const thisType& source, EvaluationEnvironment& env) {
   ComparisonResult result = compareUnsigned(source);
   if (result == CRLess)
      inherited::_assign(source);
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const PrevSignedAssignOperation&, EvaluationEnvironment& env) {
   bool isNegative = inherited::cbitArray(inherited::getSize()-1);
   inherited::dec();
   if (isNegative && !inherited::cbitArray(inherited::getSize()-1)) {
      env.setNegativeOverflow();
      if (env.doesStopErrorStates())
         env.setEmpty();
   };
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const PrevUnsignedAssignOperation&, EvaluationEnvironment& env) {
   if (inherited::dec().hasCarry()) {
      env.setNegativeOverflow();
      if (env.doesStopErrorStates())
         env.setEmpty();
   };
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const NextSignedAssignOperation&, EvaluationEnvironment& env) {
   inherited::inc();
   if (inherited::cbitArray(inherited::getSize()-1) && inherited::hasZero(inherited::getSize()-1)) {
      env.setPositiveOverflow();
      if (env.doesStopErrorStates())
         env.setEmpty();
   };
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const NextUnsignedAssignOperation&, EvaluationEnvironment& env) {
   if (inherited::inc().hasCarry()) {
      env.setPositiveOverflow();
      if (env.doesStopErrorStates())
         env.setEmpty();
   };
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const PlusSignedAssignOperation&, const thisType& source, EvaluationEnvironment& env) {
   bool isNegative = inherited::cbitArray(inherited::getSize()-1);
   bool isSourceNegative = source.cbitArray(inherited::getSize()-1);
   if (inherited::add(source).hasCarry()) {
      if (isNegative && isSourceNegative && !inherited::cbitArray(getSize()-1)) {
         env.setNegativeOverflow();
         if (env.doesStopErrorStates())
            env.setEmpty();
      };
   }
   else if (!isNegative && !isSourceNegative && inherited::cbitArray(inherited::getSize()-1)) {
      env.setPositiveOverflow();
      if (env.doesStopErrorStates())
         env.setEmpty();
   };
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const PlusUnsignedAssignOperation&,
      const thisType& source, EvaluationEnvironment& env) {
   AssumeCondition(inherited::getSize() == source.inherited::getSize())
   if (inherited::add(source).hasCarry()) {
      env.setPositiveOverflow();
      if (env.doesStopErrorStates())
         env.setEmpty();
   };
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const PlusUnsignedWithSignedAssignOperation&,
      const thisType& source, EvaluationEnvironment& env) {
   bool isSourceNegative = source.cbitArray(inherited::getSize()-1);
   inherited copySource(source);
   if (isSourceNegative) {
      copySource.neg().inc();
      if (inherited::sub(copySource).hasCarry()) {
         env.setNegativeOverflow();
         if (env.doesStopErrorStates())
            env.setEmpty();
      };
   };
   if (inherited::add(copySource).hasCarry()) {
      env.setPositiveOverflow();
      if (env.doesStopErrorStates())
         env.setEmpty();
   };
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const MinusSignedAssignOperation&,
      const thisType& source, EvaluationEnvironment& env) {
   bool isNegative = inherited::cbitArray(inherited::getSize()-1);
   bool isSourceNegative = source.cbitArray(inherited::getSize()-1);
   if (inherited::sub(source).hasCarry()) {
      if (!isNegative && isSourceNegative && inherited::cbitArray(getSize()-1)) {
         env.setPositiveOverflow();
         if (env.doesStopErrorStates())
            env.setEmpty();
      };
   }
   else if (isNegative && !isSourceNegative && !inherited::cbitArray(inherited::getSize()-1)) {
      env.setNegativeOverflow();
      if (env.doesStopErrorStates())
         env.setEmpty();
   };
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const MinusUnsignedAssignOperation&,
      const thisType& source, EvaluationEnvironment& env) {
   if (inherited::sub(source).hasCarry()) {
      env.setNegativeOverflow();
      if (env.doesStopErrorStates())
         env.setEmpty();
   };
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const TimesSignedAssignOperation&,
      const thisType& source, EvaluationEnvironment& env) {
   bool isNegative = inherited::cbitArray(inherited::getSize()-1);
   bool isSourceNegative = source.cbitArray(inherited::getSize()-1);
   typename inherited::MultResult result;
   result.setSize(2*inherited::getSize());
   inherited sourceContent = source;
   if (isSourceNegative)
      sourceContent.neg().inc();
   if (isNegative)
      inherited::neg().inc();
   inherited::mult(sourceContent, result);
   for (int index = (inherited::getSize()-1) / (8*sizeof(unsigned int)); index >= 0; --index)
      inherited::array(index) = result[index];
   inherited::normalizeLastCell();
   if (isNegative != isSourceNegative)
      inherited::neg().inc();
   bool isResultNegative = inherited::cbitArray(inherited::getSize()-1);

   typename inherited::EnhancedMultResult&
     eresult = (typename inherited::EnhancedMultResult&) result;
   if (!(eresult >>= inherited::getSize()).isZero()
         || ((isNegative != isSourceNegative) != isResultNegative)) {
      if (isNegative != isSourceNegative)
         env.setNegativeOverflow();
      else
         env.setPositiveOverflow();
      if (env.doesStopErrorStates())
         env.setEmpty();
   };
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const TimesUnsignedAssignOperation&,
      const thisType& source, EvaluationEnvironment& env) {
   typename inherited::MultResult result;
   result.setSize(2*inherited::getSize());
   inherited::mult(source, result);
   for (int index = (inherited::getSize()-1) / (8*sizeof(unsigned int)); index >= 0; --index)
      inherited::array(index) = result[index];
   inherited::normalizeLastCell();
   typename inherited::EnhancedMultResult&
     eresult = (typename inherited::EnhancedMultResult&) result;
   eresult >>= inherited::getSize();
   if (!eresult.isZero()) {
      env.setPositiveOverflow();
      if (env.doesStopErrorStates())
         env.setEmpty();
   };
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const OppositeSignedAssignOperation&,
      EvaluationEnvironment& env) {
   bool isNegative = inherited::cbitArray(inherited::getSize()-1);
   inherited::neg().inc();
   if (isNegative && inherited::cbitArray(inherited::getSize()-1)) {
      env.setPositiveOverflow();
      if (env.doesStopErrorStates())
         env.setEmpty();
   };
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const BitOrAssignOperation&, const thisType& source, EvaluationEnvironment& env) {
   inherited::operator|=(source);
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const BitAndAssignOperation&, const thisType& source, EvaluationEnvironment& env) {
   inherited::operator&=(source);
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const BitExclusiveOrAssignOperation&, const thisType& source, EvaluationEnvironment& env) {
   inherited::operator^=(source);
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const BitNegateAssignOperation&, EvaluationEnvironment& env) {
   inherited::neg();
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const LeftShiftAssignOperation&, const thisType& source, EvaluationEnvironment& env) {
   if (!inherited::isZero()) {
      inherited size;
      size.setSize(inherited::getSize());
      size[0] = inherited::getSize();
      if (source.inherited::_compare(size) >= CREqual)
         env.setPositiveOverflow();
      else {
         if (!(inherited(*this) >>= (inherited::getSize()-source.getValue())).isZero())
            env.setPositiveOverflow();
         inherited::operator<<=(source.getValue());
      };
   };
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const LogicalRightShiftAssignOperation&, const thisType& source, EvaluationEnvironment& env) {
   inherited size;
   size.setSize(inherited::getSize());
   size[0] = inherited::getSize();
   if (source.inherited::_compare(size) >= CREqual)
      inherited::clear();
   else
      inherited::operator>>=(source.getValue());
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const ArithmeticRightShiftAssignOperation&, const thisType& source, EvaluationEnvironment& env) {
   bool isNegative = inherited::cbitArray(inherited::getSize()-1);
   inherited size;
   size.setSize(inherited::getSize());
   size[0] = inherited::getSize();
   if (source.inherited::_compare(size) >= CREqual) {
      inherited::clear();
      if (isNegative)
         inherited::neg();
   }
   else {
      inherited::operator>>=(source.getValue());
      if (isNegative) {
         inherited ones;
         ones.setSize(inherited::getSize());
         inherited::operator|=(ones.neg() <<= (inherited::getSize()-source.getValue()));
      };
   };
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const LeftRotateAssignOperation&, int rotate, EvaluationEnvironment& env) {
   AssumeCondition((rotate >= 0) && (rotate < inherited::getSize()))
   inherited leftBits(*this);
   leftBits >>= (inherited::getSize()-rotate);
   inherited::operator<<=(rotate);
   inherited::operator|=(leftBits);
   return true;
}

template <class TypeBase>
inline bool
TMultiBitImplementOperation<TypeBase>::apply(const RightRotateAssignOperation&, int rotate, EvaluationEnvironment& env) {
   AssumeCondition((rotate >= 0) && (rotate < inherited::getSize()))
   inherited leftBits(*this);
   leftBits <<= (inherited::getSize()-rotate);
   inherited::operator>>=(rotate);
   inherited::operator|=(leftBits);
   return true;
}

class BaseMultiBitElement : public Numerics::BigInteger {
  private:
   typedef BaseMultiBitElement thisType;
   typedef Numerics::BigInteger inherited;
   int uBitSize;

  protected:
   BaseMultiBitElement(const inherited& source, int bitSize)
      :  inherited(source), uBitSize(bitSize) {}

  public:
   BaseMultiBitElement() : uBitSize(0) {}
   BaseMultiBitElement(const BaseMultiBitElement& source) = default;
   BaseMultiBitElement(BaseMultiBitElement&& source) = default;
   BaseMultiBitElement& operator=(Numerics::DInteger::ExtensibleCellIntegerTraits&& source)
      {  inherited::assignCells(source); return *this; }
   BaseMultiBitElement& operator=(const Numerics::DInteger::ExtensibleCellIntegerTraits& source)
      {  inherited::assignCells(source); return *this; }
   BaseMultiBitElement& operator=(BaseMultiBitElement&& source) = default;
   BaseMultiBitElement& operator=(const BaseMultiBitElement& source) = default;
   BaseMultiBitElement& operator=(unsigned int value) { return (BaseMultiBitElement&) inherited::operator=(value); }
   virtual bool isValid() const { return inherited::isValid() && (uBitSize > 0); }
   
   typedef Implementation::FormatParameters FormatParameters;
   void write(STG::IOObject::OSBase& out, const FormatParameters& params) const
      {  implementation().write(out, params); }
   void read(STG::IOObject::ISBase& in, const FormatParameters& params)
      {  implementation().read(in, params); }
   
   class BitSizeImplementation : public Implementation {
     private:
      typedef Implementation inherited;
      
     public:
      BitSizeImplementation() : inherited() {}
      BitSizeImplementation(unsigned int value) : inherited(value) {}
      BitSizeImplementation(const Implementation& source) : inherited(source) {}
      BitSizeImplementation& operator=(const Implementation& source) { return (BitSizeImplementation&) inherited::operator=(source); }
      BitSizeImplementation& operator=(unsigned int value)
         {  return (BitSizeImplementation&) inherited::operator=(value); }
      
      void adjustSize(int newSize) { inherited::adjustSize((newSize+sizeof(unsigned)*8-1)/(8*sizeof(unsigned))); }
      void assertSize(int newSize) { inherited::assertSize((newSize+sizeof(unsigned)*8-1)/(8*sizeof(unsigned))); }
      void setSize(int exactSize) { inherited::setSize((exactSize+sizeof(unsigned)*8-1)/(8*sizeof(unsigned))); }
   };
   typedef BitSizeImplementation MultResult;
   typedef BitSizeImplementation EnhancedMultResult;
   void assumeAcceptSize(int size) const {}
   thisType& neg() { return (thisType&) inherited::neg(uBitSize); }
   thisType& neg(int shift) { AssumeCondition(shift <= uBitSize) return (thisType&) inherited::neg(shift); }
   typedef inherited::Implementation::Carry Carry;

   const int& getSize() const { return uBitSize; }
   void setSize(int newSize) { uBitSize = newSize; normalize(); }
   void adjustSize(int newSize) { uBitSize = newSize; normalize(); }
   void assertSize(int newSize) { uBitSize = newSize; normalize(); }
   void clearSize() { uBitSize = 0; inherited::clear(); }
   void clear() { inherited::clear(); }

   Carry dec()
      {  Carry result;
         if (isZero()) {
            neg();
            result.setCarry();
         }
         else
            inherited::dec();
         return result;
      }
   Carry inc()
      {  Carry result;
         inherited::inc();
         if (cbitArray(uBitSize)) {
            setFalseBitArray(uBitSize);
            result.setCarry();
         }
         return result;
      }
   Carry add(const inherited& source) { return add(thisType(source, uBitSize)); }
   Carry sub(const inherited& source) { return sub(thisType(source, uBitSize)); }
   Carry add(const thisType& source)
      {  Carry result;
         inherited::add(source);
         if (cbitArray(uBitSize)) {
            setFalseBitArray(uBitSize);
            result.setCarry();
         }
         return result;
      }
   Carry sub(const thisType& source)
      {  Carry result;
         if (inherited::_compare(source) == CRLess) {
            neg().inc();
            inherited::add(source);
            setFalseBitArray(uBitSize);
            neg().inc();
            result.setCarry();
         }
         else
            inherited::sub(source);
         return result;
      }
   void mult(const thisType& source, MultResult& result)
      {  result = inherited::mult(source)->implementation();
         normalize();
      }
   int lastCellIndex() const { return (uBitSize-1) / (8*sizeof(unsigned int)); }
   int lastCellSize() const { return (uBitSize+8*sizeof(unsigned int)-1) % (8*sizeof(unsigned int)) +1; }
   int getMultResultCellSize(const EnhancedMultResult&) const { return 2*uBitSize-1; }
   int getCellSize() const { return (uBitSize-1) / (8*sizeof(unsigned int))+1; }
   Carry multAssign(unsigned int value)
      {  Carry result = implementation().multAssign(value);
         if (((uBitSize % (8*sizeof(unsigned int))) != 0)) {
            if (lastCellIndex() < implementation().getSize()-1) {
               AssumeCondition(!result.hasCarry())
               result.carry() = implementation()[lastCellIndex()+1];
            };
            int lastLogBase2 = lastCellSize();
            result.carry() <<= (8*sizeof(unsigned int) - lastLogBase2);
            if (lastLogBase2 < (int) (8*sizeof(unsigned int)))
               result.carry() |= (implementation()[lastCellIndex()] >> lastLogBase2);
         };
         normalize();
         return result;    
      }
   Carry plusAssign(const inherited& source)
      {  Carry result = implementation().add(source.implementation());
         if (!result.hasCarry() && ((uBitSize % (8*sizeof(unsigned int))) != 0)) {
            int lastLogBase2 = lastCellSize();
            result.carry() = (implementation()[lastCellIndex()] >> lastLogBase2);
            if (lastLogBase2 < (int) (sizeof(unsigned)*8))
               implementation()[lastCellIndex()] &= ~(~0U << lastLogBase2);
         };
         return result;
      }
   Carry plusAssign(unsigned int value)
      {  return plusAssign(thisType(inherited(value), uBitSize)); }
   typedef inherited::NormalizedDivisionResult NormalizedDivisionResult;
   typedef Implementation::DivisionResult DivisionResult;
   void divNormalized(const thisType& source, NormalizedDivisionResult& result) const
      {  inherited::divNormalized(source, uBitSize, result); }
   typedef BitSizeImplementation QuotientResult;
   typedef BitSizeImplementation RemainderResult;
   typedef BitSizeImplementation EnhancedRemainderResult;
   void div(const thisType& source, DivisionResult& result) const
      {  implementation().div(source.implementation(), result); }
   void normalize()
      {  implementation().setBitSize(uBitSize);
         int lastLogBase2 = lastCellSize();
         if (lastLogBase2 < (int) (8*sizeof(unsigned)))
            implementation()[lastCellIndex()] &= ~(~0U << lastLogBase2);
         implementation().normalize();
      }
   void normalizeLastCell() { normalize(); }
   thisType& operator+=(const thisType& source) { add(source); return *this; }
   thisType& operator-=(const thisType& source) { sub(source); return *this; }
   thisType& operator*=(const thisType& source)
      {  inherited::multAssign(source);
         normalize();
         return *this;
      }
   thisType& operator/=(const thisType& source)
      {  inherited::divAssign(source);
         normalize();
         return *this;
      }
   thisType& operator<<=(int shift)
      {  inherited::operator<<=(shift);
         normalize();
         return *this;
      }
};

template <class TypeMultiTraits>
class TMultiBitElement;

class MultiBitElement : public TMultiBitImplementOperation<BaseMultiBitElement> {
  private:
   typedef MultiBitElement thisType;
   typedef TMultiBitImplementOperation<BaseMultiBitElement> inherited;

  public:
   MultiBitElement(int size) : inherited(size) {}
   MultiBitElement(MultiBitElement&& source) : inherited(source) {}
   MultiBitElement(const MultiBitElement& source) : inherited(source) {}
   MultiBitElement& operator=(MultiBitElement&& source) = default;
   MultiBitElement& operator=(const MultiBitElement& source) = default;
   DefineCopy(MultiBitElement)
   DDefineAssign(MultiBitElement)
   void assign(const Scalar::Implementation::VirtualElement& source);

   enum IntNature { INSigned, INUnsigned, INPointer };
   typedef Numerics::DDouble::Access::ReadParameters ReadFloatParameters;
   typedef Numerics::DDouble::Access::WriteParameters WriteFloatParameters;
   MultiBitElement(int size, IntNature nature,
         const Floating::Implementation::Details::MultiFloatElement& floatValue, ReadFloatParameters& params);
   template <class TypeMultiTraits>
   MultiBitElement(const TMultiBitElement<TypeMultiTraits>& source);

   typedef Scalar::Floating::Implementation::MultiFloatElement MultiFloatImplementation;
   
   thisType& operator=(unsigned int value)
      {  return (thisType&) inherited::operator=(value); }

   void write(STG::IOObject::OSBase& out, const STG::IOObject::FormatParameters& params) const
      {  inherited::Implementation::FormatParameters copyParams;
         inherited::implementation().write(out, copyParams.setFullHexaDecimal(inherited::getSize()));
      }
   typedef Implementation::FormatParameters FormatParameters;
   void write(STG::IOObject::OSBase& out, const FormatParameters& params) const
      {  inherited::write(out, params); }
};

} // end of namespace Details

/*******************************************/
/* Definition of the class MultiBitElement */
/*******************************************/

class MultiBitElement : public Scalar::Implementation::ImplementationElement, public Details::MultiBitElement {
  private:
   typedef MultiBitElement thisType;
   typedef Scalar::Implementation::ImplementationElement inherited;
   typedef Details::MultiBitElement inheritedImplementation;

  protected:
   virtual ComparisonResult _compare(const EnhancedObject& asource) const override
      {  ComparisonResult result = inherited::_compare(asource);
         if (result == CREqual) {
            const thisType& source = castFromCopyHandler(asource);
            result = inheritedImplementation::_compare((const inheritedImplementation&) source);
         };
         return result;
      }

  public:
   MultiBitElement(int size) : inheritedImplementation(size) { setMultiBit(); }
   MultiBitElement(MultiBitElement&& source) = default;
   MultiBitElement(const MultiBitElement& source) = default;
   MultiBitElement& operator=(MultiBitElement&& source) = default;
   MultiBitElement& operator=(const MultiBitElement& source) = default;

   MultiBitElement(int size, IntNature nature,
         const Floating::Implementation::Details::MultiFloatElement& floatValue, ReadFloatParameters& params)
      :  inheritedImplementation(size, nature, floatValue, params) { setMultiBit(); }
   template <class TypeMultiTraits>
   MultiBitElement(const Details::TMultiBitElement<TypeMultiTraits>& source)
      :  inheritedImplementation(source) { setMultiBit(); }
   DefineCopy(MultiBitElement)
   DDefineAssign(MultiBitElement)
   DCompare(MultiBitElement)

   StaticInheritConversions(MultiBitElement, inherited)
   virtual bool isValid() const override { return inherited::isValid() && inheritedImplementation::isValid(); }
   MultiBitElement& operator++() { return (MultiBitElement&) Details::MultiBitElement::operator++(); }
   MultiBitElement& operator--() { return (MultiBitElement&) Details::MultiBitElement::operator--(); }
   MultiBitElement& operator+=(const MultiBitElement& source) { return (MultiBitElement&) Details::MultiBitElement::operator+=(source); }
   MultiBitElement& operator-=(const MultiBitElement& source) { return (MultiBitElement&) Details::MultiBitElement::operator-=(source); }
   MultiBitElement& operator*=(const MultiBitElement& source) { return (MultiBitElement&) Details::MultiBitElement::operator*=(source); }
   MultiBitElement& operator/=(const MultiBitElement& source) { return (MultiBitElement&) Details::MultiBitElement::operator/=(source); }
   MultiBitElement& operator|=(const MultiBitElement& source) { return (MultiBitElement&) Details::MultiBitElement::operator|=(source); }
   MultiBitElement& operator&=(const MultiBitElement& source) { return (MultiBitElement&) Details::MultiBitElement::operator&=(source); }

   typedef inheritedImplementation InheritedImplementation;
   
   typedef Numerics::BigInteger SimulatedType;
   void assign(const Scalar::Implementation::VirtualElement& source) { inheritedImplementation::assign(source); }
   void assignPart(const MultiBitElement& source) { MultiBitElement::_assign(castToCopyHandler(source)); }
   ComparisonResult comparePart(const MultiBitElement& source) const { return MultiBitElement::_compare(castToCopyHandler(source)); }
   thisType& operator=(unsigned int value)
      {  return (thisType&) inheritedImplementation::operator=(value); }

   void write(STG::IOObject::OSBase& out, const STG::IOObject::FormatParameters& params) const
      {  inheritedImplementation::write(out, params);
      }
   void write(STG::IOObject::OSBase& out, const inheritedImplementation::FormatParameters& params) const
      {  inheritedImplementation::write(out, params); }
};

/* Definition of the inline methods of the class MultiBitElement */

namespace Details {

template <int UMaxSize>
class TMaxIntegerTraits : public Numerics::DInteger::TIntegerTraits<UMaxSize> {
  private:
   typedef Numerics::DInteger::TIntegerTraits<UMaxSize> inherited;
   typedef TMaxIntegerTraits<UMaxSize> thisType;

  protected:
   int uBitSize;

  public:
   TMaxIntegerTraits() : uBitSize(0) {}
   TMaxIntegerTraits(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   thisType& operator=(unsigned int source)
      {  AssumeCondition(uBitSize > 0)
         return (thisType&) inherited::operator=(source);
      }

   int lastCellIndex() const { AssumeCondition(uBitSize > 0) return (uBitSize-1)/(8*sizeof(unsigned int)); }
   int lastCellSize() const { AssumeCondition(uBitSize > 0) return ((uBitSize-1) % (8*sizeof(unsigned int)) +1); }
   bool isComplete() const
      {  AssumeCondition(uBitSize > 0)
         return ((uBitSize % (8*sizeof(unsigned int))) == 0) && (UMaxSize <= uBitSize); /* (UMaxSize == uBitSize / (8*sizeof(unsigned int))); */
      }
   void normalizeLastCell()
      {  if (lastCellSize() < (int) (8*sizeof(unsigned int)))
            inherited::cellTraits()[lastCellIndex()] &= ~(~0U << lastCellSize());
      }
   void normalize()
      {  if (lastCellSize() < (int) (8*sizeof(unsigned int)))
            inherited::cellTraits()[lastCellIndex()] &= ~(~0U << lastCellSize());
         for (int index = lastCellIndex()+1; index < inherited::cellTraits().getSize(); ++index)
            inherited::cellTraits()[index] = 0U;
      }
   
   typedef unsigned int& ArrayProperty;
   ArrayProperty array(int index) { return inherited::array(index); }
   unsigned int array(int index) const { return inherited::array(index); }
   unsigned int carray(int index) const { return inherited::carray(index); }
   ArrayProperty operator[](int index) { return inherited::operator[](index); }
   unsigned int operator[](int index) const { return inherited::operator[](index); }
   int getSize() const { return uBitSize; }
   int getCellSize() const { return (uBitSize + 8*sizeof(unsigned int) - 1)/(8*sizeof(unsigned int)); }

   typedef TMaxIntegerTraits<2*UMaxSize> MultResult;
   typedef TMaxIntegerTraits<UMaxSize> QuotientResult;
   typedef TMaxIntegerTraits<UMaxSize> RemainderResult;
   typedef TMaxIntegerTraits<UMaxSize+1> NormalizedRemainderResult;
   void assertSize(int newSize)
      {  if (uBitSize < newSize)
            uBitSize = newSize;
      }
   void setSize(int newSize) { AssumeCondition(UMaxSize >= newSize) uBitSize = newSize; }
   void setBitSize(int newSize)
      {  AssumeCondition(UMaxSize >= newSize)
         inherited::assertSize(newSize);
         uBitSize = newSize;
      }
   void setCellSize(int newCellSize)
      {  AssumeCondition(newCellSize == (uBitSize + 8*sizeof(unsigned int)-1)/(8*sizeof(unsigned int))); }
};

class MultiTraitsContract {
  public:
   static const int BitSize = 64;
   static const int BitSizeMantissa = 95;
   static const int BitSizeExponent = 32;
};

template <class TypeMultiTraits>
class TBaseMultiBitElement : public Numerics::TBigInt<TMaxIntegerTraits<TypeMultiTraits::BitSize> >
{ private:
   typedef Numerics::TBigInt<TMaxIntegerTraits<TypeMultiTraits::BitSize> > inherited;
   typedef TBaseMultiBitElement<TypeMultiTraits> thisType;

  public:
   typedef Numerics::TBigInt<TMaxIntegerTraits<TypeMultiTraits::BitSize> > SimulatedType;
   typedef typename inherited::BitArray BitArray;
   BitArray bitArray(int index) { return inherited::bitArray(index); }
   bool bitArray(int index) const { return inherited::bitArray(index); }
   bool cbitArray(int index) const { return bitArray(index); }
   void setBitArray(int index, bool value) { inherited::setBitArray(index, value); }

   typedef typename inherited::ArrayProperty ArrayProperty;
   unsigned int operator[](int index) const { return inherited::operator[](index); }
   ArrayProperty operator[](int index) { return inherited::operator[](index); }
   unsigned int array(int index) const { return inherited::array(index); }
   unsigned int carray(int index) const { return array(index); }
   ArrayProperty array(int index) { return inherited::array(index); }
   bool operator<(const thisType& source) const { return inherited::compare(source) == CRLess; }
   bool operator>(const thisType& source) const { return inherited::compare(source) == CRGreater; }
   bool operator<=(const thisType& source) const
      {  typename inherited::ComparisonResult result = inherited::compare(source);
         return (result == CRLess) || (result == CREqual);
      }
   bool operator>=(const thisType& source) const
      {  ComparisonResult result = inherited::compare(source);
         return (result == CRGreater) || (result == CREqual);
      }
   bool operator==(const thisType& source) const { return inherited::compare(source) == CREqual; }
   bool operator!=(const thisType& source) const
      {  ComparisonResult result = inherited::compare(source);
         return (result == CRGreater) || (result == CRLess);
      }
   thisType& operator<<=(int shift) { return (thisType&) inherited::operator<<=(shift); }
   thisType& operator>>=(int shift) { return (thisType&) inherited::operator>>=(shift); }
   thisType& operator|=(const thisType& source) { return (thisType&) inherited::operator|=(source); }
   thisType& operator^=(const thisType& source) { return (thisType&) inherited::operator^=(source); }
   thisType& operator&=(const thisType& source) { return (thisType&) inherited::operator&=(source); }
   thisType& neg() { return (thisType&) inherited::neg(); }
   thisType& neg(int shift) { return (thisType&) inherited::neg(shift); }
   
   void normalize() { inherited::normalize(); }
   int lastCellIndex() const { return inherited::lastCellIndex(); }
   typedef typename inherited::Carry Carry;
   Carry add(const thisType& source) { return inherited::add(source); }
   Carry sub(const thisType& source) { return inherited::sub(source); }
   Carry plusAssign(const thisType& source) { return inherited::plusAssign(source); }
   Carry minusAssign(const thisType& source) { return inherited::minusAssign(source); }
   Carry inc() { return inherited::inc(); }
   Carry dec() { return inherited::dec(); }
   int getCellSize() const { return inherited::getCellSize(); }

   thisType& operator+=(const thisType& source) { return (thisType&) inherited::operator+=(source); }
   thisType& operator-=(const thisType& source) { return (thisType&) inherited::operator-=(source); }
   thisType& operator--() { return (thisType&) inherited::operator--(); }
   thisType& operator++() { return (thisType&) inherited::operator++(); }

   Carry multAssign(unsigned int source) { return inherited::multAssign(source); }
   typedef typename inherited::MultResult MultResult;
   void mult(const thisType& source, MultResult& result) const { inherited::mult(source, result); }
   thisType& operator*=(const thisType& source) { return (thisType&) inherited::operator*=(source); }
   thisType& operator*=(unsigned int source) { return (thisType&) inherited::operator*=(source); }

   typedef typename inherited::DivisionResult DivisionResult;
   typedef typename inherited::NormalizedDivisionResult NormalizedDivisionResult;
   typedef typename inherited::AtomicDivisionResult AtomicDivisionResult;

   void div(const thisType& source, DivisionResult& result) const { inherited::div(source, result); }
   void divNormalized(const thisType& source, NormalizedDivisionResult& result) const { inherited::divNormalized(source, result); }
   AtomicDivisionResult divAssign(unsigned int source) { return inherited::divAssign(source); }

   thisType& operator/=(const thisType& source) { return (thisType&) inherited::operator/=(source); }
   thisType& operator/=(unsigned int source) { return (thisType&) inherited::operator/=(source); }
   thisType& operator%=(const thisType& source) { return (thisType&) inherited::operator%=(source); }

   unsigned int log_base_2() const { return inherited::log_base_2(); }
   unsigned int getValue() const { return inherited::getValue(); }
   bool isAtomic() const { return inherited::isAtomic(); }
   bool isZero() const { return inherited::isZero(); }
   bool hasZero(int shift) const { return inherited::hasZero(shift); }
   void assertSize(int newSize) { inherited::assertSize(newSize); }
   void clear() { inherited::clear(); }
   void swap(thisType& source) { inherited::swap(source); }

  protected:
   ComparisonResult _compare(const thisType& source) const { return inherited::compare(source); }

  public:
   TBaseMultiBitElement() {}
   TBaseMultiBitElement(int size) { inherited::assertSize(size); }
   TBaseMultiBitElement(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   thisType& operator=(unsigned int value) { return (thisType&) inherited::operator=(value); }

   void assumeAcceptSize(int size) const { AssumeCondition(size <= TypeMultiTraits::BitSize) }
   typedef Numerics::TBigInt<TMaxIntegerTraits<2*TypeMultiTraits::BitSize> > EnhancedMultResult;

   void setSize(int newSize) { inherited::setSize(newSize); }
   int getSize() const { return inherited::getSize(); }
   int asInt() const { return inherited::getValue(); }
};

template <class TypeMultiTraits>
class TMultiBitElement : public Details::TMultiBitImplementOperation<TBaseMultiBitElement<TypeMultiTraits> > {
  private:
   typedef Details::TMultiBitImplementOperation<TBaseMultiBitElement<TypeMultiTraits> > inherited;
   typedef TMultiBitElement<TypeMultiTraits> thisType;

  public:
   typedef Numerics::DDouble::Access::ReadParameters ReadFloatParameters;
   typedef Numerics::DDouble::Access::WriteParameters WriteFloatParameters;

  protected:
   ComparisonResult _compare(const thisType& source) const { return inherited::_compare(source); }

  public:
   TMultiBitElement(int size) : inherited(size) {}
   enum IntNature { INSigned, INUnsigned, INPointer };
   TMultiBitElement(int size, IntNature nature,
         const Floating::Implementation::Details::TMultiFloatElement<TypeMultiTraits>& floatValue,
         ReadFloatParameters& params);
   TMultiBitElement(const thisType& source) = default;
   TMultiBitElement(const MultiBitElement& source);
   thisType& operator=(const thisType& source) = default;
   thisType& operator=(unsigned int value)
      {  return (thisType&) inherited::operator=(value); }

   typedef Scalar::Floating::Implementation::TMultiFloatElement<TypeMultiTraits> MultiFloatImplementation;
   
   void assign(const Scalar::Implementation::VirtualElement& source);
   void assign(const thisType& source) { inherited::operator=(source); }

   typedef typename inherited::FormatParameters FormatParameters;
   void write(STG::IOObject::OSBase& out, const STG::IOObject::FormatParameters& params) const
      {  FormatParameters copyParams;
         inherited::write(out, copyParams.setFullHexaDecimal(inherited::getSize()));
      }
   void write(STG::IOObject::OSBase& out, const FormatParameters& params) const
      {  inherited::write(out, params); }
};

} // end of namespace Details

template <class TypeMultiTraits>
class TMultiBitElement : public Scalar::Implementation::ImplementationElement,
                         public Details::TMultiBitElement<TypeMultiTraits> {
  private:
   typedef Scalar::Implementation::ImplementationElement inherited;
   typedef Details::TMultiBitElement<TypeMultiTraits> inheritedImplementation;
   typedef TMultiBitElement<TypeMultiTraits> thisType;

  protected:
   virtual ComparisonResult _compare(const EnhancedObject& asource) const
      {  ComparisonResult result = inherited::_compare(asource);
         const thisType& source = (const thisType&) asource;
         return (result == CREqual) ? inheritedImplementation::compareUnsigned(source) : result;
      }

  public:
   TMultiBitElement(int size) : inheritedImplementation(size) { setMultiBit(); }
   TMultiBitElement(int size, typename inheritedImplementation::IntNature nature,
         const Floating::Implementation::Details::TMultiFloatElement<TypeMultiTraits>& floatValue,
         typename inheritedImplementation::ReadFloatParameters& params)
      :  inheritedImplementation(size, nature, floatValue, params) { setMultiBit(); } 
   TMultiBitElement(const thisType& source) = default;
   TMultiBitElement(const Details::TMultiBitElement<TypeMultiTraits>& source)
      :  inheritedImplementation(source) { setMultiBit(); }
   TMultiBitElement(const Details::MultiBitElement& source) : inheritedImplementation(source) { setMultiBit(); }
   thisType& operator=(const thisType& source) = default;
   TemplateDefineCopy(TMultiBitElement, TypeMultiTraits)
   DTemplateDefineAssign(TMultiBitElement, TypeMultiTraits)
   void assign(const Scalar::Implementation::VirtualElement& source) { inheritedImplementation::assign(source); }
   void assignPart(const thisType& source) { thisType::_assign(castToCopyHandler(source)); }
   ComparisonResult comparePart(const thisType& source) const { return thisType::_compare(castToCopyHandler(source)); }
   typedef inheritedImplementation InheritedImplementation;
   
   thisType& operator=(unsigned int value)
      {  return (thisType&) inheritedImplementation::operator=(value); }
   thisType& operator++() { return (thisType&) inheritedImplementation::operator++(); }
   thisType& operator--() { return (thisType&) inheritedImplementation::operator--(); }
   thisType& operator+=(const thisType& source) { return (thisType&) inheritedImplementation::operator+=(source); }
   thisType& operator-=(const thisType& source) { return (thisType&) inheritedImplementation::operator-=(source); }
   thisType& operator*=(const thisType& source) { return (thisType&) inheritedImplementation::operator*=(source); }
   thisType& operator/=(const thisType& source) { return (thisType&) inheritedImplementation::operator/=(source); }
   thisType& operator|=(const thisType& source) { return (thisType&) inheritedImplementation::operator|=(source); }
   thisType& operator&=(const thisType& source) { return (thisType&) inheritedImplementation::operator&=(source); }

   void write(STG::IOObject::OSBase& out, const STG::IOObject::FormatParameters& params) const
      {  inheritedImplementation::write(out, params);
      }
   void write(STG::IOObject::OSBase& out, const typename inheritedImplementation::FormatParameters& params) const
      {  inheritedImplementation::write(out, params); }
};

}}}} // end of namespace Analyzer::Scalar::MultiBit::Implementation

#endif // Analyzer_Scalar_Implementation_MultiBitElementH
