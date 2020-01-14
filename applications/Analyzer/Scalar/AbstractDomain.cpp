/////////////////////////////////
//
// Library   : Static Analysis
// Unit      : AbstractDomain
// File      : AbstractDomain.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of a generic C Interface to manipulate abstract domains
//   = interval, disjunctions, ...
//

#include "Analyzer/Scalar/AbstractDomain.h"
#include "Analyzer/Scalar/ApproximateScalar.hpp"
#include "Analyzer/Scalar/Approximate/General/ConstantMultiBitShare.h"
#include "Analyzer/Scalar/Constant/ConstantElement.h"
#include <cassert>
#include <memory>

namespace Scalar = Analyzer::Scalar::Approximate;
namespace Constant = Analyzer::Scalar::Constant;
typedef Analyzer::Scalar::Approximate::Details::VirtualIntegerInterval VirtualIntegerInterval;
typedef Analyzer::Scalar::Approximate::Details::VirtualRealInterval VirtualRealInterval;

typedef Analyzer::Scalar::Approximate::VirtualElement VirtualElement;
typedef Analyzer::Scalar::Approximate::PPVirtualElement PPVirtualElement;
typedef Analyzer::Scalar::Approximate::EvaluationEnvironment EvaluationEnvironment;
typedef Analyzer::Scalar::Approximate::ConstraintEnvironment ConstraintEnvironment;

class ApproximateAccessEnvironment {
  public:
   class KeepMemoryValue : public Analyzer::Memory::VirtualMemoryModifier {
     private:
      typedef Analyzer::Memory::VirtualMemoryModifier inherited;
     
     public:
      KeepMemoryValue() {}
      KeepMemoryValue(const KeepMemoryValue& source) = default;
      DDefineCopy(KeepMemoryValue)
     
      virtual bool getContentUpdated(PNT::SharedPointer*& caller,
            Analyzer::Memory::VirtualModificationDate& date) override
         {  return false; }
   };
   class ShareMemoryValue : public Analyzer::Memory::VirtualMemoryModifier {
     private:
      typedef Analyzer::Memory::VirtualMemoryModifier inherited;
     
     public:
      ShareMemoryValue() {}
      ShareMemoryValue(const ShareMemoryValue& source) = default;
      DDefineCopy(ShareMemoryValue)
     
      virtual bool getContentUpdated(PNT::SharedPointer*& caller,
            Analyzer::Memory::VirtualModificationDate& date) override
         {  return true; }
   };

  private:
   class ModificationDate : public Analyzer::Memory::VirtualModificationDate {
     private:
      typedef Analyzer::Memory::VirtualModificationDate inherited;
      
     public:
      ModificationDate() {}
      ModificationDate(const ModificationDate& source) = default;
      DDefineCopy(ModificationDate)
   };
   ModificationDate mdModificationDate;

  public:
   ApproximateAccessEnvironment() {}

   Scalar::EvaluationEnvironment* createEnvironment(const Scalar::EvaluationEnvironment::Init& init) const;
   Scalar::ConstraintEnvironment* createConstraintEnvironment(const Scalar::EvaluationEnvironment::Init& init) const;

   Scalar::PPVirtualElement newConstant(unsigned* value, unsigned size, bool isSigned=false) const;
   Scalar::PPVirtualElement newConstant(const Analyzer::Scalar::Constant::VirtualElement& constant) const;
   Scalar::PPVirtualElement newBitConstant(bool value) const;
   Scalar::PPVirtualElement newFloatConstant(double value, unsigned sizeExponent, unsigned sizeMantissa) const;
   Scalar::PPVirtualElement newFloatConstant(const Analyzer::Scalar::Constant::VirtualElement& constant) const;
   Scalar::PPVirtualElement newInterval(unsigned* valueMin, unsigned* valueMax,
         unsigned size, bool isSigned, bool doesAcceptShareTop) const;
   Scalar::PPVirtualElement newInterval(Scalar::PPVirtualElement min, Scalar::PPVirtualElement max, bool doesAcceptShareTop) const;
   Scalar::PPVirtualElement newInterval(const VirtualIntegerInterval::Init& init, bool doesAcceptShareTop) const;
   Scalar::PPVirtualElement newFloatInterval(Scalar::PPVirtualElement min, Scalar::PPVirtualElement max, bool doesAcceptShareTop) const;
   Scalar::PPVirtualElement newFloatInterval(const VirtualRealInterval::Init& init, bool doesAcceptShareTop) const;
   PNT::TPassPointer<Scalar::VirtualDisjunction, Analyzer::Scalar::PPAbstractElement>
         newDisjunction(unsigned size) const;
   Scalar::PPVirtualElement newBitUndefined(bool doesAcceptShareTop) const;
   Scalar::PPVirtualElement newUndefined(unsigned size, bool doesAcceptShareTop) const;
   Scalar::PPVirtualElement newFloatUndefined(int sizeExponent, int sizeMantissa, bool doesAcceptShareTop) const;
   bool isConstantValue(const Scalar::VirtualElement& value) const;
   void retrieveValue(const Scalar::VirtualElement& constantValue, unsigned* result, unsigned size) const;
   bool isConstantFloatValue(const Scalar::VirtualElement& value) const;
   double getConstantFloat(const Scalar::VirtualElement& constantValue) const;
};

class ApproximateCastRingTraits {
  public:
   typedef Scalar::VirtualElement VirtualElement;
   typedef Scalar::PPVirtualElement PVirtualElement;

   static VirtualElement* createCharElement(const VirtualElement::Init& init);
   static VirtualElement* createSignedCharElement(const VirtualElement::Init& init);
   static VirtualElement* createUnsignedCharElement(const VirtualElement::Init& init);
   static VirtualElement* createShortElement(const VirtualElement::Init& init);
   static VirtualElement* createUnsignedShortElement(const VirtualElement::Init& init);
   static VirtualElement* createIntElement(const VirtualElement::Init& init);
   static VirtualElement* createUnsignedIntElement(const VirtualElement::Init& init);
   static VirtualElement* createLongElement(const VirtualElement::Init& init);
   static VirtualElement* createUnsignedLongElement(const VirtualElement::Init& init);
   static VirtualElement* createFloatElement(const VirtualElement::InitFloat& init);
   static VirtualElement* createDoubleElement(const VirtualElement::InitFloat& init);
   static VirtualElement* createLongDoubleElement(const VirtualElement::InitFloat& init);
   static VirtualElement* createBitElement(const VirtualElement::Init& init);
   static VirtualElement* createMultiBitElement(const VirtualElement::Init& init);
   static VirtualElement* createMultiFloatElement(const Scalar::VirtualElement::InitFloat& init);
   static Constant::VirtualElement* createConstantMultiFloatElement(const Constant::VirtualElement::InitFloat& init);
};

std::unique_ptr<Scalar::EvaluationEnvironment> sharedEvaluationEnvironment;
std::unique_ptr<Scalar::ConstraintEnvironment> sharedConstraintEnvironment;

#include "Numerics/HostFloating.h"
#include "Analyzer/Scalar/Approximate/MethodTable.template"
#include "Analyzer/Scalar/Approximate/General/ConstantBit.template"
#include "Analyzer/Scalar/Approximate/General/ConstantMultiBit.template"
#include "Analyzer/Scalar/Approximate/General/ConstantMultiFloat.template"
#include "Numerics/HostFloating.template"

Scalar::VirtualElement*
ApproximateCastRingTraits::createBitElement(const VirtualElement::Init& init)
   {  return new Analyzer::Scalar::Bit::Approximate::TCloseConstantElement<ApproximateCastRingTraits>(init); }

Scalar::VirtualElement*
ApproximateCastRingTraits::createMultiBitElement(const VirtualElement::Init& init)
   {  return new Analyzer::Scalar::MultiBit::Approximate::TCloseConstantElement<
            Analyzer::Scalar::MultiBit::Approximate::ConstantElement, ApproximateCastRingTraits>(init);
   }

Scalar::VirtualElement*
ApproximateCastRingTraits::createMultiFloatElement(const Scalar::VirtualElement::InitFloat& init)
   {  return new Analyzer::Scalar::Floating::Approximate::TCloseMultiConstantElement<
            Analyzer::Scalar::Floating::Approximate::MultiConstantElement, ApproximateCastRingTraits>(init);
   }

Constant::VirtualElement*
ApproximateCastRingTraits::createConstantMultiFloatElement(const Constant::VirtualElement::InitFloat& init)
   {  AssumeUnimplemented return nullptr; }


Scalar::VirtualElement*
ApproximateCastRingTraits::createCharElement(const VirtualElement::Init& init)
   {  return nullptr; }

Scalar::VirtualElement*
ApproximateCastRingTraits::createSignedCharElement(const VirtualElement::Init& init)
   {  return nullptr; }

Scalar::VirtualElement*
ApproximateCastRingTraits::createUnsignedCharElement(const VirtualElement::Init& init)
   {  return nullptr; }

Scalar::VirtualElement*
ApproximateCastRingTraits::createShortElement(const VirtualElement::Init& init)
   {  return nullptr; }

Scalar::VirtualElement*
ApproximateCastRingTraits::createUnsignedShortElement(const VirtualElement::Init& init)
   {  return nullptr; }

Scalar::VirtualElement*
ApproximateCastRingTraits::createIntElement(const VirtualElement::Init& init)
   {  return nullptr; }

Scalar::VirtualElement*
ApproximateCastRingTraits::createUnsignedIntElement(const VirtualElement::Init& init)
   {  return nullptr; }

Scalar::VirtualElement*
ApproximateCastRingTraits::createLongElement(const VirtualElement::Init& init)
   {  return nullptr; }

Scalar::VirtualElement*
ApproximateCastRingTraits::createUnsignedLongElement(const VirtualElement::Init& init)
   {  return nullptr; }

Scalar::VirtualElement*
ApproximateCastRingTraits::createFloatElement(const VirtualElement::InitFloat& init)
   {  return nullptr; }

Scalar::VirtualElement*
ApproximateCastRingTraits::createDoubleElement(const VirtualElement::InitFloat& init)
   {  return nullptr; }

Scalar::VirtualElement*
ApproximateCastRingTraits::createLongDoubleElement(const VirtualElement::InitFloat& init)
   {  return nullptr; }

Scalar::EvaluationEnvironment*
ApproximateAccessEnvironment::createEnvironment(const Scalar::EvaluationEnvironment::Init& init) const
   {  Numerics::DDouble::Access::ReadParameters params;
      params.avoidInfty().setRoundToEven().setRefuseMinusZero();
      const_cast<Scalar::EvaluationEnvironment::Init&>(init).setOpenFloating()
              .setMachine(params.queryStaticMachineField()).setMode(0U)
              .setRequired(~(~0U << 11)).close();
      auto* result = new Scalar::EvaluationEnvironment((const Scalar::EvaluationEnvironment::Init&) init);
      result->setModificationDate(const_cast<ApproximateAccessEnvironment*>(this)->mdModificationDate);
      return result;
   }

Scalar::ConstraintEnvironment*
ApproximateAccessEnvironment::createConstraintEnvironment(const Scalar::EvaluationEnvironment::Init& init) const
   {  Numerics::DDouble::Access::ReadParameters params;
      params.avoidInfty().setRoundToEven().setRefuseMinusZero();
      const_cast<Scalar::EvaluationEnvironment::Init&>(init).setOpenFloating()
              .setMachine(params.queryStaticMachineField()).setMode(0U)
              .setRequired(~(~0U << 11)).close();
      auto* result = new Scalar::ConstraintEnvironment((const Scalar::EvaluationEnvironment::Init&) init);
      result->setModificationDate(const_cast<ApproximateAccessEnvironment*>(this)->mdModificationDate);
      return result;
   }

Scalar::PPVirtualElement
ApproximateAccessEnvironment::newConstant(unsigned* value, unsigned size, bool isSigned) const {
   AssumeCondition(value)
   Scalar::PPVirtualElement result;
   typedef Analyzer::Scalar::MultiBit::Approximate::TCloseConstantElement<
      Analyzer::Scalar::MultiBit::Approximate::ConstantElement, ApproximateCastRingTraits> Result;
   Result* resultValue = new Result(Result::Init().setBitSize(size));
   result.absorbElement(resultValue);
   if (size > 0) {
      for (int index = (size-1)/(8*sizeof(unsigned)); index >= 0; --index)
         resultValue->implementation()[index] = value[index];
   };
   return result;
}

Scalar::PPVirtualElement
ApproximateAccessEnvironment::newConstant(const Analyzer::Scalar::Constant::VirtualElement& constant) const {
   typedef Analyzer::Scalar::MultiBit::Approximate::TCloseConstantElement<
      Analyzer::Scalar::MultiBit::Approximate::ConstantElement, ApproximateCastRingTraits> Result;
   Result::Init init;
   init.setBitSize(constant.getSizeInBits());
   if (constant.isInt()) {
      AssumeCondition(dynamic_cast<const Analyzer::Scalar::Constant::UnsignedIntElement*>(&constant))
      init.absorbInitialValue(new Analyzer::Scalar::Integer::Implementation::UnsignedIntElement((
               (const Analyzer::Scalar::Constant::UnsignedIntElement&) constant).implementation()));
   }
   else {
      AssumeCondition(constant.isMultiBit())
      typedef Analyzer::Scalar::MultiBit::VirtualExact::TMultiBitElement<
            Analyzer::Scalar::Constant::VirtualElement,
            Analyzer::Scalar::MultiBit::Implementation::MultiBitElement> MultiBitElement;
      AssumeCondition(dynamic_cast<const MultiBitElement*>(&constant))
      init.absorbInitialValue(new Analyzer::Scalar::MultiBit::Implementation::MultiBitElement((
               (const MultiBitElement&) constant).implementation()));
   }
   return Scalar::PPVirtualElement((Scalar::VirtualElement*) new Result(init), PNT::Pointer::Init());
}

Scalar::PPVirtualElement
ApproximateAccessEnvironment::newBitConstant(bool value) const {
   typedef Analyzer::Scalar::Bit::Approximate::TCloseConstantElement<ApproximateCastRingTraits> Result;
   return Scalar::PPVirtualElement(new Result(Result::Init().setBitSize(1)
      .absorbInitialValue(new Analyzer::Scalar::Integer::Implementation::IntElement(value))),
      PNT::Pointer::Init());
}

Scalar::PPVirtualElement
ApproximateAccessEnvironment::newFloatConstant(double value, unsigned sizeExponent, unsigned sizeMantissa) const {
   AssumeCondition(value)
   Scalar::PPVirtualElement result;
   typedef Numerics::TDoubleElement<Numerics::TFloatingBase<Numerics::DoubleTraits> > Double;
   typedef Analyzer::Scalar::Floating::Approximate::TCloseMultiConstantElement<
      Analyzer::Scalar::Floating::Approximate::MultiConstantElement, ApproximateCastRingTraits> Result;
   Result* resultValue = new Result(Result::InitFloat().setSizeExponent(sizeExponent).setSizeMantissa(sizeMantissa));
   result.absorbElement(resultValue);
   Double::BuiltDouble implementation = Double::BuiltDouble(Double(value));
   for (int index = (sizeExponent-1)/(8*sizeof(unsigned)); index >= 0; --index)
      resultValue->implementation().getSBasicExponent()[index] = implementation.getBasicExponent()[index];
   for (int index = (sizeMantissa-1)/(8*sizeof(unsigned)); index >= 0; --index)
      resultValue->implementation().getSMantissa()[index] = implementation.getMantissa()[index];
   resultValue->implementation().setNegative(implementation.isNegative());
   return result;
}

Scalar::PPVirtualElement
ApproximateAccessEnvironment::newFloatConstant(const Analyzer::Scalar::Constant::VirtualElement& constant) const {
   typedef Analyzer::Scalar::Floating::Approximate::TCloseMultiConstantElement<
      Analyzer::Scalar::Floating::Approximate::MultiConstantElement, ApproximateCastRingTraits> Result;
   Result::InitFloat init;
   // init.setBitSize(size);
   if (constant.isFloat()) {
      typedef Analyzer::Scalar::Floating::VirtualExact::TMultiFloatElement<
            Analyzer::Scalar::Constant::VirtualElement, Analyzer::Scalar::Floating::Implementation::MultiFloatElement> MultiFloatElement;
      AssumeCondition(dynamic_cast<const MultiFloatElement*>(&constant))
      init.absorbInitialValue(new Analyzer::Scalar::Floating::Implementation::MultiFloatElement((
               (const MultiFloatElement&) constant).implementation()));
   }
   else {
      AssumeCondition(constant.isFloat())
      AssumeCondition(dynamic_cast<const Analyzer::Scalar::Constant::DoubleElement*>(&constant))
      init.setSizeExponent(11).setSizeMantissa(52)
         .absorbInitialValue(new Analyzer::Scalar::Floating::Implementation::DoubleElement(
            ((const Analyzer::Scalar::Constant::DoubleElement&) constant).implementation()));
   };
   return Scalar::PPVirtualElement((Scalar::VirtualElement*) new Result(init), PNT::Pointer::Init());
}

