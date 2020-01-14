/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : implementation of general scalar elements
// File      : MultiFloatElement.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a multifloat template class providing all the operations on multifloats.
//

#ifndef Analyzer_Scalar_Implementation_MultiFloatElementH
#define Analyzer_Scalar_Implementation_MultiFloatElementH

#include "Analyzer/Scalar/Implementation/ImplementationElement.h"
#include "Numerics/Floating.h"
#include "Analyzer/Scalar/Implementation/General/MultiBitElement.h"

namespace Analyzer { namespace Scalar {

namespace MultiBit { namespace Implementation {

template <class TypeMultiTraits>
class TMultiBitElement;

class MultiBitElement;

namespace Details {

template <class TypeMultiTraits>
class TMultiBitElement;

class MultiBitElement;

}}} // end of namespace MultiBit::Implementation::Details

namespace Floating { namespace Implementation {

namespace Details {

class Access {
  public:
   typedef Numerics::DDouble::Access::ReadParameters ReadParameters;
   typedef Numerics::DDouble::Access::WriteParameters WriteParameters;

   static void assignParamsToEnv(const ReadParameters& params, EvaluationEnvironment& env, bool isNegativeResult)
      {  FloatingEnvironment& floatingEnv = env.floatingPart();
         floatingEnv.outputs().mergeValuesField(params.queryDynamicWriteField());
         floatingEnv.outputs().mergeFixedField(0x7FF);
         if (params.hasFlowException()) {
            if (params.isOverflow()) {
               if (isNegativeResult)
                  env.setNegativeOverflow();
               else
                  env.setPositiveOverflow();
            };
            if (params.isUnderflow()) {
               if (isNegativeResult)
                  env.setNegativeUnderflow();
               else
                  env.setPositiveUnderflow();
            };
         };
         if (params.isDivisionByZero())
            env.setDivisionByZero();
         if (params.hasQNaNResult() || params.hasSNaNOperand())
            env.setNaN();
      }
   static void assignEnvToParams(const EvaluationEnvironment& env, ReadParameters& params)
      {  const FloatingEnvironment& floatingEnv = env.floatingPart();
         AssumeCondition(params.doesSupportInstantiation())
         params.mergeStaticMachineField(floatingEnv.inputs().queryMachineField());
         params.mergeStaticModeField(floatingEnv.inputs().queryModeField());
      }
};

template <int uMaxSize>
class TIntegerTraits : public Scalar::MultiBit::Implementation::Details::TMaxIntegerTraits<uMaxSize> {
  private:
   typedef TIntegerTraits<uMaxSize> thisType;
   typedef Scalar::MultiBit::Implementation::Details::TMaxIntegerTraits<uMaxSize> inherited;
   
  public:
   TIntegerTraits() {}
   TIntegerTraits(const inherited& source) : inherited(source) {}
   TIntegerTraits(const thisType&) = default;

   thisType& operator=(const inherited& source) { return (thisType&) inherited::operator=(source); }
   thisType& operator=(const thisType& source) = default;
   thisType& operator=(unsigned int uValue)
      { return (thisType&) inherited::operator=(uValue); }
};

template <int BitSizeMantissa, int BitSizeExponent, int BitSizeIntCast>
class BuiltDoubleTraits : public Numerics::DDouble::BuiltAccess, public Access {
  private:
   typedef Numerics::DDouble::BuiltAccess inherited;
   typedef BuiltDoubleTraits<BitSizeMantissa, BitSizeExponent, BitSizeIntCast> thisType;

  public:
   class ExtendedMantissa;
   class Mantissa : public Numerics::TBigInt<TIntegerTraits<BitSizeMantissa> > {
     private:
      typedef Numerics::TBigInt<TIntegerTraits<BitSizeMantissa> > inherited;

     public:
      Mantissa() {}
      Mantissa(const Mantissa& source) = default;
      Mantissa(const ExtendedMantissa& source)
         {  inherited::setSize(source.getSize()-1);
            AssumeCondition(inherited::getSize() > 0)
            int cellSizeMantissa = ((inherited::getSize()+sizeof(unsigned int)*8-1) / (int) (sizeof(unsigned int)*8));
            for (int index = 0; index < cellSizeMantissa; ++index)
               inherited::array(index) = source[index];
            inherited::normalizeLastCell();
         }
      void normalizeLastCell() { inherited::normalizeLastCell(); }

      Mantissa& operator=(unsigned int value) { return (Mantissa&) inherited::operator=(value); }
      Mantissa& operator=(const Mantissa& source)
         {  return (Mantissa&) inherited::operator=(source); }
      Mantissa& operator=(const typename inherited::QuotientResult& source)
         {  return (Mantissa&) inherited::operator=(source); }
      Mantissa& operator=(const ExtendedMantissa& source)
         {  for (int index = 0; index <= inherited::lastCellIndex(); ++index)
               inherited::array(index) = source[index];
            inherited::normalizeLastCell();
            return *this;
         }
   };
   
   class Exponent : public Numerics::TBigInt<TIntegerTraits<BitSizeExponent> > {
     private:
      typedef Numerics::TBigInt<TIntegerTraits<BitSizeExponent> > inherited;
      
     public:
      class Min { public : Min() {} };
      class Zero { public : Zero() {} };
      class One { public : One() {} };
      class MinusOne { public : MinusOne() {} };
      class Max { public : Max() {} };
      class Basic { public : Basic() {} };
      
      Exponent()  {}
      Exponent(Basic, int size, unsigned int basicValue)
         {  inherited::setSize(size); inherited::operator=(basicValue); }
      Exponent(Min, int size) { inherited::setSize(size); }
      Exponent(Max, int size) { inherited::setSize(size); inherited::neg(); }
      Exponent(Zero, int size)
         {  inherited::setSize(size); inherited::neg(); inherited::bitArray(size-1) = false; }
      Exponent(MinusOne, int size)
         {  inherited::setSize(size); 
            inherited::neg();
            inherited::bitArray(size-1) = false;
            inherited::bitArray(0) = false;
         }
      Exponent(One, int size)  { inherited::setSize(size); inherited::bitArray(size-1) = true; }
      Exponent(const Exponent& source) = default;