Scalar::PPVirtualElement
ApproximateAccessEnvironment::newInterval(unsigned* valueMin, unsigned* valueMax, unsigned size, bool isSigned,
      bool doesAcceptShareTop) const {
   typedef Analyzer::Scalar::MultiBit::Approximate::TCloseConstantElement<
      Analyzer::Scalar::MultiBit::Approximate::ConstantElement, ApproximateCastRingTraits> ConstantResult;
   Scalar::PPVirtualElement result;
   if (size == 1)
      result = Scalar::Details::IntOperationElement::Methods::newTop(
            ConstantResult(ConstantResult::Init().setBitSize(size)));
   else {
      ConstantResult* minResult = new ConstantResult(ConstantResult::Init().setBitSize(size));
      Scalar::PPVirtualElement min(minResult, PNT::Pointer::Init());
      ConstantResult* maxResult = new ConstantResult(ConstantResult::Init().setBitSize(size));
      Scalar::PPVirtualElement max(maxResult, PNT::Pointer::Init());
      if (size > 0) {
         for (int index = (size-1)/(8*sizeof(unsigned)); index >= 0; --index)
            minResult->implementation()[index] = valueMin[index];
         for (int index = (size-1)/(8*sizeof(unsigned)); index >= 0; --index)
            maxResult->implementation()[index] = valueMax[index];
      };
      const Scalar::VirtualElement& source = *min;
      VirtualIntegerInterval::Init init;
      init.setInterval(min, max);
      init.setUnsigned(!isSigned);
      result = Scalar::Details::IntOperationElement::Methods::newInterval(source, init);
   };
   if (doesAcceptShareTop) {
      Analyzer::Scalar::MultiBit::Approximate::ExactToApproximateElement::Init init;
      init.setBitSize(result->getSizeInBits());
      auto* sharedResult = new Analyzer::Scalar::MultiBit::Approximate
            ::ExactToApproximateElement(init, result /*, nullptr */);
      Scalar::PPVirtualElement newResult(sharedResult, PNT::Pointer::Init());
      // MemoryModifier memoryModifier;
      // sharedResult->setParent(memoryModifier);
      result = newResult;
   };
   return result;
}

Scalar::PPVirtualElement
ApproximateAccessEnvironment::newInterval(Scalar::PPVirtualElement min, Scalar::PPVirtualElement max,
      bool doesAcceptShareTop) const {
   VirtualIntegerInterval::Init init;
   const Scalar::VirtualElement& source = (const Scalar::VirtualElement&) *min;
   init.setInterval(min, max);
   auto result = Scalar::Details::IntOperationElement::Methods::newInterval(source, init);
   if (doesAcceptShareTop) {
      Analyzer::Scalar::MultiBit::Approximate::ExactToApproximateElement::Init init;
      init.setBitSize(result->getSizeInBits());
      auto* sharedResult = new Analyzer::Scalar::MultiBit::Approximate
            ::ExactToApproximateElement(init, result /*, nullptr */);
      Scalar::PPVirtualElement newResult(sharedResult, PNT::Pointer::Init());
      // MemoryModifier memoryModifier;
      // sharedResult->setParent(memoryModifier);
      result = newResult;
   };
   return result;
}

Scalar::PPVirtualElement
ApproximateAccessEnvironment::newInterval(const VirtualIntegerInterval::Init& init,
      bool doesAcceptShareTop) const {
   Scalar::PPVirtualElement result(new Analyzer::Scalar::MultiBit::Approximate::Interval(init), PNT::Pointer::Init());
   if (doesAcceptShareTop) {
      Analyzer::Scalar::MultiBit::Approximate::ExactToApproximateElement::Init init;
      init.setBitSize(result->getSizeInBits());
      auto* sharedResult = new Analyzer::Scalar::MultiBit::Approximate
            ::ExactToApproximateElement(init, result /*, nullptr */);
      Scalar::PPVirtualElement newResult(sharedResult, PNT::Pointer::Init());
      // MemoryModifier memoryModifier;
      // sharedResult->setParent(memoryModifier);
      result = newResult;
   };
   return result;
}

Scalar::PPVirtualElement
ApproximateAccessEnvironment::newFloatInterval(Scalar::PPVirtualElement min, Scalar::PPVirtualElement max,
      bool doesAcceptShareTop) const {
   VirtualIntegerInterval::Init init;
   const Scalar::VirtualElement& source = (const Scalar::VirtualElement&) *min;
   init.setInterval(min, max);
   auto result = Scalar::Details::IntOperationElement::Methods::newInterval(source, init);
   if (doesAcceptShareTop) {
      Analyzer::Scalar::MultiBit::Approximate::ExactToApproximateElement::Init init;
      init.setBitSize(result->getSizeInBits());
      auto* sharedResult = new Analyzer::Scalar::MultiBit::Approximate
            ::ExactToApproximateElement(init, result /*, nullptr */);
      Scalar::PPVirtualElement newResult(sharedResult, PNT::Pointer::Init());
      // MemoryModifier memoryModifier;
      // sharedResult->setParent(memoryModifier);
      result = newResult;
   };
   return result;
}

Scalar::PPVirtualElement
ApproximateAccessEnvironment::newFloatInterval(const VirtualRealInterval::Init& init,
      bool doesAcceptShareTop) const {
   Scalar::PPVirtualElement result(new Analyzer::Scalar::Floating::Approximate::Interval(init), PNT::Pointer::Init());
   if (doesAcceptShareTop) {
      Analyzer::Scalar::MultiBit::Approximate::ExactToApproximateElement::Init init;
      init.setBitSize(result->getSizeInBits());
      auto* sharedResult = new Analyzer::Scalar::MultiBit::Approximate
            ::ExactToApproximateElement(init, result /*, nullptr */);
      Scalar::PPVirtualElement newResult(sharedResult, PNT::Pointer::Init());
      // MemoryModifier memoryModifier;
      // sharedResult->setParent(memoryModifier);
      result = newResult;
   };
   return result;
}

PNT::TPassPointer<Scalar::VirtualDisjunction, Analyzer::Scalar::PPAbstractElement>
ApproximateAccessEnvironment::newDisjunction(unsigned size) const {
   typedef Analyzer::Scalar::MultiBit::Approximate::TCloseConstantElement<
      Analyzer::Scalar::MultiBit::Approximate::ConstantElement, ApproximateCastRingTraits> ConstantResult;
   return Scalar::Details::IntOperationElement::Methods::newDisjunction(
         ConstantResult(ConstantResult::Init().setBitSize(size)));
}

Scalar::PPVirtualElement
ApproximateAccessEnvironment::newUndefined(unsigned size, bool doesAcceptShareTop) const {
   typedef Analyzer::Scalar::MultiBit::Approximate::TCloseConstantElement<
      Analyzer::Scalar::MultiBit::Approximate::ConstantElement, ApproximateCastRingTraits> ConstantResult;
   Scalar::PPVirtualElement result = Scalar::Details::IntOperationElement::Methods
      ::newTop(ConstantResult(ConstantResult::Init().setBitSize(size)));
   if (size > 1) {
      if (doesAcceptShareTop) {
         Analyzer::Scalar::MultiBit::Approximate::ExactToApproximateElement::Init init;
         init.setBitSize(size);
         auto* sharedResult = new Analyzer::Scalar::MultiBit::Approximate
               ::ExactToApproximateElement(init, result /*, nullptr */);
         Scalar::PPVirtualElement newResult(sharedResult, PNT::Pointer::Init());
         // MemoryModifier memoryModifier;
         // sharedResult->setParent(memoryModifier);
         result = newResult;
      };
   };
   return result;
}

Scalar::PPVirtualElement
ApproximateAccessEnvironment::newBitUndefined(bool doesAcceptShareTop) const {
   typedef Analyzer::Scalar::MultiBit::Approximate::TCloseConstantElement<
      Analyzer::Scalar::MultiBit::Approximate::ConstantElement, ApproximateCastRingTraits> ConstantResult;
   Scalar::PPVirtualElement result = Scalar::Details::IntOperationElement::Methods
      ::newTop(ConstantResult(ConstantResult::Init().setBitSize(1)));
   if (doesAcceptShareTop) {
      Analyzer::Scalar::MultiBit::Approximate::ExactToApproximateElement::Init init;
      init.setBitSize(1);
      auto* sharedResult = new Analyzer::Scalar::MultiBit::Approximate
            ::ExactToApproximateElement(init, result /*, nullptr */);
      Scalar::PPVirtualElement newResult(sharedResult, PNT::Pointer::Init());
      // MemoryModifier memoryModifier;
      // sharedResult->setParent(memoryModifier);
      result = newResult;
   };
   return Scalar::Details::IntOperationElement::Methods::apply(*result,
      Analyzer::Scalar::MultiBit::CastBitOperation(), Scalar::VirtualElement::EPMayStopErrorStates);
}

Scalar::PPVirtualElement
ApproximateAccessEnvironment::newFloatUndefined(int sizeExponent, int sizeMantissa,
      bool doesAcceptShareTop) const {
   typedef Analyzer::Scalar::Floating::Approximate::TCloseMultiConstantElement<
      Analyzer::Scalar::Floating::Approximate::MultiConstantElement, ApproximateCastRingTraits> ConstantResult;
   return Scalar::Details::RealOperationElement::Methods::newTop(
         ConstantResult(ConstantResult::InitFloat().setSizeMantissa(sizeMantissa).setSizeExponent(sizeExponent)));
}

bool
ApproximateAccessEnvironment::isConstantValue(const Scalar::VirtualElement& value) const {
   return value.getApproxKind().isConstant();
}

void
ApproximateAccessEnvironment::retrieveValue(const Scalar::VirtualElement& constantValue, unsigned* result, unsigned size) const {
   typedef Analyzer::Scalar::MultiBit::Approximate::TCloseConstantElement<
      Analyzer::Scalar::MultiBit::Approximate::ConstantElement, ApproximateCastRingTraits> Result;
   AssumeCondition(dynamic_cast<const Result*>(&constantValue))
   const auto& implementation = ((const Result&) constantValue).implementation();
   if (size > 0) {
      for (int index = size-1; index >= 0; --index)
         result[index] = implementation[index];
   };
}

bool
ApproximateAccessEnvironment::isConstantFloatValue(const Scalar::VirtualElement& value) const {
   return value.isFloat() && value.getApproxKind().isConstant();
}

double
ApproximateAccessEnvironment::getConstantFloat(const Scalar::VirtualElement& constantValue) const {
   typedef Numerics::TDoubleElement<Numerics::TFloatingBase<Numerics::DoubleTraits> > Double;
   typedef Analyzer::Scalar::Floating::Approximate::TCloseMultiConstantElement<
      Analyzer::Scalar::Floating::Approximate::MultiConstantElement, ApproximateCastRingTraits> Result;
   AssumeCondition(dynamic_cast<const Result*>(&constantValue))
   const Result& resultValue = (const Result&) constantValue;
   Double::BuiltDouble implementation;
   int sizeExponent = ((const Result&) constantValue).getSizeExponent();
   int sizeMantissa = ((const Result&) constantValue).getSizeMantissa();
   if (sizeExponent > 0 && sizeMantissa > 0) {
      for (int index = (sizeExponent-1)/(8*sizeof(unsigned)); index >= 0; --index)
         implementation.getSBasicExponent()[index] = resultValue.implementation().getBasicExponent()[index];
      for (int index = (sizeMantissa-1)/(8*sizeof(unsigned)); index >= 0; --index)
          implementation.getSMantissa()[index] = resultValue.implementation().getMantissa()[index];
      implementation.setNegative(resultValue.implementation().isNegative());
   };
   return Double(implementation).implementation();
}

class WriteStream : public STG::IOObject::OSBase {
  public:
   typedef STG::IOObject::OSBase OSBase;

  private:
   char* szBuffer;
   int* puBufferLength;
   int uBufferSize;
   char* (*fnIncreaseBufferSize)(char* buffer, int oldLength, int newLength, void* writer);
   void* pvWriter;
   static STG::SubString ssTemp;

  protected:
   bool realloc(int places)
      {  if (*puBufferLength + places + 1 > uBufferSize) {
            int newSize = *puBufferLength + places + 1;
            if (newSize < 3*uBufferSize/2+1)
               newSize = 3*uBufferSize/2+1;
            szBuffer = fnIncreaseBufferSize(szBuffer, uBufferSize, newSize, pvWriter);
            if (szBuffer == nullptr)
               return false;
            uBufferSize = newSize;
         }
         return true;
      }

  public:
   WriteStream(char* buffer, int buffer_size, int* length, void* writer,
         char* (*increase_buffer_size)(char* buffer, int old_length, int new_length, void*))
      :  szBuffer(buffer), puBufferLength(length), uBufferSize(buffer_size),
         fnIncreaseBufferSize(increase_buffer_size), pvWriter(writer)
      {  AssumeCondition(puBufferLength) *puBufferLength = 0; }
   WriteStream(const WriteStream& source) = delete;

   bool isValid() const { return true; }
   char* getBuffer() const { return szBuffer; }

   /* overload methods */
   virtual OSBase& put(char c) override
      {  if (!szBuffer || !realloc(1))
            return *this;
         AssumeCondition((*puBufferLength+1) < uBufferSize)
         szBuffer[(*puBufferLength)++] = c;
         szBuffer[(*puBufferLength)+1] = '\0';
         return *this;
      }
   virtual int tellp() override { return *puBufferLength; }
   virtual OSBase& seekp(int pos) override { AssumeUncalled return *this; }
   virtual OSBase& seekp(int pos, int origin) override { AssumeUncalled return *this; }

   /* high-level methods */
   virtual OSBase& write(char ch, bool isRaw) override
      {  if (!szBuffer || !realloc(1))
            return *this;
         AssumeCondition((*puBufferLength+1) < uBufferSize)
         szBuffer[(*puBufferLength)++] = ch;
         szBuffer[(*puBufferLength)+1] = '\0';
         return *this;
      }
   virtual OSBase& write(int n, bool isRaw) override
      {  if (isRaw) {
            if (!szBuffer || !realloc(sizeof(n)))
               return *this;
            memcpy(&szBuffer[*puBufferLength], &n, sizeof(n));
            *puBufferLength += sizeof(n);
         }      
         else {
            ssTemp.copyInteger(n);
            if (!szBuffer || !realloc(ssTemp.length()))
               return *this;
            memcpy(&szBuffer[*puBufferLength], ssTemp.getChunk().string, ssTemp.length());
            *puBufferLength += ssTemp.length();
         };
         szBuffer[(*puBufferLength)+1] = '\0';
         return *this;
      }
   virtual OSBase& writeHexa(int n) override
      {  ssTemp.copyHexaInteger(n);
         if (!szBuffer || !realloc(ssTemp.length()))
            return *this;
         memcpy(&szBuffer[*puBufferLength], ssTemp.getChunk().string, ssTemp.length());
         *puBufferLength += ssTemp.length();
         szBuffer[(*puBufferLength)+1] = '\0';
         return *this;
      }
   virtual OSBase& write(unsigned int n, bool isRaw) override
      {  if (isRaw) {
            if (!szBuffer || !realloc(sizeof(n)))
               return *this;
            memcpy(&szBuffer[*puBufferLength], &n, sizeof(n));
            *puBufferLength += sizeof(n);
         }      
         else {
            ssTemp.copyInteger(n);
            if (!szBuffer || !realloc(ssTemp.length()))
               return *this;
            memcpy(&szBuffer[*puBufferLength], ssTemp.getChunk().string, ssTemp.length());
            *puBufferLength += ssTemp.length();
         };
         szBuffer[(*puBufferLength)+1] = '\0';
         return *this;
      }
   virtual OSBase& writeHexa(unsigned int n) override
      {  ssTemp.copyHexaInteger(n);
         if (!szBuffer || !realloc(ssTemp.length()))
            return *this;
         memcpy(&szBuffer[*puBufferLength], ssTemp.getChunk().string, ssTemp.length());
         *puBufferLength += ssTemp.length();
         szBuffer[(*puBufferLength)+1] = '\0';
         return *this;
      }
   virtual OSBase& write(long int n, bool isRaw) override
      {  if (isRaw) {
            if (!szBuffer || !realloc(sizeof(n)))
               return *this;
            memcpy(&szBuffer[*puBufferLength], &n, sizeof(n));
            *puBufferLength += sizeof(n);
         }      
         else {
            ssTemp.copyInteger(n);
            if (!szBuffer || !realloc(ssTemp.length()))
               return *this;
            memcpy(&szBuffer[*puBufferLength], ssTemp.getChunk().string, ssTemp.length());
            *puBufferLength += ssTemp.length();
         };
         szBuffer[(*puBufferLength)+1] = '\0';
         return *this;
      }
   virtual OSBase& writeHexa(long int n) override
      {  ssTemp.copyHexaInteger(n);
         if (!szBuffer || !realloc(ssTemp.length()))
            return *this;
         memcpy(&szBuffer[*puBufferLength], ssTemp.getChunk().string, ssTemp.length());
         *puBufferLength += ssTemp.length();
         szBuffer[(*puBufferLength)+1] = '\0';
         return *this;
      }
   virtual OSBase& write(unsigned long int n, bool isRaw) override
      {  if (isRaw) {
            if (!szBuffer || !realloc(sizeof(n)))
               return *this;
            memcpy(&szBuffer[*puBufferLength], &n, sizeof(n));
            *puBufferLength += sizeof(n);
         }      
         else {
            ssTemp.copyInteger(n);
            if (!szBuffer || !realloc(ssTemp.length()))
               return *this;
            memcpy(&szBuffer[*puBufferLength], ssTemp.getChunk().string, ssTemp.length());
            *puBufferLength += ssTemp.length();
         };
         szBuffer[(*puBufferLength)+1] = '\0';
         return *this;
      }
   virtual OSBase& writeHexa(unsigned long int n) override
      {  ssTemp.copyHexaInteger(n);
         if (!szBuffer || !realloc(ssTemp.length()))
            return *this;
         memcpy(&szBuffer[*puBufferLength], ssTemp.getChunk().string, ssTemp.length());
         *puBufferLength += ssTemp.length();
         szBuffer[(*puBufferLength)+1] = '\0';
         return *this;
      }
   virtual OSBase& write(double f, bool isRaw) override
      {  if (isRaw) {
            if (!szBuffer || !realloc(sizeof(f)))
               return *this;
            memcpy(&szBuffer[*puBufferLength], &f, sizeof(f));
            *puBufferLength += sizeof(f);
         }
         else {
            ssTemp.copyFloat(f);
            if (!szBuffer || !realloc(ssTemp.length()))
               return *this;
            memcpy(&szBuffer[*puBufferLength], ssTemp.getChunk().string, ssTemp.length());
            *puBufferLength += ssTemp.length();
         }
         szBuffer[(*puBufferLength)+1] = '\0';
         return *this;
      }
   virtual OSBase& write(bool b, bool isRaw) override
      {  if (!szBuffer || !realloc(1))
            return *this;
         AssumeCondition((*puBufferLength+1) < uBufferSize)
         szBuffer[(*puBufferLength)++] = b ? '1' : '0';
         szBuffer[(*puBufferLength)+1] = '\0';
         return *this;
      }
   virtual OSBase& write(const STG::VirtualStringProperty& source, bool isRaw) override
      {  int length = source.length();
         if (isRaw) {
            if (!szBuffer || !realloc(sizeof(length)))
               return *this;
            memcpy(&szBuffer[*puBufferLength], &length, sizeof(length));
            *puBufferLength += sizeof(length);
         }
         else {
            ssTemp.copyInteger(length);
            if (!szBuffer || !realloc(ssTemp.length()+1))
               return *this;
            memcpy(&szBuffer[*puBufferLength], ssTemp.getChunk().string, ssTemp.length());
            *puBufferLength += ssTemp.length();
            szBuffer[(*puBufferLength)++] = ' ';
         };
         if (source.setToFirstChunk()) {
            STG::TChunk<char>* chunk = nullptr;
            do {
               chunk = (STG::TChunk<char>*) source.getChunk();
               if (!szBuffer || !realloc(chunk->length))
                  return *this;
               memcpy(&szBuffer[*puBufferLength], chunk->string, chunk->length);
               *puBufferLength += chunk->length;
            } while (source.setToNextChunk());
         };
         szBuffer[(*puBufferLength)+1] = '\0';
         return *this;
      }
   virtual OSBase& writeall(const STG::VirtualStringProperty& source) override
      {  if (source.setToFirstChunk()) {
            STG::TChunk<char>* chunk = nullptr;
            do {
               chunk = (STG::TChunk<char>*) source.getChunk();
               if (!szBuffer || !realloc(chunk->length))
                  return *this;
               memcpy(&szBuffer[*puBufferLength], chunk->string, chunk->length);
               *puBufferLength += chunk->length;
            } while (source.setToNextChunk());
         };
         return *this;
      }
   virtual OSBase& writechunk(void* achunk) override
      {  STG::TChunk<char>& chunk = *((STG::TChunk<char>*) achunk);
         memcpy(&szBuffer[*puBufferLength], chunk.string, chunk.length);
         *puBufferLength += chunk.length;
         return *this;
      }
   virtual OSBase& flush() override { return *this; }
};

STG::SubString WriteStream::ssTemp = STG::SString();

extern "C" {

DLL_API DomainType
domain_get_type(DomainElement domain) {
   VirtualElement* element = reinterpret_cast<VirtualElement*>(domain.content);
   assert(element);
   return element->isMultiBit() ? DTInteger : (element->isBoolean() ? DTBit : DTFloating);
}

DLL_API int
domain_get_size_in_bits(DomainElement domain) {
   VirtualElement* element = reinterpret_cast<VirtualElement*>(domain.content);
   assert(element);
   return element->getSizeInBits();
}

DLL_API ZeroResult
domain_query_zero_result(DomainElement domain) {
   VirtualElement* element = reinterpret_cast<VirtualElement*>(domain.content);
   assert(element);
   auto result = element->queryZeroResult();
   return result.isExact() ? (result.isTrue() ? ZRDifferentZero : ZRZero) : ZRUndefined;
}

DLL_API void
domain_free(DomainElement* domain) {
   VirtualElement* element = reinterpret_cast<VirtualElement*>(domain->content);
   if (element) {
      delete element;
      domain->content = nullptr;
   }
}

DLL_API DomainElement
domain_clone(DomainElement domain) {
   VirtualElement* element = reinterpret_cast<VirtualElement*>(domain.content);
   if (element)
      return DomainElement{element->createSCopy()};
   return domain;
}

DLL_API DomainBitElement
domain_bit_create_constant(bool value) {
   return DomainElement{ApproximateAccessEnvironment().newBitConstant(value).extractElement()};
}

DLL_API DomainBitElement
domain_bit_create_top(bool isSymbolic) {
   return DomainElement{ApproximateAccessEnvironment().newBitUndefined(isSymbolic).extractElement()};
}

DLL_API DomainMultiBitElement
domain_bit_create_cast_multibit(DomainBitElement bitDomain, int sizeInBits,
      DomainEvaluationEnvironment* env) {
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(bitDomain.content);
   PPVirtualElement result = VirtualElement::Methods::apply(*element,
      Analyzer::Scalar::Bit::CastMultiBitOperation()
         .setSize(sizeInBits), *sharedEvaluationEnvironment);
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainElement{result.extractElement()};
}

DLL_API bool
domain_bit_unary_apply_assign(DomainBitElement* bitDomain,
      DomainBitUnaryOperation operation, DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::Bit::Operation Operation;
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(bitDomain->content), PNT::Pointer::Init());
   bitDomain->content = nullptr;
   Operation bitOperation;
   bitOperation.setArgumentsNumber(0);
   if (operation >= DBUOPrev && operation <= DBUONext)
      bitOperation.setType((Operation::Type) ((operation-DBUOPrev) + Operation::TPrevAssign));
   else if (operation == DBUONegate)
      bitOperation.setType(Operation::TNegateAssign);
   else
      {  AssumeUncalled }
   element = VirtualElement::Methods::applyAssign(element, bitOperation, *sharedEvaluationEnvironment);
   bitDomain->content = element.extractElement();
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return bitDomain->content != nullptr && !env->emptyResult;
}

DLL_API DomainBitElement
domain_bit_create_unary_apply(DomainBitElement bitDomain, DomainBitUnaryOperation operation,
      DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::Bit::Operation Operation;
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(bitDomain.content);
   AssumeCondition(element)
   Operation bitOperation;
   bitOperation.setArgumentsNumber(0);
   if (operation >= DBUOPrev && operation <= DBUONext)
      bitOperation.setType((Operation::Type) ((operation-DBUOPrev) + Operation::TPrevAssign));
   else if (operation == DBUONegate)
      bitOperation.setType(Operation::TNegateAssign);
   else
      {  AssumeUncalled }
   bitOperation.setConstWithAssign();

   PPVirtualElement result = VirtualElement::Methods::apply(*element, bitOperation,
         *sharedEvaluationEnvironment);
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainElement{result.extractElement()};
}

DLL_API bool
domain_bit_binary_apply_assign(DomainBitElement* bitDomain, DomainBitBinaryOperation operation,
      DomainBitElement sourceBitDomain, DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::Bit::Operation Operation;
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(bitDomain->content), PNT::Pointer::Init());
   bitDomain->content = nullptr;
   Operation bitOperation;
   bitOperation.setArgumentsNumber(1);
   if (operation >= DBBOPlus && operation <= DBBOMinus)
      bitOperation.setType((Operation::Type) ((operation-DBBOPlus) + Operation::TPlusAssign));
   else if (operation >= DBBOMin && operation <= DBBOExclusiveOr)
      bitOperation.setType((Operation::Type) ((operation-DBBOMin) + Operation::TMinAssign));
   else
      {  AssumeUncalled }
   VirtualElement* source = reinterpret_cast<VirtualElement*>(sourceBitDomain.content);
   AssumeCondition(source)
   element = VirtualElement::Methods::applyAssign(element, bitOperation, *source,
         *sharedEvaluationEnvironment);
   bitDomain->content = element.extractElement();
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return bitDomain->content != nullptr && !env->emptyResult;
}

DLL_API DomainBitElement
domain_bit_create_binary_apply(DomainBitElement bitDomain, DomainBitBinaryOperation operation,
      DomainBitElement sourceBitDomain, DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::Bit::Operation Operation;
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(bitDomain.content);
   VirtualElement* source = reinterpret_cast<VirtualElement*>(sourceBitDomain.content);
   AssumeCondition(element && source)
   Operation bitOperation;
   bitOperation.setArgumentsNumber(1);
   if (operation >= DBBOPlus && operation <= DBBOMinus)
      bitOperation.setType((Operation::Type) ((operation-DBBOPlus) + Operation::TPlusAssign));
   else if (operation >= DBBOMin && operation <= DBBOExclusiveOr)
      bitOperation.setType((Operation::Type) ((operation-DBBOMin) + Operation::TMinAssign));
   else
      {  AssumeUncalled }
   bitOperation.setConstWithAssign();

   PPVirtualElement result = VirtualElement::Methods::apply(*element, bitOperation,
         *source, *sharedEvaluationEnvironment);
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainElement{result.extractElement()};
}

DLL_API ZeroResult
domain_bit_binary_compare(DomainBitElement bitDomain, DomainBitCompareOperation operation,
      DomainBitElement sourceBitDomain, DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::Bit::Operation Operation;
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(bitDomain.content);
   VirtualElement* source = reinterpret_cast<VirtualElement*>(sourceBitDomain.content);
   AssumeCondition(element && source)
   Operation bitOperation;
   bitOperation.setArgumentsNumber(1);
   if (operation >= DBCOCompareLess && operation <= DBCOCompareGreater)
      bitOperation.setType((Operation::Type) ((operation-DBCOCompareLess) + Operation::TCompareLess));
   else
      {  AssumeUncalled }
   bitOperation.setConst();
   PPVirtualElement resultDomain = VirtualElement::Methods::apply(*element, bitOperation,
         *source, *sharedEvaluationEnvironment);
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   auto result = resultDomain->queryZeroResult();
   return result.isValid() ? (result.isZero() ? ZRZero : ZRDifferentZero) : ZRUndefined;
}

DLL_API DomainBitElement
domain_bit_binary_compare_domain(DomainBitElement bitDomain, DomainBitCompareOperation operation,
      DomainBitElement sourceBitDomain, DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::Bit::Operation Operation;
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(bitDomain.content);
   VirtualElement* source = reinterpret_cast<VirtualElement*>(sourceBitDomain.content);
   AssumeCondition(element && source)
   Operation bitOperation;
   bitOperation.setArgumentsNumber(1);
   if (operation >= DBCOCompareLess && operation <= DBCOCompareGreater)
      bitOperation.setType((Operation::Type) ((operation-DBCOCompareLess) + Operation::TCompareLess));
   else
      {  AssumeUncalled }
   bitOperation.setConst();
   PPVirtualElement result = VirtualElement::Methods::apply(*element, bitOperation,
         *source, *sharedEvaluationEnvironment);
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainElement{result.extractElement()};
}

DLL_API DomainBitElement
domain_bit_guard_assign(DomainBitElement* conditionDomain, DomainBitElement* firstDomain,
      DomainBitElement* secondDomain, DomainEvaluationEnvironment* env) {
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement condition(reinterpret_cast<VirtualElement*>(conditionDomain->content), PNT::Pointer::Init());
   conditionDomain->content = nullptr;
   PPVirtualElement first(reinterpret_cast<VirtualElement*>(firstDomain->content), PNT::Pointer::Init());
   firstDomain->content = nullptr;
   PPVirtualElement second(reinterpret_cast<VirtualElement*>(secondDomain->content), PNT::Pointer::Init());
   secondDomain->content = nullptr;
   bool result = condition->guard(first, second, *sharedEvaluationEnvironment);
   AssumeCondition(result)
   if (sharedEvaluationEnvironment->hasResult())
      condition = sharedEvaluationEnvironment->presult();
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainElement{condition.extractElement()};
}

DLL_API ZeroResult
domain_bit_query_boolean(DomainBitElement bitDomain) {
   VirtualElement* element = reinterpret_cast<VirtualElement*>(bitDomain.content);
   AssumeCondition(element)
   auto result = element->queryZeroResult();
   return result.isValid() ? (result.isZero() ? ZRZero : ZRDifferentZero) : ZRUndefined;
}

DLL_API bool
domain_bit_cast_multibit_constraint(DomainMultiBitElement resultDomain,
      int sizeInBits, DomainBitElement* bitDomain, DomainEvaluationEnvironment* env) {
   if (!sharedConstraintEnvironment.get())
      sharedConstraintEnvironment.reset(ApproximateAccessEnvironment().createConstraintEnvironment(
            Scalar::ConstraintEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedConstraintEnvironment);
   sharedConstraintEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(bitDomain->content), PNT::Pointer::Init());
   bitDomain->content = nullptr;
   VirtualElement* result = reinterpret_cast<VirtualElement*>(resultDomain.content);
   AssumeCondition(result)
   // element = VirtualElement::Methods::constraint(element,
   //       Analyzer::Scalar::Bit::CastMultiBitOperation().setSize(sizeInBits), *result,
   //       *sharedConstraintEnvironment);
   bool booleanResult = element->constraint(
         Analyzer::Scalar::Bit::CastMultiBitOperation().setSize(sizeInBits), *result,
         *sharedConstraintEnvironment);
   AssumeCondition(booleanResult)
   if (sharedConstraintEnvironment->isEmpty())
      bitDomain->content = nullptr;
   else if (sharedConstraintEnvironment->hasResult())
      bitDomain->content = sharedConstraintEnvironment->presult().extractElement();
   else
      bitDomain->content = element.extractElement();
   env->warnings = sharedConstraintEnvironment->getErrorCode();
   env->emptyResult = sharedConstraintEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedConstraintEnvironment->getVerdict().verdict();
   sharedConstraintEnvironment->clear();
   return bitDomain->content != nullptr && !env->emptyResult;
}

DLL_API bool
domain_bit_unary_constraint(DomainBitElement resultDomain, DomainBitUnaryOperation operation,
      DomainBitElement* bitDomain, DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::Bit::Operation Operation;
   if (!sharedConstraintEnvironment.get())
      sharedConstraintEnvironment.reset(ApproximateAccessEnvironment().createConstraintEnvironment(
            Scalar::ConstraintEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedConstraintEnvironment);
   sharedConstraintEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(bitDomain->content), PNT::Pointer::Init());
   bitDomain->content = nullptr;
   VirtualElement* result = reinterpret_cast<VirtualElement*>(resultDomain.content);
   AssumeCondition(result)
   Operation bitOperation;
   bitOperation.setArgumentsNumber(0);
   if (operation >= DBUOPrev && operation <= DBUONext)
      bitOperation.setType((Operation::Type) ((operation-DBUOPrev) + Operation::TPrevAssign));
   else if (operation == DBUONegate)
      bitOperation.setType(Operation::TNegateAssign);
   else
      {  AssumeUncalled }
   bool booleanResult = element->constraint(bitOperation, *result, *sharedConstraintEnvironment);
   AssumeCondition(booleanResult)
   if (sharedConstraintEnvironment->isEmpty())
      bitDomain->content = nullptr;
   else if (sharedConstraintEnvironment->hasResult())
      bitDomain->content = sharedConstraintEnvironment->presult().extractElement();
   else
      bitDomain->content = element.extractElement();
   env->warnings = sharedConstraintEnvironment->getErrorCode();
   env->emptyResult = sharedConstraintEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedConstraintEnvironment->getVerdict().verdict();
   sharedConstraintEnvironment->clear();
   return bitDomain->content != nullptr && !env->emptyResult;
}