      Exponent& operator=(const Exponent& source) = default;
      Exponent& operator=(unsigned int source) { return (Exponent&) inherited::operator=(source); }
      Exponent& operator-=(const Exponent& source) { return (Exponent&) inherited::operator-=(source); }
      Exponent& operator+=(const Exponent& source) { return (Exponent&) inherited::operator+=(source); }
      Exponent& operator--() { return (Exponent&) inherited::operator--(); }
      Exponent& operator++() { return (Exponent&) inherited::operator++(); }
   };

   Exponent getZeroExponent(const Exponent& base) const { return Exponent(typename Exponent::Zero(), base.getSize()); }
   Exponent getBasicExponent(const Exponent& base, unsigned int exponent) const
      {  return Exponent(typename Exponent::Basic(), base.getSize(), exponent); }
   Exponent getOneExponent(const Exponent& base) { return Exponent(typename Exponent::One(), base.getSize()); }
   Exponent getMinusOneExponent(const Exponent& base) const { return Exponent(typename Exponent::MinusOne(), base.getSize()); }
   Exponent getInftyExponent(const Exponent& base) const { return Exponent(typename Exponent::Max(), base.getSize()); }
   Exponent getMaxExponent(const Exponent& base) const { return Exponent(typename Exponent::Max(), base.getSize()); }

   int bitSizeMantissa(const Mantissa& mantissa) const { return mantissa.getSize(); }
   int bitSizeExponent(const Exponent& exponent) const { return exponent.getSize(); }
   
   class ExtendedMantissa : public Numerics::TBigInt<TIntegerTraits<BitSizeMantissa+1> > {
     private:
      typedef Numerics::TBigInt<TIntegerTraits<BitSizeMantissa+1> > inherited;

     public:
      ExtendedMantissa(const ExtendedMantissa& source) = default;
      ExtendedMantissa(const Mantissa& source)
         {  inherited::setSize(source.getSize()+1);
            int index = 0;
            int lastCellIndex = source.lastCellIndex();
            for (; index <= lastCellIndex; ++index)
               inherited::array(index) = source[index];
            inherited::bitArray(source.getSize()/* BitSizeMantissa */) = true;
         }
      typedef Numerics::TBigInt<typename inherited::MultResult> EnhancedMultResult;
      typedef Numerics::TBigInt<typename inherited::RemainderResult> EnhancedRemainderResult;
      int getMultResultCellSize(const EnhancedMultResult& result) const
         {  return result.getCellSize(); }
   };
   
   class IntConversion {
     public:
      typedef Numerics::TBigInt<Scalar::MultiBit::Implementation::Details::
         TMaxIntegerTraits<BitSizeIntCast> > BigInt;

     private:
      BigInt biResult;
      bool fUnsigned;
      bool fNegative;

     public:
      IntConversion() : fUnsigned(false), fNegative(false) {}
      IntConversion(const IntConversion& source) = default;

      IntConversion& setSigned() { fUnsigned = false; return *this; }
      IntConversion& setUnsigned() { fUnsigned = true; return *this; }
      IntConversion& assignSigned(const BigInt& result)
         {  AssumeCondition(!fUnsigned)
            biResult = result;
            if (biResult.cbitArray(biResult.getSize()-1)) {
               fNegative = true;
               biResult.neg().inc();
            }
            else
               fNegative = false;
            return *this;
         }
      IntConversion& assignUnsigned(const BigInt& result)
         {  AssumeCondition(fUnsigned)
            biResult = result;
            fNegative = false;
            return *this;
         }
      IntConversion& assign(int value)
         {  AssumeCondition(!fUnsigned)
            biResult = ((fNegative = (value < 0)) != false)
               ? (unsigned int) value : (unsigned int) -value;
            return *this;
         }
      IntConversion& assign(unsigned int value)
         {  AssumeCondition(fUnsigned) biResult = value; return *this; }

      int getSize() const { return biResult.getSize(); }
      void setSize(int bitSize) { biResult.setSize(bitSize); }
      int getMaxDigits() const
         {  return fUnsigned ? biResult.getSize() : (biResult.getSize()-1); }
      bool isUnsigned() const { return fUnsigned; }
      bool isSigned() const   { return !fUnsigned; }
      BigInt asInt() const
         {  AssumeCondition(!fUnsigned)
            BigInt result = biResult;
            if (fNegative)
               result.neg().inc();
            return result;
         }
      const BigInt& asUnsignedInt() const { AssumeCondition(fUnsigned) return biResult; }
      void opposite() { AssumeCondition(!fUnsigned) fNegative = !fNegative; }
      bool isPositive() const { return fUnsigned || !fNegative; }
      bool isNegative() const { return !fUnsigned && fNegative; }
      bool isDifferentZero() const { return !biResult.isZero(); }
      int log_base_2() const { return biResult.log_base_2(); }
      bool hasZero(int digits) const { return biResult.hasZero(digits); }
      bool cbitArray(int localIndex) const { return biResult.cbitArray(localIndex); }
      IntConversion& operator>>=(int shift)
         {  AssumeCondition(isPositive()) biResult >>= shift; return *this; }
      IntConversion& operator<<=(int shift)
         {  AssumeCondition(isPositive()) biResult <<= shift; return *this; }
      IntConversion& operator&=(const IntConversion& source)
         {  AssumeCondition(isPositive() && source.isPositive())
            biResult &= source.biResult;
            return *this;
         }
      IntConversion& neg() { if (!fUnsigned) fNegative = !fNegative; biResult.neg(); return *this; }
      IntConversion& inc() { biResult.inc(); return *this; }

      IntConversion& operator=(const IntConversion& source) = default;
      typedef typename BigInt::ArrayProperty BitArray;
      BitArray bitArray(int index) { AssumeCondition(isPositive()) return biResult.bitArray(index); }
      void setBitArray(int index, bool value)
         {  AssumeCondition(isPositive()) biResult.setBitArray(index, value); }
      void setTrueBitArray(int index)
         {  AssumeCondition(isPositive()) biResult.setTrueBitArray(index); }
      void setFalseBitArray(int index)
         {  AssumeCondition(isPositive()) biResult.setFalseBitArray(index); }