DLL_API bool domain_bit_binary_constraint(DomainBitElement resultDomain,
      DomainBitBinaryOperation operation, DomainBitElement* firstDomain,
      DomainBitElement* secondDomain, DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::Bit::Operation Operation;
   if (!sharedConstraintEnvironment.get())
      sharedConstraintEnvironment.reset(ApproximateAccessEnvironment().createConstraintEnvironment(
            Scalar::ConstraintEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedConstraintEnvironment);
   sharedConstraintEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement first(reinterpret_cast<VirtualElement*>(firstDomain->content), PNT::Pointer::Init());
   firstDomain->content = nullptr;
   PPVirtualElement second(reinterpret_cast<VirtualElement*>(secondDomain->content), PNT::Pointer::Init());
   secondDomain->content = nullptr;
   VirtualElement* result = reinterpret_cast<VirtualElement*>(resultDomain.content);
   AssumeCondition(result)
   Operation bitOperation;
   bitOperation.setArgumentsNumber(1);
   if (operation >= DBBOPlus && operation <= DBBOMinus)
      bitOperation.setType((Operation::Type) ((operation-DBBOPlus) + Operation::TPlusAssign));
   else if (operation >= DBBOMin && operation <= DBBOExclusiveOr)
      bitOperation.setType((Operation::Type) ((operation-DBBOMin) + Operation::TMinAssign));
   else
      {  AssumeUncalled }
   sharedConstraintEnvironment->absorbFirstArgument(second.extractElement());
   bool booleanResult = first->constraint(bitOperation, *result, *sharedConstraintEnvironment);
   AssumeCondition(booleanResult)
   secondDomain->content = (VirtualElement*) sharedConstraintEnvironment->extractFirstArgument();
   if (sharedConstraintEnvironment->isEmpty())
      firstDomain->content = nullptr;
   else if (sharedConstraintEnvironment->hasResult())
      firstDomain->content = sharedConstraintEnvironment->presult().extractElement();
   else
      firstDomain->content = first.extractElement();
   env->warnings = sharedConstraintEnvironment->getErrorCode();
   env->emptyResult = sharedConstraintEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedConstraintEnvironment->getVerdict().verdict();
   sharedConstraintEnvironment->clear();
   return firstDomain->content != nullptr && secondDomain->content != nullptr && !env->emptyResult;
}

DLL_API bool
domain_bit_compare_constraint(DomainBitElement resultDomain,
      DomainBitCompareOperation operation, DomainBitElement* firstDomain,
      DomainBitElement* secondDomain, DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::Bit::Operation Operation;
   if (!sharedConstraintEnvironment.get())
      sharedConstraintEnvironment.reset(ApproximateAccessEnvironment().createConstraintEnvironment(
            Scalar::ConstraintEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedConstraintEnvironment);
   sharedConstraintEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement first(reinterpret_cast<VirtualElement*>(firstDomain->content), PNT::Pointer::Init());
   firstDomain->content = nullptr;
   PPVirtualElement second(reinterpret_cast<VirtualElement*>(secondDomain->content), PNT::Pointer::Init());
   secondDomain->content = nullptr;
   VirtualElement* result = reinterpret_cast<VirtualElement*>(resultDomain.content);
   AssumeCondition(result)
   Operation bitOperation;
   bitOperation.setArgumentsNumber(1);
   if (operation >= DBCOCompareLess && operation <= DBCOCompareGreater)
      bitOperation.setType((Operation::Type) ((operation-DBCOCompareLess) + Operation::TCompareLess));
   else
      {  AssumeUncalled }
   bitOperation.setConst();
   sharedConstraintEnvironment->absorbFirstArgument(second.extractElement());
   bool booleanResult = first->constraint(bitOperation, *result, *sharedConstraintEnvironment);
   AssumeCondition(booleanResult)
   secondDomain->content = (VirtualElement*) sharedConstraintEnvironment->extractFirstArgument();
   if (sharedConstraintEnvironment->isEmpty())
      firstDomain->content = nullptr;
   else if (sharedConstraintEnvironment->hasResult())
      firstDomain->content = sharedConstraintEnvironment->presult().extractElement();
   else
      firstDomain->content = first.extractElement();
   env->warnings = sharedConstraintEnvironment->getErrorCode();
   env->emptyResult = sharedConstraintEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedConstraintEnvironment->getVerdict().verdict();
   sharedConstraintEnvironment->clear();
   return firstDomain->content != nullptr && secondDomain->content != nullptr && !env->emptyResult;
}

DLL_API bool
domain_bit_is_constant_value(DomainBitElement domain, bool* value) {
   const VirtualElement* element = reinterpret_cast<VirtualElement*>(domain.content);
   AssumeCondition(element)
   auto zeroResult = element->queryZeroResult();
   if (!zeroResult.isValid())
      return false;
   if (value)
      *value = zeroResult.isTrue();
   return true;
}

DLL_API DomainMultiBitElement
domain_multibit_create_constant(DomainIntegerConstant value) {
   return DomainElement{ApproximateAccessEnvironment().newConstant(
         reinterpret_cast<unsigned*>(&value.integerValue), value.sizeInBits, value.isSigned)
      .extractElement()};
}

DLL_API DomainMultiBitElement
domain_multibit_create_top(int sizeInBits, bool isSymbolic) {
   return DomainElement{ApproximateAccessEnvironment().newUndefined(sizeInBits, isSymbolic).extractElement()};
}

DLL_API DomainMultiBitElement
domain_multibit_create_interval_and_absorb(
      DomainMultiBitElement* domain_min, DomainMultiBitElement* domain_max,
      bool isSigned, bool isSymbolic) {
   PPVirtualElement min(reinterpret_cast<VirtualElement*>(domain_min->content), PNT::Pointer::Init());
   domain_min->content = nullptr;
   PPVirtualElement max(reinterpret_cast<VirtualElement*>(domain_max->content), PNT::Pointer::Init());
   domain_max->content = nullptr;

   const VirtualElement& source = *min;
   Analyzer::Scalar::Approximate::Details::VirtualIntegerInterval::Init init;
   init.setInterval(min, max);
   init.setUnsigned(!isSigned);
   PPVirtualElement result = Analyzer::Scalar::Approximate::Details::IntOperationElement::Methods
      ::newInterval(source, init);

   if (isSymbolic) {
      Analyzer::Scalar::MultiBit::Approximate::ExactToApproximateElement::Init init;
      init.setBitSize(result->getSizeInBits());
      auto* sharedResult = new Analyzer::Scalar::MultiBit::Approximate
            ::ExactToApproximateElement(init, result /*, nullptr */);
      PPVirtualElement newResult(sharedResult, PNT::Pointer::Init());
      result = newResult;
   }
   return DomainMultiBitElement{result.extractElement()};
}

DLL_API DomainBitElement
domain_multibit_create_cast_bit(DomainMultiBitElement multibitDomain,
      DomainEvaluationEnvironment* env) {
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(multibitDomain.content);
   PPVirtualElement result = VirtualElement::Methods::apply(*element,
      Analyzer::Scalar::MultiBit::CastBitOperation(), *sharedEvaluationEnvironment);
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainElement{result.extractElement()};
}

DLL_API DomainBitElement
domain_multibit_create_cast_shift_bit(DomainMultiBitElement multibitDomain,
      int shift, DomainEvaluationEnvironment* env) {
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(multibitDomain.content);
   PPVirtualElement result = VirtualElement::Methods::apply(*element,
      Analyzer::Scalar::MultiBit::CastShiftBitOperation(shift), *sharedEvaluationEnvironment);
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainElement{result.extractElement()};
}

DLL_API DomainMultiFloatElement
domain_multibit_create_cast_multifloat(DomainMultiBitElement multibitDomain, int sizeInBits,
      bool isSigned, DomainEvaluationEnvironment* env) {
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(multibitDomain.content);
   int sizeExponent, sizeMantissa;
   if (sizeInBits == 32) {
      sizeExponent = 8;
      sizeMantissa = 23;
   }
   else if (sizeInBits == 64) {
      sizeExponent = 11;
      sizeMantissa = 52;
   }
   else if (sizeInBits == 80) {
      sizeExponent = 15;
      sizeMantissa = 64;
   }
   else
      {  AssumeUncalled return DomainElement{}; }
   PPVirtualElement result = VirtualElement::Methods::apply(*element,
      Analyzer::Scalar::MultiBit::CastMultiFloatOperation().setSizeExponent(sizeExponent)
         .setSizeMantissa(sizeMantissa).setSigned(isSigned), *sharedEvaluationEnvironment);
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainElement{result.extractElement()};
}

DLL_API DomainMultiFloatElement
domain_multibit_create_cast_multifloat_ptr(DomainMultiBitElement multibitDomain, int sizeInBits,
      bool isSigned, DomainEvaluationEnvironment* env) {
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(multibitDomain.content);
   int sizeExponent, sizeMantissa;
   if (sizeInBits == 32) {
      sizeExponent = 8;
      sizeMantissa = 23;
   }
   else if (sizeInBits == 64) {
      sizeExponent = 11;
      sizeMantissa = 52;
   }
   else if (sizeInBits == 80) {
      sizeExponent = 15;
      sizeMantissa = 64;
   }
   else
      {  AssumeUncalled return DomainElement{}; }
   PPVirtualElement result = VirtualElement::Methods::apply(*element,
      Analyzer::Scalar::MultiBit::CastMultiFloatPointerOperation().setSizeExponent(sizeExponent)
         .setSizeMantissa(sizeMantissa).setSigned(isSigned), *sharedEvaluationEnvironment);
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainElement{result.extractElement()};
}

DLL_API bool
domain_multibit_unary_apply_assign(DomainMultiBitElement* multibitDomain,
      DomainMultiBitUnaryOperation operation, DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::MultiBit::Operation Operation;
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(multibitDomain->content), PNT::Pointer::Init());
   multibitDomain->content = nullptr;
   Operation multibitOperation;
   multibitOperation.setArgumentsNumber(0);
   if (operation >= DMBUOPrevSigned && operation <= DMBUONextUnsigned)
      multibitOperation.setType((Operation::Type) ((operation-DMBUOPrevSigned) + Operation::TPrevSignedAssign));
   else if (operation >= DMBUOOppositeSigned && operation <= DMBUOOppositeFloat)
      multibitOperation.setType((Operation::Type) ((operation-DMBUOOppositeSigned) + Operation::TOppositeSignedAssign));
   else if (operation == DMBUOBitNegate)
      multibitOperation.setType(Operation::TBitNegateAssign);
   else
      {  AssumeUncalled }
   element = VirtualElement::Methods::applyAssign(element, multibitOperation, *sharedEvaluationEnvironment);
   multibitDomain->content = element.extractElement();
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return multibitDomain->content != nullptr && !env->emptyResult;
}

DLL_API DomainMultiBitElement
domain_multibit_create_unary_apply(DomainMultiBitElement multibitDomain,
      DomainMultiBitUnaryOperation operation, DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::MultiBit::Operation Operation;
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(multibitDomain.content);
   AssumeCondition(element)
   Operation multibitOperation;
   multibitOperation.setArgumentsNumber(0);
   if (operation >= DMBUOPrevSigned && operation <= DMBUONextUnsigned)
      multibitOperation.setType((Operation::Type) ((operation-DMBUOPrevSigned) + Operation::TPrevSignedAssign));
   else if (operation >= DMBUOOppositeSigned && operation <= DMBUOOppositeFloat)
      multibitOperation.setType((Operation::Type) ((operation-DMBUOOppositeSigned) + Operation::TOppositeSignedAssign));
   else if (operation == DMBUOBitNegate)
      multibitOperation.setType(Operation::TBitNegateAssign);
   else
      {  AssumeUncalled }
   multibitOperation.setConstWithAssign();
   PPVirtualElement result = VirtualElement::Methods::apply(*element, multibitOperation,
         *sharedEvaluationEnvironment);
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainElement{result.extractElement()};
}

DLL_API bool domain_multibit_extend_apply_assign(DomainMultiBitElement* multibitDomain,
      DomainMultiBitExtendOperation operation, DomainEvaluationEnvironment* env) {
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(multibitDomain->content), PNT::Pointer::Init());
   multibitDomain->content = nullptr;
   typedef Analyzer::Scalar::MultiBit::ExtendOperation Operation;
   Operation multibitOperation;
   if (operation.type == DMBEOExtendWithZero)
      multibitOperation.setExtendWithZero(operation.new_size);
   else if (operation.type == DMBEOExtendWithSign)
      multibitOperation.setExtendWithSign(operation.new_size);
   else
      {  AssumeUncalled }
   element = VirtualElement::Methods::applyAssign(element, multibitOperation, *sharedEvaluationEnvironment);
   multibitDomain->content = element.extractElement();
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return multibitDomain->content != nullptr && !env->emptyResult;
}

DLL_API DomainMultiBitElement
domain_multibit_create_extend_apply(DomainMultiBitElement multibitDomain,
      DomainMultiBitExtendOperation operation, DomainEvaluationEnvironment* env) {
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(multibitDomain.content);
   AssumeCondition(element)
   typedef Analyzer::Scalar::MultiBit::ExtendOperation Operation;
   Operation multibitOperation;
   if (operation.type == DMBEOExtendWithZero)
      multibitOperation.setExtendWithZero(operation.new_size);
   else if (operation.type == DMBEOExtendWithSign)
      multibitOperation.setExtendWithSign(operation.new_size);
   else
      {  AssumeUncalled }
   multibitOperation.setConstWithAssign();
   PPVirtualElement result = VirtualElement::Methods::apply(*element, multibitOperation,
         *sharedEvaluationEnvironment);
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainElement{result.extractElement()};
}

DLL_API bool
domain_multibit_reduce_apply_assign(DomainMultiBitElement* multibitDomain,
      DomainMultiBitReduceOperation operation, DomainEvaluationEnvironment* env) {
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(multibitDomain->content), PNT::Pointer::Init());
   multibitDomain->content = nullptr;
   element = VirtualElement::Methods::applyAssign(element, Analyzer::Scalar::MultiBit
         ::ReduceOperation().setLowBit(operation.first_bit).setHighBit(operation.last_bit),
         *sharedEvaluationEnvironment);
   multibitDomain->content = element.extractElement();
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return multibitDomain->content != nullptr && !env->emptyResult;
}

DLL_API DomainMultiBitElement
domain_multibit_create_reduce_apply(DomainMultiBitElement multibitDomain,
      DomainMultiBitReduceOperation operation, DomainEvaluationEnvironment* env) {
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(multibitDomain.content);
   AssumeCondition(element)
   PPVirtualElement result = VirtualElement::Methods::apply(*element, Analyzer::Scalar::MultiBit
         ::ReduceOperation().setLowBit(operation.first_bit).setHighBit(operation.last_bit),
         *sharedEvaluationEnvironment);
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainElement{result.extractElement()};
}

DLL_API bool
domain_multibit_bitset_apply_assign(DomainMultiBitElement* multibitDomain,
      DomainMultiBitSetOperation operation, DomainMultiBitElement sourceMultibitDomain,
      DomainEvaluationEnvironment* env) {
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(multibitDomain->content), PNT::Pointer::Init());
   multibitDomain->content = nullptr;
   VirtualElement* source = reinterpret_cast<VirtualElement*>(sourceMultibitDomain.content);
   AssumeCondition(source)
   element = VirtualElement::Methods::applyAssign(element, Analyzer::Scalar::MultiBit
         ::BitSetOperation().setLowBit(operation.first_bit).setHighBit(operation.last_bit),
         *source, *sharedEvaluationEnvironment);
   multibitDomain->content = element.extractElement();
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return multibitDomain->content != nullptr && !env->emptyResult;
}

DLL_API DomainMultiBitElement
domain_multibit_create_bitset_apply(DomainMultiBitElement multibitDomain,
      DomainMultiBitSetOperation operation, DomainMultiBitElement sourceMultibitDomain,
      DomainEvaluationEnvironment* env) {
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(multibitDomain.content);
   AssumeCondition(element)
   VirtualElement* source = reinterpret_cast<VirtualElement*>(sourceMultibitDomain.content);
   AssumeCondition(source)
   PPVirtualElement result = VirtualElement::Methods::apply(*element, Analyzer::Scalar::MultiBit
         ::BitSetOperation().setLowBit(operation.first_bit).setHighBit(operation.last_bit),
         *source, *sharedEvaluationEnvironment);
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainElement{result.extractElement()};
}