      void setMin()
         {  biResult.clear();
            if (!fUnsigned)
               biResult.setTrueBitArray(biResult.getSize()-1);
         }
      void setMax()
         {  biResult.clear(); 
            if (fUnsigned)
               biResult.neg();
            else
               biResult.neg(biResult.getSize()-1);
         }
      unsigned int& operator[](int index) { return biResult[index]; }
      const unsigned int& operator[](int index) const { return biResult[index]; }
   };

   typedef typename inherited::TFloatConversion<(BitSizeMantissa+8*sizeof(unsigned int)-1)/(8*sizeof(unsigned int)),
           (BitSizeExponent+8*sizeof(unsigned int)-1)/(8*sizeof(unsigned int))> FloatConversion;

   typedef Numerics::TBigInt<TIntegerTraits<BitSizeMantissa+BitSizeExponent+1> > DiffDouble;
   typedef BuiltDoubleTraits<2*BitSizeMantissa+1, BitSizeExponent+1, BitSizeIntCast> MultExtension;
};

class ExtensibleBuiltDoubleTraits : public Numerics::DDouble::BuiltAccess, public Access {
  private:
   typedef Numerics::DDouble::BuiltAccess inherited;
   typedef ExtensibleBuiltDoubleTraits thisType;

  public:
   class ExtendedMantissa;
   class Mantissa : public Scalar::MultiBit::Implementation::Details::BaseMultiBitElement {
     private:
      typedef Scalar::MultiBit::Implementation::Details::BaseMultiBitElement inherited;

     public:
      Mantissa() {}
      Mantissa(Mantissa&& source) = default;
      Mantissa(const Mantissa& source) = default;
      Mantissa(const ExtendedMantissa& source)
         {  assign(source);
            AssumeCondition(source.getSize() > 1)
            inherited::setSize(source.getSize()-1);
         }
      void normalizeLastCell() { inherited::normalizeLastCell(); }

      Mantissa& operator=(unsigned int value) { return (Mantissa&) inherited::operator=(value); }
      Mantissa& operator=(Mantissa&& source) = default;
      Mantissa& operator=(const Mantissa& source) = default;
      Mantissa& operator=(const ExtendedMantissa& source)
         {  inherited::operator=(source);
            AssumeCondition(source.getSize() > 1)
            inherited::setSize(source.getSize()-1);
            return *this;
         }
      Mantissa& operator=(Numerics::DInteger::ExtensibleCellIntegerTraits&& source)
         {  return (Mantissa&) inherited::operator=(source); }
      Mantissa& operator=(const Numerics::DInteger::ExtensibleCellIntegerTraits& source)
         {  return (Mantissa&) inherited::operator=(source); }
   };
   
   class Exponent : public Scalar::MultiBit::Implementation::Details::BaseMultiBitElement {
     private:
      typedef Scalar::MultiBit::Implementation::Details::BaseMultiBitElement inherited;
      
     public:
      class Min { public : Min() {} };
      class Zero { public : Zero() {} };
      class One { public : One() {} };
      class MinusOne { public : MinusOne() {} };
      class Max { public : Max() {} };
      class Basic { public : Basic() {} };
      
      Exponent()  {}
      Exponent(Basic, int size, unsigned int basicValue)
         {  inherited::setSize(size); inherited::operator=(basicValue); }
      Exponent(Min, int size)   { inherited::setSize(size); }
      Exponent(Max, int size)   { inherited::setSize(size); inherited::neg(); }
      Exponent(Zero, int size)
         {  inherited::setSize(size); inherited::neg(); inherited::bitArray(size-1) = false; }
      Exponent(MinusOne, int size)
         {  inherited::setSize(size); 
            inherited::neg();
            inherited::bitArray(size-1) = false;
            inherited::bitArray(0) = false;
         }
      Exponent(One, int size)  { inherited::setSize(size); inherited::bitArray(size-1) = true; }
      Exponent(Exponent&& source) = default;
      Exponent(const Exponent& source) = default;

      Exponent& operator=(Exponent&& source) = default;
      Exponent& operator=(const Exponent& source) = default;
      Exponent& operator=(unsigned int source) { return (Exponent&) inherited::operator=(source); }
      Exponent& operator-=(const Exponent& source) { return (Exponent&) inherited::operator-=(source); }
      Exponent& operator+=(const Exponent& source) { return (Exponent&) inherited::operator+=(source); }
      Exponent& operator--() { return (Exponent&) inherited::operator--(); }
      Exponent& operator++() { return (Exponent&) inherited::operator++(); }
   };
   Exponent getZeroExponent(const Exponent& base) const { return Exponent(Exponent::Zero(), base.getSize()); }
   Exponent getBasicExponent(const Exponent& base, unsigned int exponent) const
      {  return Exponent(Exponent::Basic(), base.getSize(), exponent); }
   Exponent getOneExponent(const Exponent& base) { return Exponent(Exponent::One(), base.getSize()); }
   Exponent getMinusOneExponent(const Exponent& base) const { return Exponent(Exponent::MinusOne(), base.getSize()); }
   Exponent getInftyExponent(const Exponent& base) const { return Exponent(Exponent::Max(), base.getSize()); }
   Exponent getMaxExponent(const Exponent& base) const { return Exponent(Exponent::Max(), base.getSize()); }
   int bitSizeMantissa(const Mantissa& mantissa) const { return mantissa.getSize(); }
   int bitSizeExponent(const Exponent& exponent) const { return exponent.getSize(); }
   
   class ExtendedMantissa : public Mantissa {
     private:
      typedef Mantissa inherited;

     public:
      ExtendedMantissa(const ExtendedMantissa& source) = default;
      ExtendedMantissa(const Mantissa& source)
         {  assign(source);
            int size = source.getSize();
            setSize(size+1);
            bitArray(size) = true;
         }
   };
   
   class IntConversion {
     public:
      typedef Scalar::MultiBit::Implementation::Details::BaseMultiBitElement BigInt;

     private:
      BigInt biResult;
      bool fUnsigned;
      bool fNegative;