DLL_API bool
domain_multibit_binary_apply_assign(DomainMultiBitElement* multibitDomain,
      DomainMultiBitBinaryOperation operation, DomainMultiBitElement sourceMultibitDomain,
      DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::MultiBit::Operation Operation;
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(multibitDomain->content), PNT::Pointer::Init());
   multibitDomain->content = nullptr;
   VirtualElement* source = reinterpret_cast<VirtualElement*>(sourceMultibitDomain.content);
   AssumeCondition(source)
   Operation multibitOperation;
   multibitOperation.setArgumentsNumber(1);
   if (operation == DMBBOConcat)
      multibitOperation.setType(Operation::TConcat);
   else if (operation >= DMBBOPlusSigned && operation <= DMBBOMinusFloat)
      multibitOperation.setType((Operation::Type) ((operation-DMBBOPlusSigned) + Operation::TPlusSignedAssign));
   else if (operation >= DMBBOMinSigned && operation <= DMBBODivideFloat)
      multibitOperation.setType((Operation::Type) ((operation-DMBBOMinSigned) + Operation::TMinSignedAssign));
   else if (operation >= DMBBOModuloSigned && operation <= DMBBOBitExclusiveOr)
      multibitOperation.setType((Operation::Type) ((operation-DMBBOModuloSigned) + Operation::TModuloSignedAssign));
   else if (operation >= DMBBOLeftShift && operation <= DMBBORightRotate)
      multibitOperation.setType((Operation::Type) ((operation-DMBBOLeftShift) + Operation::TLeftShiftAssign));
   else
      {  AssumeUncalled }
   element = VirtualElement::Methods::applyAssign(element, multibitOperation,
         *source, *sharedEvaluationEnvironment);
   multibitDomain->content = element.extractElement();
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return multibitDomain->content != nullptr && !env->emptyResult;
}

DLL_API DomainMultiBitElement
domain_multibit_create_binary_apply(DomainMultiBitElement multibitDomain,
      DomainMultiBitBinaryOperation operation, DomainMultiBitElement sourceMultibitDomain,
      DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::MultiBit::Operation Operation;
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(multibitDomain.content);
   VirtualElement* source = reinterpret_cast<VirtualElement*>(sourceMultibitDomain.content);
   AssumeCondition(element && source)
   Operation multibitOperation;
   multibitOperation.setArgumentsNumber(1);
   if (operation == DMBBOConcat)
      multibitOperation.setType(Operation::TConcat);
   else if (operation >= DMBBOPlusSigned && operation <= DMBBOMinusFloat)
      multibitOperation.setType((Operation::Type) ((operation-DMBBOPlusSigned) + Operation::TPlusSignedAssign));
   else if (operation >= DMBBOMinSigned && operation <= DMBBODivideFloat)
      multibitOperation.setType((Operation::Type) ((operation-DMBBOMinSigned) + Operation::TMinSignedAssign));
   else if (operation >= DMBBOModuloSigned && operation <= DMBBOBitExclusiveOr)
      multibitOperation.setType((Operation::Type) ((operation-DMBBOModuloSigned) + Operation::TModuloSignedAssign));
   else if (operation >= DMBBOLeftShift && operation <= DMBBORightRotate)
      multibitOperation.setType((Operation::Type) ((operation-DMBBOLeftShift) + Operation::TLeftShiftAssign));
   else
      {  AssumeUncalled }
   multibitOperation.setConstWithAssign();
   PPVirtualElement result = VirtualElement::Methods::apply(*element, multibitOperation,
         *source, *sharedEvaluationEnvironment);
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainElement{result.extractElement()};
}

DLL_API ZeroResult
domain_multibit_binary_compare(DomainMultiBitElement multibitDomain,
      DomainMultiBitCompareOperation operation, DomainMultiBitElement sourceMultibitDomain,
      DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::MultiBit::Operation Operation;
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(multibitDomain.content);
   VirtualElement* source = reinterpret_cast<VirtualElement*>(sourceMultibitDomain.content);
   AssumeCondition(element && source)
   Operation multibitOperation;
   multibitOperation.setArgumentsNumber(1);
   if (operation >= DMBCOCompareLessSigned && operation <= DMBCOCompareGreaterSigned)
      multibitOperation.setType((Operation::Type) ((operation-DMBCOCompareLessSigned) + Operation::TCompareLessSigned));
   else
      {  AssumeUncalled }
   multibitOperation.setConst();
   PPVirtualElement resultDomain = VirtualElement::Methods::apply(*element, multibitOperation,
         *source, *sharedEvaluationEnvironment);
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   auto result = resultDomain->queryZeroResult();
   return result.isValid() ? (result.isZero() ? ZRZero : ZRDifferentZero) : ZRUndefined;
}

DLL_API DomainBitElement
domain_multibit_binary_compare_domain(DomainMultiBitElement multibitDomain,
      DomainMultiBitCompareOperation operation, DomainMultiBitElement sourceMultibitDomain,
      DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::MultiBit::Operation Operation;
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(multibitDomain.content);
   VirtualElement* source = reinterpret_cast<VirtualElement*>(sourceMultibitDomain.content);
   AssumeCondition(element && source)
   Operation multibitOperation;
   multibitOperation.setArgumentsNumber(1);
   if (operation >= DMBCOCompareLessSigned && operation <= DMBCOCompareGreaterSigned)
      multibitOperation.setType((Operation::Type) ((operation-DMBCOCompareLessSigned) + Operation::TCompareLessSigned));
   else
      {  AssumeUncalled }
   multibitOperation.setConst();
   PPVirtualElement result = VirtualElement::Methods::apply(*element, multibitOperation,
         *source, *sharedEvaluationEnvironment);
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainElement{result.extractElement()};
}

DLL_API DomainMultiBitElement
domain_multibit_guard_assign(DomainBitElement* conditionDomain, DomainMultiBitElement* firstDomain,
      DomainMultiBitElement* secondDomain, DomainEvaluationEnvironment* env) {
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement condition(reinterpret_cast<VirtualElement*>(conditionDomain->content), PNT::Pointer::Init());
   conditionDomain->content = nullptr;
   PPVirtualElement first(reinterpret_cast<VirtualElement*>(firstDomain->content), PNT::Pointer::Init());
   firstDomain->content = nullptr;
   PPVirtualElement second(reinterpret_cast<VirtualElement*>(secondDomain->content), PNT::Pointer::Init());
   secondDomain->content = nullptr;
   bool result = condition->guard(first, second, *sharedEvaluationEnvironment);
   AssumeCondition(result)
   if (sharedEvaluationEnvironment->hasResult())
      condition = sharedEvaluationEnvironment->presult();
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainMultiBitElement{condition.extractElement()};
}

DLL_API ZeroResult
domain_multibit_query_boolean(DomainMultiBitElement multibitDomain) {
   VirtualElement* element = reinterpret_cast<VirtualElement*>(multibitDomain.content);
   AssumeCondition(element)
   auto result = element->queryZeroResult();
   return result.isValid() ? (result.isZero() ? ZRZero : ZRDifferentZero) : ZRUndefined;
}

DLL_API bool
domain_multibit_cast_bit_constraint(DomainBitElement resultDomain,
      DomainMultiBitElement* multibitDomain, DomainEvaluationEnvironment* env) {
   if (!sharedConstraintEnvironment.get())
      sharedConstraintEnvironment.reset(ApproximateAccessEnvironment().createConstraintEnvironment(
            Scalar::ConstraintEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedConstraintEnvironment);
   sharedConstraintEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(multibitDomain->content), PNT::Pointer::Init());
   multibitDomain->content = nullptr;
   VirtualElement* result = reinterpret_cast<VirtualElement*>(resultDomain.content);
   AssumeCondition(result)
   bool booleanResult = element->constraint(Analyzer::Scalar::MultiBit::CastBitOperation(),
         *result, *sharedConstraintEnvironment);
   AssumeCondition(booleanResult)
   if (sharedConstraintEnvironment->isEmpty())
      multibitDomain->content = nullptr;
   else if (sharedConstraintEnvironment->hasResult())
      multibitDomain->content = sharedConstraintEnvironment->presult().extractElement();
   else
      multibitDomain->content = element.extractElement();
   env->warnings = sharedConstraintEnvironment->getErrorCode();
   env->emptyResult = sharedConstraintEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedConstraintEnvironment->getVerdict().verdict();
   sharedConstraintEnvironment->clear();
   return multibitDomain->content != nullptr && !env->emptyResult;
}

DLL_API bool
domain_multibit_cast_shift_bit_constraint(DomainBitElement resultDomain,
      int shift, DomainMultiBitElement* multibitDomain, DomainEvaluationEnvironment* env) {
   if (!sharedConstraintEnvironment.get())
      sharedConstraintEnvironment.reset(ApproximateAccessEnvironment().createConstraintEnvironment(
            Scalar::ConstraintEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedConstraintEnvironment);
   sharedConstraintEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(multibitDomain->content), PNT::Pointer::Init());
   multibitDomain->content = nullptr;
   VirtualElement* result = reinterpret_cast<VirtualElement*>(resultDomain.content);
   AssumeCondition(result)
   bool booleanResult = element->constraint(Analyzer::Scalar::MultiBit
         ::CastShiftBitOperation(shift), *result, *sharedConstraintEnvironment);
   AssumeCondition(booleanResult)
   if (sharedConstraintEnvironment->isEmpty())
      multibitDomain->content = nullptr;
   else if (sharedConstraintEnvironment->hasResult())
      multibitDomain->content = sharedConstraintEnvironment->presult().extractElement();
   else
      multibitDomain->content = element.extractElement();
   env->warnings = sharedConstraintEnvironment->getErrorCode();
   env->emptyResult = sharedConstraintEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedConstraintEnvironment->getVerdict().verdict();
   sharedConstraintEnvironment->clear();
   return multibitDomain->content != nullptr && !env->emptyResult;
}

DLL_API bool
domain_multibit_cast_multifloat_constraint(DomainMultiFloatElement resultDomain,
      int sizeInBits, bool isSigned, DomainMultiBitElement* multibitDomain, DomainEvaluationEnvironment* env) {
   if (!sharedConstraintEnvironment.get())
      sharedConstraintEnvironment.reset(ApproximateAccessEnvironment().createConstraintEnvironment(
            Scalar::ConstraintEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedConstraintEnvironment);
   sharedConstraintEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(multibitDomain->content), PNT::Pointer::Init());
   multibitDomain->content = nullptr;
   VirtualElement* result = reinterpret_cast<VirtualElement*>(resultDomain.content);
   AssumeCondition(result)
   int sizeExponent, sizeMantissa;
   if (sizeInBits == 32) {
      sizeExponent = 8;
      sizeMantissa = 23;
   }
   else if (sizeInBits == 64) {
      sizeExponent = 11;
      sizeMantissa = 52;
   }
   else if (sizeInBits == 80) {
      sizeExponent = 15;
      sizeMantissa = 64;
   }
   else
      {  AssumeUncalled return false; }
   bool booleanResult = element->constraint(Analyzer::Scalar::MultiBit
         ::CastMultiFloatOperation().setSizeExponent(sizeExponent).setSizeMantissa(sizeMantissa)
            .setSigned(isSigned), *result, *sharedConstraintEnvironment);
   AssumeCondition(booleanResult)
   if (sharedConstraintEnvironment->isEmpty())
      multibitDomain->content = nullptr;
   else if (sharedConstraintEnvironment->hasResult())
      multibitDomain->content = sharedConstraintEnvironment->presult().extractElement();
   else
      multibitDomain->content = element.extractElement();
   env->warnings = sharedConstraintEnvironment->getErrorCode();
   env->emptyResult = sharedConstraintEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedConstraintEnvironment->getVerdict().verdict();
   sharedConstraintEnvironment->clear();
   return multibitDomain->content != nullptr && !env->emptyResult;
}

DLL_API bool
domain_multibit_cast_multifloat_ptr_constraint(DomainMultiFloatElement resultDomain,
      int sizeInBits, bool isSigned, DomainMultiBitElement* multibitDomain, DomainEvaluationEnvironment* env) {
   if (!sharedConstraintEnvironment.get())
      sharedConstraintEnvironment.reset(ApproximateAccessEnvironment().createConstraintEnvironment(
            Scalar::ConstraintEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedConstraintEnvironment);
   sharedConstraintEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(multibitDomain->content), PNT::Pointer::Init());
   multibitDomain->content = nullptr;
   VirtualElement* result = reinterpret_cast<VirtualElement*>(resultDomain.content);
   AssumeCondition(result)
   int sizeExponent, sizeMantissa;
   if (sizeInBits == 32) {
      sizeExponent = 8;
      sizeMantissa = 23;
   }
   else if (sizeInBits == 64) {
      sizeExponent = 11;
      sizeMantissa = 52;
   }
   else if (sizeInBits == 80) {
      sizeExponent = 15;
      sizeMantissa = 64;
   }
   else
      {  AssumeUncalled return false; }
   bool booleanResult = element->constraint(Analyzer::Scalar::MultiBit
         ::CastMultiFloatPointerOperation().setSizeExponent(sizeExponent)
            .setSizeMantissa(sizeMantissa).setSigned(isSigned),
         *result, *sharedConstraintEnvironment);
   AssumeCondition(booleanResult)
   if (sharedConstraintEnvironment->isEmpty())
      multibitDomain->content = nullptr;
   else if (sharedConstraintEnvironment->hasResult())
      multibitDomain->content = sharedConstraintEnvironment->presult().extractElement();
   else
      multibitDomain->content = element.extractElement();
   env->warnings = sharedConstraintEnvironment->getErrorCode();
   env->emptyResult = sharedConstraintEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedConstraintEnvironment->getVerdict().verdict();
   sharedConstraintEnvironment->clear();
   return multibitDomain->content != nullptr && !env->emptyResult;
}

DLL_API bool
domain_multibit_unary_constraint(DomainMultiBitElement resultDomain,
      DomainMultiBitUnaryOperation operation, DomainMultiBitElement* multibitDomain,
      DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::MultiBit::Operation Operation;
   if (!sharedConstraintEnvironment.get())
      sharedConstraintEnvironment.reset(ApproximateAccessEnvironment().createConstraintEnvironment(
            Scalar::ConstraintEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedConstraintEnvironment);
   sharedConstraintEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(multibitDomain->content), PNT::Pointer::Init());
   multibitDomain->content = nullptr;
   VirtualElement* result = reinterpret_cast<VirtualElement*>(resultDomain.content);
   AssumeCondition(result)
   Operation multibitOperation;
   multibitOperation.setArgumentsNumber(0);
   if (operation >= DMBUOPrevSigned && operation <= DMBUONextUnsigned)
      multibitOperation.setType((Operation::Type) ((operation-DMBUOPrevSigned) + Operation::TPrevSignedAssign));
   else if (operation >= DMBUOOppositeSigned && operation <= DMBUOOppositeFloat)
      multibitOperation.setType((Operation::Type) ((operation-DMBUOOppositeSigned) + Operation::TOppositeSignedAssign));
   else if (operation == DMBUOBitNegate)
      multibitOperation.setType(Operation::TBitNegateAssign);
   else
      {  AssumeUncalled }
   bool booleanResult = element->constraint(multibitOperation, *result, *sharedConstraintEnvironment);
   AssumeCondition(booleanResult)
   if (sharedConstraintEnvironment->isEmpty())
      multibitDomain->content = nullptr;
   else if (sharedConstraintEnvironment->hasResult())
      multibitDomain->content = sharedConstraintEnvironment->presult().extractElement();
   else
      multibitDomain->content = element.extractElement();
   env->warnings = sharedConstraintEnvironment->getErrorCode();
   env->emptyResult = sharedConstraintEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedConstraintEnvironment->getVerdict().verdict();
   sharedConstraintEnvironment->clear();
   return multibitDomain->content != nullptr && !env->emptyResult;
}

DLL_API bool
domain_multibit_extend_constraint(DomainMultiBitElement resultDomain,
      DomainMultiBitExtendOperation operation, DomainMultiBitElement* multibitDomain,
      DomainEvaluationEnvironment* env) {
   if (!sharedConstraintEnvironment.get())
      sharedConstraintEnvironment.reset(ApproximateAccessEnvironment().createConstraintEnvironment(
            Scalar::ConstraintEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedConstraintEnvironment);
   sharedConstraintEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(multibitDomain->content), PNT::Pointer::Init());
   multibitDomain->content = nullptr;
   VirtualElement* result = reinterpret_cast<VirtualElement*>(resultDomain.content);
   AssumeCondition(result)
   typedef Analyzer::Scalar::MultiBit::ExtendOperation Operation;
   Operation multibitOperation;
   if (operation.type == DMBEOExtendWithZero)
      multibitOperation.setExtendWithZero(operation.new_size);
   else if (operation.type == DMBEOExtendWithSign)
      multibitOperation.setExtendWithSign(operation.new_size);
   else
      {  AssumeUncalled }
   bool booleanResult = element->constraint(multibitOperation, *result, *sharedConstraintEnvironment);
   AssumeCondition(booleanResult)
   if (sharedConstraintEnvironment->isEmpty())
      multibitDomain->content = nullptr;
   else if (sharedConstraintEnvironment->hasResult())
      multibitDomain->content = sharedConstraintEnvironment->presult().extractElement();
   else
      multibitDomain->content = element.extractElement();
   env->warnings = sharedConstraintEnvironment->getErrorCode();
   env->emptyResult = sharedConstraintEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedConstraintEnvironment->getVerdict().verdict();
   sharedConstraintEnvironment->clear();
   return multibitDomain->content != nullptr && !env->emptyResult;
}

DLL_API
bool domain_multibit_reduce_constraint(DomainMultiBitElement resultDomain,
      DomainMultiBitReduceOperation operation, DomainMultiBitElement* multibitDomain,
      DomainEvaluationEnvironment* env) {
   if (!sharedConstraintEnvironment.get())
      sharedConstraintEnvironment.reset(ApproximateAccessEnvironment().createConstraintEnvironment(
            Scalar::ConstraintEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedConstraintEnvironment);
   sharedConstraintEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(multibitDomain->content), PNT::Pointer::Init());
   multibitDomain->content = nullptr;
   VirtualElement* result = reinterpret_cast<VirtualElement*>(resultDomain.content);
   AssumeCondition(result)
   bool booleanResult = element->constraint(Analyzer::Scalar::MultiBit::ReduceOperation()
         .setLowBit(operation.first_bit).setHighBit(operation.last_bit), *result, *sharedConstraintEnvironment);
   AssumeCondition(booleanResult)
   if (sharedConstraintEnvironment->isEmpty())
      multibitDomain->content = nullptr;
   else if (sharedConstraintEnvironment->hasResult())
      multibitDomain->content = sharedConstraintEnvironment->presult().extractElement();
   else
      multibitDomain->content = element.extractElement();
   env->warnings = sharedConstraintEnvironment->getErrorCode();
   env->emptyResult = sharedConstraintEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedConstraintEnvironment->getVerdict().verdict();
   sharedConstraintEnvironment->clear();
   return multibitDomain->content != nullptr && !env->emptyResult;
}

DLL_API bool
domain_multibit_bitset_constraint(DomainMultiBitElement resultDomain,
      DomainMultiBitSetOperation operation, DomainMultiBitElement* firstDomain,
      DomainMultiBitElement* secondDomain, DomainEvaluationEnvironment* env) {
   if (!sharedConstraintEnvironment.get())
      sharedConstraintEnvironment.reset(ApproximateAccessEnvironment().createConstraintEnvironment(
            Scalar::ConstraintEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedConstraintEnvironment);
   sharedConstraintEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement first(reinterpret_cast<VirtualElement*>(firstDomain->content), PNT::Pointer::Init());
   firstDomain->content = nullptr;
   PPVirtualElement second(reinterpret_cast<VirtualElement*>(secondDomain->content), PNT::Pointer::Init());
   secondDomain->content = nullptr;
   VirtualElement* result = reinterpret_cast<VirtualElement*>(resultDomain.content);
   AssumeCondition(result)
   sharedConstraintEnvironment->absorbFirstArgument(second.extractElement());
   bool booleanResult = first->constraint(Analyzer::Scalar::MultiBit::BitSetOperation()
         .setLowBit(operation.first_bit).setHighBit(operation.last_bit), *result, *sharedConstraintEnvironment);
   AssumeCondition(booleanResult)
   secondDomain->content = (VirtualElement*) sharedConstraintEnvironment->extractFirstArgument();
   if (sharedConstraintEnvironment->isEmpty())
      firstDomain->content = nullptr;
   else if (sharedConstraintEnvironment->hasResult())
      firstDomain->content = sharedConstraintEnvironment->presult().extractElement();
   else
      firstDomain->content = first.extractElement();
   env->warnings = sharedConstraintEnvironment->getErrorCode();
   env->emptyResult = sharedConstraintEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedConstraintEnvironment->getVerdict().verdict();
   sharedConstraintEnvironment->clear();
   return firstDomain->content != nullptr && secondDomain->content != nullptr && !env->emptyResult;
}

DLL_API bool
domain_multibit_binary_constraint(DomainMultiBitElement resultDomain,
      DomainMultiBitBinaryOperation operation, DomainMultiBitElement* firstDomain,
      DomainMultiBitElement* secondDomain, DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::MultiBit::Operation Operation;
   if (!sharedConstraintEnvironment.get())
      sharedConstraintEnvironment.reset(ApproximateAccessEnvironment().createConstraintEnvironment(
            Scalar::ConstraintEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedConstraintEnvironment);
   sharedConstraintEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement first(reinterpret_cast<VirtualElement*>(firstDomain->content), PNT::Pointer::Init());
   firstDomain->content = nullptr;
   PPVirtualElement second(reinterpret_cast<VirtualElement*>(secondDomain->content), PNT::Pointer::Init());
   secondDomain->content = nullptr;
   VirtualElement* result = reinterpret_cast<VirtualElement*>(resultDomain.content);
   AssumeCondition(result)
   Operation multibitOperation;
   multibitOperation.setArgumentsNumber(1);
   if (operation == DMBBOConcat)
      multibitOperation.setType(Operation::TConcat);
   else if (operation >= DMBBOPlusSigned && operation <= DMBBOMinusFloat)
      multibitOperation.setType((Operation::Type) ((operation-DMBBOPlusSigned) + Operation::TPlusSignedAssign));
   else if (operation >= DMBBOMinSigned && operation <= DMBBODivideFloat)
      multibitOperation.setType((Operation::Type) ((operation-DMBBOMinSigned) + Operation::TMinSignedAssign));
   else if (operation >= DMBBOModuloSigned && operation <= DMBBOBitExclusiveOr)
      multibitOperation.setType((Operation::Type) ((operation-DMBBOModuloSigned) + Operation::TModuloSignedAssign));
   else if (operation >= DMBBOLeftShift && operation <= DMBBORightRotate)
      multibitOperation.setType((Operation::Type) ((operation-DMBBOLeftShift) + Operation::TLeftShiftAssign));
   else
      {  AssumeUncalled }
   sharedConstraintEnvironment->absorbFirstArgument(second.extractElement());
   bool booleanResult = first->constraint(multibitOperation, *result, *sharedConstraintEnvironment);
   AssumeCondition(booleanResult)
   secondDomain->content = (VirtualElement*) sharedConstraintEnvironment->extractFirstArgument();
   if (sharedConstraintEnvironment->isEmpty())
      firstDomain->content = nullptr;
   else if (sharedConstraintEnvironment->hasResult())
      firstDomain->content = sharedConstraintEnvironment->presult().extractElement();
   else
      firstDomain->content = first.extractElement();
   env->warnings = sharedConstraintEnvironment->getErrorCode();
   env->emptyResult = sharedConstraintEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedConstraintEnvironment->getVerdict().verdict();
   sharedConstraintEnvironment->clear();
   return firstDomain->content != nullptr && secondDomain->content != nullptr && !env->emptyResult;
}

DLL_API bool
domain_multibit_compare_constraint(DomainBitElement resultDomain,
      DomainMultiBitCompareOperation operation, DomainMultiBitElement* firstDomain,
      DomainMultiBitElement* secondDomain, DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::MultiBit::Operation Operation;
   if (!sharedConstraintEnvironment.get())
      sharedConstraintEnvironment.reset(ApproximateAccessEnvironment().createConstraintEnvironment(
            Scalar::ConstraintEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedConstraintEnvironment);
   sharedConstraintEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement first(reinterpret_cast<VirtualElement*>(firstDomain->content), PNT::Pointer::Init());
   firstDomain->content = nullptr;
   PPVirtualElement second(reinterpret_cast<VirtualElement*>(secondDomain->content), PNT::Pointer::Init());
   secondDomain->content = nullptr;
   VirtualElement* result = reinterpret_cast<VirtualElement*>(resultDomain.content);
   AssumeCondition(result)
   Operation multibitOperation;
   multibitOperation.setArgumentsNumber(1);
   if (operation >= DMBCOCompareLessSigned && operation <= DMBCOCompareGreaterSigned)
      multibitOperation.setType((Operation::Type) ((operation-DMBCOCompareLessSigned) + Operation::TCompareLessSigned));
   else
      {  AssumeUncalled }
   multibitOperation.setConst();
   sharedConstraintEnvironment->absorbFirstArgument(second.extractElement());
   bool booleanResult = first->constraint(multibitOperation, *result, *sharedConstraintEnvironment);
   AssumeCondition(booleanResult)
   secondDomain->content = (VirtualElement*) sharedConstraintEnvironment->extractFirstArgument();
   if (sharedConstraintEnvironment->isEmpty())
      firstDomain->content = nullptr;
   else if (sharedConstraintEnvironment->hasResult())
      firstDomain->content = sharedConstraintEnvironment->presult().extractElement();
   else
      firstDomain->content = first.extractElement();
   env->warnings = sharedConstraintEnvironment->getErrorCode();
   env->emptyResult = sharedConstraintEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedConstraintEnvironment->getVerdict().verdict();
   sharedConstraintEnvironment->clear();
   return firstDomain->content != nullptr && secondDomain->content != nullptr && !env->emptyResult;
}

DLL_API bool
domain_multibit_is_constant_value(DomainMultiBitElement multibitDomain,
      DomainIntegerConstant* value) {
   const VirtualElement* element = reinterpret_cast<VirtualElement*>(multibitDomain.content);
   AssumeCondition(element)
   ApproximateAccessEnvironment access;
   if (!access.isConstantValue(*element))
      return false;
   if (value) {
      AssumeCondition(value->sizeInBits < 64)
      access.retrieveValue(*element, reinterpret_cast<unsigned*>(&value->integerValue),
            (value->sizeInBits+8*sizeof(unsigned)-1)/(8*sizeof(unsigned)));
   }
   return true;
}

DLL_API
DomainMultiFloatElement
domain_multifloat_create_constant(DomainFloatingPointConstant value) {
   int sizeMantissa, sizeExponent;
   if (value.sizeInBits == 32) {
      sizeExponent = 8;
      sizeMantissa = 23;
   }
   else if (value.sizeInBits == 64) {
      sizeExponent = 11;
      sizeMantissa = 52;
   }
   else if (value.sizeInBits == 80) {
      sizeExponent = 15;
      sizeMantissa = 64;
   }
   else
      {  AssumeUncalled return DomainElement{}; }
   return DomainElement{ApproximateAccessEnvironment().newFloatConstant(
         value.floatValue, sizeExponent, sizeMantissa)
      .extractElement()};
}

DLL_API DomainMultiFloatElement
domain_multifloat_create_top(int sizeInBits, bool isSymbolic) {
   int sizeMantissa, sizeExponent;
   if (sizeInBits == 32) {
      sizeExponent = 8;
      sizeMantissa = 23;
   }
   else if (sizeInBits == 64) {
      sizeExponent = 11;
      sizeMantissa = 52;
   }
   else if (sizeInBits == 80) {
      sizeExponent = 15;
      sizeMantissa = 64;
   }
   else
      {  AssumeUncalled return DomainElement{}; }
   return DomainElement{ApproximateAccessEnvironment()
      .newFloatUndefined(sizeExponent, sizeMantissa, isSymbolic).extractElement()};
}

DLL_API DomainMultiFloatElement
domain_multifloat_create_interval_and_absorb(DomainMultiFloatElement* domain_min,
      DomainMultiFloatElement* domain_max, bool isSymbolic) {
   PPVirtualElement min(reinterpret_cast<VirtualElement*>(domain_min->content), PNT::Pointer::Init());
   domain_min->content = nullptr;
   PPVirtualElement max(reinterpret_cast<VirtualElement*>(domain_max->content), PNT::Pointer::Init());
   domain_max->content = nullptr;

   const VirtualElement& source = *min;
   Analyzer::Scalar::Approximate::Details::VirtualRealInterval::Init init;
   init.setInterval(min, max);
   PPVirtualElement result = Analyzer::Scalar::Approximate::Details::RealOperationElement::Methods
      ::newInterval(source, init);
   if (isSymbolic) {
      Analyzer::Scalar::MultiBit::Approximate::ExactToApproximateElement::Init init;
      init.setBitSize(result->getSizeInBits());
      auto* sharedResult = new Analyzer::Scalar::MultiBit::Approximate
            ::ExactToApproximateElement(init, result /*, nullptr */);
      PPVirtualElement newResult(sharedResult, PNT::Pointer::Init());
      result = newResult;
   };
   return DomainMultiBitElement{result.extractElement()};
}

DLL_API DomainMultiBitElement
domain_multifloat_create_cast_multibit(DomainMultiFloatElement multifloatDomain,
      int sizeInBits, DomainEvaluationEnvironment* env) {
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(multifloatDomain.content);
   PPVirtualElement result = VirtualElement::Methods::apply(*element,
      Analyzer::Scalar::Floating::CastMultiBitOperation()
         .setSize(sizeInBits), *sharedEvaluationEnvironment);
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainElement{result.extractElement()};
}

DLL_API DomainMultiBitElement
domain_multifloat_query_to_multibit(DomainMultiFloatElement multifloatDomain,
      DomainMultiFloatToIntOperation operation, DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::Floating::Operation Operation;
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(multifloatDomain.content);
   Operation floatOperation;
   floatOperation.setArgumentsNumber(0);
   if (operation >= DMFIOIsInftyExponent && operation <= DMFIOCastMantissa)
      floatOperation.setType((Operation::Type) ((operation-DMFIOIsInftyExponent) + Operation::TIsInftyExponent));
   else
      {  AssumeUncalled }
   PPVirtualElement result = VirtualElement::Methods::apply(*element,
         floatOperation, *sharedEvaluationEnvironment);
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainElement{result.extractElement()};
}

DLL_API bool
domain_multifloat_cast_multifloat_assign(DomainMultiFloatElement* multifloatDomain,
      int sizeInBits, DomainEvaluationEnvironment* env) {
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(multifloatDomain->content), PNT::Pointer::Init());
   multifloatDomain->content = nullptr;
   int sizeMantissa, sizeExponent;
   if (sizeInBits == 32) {
      sizeExponent = 8;
      sizeMantissa = 23;
   }
   else if (sizeInBits == 64) {
      sizeExponent = 11;
      sizeMantissa = 52;
   }
   else if (sizeInBits == 80) {
      sizeExponent = 15;
      sizeMantissa = 64;
   }
   else
      {  AssumeUncalled return false; }
   element = VirtualElement::Methods::applyAssign(element, Analyzer::Scalar::Floating
         ::CastMultiFloatOperation().setSizeExponent(sizeExponent)
               .setSizeMantissa(sizeMantissa).setSigned(true), *sharedEvaluationEnvironment);
   multifloatDomain->content = element.extractElement();
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return multifloatDomain->content != nullptr && !env->emptyResult;
}

DLL_API DomainMultiFloatElement
domain_multifloat_cast_multifloat(
      DomainMultiFloatElement multifloatDomain, int sizeInBits,
      DomainEvaluationEnvironment* env) {
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(multifloatDomain.content);
   AssumeCondition(element)
   int sizeMantissa, sizeExponent;
   if (sizeInBits == 32) {
      sizeExponent = 8;
      sizeMantissa = 23;
   }
   else if (sizeInBits == 64) {
      sizeExponent = 11;
      sizeMantissa = 52;
   }
   else if (sizeInBits == 80) {
      sizeExponent = 15;
      sizeMantissa = 64;
   }
   else
      {  AssumeUncalled return DomainElement{}; }
   PPVirtualElement result = VirtualElement::Methods::apply(*element, Analyzer::Scalar::Floating
         ::CastMultiFloatOperation().setSizeExponent(sizeExponent)
               .setSizeMantissa(sizeMantissa).setSigned(true), *sharedEvaluationEnvironment);
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainElement{result.extractElement()};
}

DLL_API bool
domain_multifloat_unary_apply_assign(DomainMultiFloatElement* multifloatDomain,
      DomainMultiFloatUnaryOperation operation, DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::Floating::Operation Operation;
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(multifloatDomain->content), PNT::Pointer::Init());
   multifloatDomain->content = nullptr;
   Operation multifloatOperation;
   multifloatOperation.setArgumentsNumber(0);
   if (operation >= DMFUOCastFloat && operation <= DMFUOCastDouble)
      multifloatOperation.setType((Operation::Type) ((operation-DMFUOCastFloat) + Operation::TCastFloat));
   else if (operation >= DMFUOOpposite && operation <= DMFUOAbs)
      multifloatOperation.setType((Operation::Type) ((operation-DMFUOOpposite) + Operation::TOppositeAssign));
   else if (operation >= DMFUOAcos && operation <= DMFUOAtan)
      multifloatOperation.setType((Operation::Type) ((operation-DMFUOAcos) + Operation::TAcos));
   else if (operation >= DMFUOCeil && operation <= DMFUOFloor)
      multifloatOperation.setType((Operation::Type) ((operation-DMFUOCeil) + Operation::TFloor));
   else if (operation >= DMFUOLog && operation <= DMFUOLog10)
      multifloatOperation.setType((Operation::Type) ((operation-DMFUOLog) + Operation::TLog));
   else if (operation >= DMFUOPow && operation <= DMFUOTanh)
      multifloatOperation.setType((Operation::Type) ((operation-DMFUOPow) + Operation::TPow));
   else
      {  AssumeUncalled }
   element = VirtualElement::Methods::applyAssign(element, multifloatOperation, *sharedEvaluationEnvironment);
   multifloatDomain->content = element.extractElement();
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return multifloatDomain->content != nullptr && !env->emptyResult;
}

DLL_API DomainMultiFloatElement domain_multifloat_create_unary_apply(
      DomainMultiFloatElement multifloatDomain, DomainMultiFloatUnaryOperation operation,
      DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::Floating::Operation Operation;
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(multifloatDomain.content);
   AssumeCondition(element)
   Operation multifloatOperation;
   multifloatOperation.setArgumentsNumber(0);
   if (operation >= DMFUOCastFloat && operation <= DMFUOCastDouble)
      multifloatOperation.setType((Operation::Type) ((operation-DMFUOCastFloat) + Operation::TCastFloat));
   else if (operation >= DMFUOOpposite && operation <= DMFUOAbs)
      multifloatOperation.setType((Operation::Type) ((operation-DMFUOOpposite) + Operation::TOppositeAssign));
   else if (operation >= DMFUOAcos && operation <= DMFUOAtan)
      multifloatOperation.setType((Operation::Type) ((operation-DMFUOAcos) + Operation::TAcos));
   else if (operation >= DMFUOCeil && operation <= DMFUOFloor)
      multifloatOperation.setType((Operation::Type) ((operation-DMFUOCeil) + Operation::TFloor));
   else if (operation >= DMFUOLog && operation <= DMFUOLog10)
      multifloatOperation.setType((Operation::Type) ((operation-DMFUOLog) + Operation::TLog));
   else if (operation >= DMFUOPow && operation <= DMFUOTanh)
      multifloatOperation.setType((Operation::Type) ((operation-DMFUOPow) + Operation::TPow));
   else
      {  AssumeUncalled }
   multifloatOperation.setConstWithAssign();
   PPVirtualElement result = VirtualElement::Methods::apply(*element, multifloatOperation,
         *sharedEvaluationEnvironment);
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainElement{result.extractElement()};
}

DLL_API bool
domain_multifloat_binary_apply_assign(DomainMultiFloatElement* multifloatDomain,
      DomainMultiFloatBinaryOperation operation, DomainMultiFloatElement sourceMultifloatDomain,
      DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::Floating::Operation Operation;
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(multifloatDomain->content), PNT::Pointer::Init());
   multifloatDomain->content = nullptr;
   VirtualElement* source = reinterpret_cast<VirtualElement*>(sourceMultifloatDomain.content);
   AssumeCondition(source)
   Operation multifloatOperation;
   multifloatOperation.setArgumentsNumber(1);
   if (operation >= DMFBOPlus && operation <= DMFBOMinus)
      multifloatOperation.setType((Operation::Type) ((operation-DMFBOPlus) + Operation::TPlusAssign));
   else if (operation >= DMFBOMin && operation <= DMFBODivide)
      multifloatOperation.setType((Operation::Type) ((operation-DMFBOMin) + Operation::TMinAssign));
   else if (operation == DMFBOAtan2)
      multifloatOperation.setType(Operation::TAtan2);
   else if (operation >= DMFBOFmod && operation <= DMFBOLdexp)
      multifloatOperation.setType((Operation::Type) ((operation-DMFBOFmod) + Operation::TFmod));
   else if (operation == DMFBOModf)
      multifloatOperation.setType(Operation::TModf);
   else
      {  AssumeUncalled }
   element = VirtualElement::Methods::applyAssign(element, multifloatOperation,
         *source, *sharedEvaluationEnvironment);
   multifloatDomain->content = element.extractElement();
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return multifloatDomain->content != nullptr && !env->emptyResult;
}