     public:
      IntConversion() : fUnsigned(false), fNegative(false) {}
      IntConversion(IntConversion&& source) = default;
      IntConversion(const IntConversion& source) = default;

      IntConversion& setSigned() { fUnsigned = false; return *this; }
      IntConversion& setUnsigned() { fUnsigned = true; return *this; }
      IntConversion& assignSigned(const BigInt& result)
         {  AssumeCondition(!fUnsigned)
            biResult = result;
            if (biResult.cbitArray(biResult.getSize()-1)) {
               fNegative = true;
               biResult.neg().inc();
            }
            else
               fNegative = false;
            return *this;
         }
      IntConversion& assignUnsigned(const BigInt& result)
         {  AssumeCondition(fUnsigned)
            biResult = result;
            fNegative = false;
            return *this;
         }
      IntConversion& assign(int value)
         {  AssumeCondition(!fUnsigned)
            biResult = ((fNegative = (value < 0)) != false)
               ? (unsigned int) value : (unsigned int) -value;
            return *this;
         }
      IntConversion& assign(unsigned int value)
         {  AssumeCondition(fUnsigned) biResult = value; return *this; }

      int getSize() const { return biResult.getSize(); }
      void setSize(int bitSize) { biResult.setSize(bitSize); }
      int getMaxDigits() const
         {  return fUnsigned ? biResult.getSize() : (biResult.getSize()-1); }
      bool isUnsigned() const { return fUnsigned; }
      bool isSigned() const   { return !fUnsigned; }
      BigInt asInt() const
         {  AssumeCondition(!fUnsigned)
            BigInt result = biResult;
            if (fNegative)
               result.neg().inc();
            return result;
         }
      const BigInt& asUnsignedInt() const { AssumeCondition(fUnsigned) return biResult; }
      void opposite() { AssumeCondition(!fUnsigned) fNegative = !fNegative; }
      bool isPositive() const { return fUnsigned || !fNegative; }
      bool isNegative() const { return !fUnsigned && fNegative; }
      bool isDifferentZero() const { return !biResult.isZero(); }
      int log_base_2() const { return biResult.log_base_2(); }
      bool hasZero(int digits) const { return biResult.hasZero(digits); }
      bool cbitArray(int localIndex) const { return biResult.cbitArray(localIndex); }
      IntConversion& operator>>=(int shift)
         {  AssumeCondition(isPositive()) biResult >>= shift; return *this; }
      IntConversion& operator<<=(int shift)
         {  AssumeCondition(isPositive()) biResult <<= shift; return *this; }
      IntConversion& operator&=(const IntConversion& source)
         {  AssumeCondition(isPositive() && source.isPositive())
            biResult &= source.biResult;
            return *this;
         }
      IntConversion& neg() { if (!fUnsigned) fNegative = !fNegative; biResult.neg(); return *this; }
      IntConversion& inc() { biResult.inc(); return *this; }

      IntConversion& operator=(IntConversion&& source) = default;
      IntConversion& operator=(const IntConversion& source) = default;
      typedef BigInt::BitArray BitArray;
      BitArray bitArray(int index) { AssumeCondition(isPositive()) return biResult.bitArray(index); }
      void setBitArray(int index, bool value)
         {  AssumeCondition(isPositive()) biResult.setBitArray(index, value); }
      void setTrueBitArray(int index)
         {  AssumeCondition(isPositive()) biResult.setTrueBitArray(index); }
      void setFalseBitArray(int index)
         {  AssumeCondition(isPositive()) biResult.setFalseBitArray(index); }

      void setMin()
         {  biResult.clear();
            if (!fUnsigned)
               biResult.setTrueBitArray(biResult.getSize()-1);
         }
      void setMax()
         {  biResult.clear(); 
            if (fUnsigned)
               biResult.neg();
            else
               biResult.neg(biResult.getSize()-1);
         }
      unsigned int& operator[](int index) { return biResult[index]; }
      unsigned int operator[](int index) const { return biResult[index]; }
   };

   class FloatConversion {
     public:
      typedef Scalar::MultiBit::Implementation::Details::BaseMultiBitElement Mantissa;
      typedef Scalar::MultiBit::Implementation::Details::BaseMultiBitElement Exponent;

     private:
      typedef FloatConversion thisType;

      Mantissa bciMantissa;
      Exponent bciExponent;
      bool fNegative;

     public:
      FloatConversion() : bciMantissa(), bciExponent(), fNegative(false) {}
      FloatConversion(thisType&& source) = default;
      FloatConversion(const thisType& source) = default;

      bool isPositive() const { return !fNegative; }
      bool isNegative() const { return fNegative; }
      bool isInftyExponent() const { return Exponent(bciExponent).neg().isZero(); }
      bool isZeroExponent() const { return bciExponent.isZero(); }
      bool isZeroMantissa() const { return bciMantissa.isZero(); }
      int getSizeMantissa() const { return bciMantissa.getSize(); }
      int getSizeExponent() const { return bciExponent.getSize(); }

      thisType& setSizeMantissa(int size) { bciMantissa.setSize(size); return *this; }
      thisType& setSizeExponent(int size) { bciExponent.setSize(size); return *this; }

      void setPositive(bool isPositive) { fNegative = !isPositive; }
      void setNegative(bool isNegative) { fNegative = isNegative; }
      const Mantissa& mantissa() const { return bciMantissa; }
      Mantissa& mantissa() { return bciMantissa; }
      const Exponent& exponent() const { return bciExponent; }
      Exponent& exponent() { return bciExponent; }
   };

   typedef Scalar::MultiBit::Implementation::Details::BaseMultiBitElement DiffDouble;
   typedef ExtensibleBuiltDoubleTraits MultExtension;
};

template <class TypeBase>
class TMultiFloatImplementOperation : public TypeBase {
  private:
   typedef TypeBase inherited;
   typedef TMultiFloatImplementOperation<TypeBase> thisType;

  public:
   static void assignParamsToEnv(const typename inherited::ReadParameters& params, EvaluationEnvironment& env, bool isNegativeResult)
      {  Details::Access::assignParamsToEnv(params, env, isNegativeResult); }
   void assignParamsToEnv(const typename inherited::ReadParameters& params, EvaluationEnvironment& env) const
      {  Details::Access::assignParamsToEnv(params, env, inherited::isNegative()); }
   static void assignEnvToParams(const EvaluationEnvironment& env, typename inherited::ReadParameters& params)
      {  Details::Access::assignEnvToParams(env, params); }