DLL_API DomainMultiFloatElement
domain_multifloat_create_binary_apply(DomainMultiFloatElement multifloatDomain,
      DomainMultiFloatBinaryOperation operation, DomainMultiFloatElement sourceMultifloatDomain,
      DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::Floating::Operation Operation;
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(multifloatDomain.content);
   VirtualElement* source = reinterpret_cast<VirtualElement*>(sourceMultifloatDomain.content);
   AssumeCondition(element && source)
   Operation multifloatOperation;
   multifloatOperation.setArgumentsNumber(1);
   if (operation >= DMFBOPlus && operation <= DMFBOMinus)
      multifloatOperation.setType((Operation::Type) ((operation-DMFBOPlus) + Operation::TPlusAssign));
   else if (operation >= DMFBOMin && operation <= DMFBODivide)
      multifloatOperation.setType((Operation::Type) ((operation-DMFBOMin) + Operation::TMinAssign));
   else if (operation == DMFBOAtan2)
      multifloatOperation.setType(Operation::TAtan2);
   else if (operation >= DMFBOFmod && operation <= DMFBOLdexp)
      multifloatOperation.setType((Operation::Type) ((operation-DMFBOFmod) + Operation::TFmod));
   else if (operation == DMFBOModf)
      multifloatOperation.setType(Operation::TModf);
   else
      {  AssumeUncalled }
   multifloatOperation.setConstWithAssign();
   PPVirtualElement result = VirtualElement::Methods::apply(*element, multifloatOperation,
         *source, *sharedEvaluationEnvironment);
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainElement{result.extractElement()};
}

DLL_API ZeroResult
domain_multifloat_binary_compare(DomainMultiFloatElement multifloatDomain,
      DomainMultiFloatCompareOperation operation, DomainMultiFloatElement sourceMultifloatDomain,
      DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::Floating::Operation Operation;
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(multifloatDomain.content);
   VirtualElement* source = reinterpret_cast<VirtualElement*>(sourceMultifloatDomain.content);
   AssumeCondition(element && source)
   Operation multifloatOperation;
   multifloatOperation.setArgumentsNumber(1);
   if (operation >= DMFCOCompareLess && operation <= DMFCOCompareGreater)
      multifloatOperation.setType((Operation::Type) ((operation-DMFCOCompareLess) + Operation::TCompareLess));
   else
      {  AssumeUncalled }
   multifloatOperation.setConst();
   PPVirtualElement resultDomain = VirtualElement::Methods::apply(*element, multifloatOperation,
         *source, *sharedEvaluationEnvironment);
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   auto result = resultDomain->queryZeroResult();
   return result.isValid() ? (result.isZero() ? ZRZero : ZRDifferentZero) : ZRUndefined;
}

DLL_API DomainBitElement
domain_multifloat_binary_compare_domain(DomainMultiFloatElement multifloatDomain,
      DomainMultiFloatCompareOperation operation, DomainMultiFloatElement sourceMultifloatDomain,
      DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::Floating::Operation Operation;
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(multifloatDomain.content);
   VirtualElement* source = reinterpret_cast<VirtualElement*>(sourceMultifloatDomain.content);
   AssumeCondition(element && source)
   Operation multifloatOperation;
   multifloatOperation.setArgumentsNumber(1);
   if (operation >= DMFCOCompareLess && operation <= DMFCOCompareGreater)
      multifloatOperation.setType((Operation::Type) ((operation-DMFCOCompareLess) + Operation::TCompareLess));
   else
      {  AssumeUncalled }
   multifloatOperation.setConst();
   PPVirtualElement result = VirtualElement::Methods::apply(*element, multifloatOperation,
         *source, *sharedEvaluationEnvironment);
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainElement{result.extractElement()};
}

DLL_API DomainMultiFloatElement
domain_multifloat_guard_assign(DomainBitElement* conditionDomain, DomainMultiFloatElement* firstDomain,
      DomainMultiFloatElement* secondDomain, DomainEvaluationEnvironment* env) {
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement condition(reinterpret_cast<VirtualElement*>(conditionDomain->content), PNT::Pointer::Init());
   conditionDomain->content = nullptr;
   PPVirtualElement first(reinterpret_cast<VirtualElement*>(firstDomain->content), PNT::Pointer::Init());
   firstDomain->content = nullptr;
   PPVirtualElement second(reinterpret_cast<VirtualElement*>(secondDomain->content), PNT::Pointer::Init());
   secondDomain->content = nullptr;
   bool result = condition->guard(first, second, *sharedEvaluationEnvironment);
   AssumeCondition(result)
   if (sharedEvaluationEnvironment->hasResult())
      condition = sharedEvaluationEnvironment->presult();
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainMultiFloatElement{condition.extractElement()};
}

DLL_API bool
domain_multifloat_ternary_apply_assign(DomainMultiFloatElement* multifloatDomain,
      DomainMultiFloatTernaryOperation operation, DomainMultiFloatElement secondMultifloatDomain,
      DomainMultiFloatElement thirdMultifloatDomain, DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::Floating::Operation Operation;
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(multifloatDomain->content), PNT::Pointer::Init());
   multifloatDomain->content = nullptr;
   VirtualElement* second = reinterpret_cast<VirtualElement*>(secondMultifloatDomain.content);
   VirtualElement* third = reinterpret_cast<VirtualElement*>(thirdMultifloatDomain.content);
   AssumeCondition(second && third)
   Operation multifloatOperation;
   multifloatOperation.setArgumentsNumber(2);
   if (operation >= DMFTOMultAdd && operation <= DMFTONegMultSub)
      multifloatOperation.setType((Operation::Type) ((operation-DMFTOMultAdd) + Operation::TMultAddAssign));
   else
      {  AssumeUncalled }
   {  Analyzer::Scalar::Approximate::TransmitEnvironment copyEnv(*sharedEvaluationEnvironment);
      copyEnv.setFirstArgument(*second);
      copyEnv.setSecondArgument(*third);
      bool result = element->apply(multifloatOperation, copyEnv);
      AssumeCondition(result)
      if (!copyEnv.isEmpty()) {
         if (copyEnv.hasResult())
            element = copyEnv.presult();
      }
      else
         element.release();
   }
   multifloatDomain->content = element.extractElement();
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return multifloatDomain->content != nullptr && !env->emptyResult;
}