   typename TypeBase::Mantissa& mantissa() { return inherited::getSMantissa(); }
   const typename TypeBase::Mantissa& mantissa() const { return inherited::getMantissa(); }
   typename TypeBase::Exponent& exponent() { return inherited::getSBasicExponent(); }
   const typename TypeBase::Exponent& exponent() const { return inherited::getBasicExponent(); }
   
  public:
   TMultiFloatImplementOperation() {}
   TMultiFloatImplementOperation(int sizeMantissa, int sizeExponent)
      {  mantissa().setSize(sizeMantissa);
         exponent().setSize(sizeExponent);
      }
   TMultiFloatImplementOperation(int sizeMantissa, int sizeExponent,
         const thisType& floatValue, typename inherited::ReadParameters& params)
      {  mantissa().setSize(sizeMantissa);
         exponent().setSize(sizeExponent);
         typename inherited::FloatConversion floatConversion;
         floatConversion.setSizeMantissa(floatValue.mantissa().getSize());
         floatConversion.setSizeExponent(floatValue.exponent().getSize());
         floatConversion.setNegative(floatValue.isNegative());
         int nbCellMantissa = (floatValue.mantissa().getSize()+sizeof(unsigned int)*8-1)/(8*sizeof(unsigned int));
         int nbCellExponent = (floatValue.exponent().getSize()+sizeof(unsigned int)*8-1)/(8*sizeof(unsigned int));
         while (--nbCellMantissa >= 0)
            floatConversion.mantissa()[nbCellMantissa] = floatValue.mantissa()[nbCellMantissa];
         while (--nbCellExponent >= 0)
            floatConversion.exponent()[nbCellExponent] = floatValue.exponent()[nbCellExponent];
         inherited::setFloat(floatConversion, params);
      }
   TMultiFloatImplementOperation(thisType&& source) = default;
   TMultiFloatImplementOperation(const thisType& source) = default;

   void setSizeMantissa(int sizeMantissa) { mantissa().setSize(sizeMantissa); }
   void setSizeExponent(int sizeExponent) { exponent().setSize(sizeExponent); }

   int getSizeMantissa() const { return mantissa().getSize(); }
   int getSizeExponent() const { return exponent().getSize(); }
   int getSize() const { return mantissa().getSize() + exponent().getSize() + 1; }

   thisType& operator=(thisType&& source) = default;
   thisType& operator=(const thisType& source) = default;
   ComparisonResult compare(const thisType& source) const { return inherited::compare(source); }

   bool apply(const CastMultiFloatOperation& operation, EvaluationEnvironment& env)
      {  typename inherited::FloatConversion floatConversion;
         floatConversion.setSizeMantissa(mantissa().getSize());
         floatConversion.setSizeExponent(exponent().getSize());
         floatConversion.setNegative(inherited::isNegative());
         int nbCellMantissa = (mantissa().getSize()+sizeof(unsigned int)*8-1)/(8*sizeof(unsigned int));
         int nbCellExponent = (exponent().getSize()+sizeof(unsigned int)*8-1)/(8*sizeof(unsigned int));
         while (--nbCellMantissa >= 0)
            floatConversion.mantissa()[nbCellMantissa] = mantissa()[nbCellMantissa];
         while (--nbCellExponent >= 0)
            floatConversion.exponent()[nbCellExponent] = exponent()[nbCellExponent];
         typename inherited::ReadParameters params;
         assignEnvToParams(env, params);
         mantissa().setSize(operation.getSizeMantissa());
         exponent().setSize(operation.getSizeExponent());
         inherited::setFloat(floatConversion, params);
         assignParamsToEnv(params, env);
         return true;
      }
   bool apply(const PlusAssignOperation&, const thisType& source, EvaluationEnvironment& env)
      {  typename inherited::ReadParameters params;
         assignEnvToParams(env, params);
         inherited::plusAssign(source, params);
         assignParamsToEnv(params, env);
         return true;
      }
   bool apply(const MinusAssignOperation&, const thisType& source, EvaluationEnvironment& env)
      {  typename inherited::ReadParameters params;
         assignEnvToParams(env, params);
         inherited::minusAssign(source, params);
         assignParamsToEnv(params, env);
         return true;
      }
   bool apply(const IsInftyExponentOperation&, bool& result, EvaluationEnvironment& env)
      {  result = inherited::getBasicExponent() == inherited::getInftyExponent();
         return true;
      }
   bool apply(const IsNaNOperation&, bool& result, EvaluationEnvironment& env)
      {  result = inherited::isNaN();
         return true;
      }
   bool apply(const IsQNaNOperation&, bool& result, EvaluationEnvironment& env)
      {  result = inherited::isQNaN();
         return true;
      }
   bool apply(const IsSNaNOperation&, bool& result, EvaluationEnvironment& env)
      {  result = inherited::isSNaN();
         return true;
      }
   bool apply(const IsPositiveOperation&, bool& result, EvaluationEnvironment& env)
      {  result = inherited::isPositive();
         return true;
      }
   bool apply(const IsZeroExponentOperation&, bool& result, EvaluationEnvironment& env)
      {  result = !inherited::getBasicExponent().isZero();
         return true;
      }
   bool apply(const IsNegativeOperation&, bool& result, EvaluationEnvironment& env)
      {  result = inherited::isNegative();
         return true;
      }
   
   bool apply(const MinAssignOperation&, const thisType& source, EvaluationEnvironment& env)
      {  ComparisonResult result = inherited::compare(source);
         if (result == CRGreater)
            inherited::operator=(source);
         else if (result == CRNonComparable)
            env.setNaN();
         return true;
      }
   bool apply(const MaxAssignOperation&, const thisType& source, EvaluationEnvironment& env)
      {  ComparisonResult result = inherited::compare(source);
         if (result == CRLess)
            inherited::operator=(source);
         else if (result == CRNonComparable)
            env.setNaN();
         return true;
      }
   bool apply(const TimesAssignOperation&, const thisType& source, EvaluationEnvironment& env)
      {  typename inherited::ReadParameters params;
         assignEnvToParams(env, params);
         inherited::multAssign(source, params);
         assignParamsToEnv(params, env);
         return true;
      }
   bool apply(const DivideAssignOperation&, const thisType& source, EvaluationEnvironment& env)
      {  typename inherited::ReadParameters params;
         assignEnvToParams(env, params);
         inherited::divAssign(source, params);
         assignParamsToEnv(params, env);
         return true;
      }
   bool apply(const OppositeAssignOperation&, EvaluationEnvironment&)
      {  inherited::opposite(); return true; }
   bool apply(const AbsAssignOperation&, EvaluationEnvironment&)
      {  if (inherited::isNegative())
            inherited::opposite();
         return true;
      }
   bool apply(const MultAddAssignOperation&, const thisType& fst, const thisType& snd, EvaluationEnvironment& env)
      {  typename inherited::ReadParameters params;
         assignEnvToParams(env, params);
         inherited::multAndAddAssign(fst, snd, params);
         assignParamsToEnv(params, env);
         return true;
      }
   bool apply(const MultSubAssignOperation&, const thisType& fst, const thisType& snd, EvaluationEnvironment& env)
      {  typename inherited::ReadParameters params;
         assignEnvToParams(env, params);
         inherited::multAndSubAssign(fst, snd, params);
         assignParamsToEnv(params, env);
         return true;
      }
   bool apply(const NegMultAddAssignOperation&, const thisType& fst, const thisType& snd, EvaluationEnvironment& env)
      {  typename inherited::ReadParameters params;
         assignEnvToParams(env, params);
         inherited::multNegativeAndAddAssign(fst, snd, params);
         assignParamsToEnv(params, env);
         return true;
      }
   bool apply(const NegMultSubAssignOperation&, const thisType& fst, const thisType& snd, EvaluationEnvironment& env)
      {  typename inherited::ReadParameters params;
         assignEnvToParams(env, params);
         inherited::multNegativeAndSubAssign(fst, snd, params);
         assignParamsToEnv(params, env);
         return true;
      }
   bool apply(const AcosOperation&, EvaluationEnvironment&) { return false; }
   bool apply(const AsinOperation&, EvaluationEnvironment&) { return false; }
   bool apply(const AtanOperation&, EvaluationEnvironment&) { return false; }
   bool apply(const Atan2Operation&, EvaluationEnvironment&) { return false; }
   bool apply(const CeilOperation&, EvaluationEnvironment&) { return false; }
   bool apply(const CosOperation&, EvaluationEnvironment&) { return false; }
   bool apply(const CoshOperation&, EvaluationEnvironment&) { return false; }
   bool apply(const ExpOperation&, EvaluationEnvironment&) { return false; }
   bool apply(const FabsOperation&, EvaluationEnvironment&) { return false; }
   bool apply(const FloorOperation&, EvaluationEnvironment&) { return false; }
   bool apply(const FmodOperation&, EvaluationEnvironment&) { return false; }
   bool apply(const FrexpOperation&, EvaluationEnvironment&) { return false; }
   bool apply(const LdexpOperation&, EvaluationEnvironment&) { return false; }
   bool apply(const LogOperation&, EvaluationEnvironment&) { return false; }
   bool apply(const Log10Operation&, EvaluationEnvironment&) { return false; }
   bool apply(const ModfOperation&, EvaluationEnvironment&) { return false; }
   bool apply(const PowOperation&, EvaluationEnvironment&) { return false; }
   bool apply(const SinOperation&, EvaluationEnvironment&) { return false; }
   bool apply(const SinhOperation&, EvaluationEnvironment&) { return false; }
   bool apply(const SqrtOperation&, EvaluationEnvironment&) { return false; }
   bool apply(const TanOperation&, EvaluationEnvironment&) { return false; }
   bool apply(const TanhOperation&, EvaluationEnvironment&) { return false; }
};

} // end of namespace Details

namespace Details {
   
template <class TypeMultiTraits>
class TMultiFloatElement;

class MultiFloatElement : public TMultiFloatImplementOperation<Numerics::TBuiltDouble<ExtensibleBuiltDoubleTraits> > {
  private:
   typedef TMultiFloatImplementOperation<Numerics::TBuiltDouble<ExtensibleBuiltDoubleTraits> > inherited;
   typedef MultiFloatElement thisType;

  protected:
   typedef inherited BasePersistenceElement;

  public:
   MultiFloatElement() {}
   MultiFloatElement(int sizeMantissa, int sizeExponent) : inherited(sizeMantissa, sizeExponent) {}
   enum IntNature { INSigned, INUnsigned, INPointer }; // little endian
   MultiFloatElement(int sizeMantissa, int sizeExponent, IntNature nature,
         const MultiBit::Implementation::Details::MultiBitElement& intValue, ReadParameters& params);
   MultiFloatElement(int sizeMantissa, int sizeExponent,
         const MultiFloatElement& floatValue, ReadParameters& params)
      :  inherited(sizeMantissa, sizeExponent, floatValue, params) {}
   template <class TypeMultiTraits>
   MultiFloatElement(const TMultiFloatElement<TypeMultiTraits>& source);
   MultiFloatElement(thisType&& source) = default;
   MultiFloatElement(const thisType& source) = default;

   MultiFloatElement& operator=(const MultiFloatElement&) = default;
   MultiFloatElement& operator=(MultiFloatElement&&) = default;

   typedef Scalar::MultiBit::Implementation::MultiBitElement MultiBitImplementation;
   void assign(const Scalar::Implementation::VirtualElement& source);
   bool isValid() const { return (mantissa().getSize() > 0) && (exponent().getSize() > 0); }
   