DLL_API DomainMultiFloatElement domain_multifloat_create_ternary_apply(
      DomainMultiFloatElement multifloatDomain, DomainMultiFloatTernaryOperation operation,
      DomainMultiFloatElement secondMultifloatDomain, DomainMultiFloatElement thirdMultifloatDomain,
      DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::Floating::Operation Operation;
   if (!sharedEvaluationEnvironment.get())
      sharedEvaluationEnvironment.reset(ApproximateAccessEnvironment().createEnvironment(
            Scalar::EvaluationEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedEvaluationEnvironment);
   sharedEvaluationEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   VirtualElement* element = reinterpret_cast<VirtualElement*>(multifloatDomain.content);
   VirtualElement* second = reinterpret_cast<VirtualElement*>(secondMultifloatDomain.content);
   VirtualElement* third = reinterpret_cast<VirtualElement*>(thirdMultifloatDomain.content);
   AssumeCondition(element && second && third)
   Operation multifloatOperation;
   multifloatOperation.setArgumentsNumber(2);
   if (operation >= DMFTOMultAdd && operation <= DMFTONegMultSub)
      multifloatOperation.setType((Operation::Type) ((operation-DMFTOMultAdd) + Operation::TMultAddAssign));
   else
      {  AssumeUncalled }
   multifloatOperation.setConstWithAssign();
   PPVirtualElement result;
   {  Analyzer::Scalar::Approximate::TransmitEnvironment copyEnv(*sharedEvaluationEnvironment);
      copyEnv.setFirstArgument(*second);
      copyEnv.setSecondArgument(*third);
      bool boolResult = element->apply(multifloatOperation, copyEnv);
      AssumeCondition(boolResult)
      if (!copyEnv.isEmpty())
         result = copyEnv.presult();
   }
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return DomainElement{result.extractElement()};
}

DLL_API bool domain_multifloat_cast_multibit_constraint(DomainMultiBitElement resultDomain,
      int sizeInBits, DomainMultiFloatElement* multifloatDomain, DomainEvaluationEnvironment* env) {
   if (!sharedConstraintEnvironment.get())
      sharedConstraintEnvironment.reset(ApproximateAccessEnvironment().createConstraintEnvironment(
            Scalar::ConstraintEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedConstraintEnvironment);
   sharedConstraintEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(multifloatDomain->content), PNT::Pointer::Init());
   multifloatDomain->content = nullptr;
   VirtualElement* result = reinterpret_cast<VirtualElement*>(resultDomain.content);
   AssumeCondition(result)
   bool booleanResult = element->constraint(Analyzer::Scalar::Floating
         ::CastMultiBitOperation().setSize(sizeInBits), *result, *sharedConstraintEnvironment);
   AssumeCondition(booleanResult)
   if (sharedConstraintEnvironment->isEmpty())
      multifloatDomain->content = nullptr;
   else if (sharedConstraintEnvironment->hasResult())
      multifloatDomain->content = sharedConstraintEnvironment->presult().extractElement();
   else
      multifloatDomain->content = element.extractElement();
   env->warnings = sharedConstraintEnvironment->getErrorCode();
   env->emptyResult = sharedConstraintEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedConstraintEnvironment->getVerdict().verdict();
   sharedConstraintEnvironment->clear();
   return multifloatDomain->content != nullptr && !env->emptyResult;
}

DLL_API bool domain_multifloat_query_to_multibit_constraint(DomainMultiBitElement resultDomain,
      DomainMultiFloatToIntOperation operation, DomainMultiFloatElement* multifloatDomain,
      DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::Floating::Operation Operation;
   if (!sharedConstraintEnvironment.get())
      sharedConstraintEnvironment.reset(ApproximateAccessEnvironment().createConstraintEnvironment(
            Scalar::ConstraintEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedConstraintEnvironment);
   sharedConstraintEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(multifloatDomain->content), PNT::Pointer::Init());
   multifloatDomain->content = nullptr;
   VirtualElement* result = reinterpret_cast<VirtualElement*>(resultDomain.content);
   AssumeCondition(result)
   Operation multifloatOperation;
   multifloatOperation.setArgumentsNumber(0);
   if (operation >= DMFIOIsInftyExponent && operation <= DMFIOCastMantissa)
      multifloatOperation.setType((Operation::Type) ((operation-DMFIOIsInftyExponent) + Operation::TIsInftyExponent));
   else
      {  AssumeUncalled }
   bool booleanResult = element->constraint(multifloatOperation, *result, *sharedConstraintEnvironment);
   AssumeCondition(booleanResult)
   if (sharedConstraintEnvironment->isEmpty())
      multifloatDomain->content = nullptr;
   else if (sharedConstraintEnvironment->hasResult())
      multifloatDomain->content = sharedConstraintEnvironment->presult().extractElement();
   else
      multifloatDomain->content = element.extractElement();
   env->warnings = sharedConstraintEnvironment->getErrorCode();
   env->emptyResult = sharedConstraintEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedConstraintEnvironment->getVerdict().verdict();
   sharedConstraintEnvironment->clear();
   return multifloatDomain->content != nullptr && !env->emptyResult;
}

DLL_API bool
domain_multifloat_cast_multifloat_constraint(DomainMultiFloatElement resultDomain,
      int sizeInBits, DomainMultiFloatElement* multifloatDomain,
      DomainEvaluationEnvironment* env) {
   if (!sharedConstraintEnvironment.get())
      sharedConstraintEnvironment.reset(ApproximateAccessEnvironment().createConstraintEnvironment(
            Scalar::ConstraintEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedConstraintEnvironment);
   sharedConstraintEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(multifloatDomain->content), PNT::Pointer::Init());
   multifloatDomain->content = nullptr;
   VirtualElement* result = reinterpret_cast<VirtualElement*>(resultDomain.content);
   AssumeCondition(result)
   int sizeMantissa, sizeExponent;
   if (sizeInBits == 32) {
      sizeExponent = 8;
      sizeMantissa = 23;
   }
   else if (sizeInBits == 64) {
      sizeExponent = 11;
      sizeMantissa = 52;
   }
   else if (sizeInBits == 80) {
      sizeExponent = 15;
      sizeMantissa = 64;
   }
   else
      {  AssumeUncalled return false; }
   bool booleanResult = element->constraint(Analyzer::Scalar::Floating::CastMultiFloatOperation()
         .setSizeExponent(sizeExponent).setSizeMantissa(sizeMantissa).setSigned(true),
         *result, *sharedConstraintEnvironment);
   AssumeCondition(booleanResult)
   if (sharedConstraintEnvironment->isEmpty())
      multifloatDomain->content = nullptr;
   else if (sharedConstraintEnvironment->hasResult())
      multifloatDomain->content = sharedConstraintEnvironment->presult().extractElement();
   else
      multifloatDomain->content = element.extractElement();
   env->warnings = sharedConstraintEnvironment->getErrorCode();
   env->emptyResult = sharedConstraintEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedConstraintEnvironment->getVerdict().verdict();
   sharedConstraintEnvironment->clear();
   return multifloatDomain->content != nullptr && !env->emptyResult;
}

DLL_API bool
domain_multifloat_unary_constraint(DomainMultiFloatElement resultDomain,
      DomainMultiFloatUnaryOperation operation, DomainMultiFloatElement* multifloatDomain,
      DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::Floating::Operation Operation;
   if (!sharedConstraintEnvironment.get())
      sharedConstraintEnvironment.reset(ApproximateAccessEnvironment().createConstraintEnvironment(
            Scalar::ConstraintEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedConstraintEnvironment);
   sharedConstraintEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement element(reinterpret_cast<VirtualElement*>(multifloatDomain->content), PNT::Pointer::Init());
   multifloatDomain->content = nullptr;
   VirtualElement* result = reinterpret_cast<VirtualElement*>(resultDomain.content);
   AssumeCondition(result)
   Operation multifloatOperation;
   multifloatOperation.setArgumentsNumber(0);
   if (operation >= DMFUOCastFloat && operation <= DMFUOCastDouble)
      multifloatOperation.setType((Operation::Type) ((operation-DMFUOCastFloat) + Operation::TCastFloat));
   else if (operation >= DMFUOOpposite && operation <= DMFUOAbs)
      multifloatOperation.setType((Operation::Type) ((operation-DMFUOOpposite) + Operation::TOppositeAssign));
   else if (operation >= DMFUOAcos && operation <= DMFUOAtan)
      multifloatOperation.setType((Operation::Type) ((operation-DMFUOAcos) + Operation::TAcos));
   else if (operation >= DMFUOCeil && operation <= DMFUOFloor)
      multifloatOperation.setType((Operation::Type) ((operation-DMFUOCeil) + Operation::TFloor));
   else if (operation >= DMFUOLog && operation <= DMFUOLog10)
      multifloatOperation.setType((Operation::Type) ((operation-DMFUOLog) + Operation::TLog));
   else if (operation >= DMFUOPow && operation <= DMFUOTanh)
      multifloatOperation.setType((Operation::Type) ((operation-DMFUOPow) + Operation::TPow));
   else
      {  AssumeUncalled }
   bool booleanResult = element->constraint(multifloatOperation, *result, *sharedConstraintEnvironment);
   AssumeCondition(booleanResult)
   if (sharedConstraintEnvironment->isEmpty())
      multifloatDomain->content = nullptr;
   else if (sharedConstraintEnvironment->hasResult())
      multifloatDomain->content = sharedConstraintEnvironment->presult().extractElement();
   else
      multifloatDomain->content = element.extractElement();
   env->warnings = sharedConstraintEnvironment->getErrorCode();
   env->emptyResult = sharedConstraintEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedConstraintEnvironment->getVerdict().verdict();
   sharedConstraintEnvironment->clear();
   return multifloatDomain->content != nullptr && !env->emptyResult;
}

DLL_API bool
domain_multifloat_binary_constraint(DomainMultiFloatElement resultDomain,
      DomainMultiFloatBinaryOperation operation, DomainMultiFloatElement* firstDomain,
      DomainMultiFloatElement* secondDomain, DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::Floating::Operation Operation;
   if (!sharedConstraintEnvironment.get())
      sharedConstraintEnvironment.reset(ApproximateAccessEnvironment().createConstraintEnvironment(
            Scalar::ConstraintEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedConstraintEnvironment);
   sharedConstraintEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement first(reinterpret_cast<VirtualElement*>(firstDomain->content), PNT::Pointer::Init());
   firstDomain->content = nullptr;
   PPVirtualElement second(reinterpret_cast<VirtualElement*>(secondDomain->content), PNT::Pointer::Init());
   secondDomain->content = nullptr;
   VirtualElement* result = reinterpret_cast<VirtualElement*>(resultDomain.content);
   AssumeCondition(result)
   Operation multifloatOperation;
   multifloatOperation.setArgumentsNumber(1);
   if (operation >= DMFBOPlus && operation <= DMFBOMinus)
      multifloatOperation.setType((Operation::Type) ((operation-DMFBOPlus) + Operation::TPlusAssign));
   else if (operation >= DMFBOMin && operation <= DMFBODivide)
      multifloatOperation.setType((Operation::Type) ((operation-DMFBOMin) + Operation::TMinAssign));
   else if (operation == DMFBOAtan2)
      multifloatOperation.setType(Operation::TAtan2);
   else if (operation >= DMFBOFmod && operation <= DMFBOLdexp)
      multifloatOperation.setType((Operation::Type) ((operation-DMFBOFmod) + Operation::TFmod));
   else if (operation == DMFBOModf)
      multifloatOperation.setType(Operation::TModf);
   else
      {  AssumeUncalled }
   sharedConstraintEnvironment->absorbFirstArgument(second.extractElement());
   bool booleanResult = first->constraint(multifloatOperation, *result, *sharedConstraintEnvironment);
   AssumeCondition(booleanResult)
   secondDomain->content = (VirtualElement*) sharedConstraintEnvironment->extractFirstArgument();
   if (sharedConstraintEnvironment->isEmpty())
      firstDomain->content = nullptr;
   else if (sharedConstraintEnvironment->hasResult())
      firstDomain->content = sharedConstraintEnvironment->presult().extractElement();
   else
      firstDomain->content = first.extractElement();
   env->warnings = sharedConstraintEnvironment->getErrorCode();
   env->emptyResult = sharedConstraintEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedConstraintEnvironment->getVerdict().verdict();
   sharedConstraintEnvironment->clear();
   return firstDomain->content != nullptr && secondDomain->content != nullptr && !env->emptyResult;
}

DLL_API bool
domain_multifloat_compare_constraint(DomainBitElement resultDomain,
      DomainMultiFloatCompareOperation operation, DomainMultiFloatElement* firstDomain,
      DomainMultiFloatElement* secondDomain, DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::Floating::Operation Operation;
   if (!sharedConstraintEnvironment.get())
      sharedConstraintEnvironment.reset(ApproximateAccessEnvironment().createConstraintEnvironment(
            Scalar::ConstraintEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedConstraintEnvironment);
   sharedConstraintEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement first(reinterpret_cast<VirtualElement*>(firstDomain->content), PNT::Pointer::Init());
   firstDomain->content = nullptr;
   PPVirtualElement second(reinterpret_cast<VirtualElement*>(secondDomain->content), PNT::Pointer::Init());
   secondDomain->content = nullptr;
   VirtualElement* result = reinterpret_cast<VirtualElement*>(resultDomain.content);
   AssumeCondition(result)
   Operation multifloatOperation;
   multifloatOperation.setArgumentsNumber(1);
   if (operation >= DMFCOCompareLess && operation <= DMFCOCompareGreater)
      multifloatOperation.setType((Operation::Type) ((operation-DMFCOCompareLess) + Operation::TCompareLess));
   else
      {  AssumeUncalled }
   multifloatOperation.setConst();
   sharedConstraintEnvironment->absorbFirstArgument(second.extractElement());
   bool booleanResult = first->constraint(multifloatOperation, *result, *sharedConstraintEnvironment);
   AssumeCondition(booleanResult)
   secondDomain->content = (VirtualElement*) sharedConstraintEnvironment->extractFirstArgument();
   if (sharedConstraintEnvironment->isEmpty())
      firstDomain->content = nullptr;
   else if (sharedConstraintEnvironment->hasResult())
      firstDomain->content = sharedConstraintEnvironment->presult().extractElement();
   else
      firstDomain->content = first.extractElement();
   env->warnings = sharedConstraintEnvironment->getErrorCode();
   env->emptyResult = sharedConstraintEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedConstraintEnvironment->getVerdict().verdict();
   sharedConstraintEnvironment->clear();
   return firstDomain->content != nullptr && secondDomain->content != nullptr && !env->emptyResult;
}

DLL_API bool
domain_multifloat_ternary_constraint(DomainMultiFloatElement resultDomain,
      DomainMultiFloatTernaryOperation operation, DomainMultiFloatElement* firstDomain,
      DomainMultiFloatElement* secondDomain, DomainMultiFloatElement* thirdDomain,
      DomainEvaluationEnvironment* env) {
   typedef Analyzer::Scalar::Floating::Operation Operation;
   if (!sharedConstraintEnvironment.get())
      sharedConstraintEnvironment.reset(ApproximateAccessEnvironment().createConstraintEnvironment(
            Scalar::ConstraintEnvironment::Init()));
   EvaluationEnvironment::GuardLatticeCreation guard(*sharedConstraintEnvironment);
   sharedConstraintEnvironment->setLatticeCreation(EvaluationEnvironment::LatticeCreation
         ((EvaluationEnvironment::LatticeCreation::Creation) env->defaultDomainType));
   PPVirtualElement first(reinterpret_cast<VirtualElement*>(firstDomain->content), PNT::Pointer::Init());
   firstDomain->content = nullptr;
   PPVirtualElement second(reinterpret_cast<VirtualElement*>(secondDomain->content), PNT::Pointer::Init());
   secondDomain->content = nullptr;
   PPVirtualElement third(reinterpret_cast<VirtualElement*>(thirdDomain->content), PNT::Pointer::Init());
   thirdDomain->content = nullptr;
   VirtualElement* result = reinterpret_cast<VirtualElement*>(resultDomain.content);
   AssumeCondition(result)
   Operation multifloatOperation;
   multifloatOperation.setArgumentsNumber(2);
   if (operation >= DMFTOMultAdd && operation <= DMFTONegMultSub)
      multifloatOperation.setType((Operation::Type) ((operation-DMFTOMultAdd) + Operation::TMultAddAssign));
   else
      {  AssumeUncalled }
   {  sharedConstraintEnvironment->absorbFirstArgument(second.extractElement());
      sharedConstraintEnvironment->absorbSecondArgument(third.extractElement());
      bool constraintResult = first->constraint(multifloatOperation, *result, *sharedConstraintEnvironment);
      AssumeCondition(constraintResult)
      first.absorbElement((VirtualElement*) sharedConstraintEnvironment->extractFirstArgument());
      second.absorbElement((VirtualElement*) sharedConstraintEnvironment->extractSecondArgument());
      if (!sharedConstraintEnvironment->isEmpty()) {
         if (sharedConstraintEnvironment->hasResult())
            first = sharedConstraintEnvironment->presult();
      }
      else
         first.release();
   }
   env->warnings = sharedConstraintEnvironment->getErrorCode();
   env->emptyResult = sharedConstraintEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedConstraintEnvironment->getVerdict().verdict();
   sharedConstraintEnvironment->clear();
   firstDomain->content = first.extractElement();
   secondDomain->content = second.extractElement();
   thirdDomain->content = third.extractElement();
   return firstDomain->content != nullptr && secondDomain->content != nullptr
         && thirdDomain->content != nullptr && !env->emptyResult;
}

DLL_API bool domain_multifloat_is_constant_value(DomainMultiFloatElement domain,
      DomainFloatingPointConstant* value) {
   const VirtualElement* element = reinterpret_cast<VirtualElement*>(domain.content);
   AssumeCondition(element)
   ApproximateAccessEnvironment access;
   if (!access.isConstantValue(*element))
      return false;
   if (value) {
      AssumeCondition(value->sizeInBits < 64)
      value->floatValue = access.getConstantFloat(*element);
   }
   return true;
}

DLL_API char*
domain_write(DomainElement domain, char* buffer, int buffer_size, int* length, void* writer,
      char* (*increase_buffer_size)(char* buffer, int old_length, int new_length, void* writer)) {
   WriteStream out(buffer, buffer_size, length, writer, increase_buffer_size);
   const VirtualElement* element = reinterpret_cast<VirtualElement*>(domain.content);
   element->write(out, STG::IOObject::FormatParameters());
   return out.getBuffer();
}

DLL_API bool domain_merge(DomainElement* domain, DomainElement sourceDomain,
      DomainEvaluationEnvironment* env) {
   VirtualElement* element = reinterpret_cast<VirtualElement*>(domain->content);
   VirtualElement* source = reinterpret_cast<VirtualElement*>(sourceDomain.content);
   AssumeCondition(element && source)
   {  Analyzer::Scalar::Approximate::TransmitEnvironment copyEnv(*sharedEvaluationEnvironment);
      bool boolResult = element->mergeWith(*source, copyEnv);
      AssumeCondition(boolResult)
      if (!copyEnv.isEmpty()) {
         if (copyEnv.hasResult()) {
            delete element;
            domain->content = copyEnv.presult().extractElement();
         }
      }
      else {
         delete element;
         domain->content = nullptr;
      };
   }
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return domain->content != nullptr && !env->emptyResult;
}

DLL_API bool
domain_intersect(DomainElement* domain, DomainElement sourceDomain,
      DomainEvaluationEnvironment* env) {
   VirtualElement* element = reinterpret_cast<VirtualElement*>(domain->content);
   VirtualElement* source = reinterpret_cast<VirtualElement*>(sourceDomain.content);
   AssumeCondition(element && source)
   {  Analyzer::Scalar::Approximate::TransmitEnvironment copyEnv(*sharedEvaluationEnvironment);
      bool boolResult = element->intersectWith(*source, copyEnv);
      AssumeCondition(boolResult)
      if (!copyEnv.isEmpty()) {
         if (copyEnv.hasResult()) {
            delete element;
            domain->content = copyEnv.presult().extractElement();
         }
      }
      else {
         delete element;
         domain->content = nullptr;
      };
   }
   env->warnings = sharedEvaluationEnvironment->getErrorCode();
   env->emptyResult = sharedEvaluationEnvironment->isEmpty();
   env->verdict = (DomainTypeVerdict) sharedEvaluationEnvironment->getVerdict().verdict();
   sharedEvaluationEnvironment->clear();
   return domain->content != nullptr && !env->emptyResult;
}

DLL_API bool
domain_contain(DomainElement domain, DomainElement sourceDomain,
      DomainEvaluationEnvironment* env) {
   const VirtualElement* element = reinterpret_cast<VirtualElement*>(domain.content);
   const VirtualElement* source = reinterpret_cast<VirtualElement*>(sourceDomain.content);
   AssumeCondition(element && source)
   Analyzer::Scalar::Approximate::EvaluationEnvironment copyEnv = Analyzer::Scalar::Approximate::EvaluationEnvironment(
      VirtualElement::EvaluationParameters(*sharedEvaluationEnvironment));
   bool boolResult = element->contain(*source, copyEnv);
   AssumeCondition(boolResult)
   return copyEnv.isTotalApplied();
}

DLL_API DomainElement
domain_create_disjunction_and_absorb(DomainElement* domain) {
   VirtualElement* element = reinterpret_cast<VirtualElement*>(domain->content);
   PNT::TPassPointer<Analyzer::Scalar::Approximate::VirtualDisjunction, Analyzer::Scalar::PPAbstractElement>
         result = VirtualElement::Methods::newDisjunction(*element);
   result->addExact(element);
   domain->content = nullptr;
   return DomainElement{result.extractElement()};
}

DLL_API void
domain_disjunction_absorb(DomainElement* disjunctionDomain, DomainElement* domain) {
   VirtualElement* element = reinterpret_cast<VirtualElement*>(domain->content);
   typedef Analyzer::Scalar::Approximate::VirtualDisjunction VirtualDisjunction;
   AssumeCondition(dynamic_cast<const VirtualDisjunction*>(reinterpret_cast<VirtualElement*>(disjunctionDomain->content)))
   PNT::TPassPointer<VirtualDisjunction, Analyzer::Scalar::PPAbstractElement>
         disjunction(static_cast<VirtualDisjunction*>(reinterpret_cast<VirtualElement*>(disjunctionDomain->content)), PNT::Pointer::Init());
   disjunctionDomain->content = nullptr;
   disjunction->addExact(element);
   domain->content = nullptr;
   disjunctionDomain->content = disjunction.extractElement();
}

DLL_API void
domain_specialize(DomainElement* domain) {
   VirtualElement* element = reinterpret_cast<VirtualElement*>(domain->content);
   AssumeCondition(element)
   ApproximateAccessEnvironment::KeepMemoryValue memoryModifier;
   Scalar::VirtualQueryOperation::DuplicationEnvironment duplicateEnv(&memoryModifier);
   element->query(Scalar::VirtualQueryOperation().setSpecialize(), duplicateEnv);
   if (duplicateEnv.presult().isValid()) {
      delete element;
      domain->content = duplicateEnv.presult().extractElement();
   }
}

} // end of extern "C"