   void write(STG::IOObject::OSBase& out, const STG::IOObject::FormatParameters& params) const;
   void write(STG::IOObject::OSBase& out, const inherited::WriteParameters& params) const;
   void read(STG::IOObject::ISBase& in, const STG::IOObject::FormatParameters& params);
   void read(STG::IOObject::ISBase& in, inherited::ReadParameters& params);
};

class EnhancedMultiFloatElement : public MultiFloatElement {
  public:
   EnhancedMultiFloatElement() {}
   EnhancedMultiFloatElement(const MultiFloatElement& source) : MultiFloatElement(source) {}
   EnhancedMultiFloatElement(MultiFloatElement&& source) : MultiFloatElement(source) {}
   EnhancedMultiFloatElement& operator=(const MultiFloatElement& source)
      {  return (EnhancedMultiFloatElement&) MultiFloatElement::operator=(source); }
   EnhancedMultiFloatElement& operator=(MultiFloatElement&& source)
      {  return (EnhancedMultiFloatElement&) MultiFloatElement::operator=(source); }

   void write(STG::IOObject::OSBase& out, const STG::IOObject::FormatParameters& params) const
      {  BasePersistenceElement::WriteParameters copyParams;
         ((STG::IOObject::FormatParameters&) copyParams).assignPart(params);
         BasePersistenceElement::write(out, copyParams.setDecimal());
      }
   void write(STG::IOObject::OSBase& out, const BasePersistenceElement::WriteParameters& params) const
      {  BasePersistenceElement::write(out, params); }
   void read(STG::IOObject::ISBase& in, const STG::IOObject::FormatParameters& params)
      {  BasePersistenceElement::ReadParameters copyParams;
         ((STG::IOObject::FormatParameters&) copyParams).assignPart(params);
         BasePersistenceElement::read(in, copyParams);
      }
   void read(STG::IOObject::ISBase& in, BasePersistenceElement::ReadParameters& params)
      {  BasePersistenceElement::read(in, params); }
};

inline void
MultiFloatElement::write(STG::IOObject::OSBase& out, const STG::IOObject::FormatParameters& params) const
{  inherited::WriteParameters copyParams;
   ((STG::IOObject::FormatParameters&) copyParams).assignPart(params);
   Numerics::TReadDouble<MultiFloatElement, EnhancedMultiFloatElement>::EnhancedDouble improvedThis;
   improvedThis.setSizeExponent(getSizeExponent());
   improvedThis.setSizeMantissa(getSizeMantissa() + 5);
   improvedThis = *this;
   improvedThis.write(out, copyParams.setDecimal());
}

inline void
MultiFloatElement::write(STG::IOObject::OSBase& out, const inherited::WriteParameters& params) const
{  Numerics::TReadDouble<MultiFloatElement, EnhancedMultiFloatElement>::EnhancedDouble improvedThis;
   improvedThis.setSizeExponent(getSizeExponent());
   improvedThis.setSizeMantissa(getSizeMantissa() + 5);
   improvedThis = *this;
   improvedThis.write(out, params);
}

inline void
MultiFloatElement::read(STG::IOObject::ISBase& in, const STG::IOObject::FormatParameters& params)
{  inherited::ReadParameters copyParams;
   ((STG::IOObject::FormatParameters&) copyParams).assignPart(params);
   Numerics::TReadDouble<MultiFloatElement, EnhancedMultiFloatElement>::EnhancedDouble improvedThis;
   improvedThis.setSizeExponent(getSizeExponent());
   improvedThis.setSizeMantissa(getSizeMantissa() + 5);
   improvedThis.read(in, copyParams);
   improvedThis.retrieveDouble(*this, copyParams);
   if (inherited::isZero() && !inherited::isPositive())
      inherited::opposite();
}

inline void
MultiFloatElement::read(STG::IOObject::ISBase& in, inherited::ReadParameters& params)
{  Numerics::TReadDouble<MultiFloatElement, EnhancedMultiFloatElement>::EnhancedDouble improvedThis;
   improvedThis.setSizeExponent(getSizeExponent());
   improvedThis.setSizeMantissa(getSizeMantissa() + 5);
   improvedThis.read(in, params);
   improvedThis.retrieveDouble(*this, params);
   if (inherited::isZero() && !inherited::isPositive())
      inherited::opposite();
}

} // end of namespace Details

class MultiFloatElement
   :  public Scalar::Implementation::ImplementationElement,
      public Details::MultiFloatElement {
  private:
   typedef Scalar::Implementation::ImplementationElement inherited;
   typedef Details::MultiFloatElement inheritedImplementation;
   typedef MultiFloatElement thisType;

  protected:
   virtual ComparisonResult _compare(const EnhancedObject& asource) const override
      {  ComparisonResult result = inherited::_compare(asource);
         const thisType& source = (const thisType&) asource;
         return (result == CREqual) ? inheritedImplementation::compare(source) : result;
      }

  public:
   MultiFloatElement(int sizeMantissa, int sizeExponent) : inheritedImplementation(sizeMantissa, sizeExponent)
      {  setMultiFloat(); }
   MultiFloatElement(int sizeMantissa, int sizeExponent, IntNature nature,
         const MultiBit::Implementation::Details::MultiBitElement& intValue, ReadParameters& params)
      :  inheritedImplementation(sizeMantissa, sizeExponent, nature, intValue, params)
      {  setMultiFloat(); }
   MultiFloatElement(int sizeMantissa, int sizeExponent,
         const Details::MultiFloatElement& floatValue, ReadParameters& params)
      :  inheritedImplementation(sizeMantissa, sizeExponent, floatValue, params) { setMultiFloat(); }
   template <class TypeMultiTraits>
   MultiFloatElement(const Details::TMultiFloatElement<TypeMultiTraits>& source)
      :  inheritedImplementation(source) { setMultiFloat(); }
   MultiFloatElement(thisType&& source) = default;
   MultiFloatElement(const thisType& source) = default;
   MultiFloatElement& operator=(MultiFloatElement&& source) = default;
   MultiFloatElement& operator=(const MultiFloatElement& source) = default;
   DefineCopy(MultiFloatElement)
   DDefineAssign(MultiFloatElement)
   DCompare(MultiFloatElement)
   void assign(const Scalar::Implementation::VirtualElement& source) { inheritedImplementation::assign(source); }
   void assignPart(const MultiFloatElement& source) { MultiFloatElement::_assign(source); }
   typedef inheritedImplementation InheritedImplementation;
   virtual bool isValid() const override { return inheritedImplementation::isValid(); }

   void write(STG::IOObject::OSBase& out, const STG::IOObject::FormatParameters& params) const
      {  inheritedImplementation::write(out, params); }
   void write(STG::IOObject::OSBase& out, const inheritedImplementation::WriteParameters& params) const
      {  inheritedImplementation::write(out, params); }
};

namespace Details {
   
template <class TypeMultiTraits>
class TMultiFloatElement : public TMultiFloatImplementOperation<Numerics::TBuiltDouble<
      BuiltDoubleTraits<TypeMultiTraits::BitSizeMantissa, TypeMultiTraits::BitSizeExponent, TypeMultiTraits::BitSize> > > {
  private:
   typedef TMultiFloatImplementOperation<Numerics::TBuiltDouble<BuiltDoubleTraits
         <TypeMultiTraits::BitSizeMantissa, TypeMultiTraits::BitSizeExponent, TypeMultiTraits::BitSize> > >
      inherited;
   typedef TMultiFloatElement<TypeMultiTraits> thisType;

  public:
   typedef typename inherited::Mantissa Mantissa;
   typedef typename inherited::Exponent Exponent;
   typedef typename inherited::ReadParameters ReadParameters;
   const Mantissa& mantissa() const { return inherited::mantissa(); }
   Mantissa& mantissa() { return inherited::mantissa(); }
   const Exponent& exponent() const { return inherited::exponent(); }
   Exponent& exponent() { return inherited::exponent(); }

  public:      
   TMultiFloatElement(int sizeMantissa, int sizeExponent) : inherited(sizeMantissa, sizeExponent) {}
   enum IntNature { INSigned, INUnsigned, INPointer }; // little endian
   TMultiFloatElement(int sizeMantissa, int sizeExponent, IntNature nature,
         const MultiBit::Implementation::Details::TMultiBitElement<TypeMultiTraits>& intValue,
         ReadParameters& params);
   TMultiFloatElement(const MultiFloatElement& source);
   TMultiFloatElement(int sizeMantissa, int sizeExponent,
         const TMultiFloatElement<TypeMultiTraits>& floatValue, ReadParameters& params)
      :  inherited(sizeMantissa, sizeExponent, floatValue, params) {}
   TMultiFloatElement(const thisType& source) : inherited(source) {}
   
   typedef Scalar::MultiBit::Implementation::TMultiBitElement<TypeMultiTraits> MultiBitImplementation;
   
   void assign(const Scalar::Implementation::VirtualElement& source);
   void assign(const thisType& source) { inherited::operator=(source); }
   bool isValid() const { return (mantissa().getSize() > 0) && (exponent().getSize() > 0); }

   void write(STG::IOObject::OSBase& out, const STG::IOObject::FormatParameters& params) const
      {  typename inherited::FormatParameters copyParams;
         inherited::write(out, copyParams.setFullHexaDecimal(inherited::getSize()));
      }
   void write(STG::IOObject::OSBase& out, const typename inherited::WriteParameters& params) const
      {  inherited::write(out, params); }
};

} // end of namespace Details

template <class TypeMultiTraits>
class TMultiFloatElement
   :  public Scalar::Implementation::ImplementationElement,
      public Details::TMultiFloatElement<TypeMultiTraits> {
  private:
   typedef Scalar::Implementation::ImplementationElement inherited;
   typedef Details::TMultiFloatElement<TypeMultiTraits> inheritedImplementation;
   typedef TMultiFloatElement<TypeMultiTraits> thisType;

  protected:
   virtual ComparisonResult _compare(const EnhancedObject& asource) const
      {  ComparisonResult result = inherited::_compare(asource);
         const thisType& source = (const thisType&) castFromCopyHandler(asource);
         return (result == CREqual) ? inheritedImplementation::compare(source) : result;
      }

  public:
   TMultiFloatElement(int sizeMantissa, int sizeExponent)
      :  inheritedImplementation(sizeMantissa, sizeExponent) { inherited::setMultiFloat(); }
   TMultiFloatElement(int sizeMantissa, int sizeExponent, typename inheritedImplementation::IntNature nature,
         const MultiBit::Implementation::Details::TMultiBitElement<TypeMultiTraits>& intValue,
         typename inheritedImplementation::ReadParameters& params)
      :  inheritedImplementation(sizeMantissa, sizeExponent, nature, intValue, params) { inherited::setMultiFloat(); }
   TMultiFloatElement(const Details::MultiFloatElement& source) : inheritedImplementation(source) { inherited::setMultiFloat(); }
   TMultiFloatElement(int sizeMantissa, int sizeExponent,
         const Details::TMultiFloatElement<TypeMultiTraits>& floatValue,
         typename inheritedImplementation::ReadParameters& params)
      :  inheritedImplementation(sizeMantissa, sizeExponent, floatValue, params) { inherited::setMultiFloat(); }
   TMultiFloatElement(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   TemplateDefineCopy(TMultiFloatElement, TypeMultiTraits)
   DTemplateDefineAssign(TMultiFloatElement, TypeMultiTraits)
   DTemplateCompare(TMultiFloatElement, TypeMultiTraits)
   StaticInheritConversions(thisType, inherited)

   virtual bool isValid() const { return inherited::isValid() && inheritedImplementation::isValid(); }
   void assign(const Scalar::Implementation::VirtualElement& source) { inheritedImplementation::assign(source); }
   void assignPart(const thisType& source) { thisType::_assign(source); }

   // thisType getEpsilon() const { return ... }
   typedef inheritedImplementation InheritedImplementation;
   const inheritedImplementation& implementation() const { return *this; }
   inheritedImplementation& implementation() { return *this; }
   void write(STG::IOObject::OSBase& out, const STG::IOObject::FormatParameters& params) const
      {  inheritedImplementation::write(out, params); }
   void write(STG::IOObject::OSBase& out, const typename inheritedImplementation::WriteParameters& params) const
      {  inheritedImplementation::write(out, params); }
};

}}}} // end of namespace Analyzer::Scalar::Floating::Implementation

#endif // Analyzer_Scalar_Implementation_MultiFloatElementH
