/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : scalar elements common to compilation constants and symbolic execution
// File      : MultiFloatElement.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a multi-float class used in simulation/symbolic execution.
//   All this stuff is rather defined for host independent code analysis.
//

#ifndef Analyzer_Scalar_VirtualExact_MultiFloatElementH
#define Analyzer_Scalar_VirtualExact_MultiFloatElementH

#include "Analyzer/Scalar/Implementation/General/MultiFloatElement.h"
#include "Pointer/TreeMethods.h"

namespace Analyzer { namespace Scalar { namespace Floating { namespace VirtualExact {

#define DefineDeclareMethod(TypeOperation)                                                   \
bool apply##TypeOperation(const Operation& operation, EvaluationEnvironment& env);

#define DefineDeclareOMethod(TypeOperation, TypeArg)                                         \
bool apply##TypeOperation##TypeArg(const Operation& operation, EvaluationEnvironment& env);

/************************************************/
/* Definition of the template class TIntElement */
/************************************************/

// TypeImplementation = Implementation::Details::TMultiFloatElement<TypeMultiTraits>
template <class TypeBase, class TypeImplementation>
class TMultiFloatElement : public TypeBase, protected TypeImplementation {
  private:
   typedef TypeImplementation inheritedImplementation;
   typedef TypeBase inherited;
   typedef TMultiFloatElement<TypeBase, TypeImplementation> thisType;

   class MethodApplyTable;
   #include "Analyzer/Scalar/VirtualExact/SimpleMethodTable.inch"

   static MethodApplyTable matMethodTable;

  protected:
   virtual ComparisonResult _compare(const EnhancedObject& asource) const override
      {  ComparisonResult result = inherited::_compare(asource);
         return (result == CREqual)
            ? inheritedImplementation::compare((const thisType&) castFromCopyHandler(asource))
            : result;
      }
   virtual int _getSizeInBits() const override { return inheritedImplementation::getSize(); }
   
  public:
   TMultiFloatElement(const typename TypeBase::InitFloat& init)
      :  inherited(init), inheritedImplementation(init.sizeMantissa(), init.sizeExponent())
      {  if (init.hasInitialValue())
            inheritedImplementation::assign(init.getInitialValue());
      }
   TMultiFloatElement(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   Template2DefineCopy(TMultiFloatElement, TypeBase, TypeImplementation)
   DTemplate2DefineAssign(TMultiFloatElement, TypeBase, TypeImplementation)
   StaticInheritConversions(thisType, inherited)

   virtual bool isValid() const override { return inherited::isValid() && inheritedImplementation::isValid(); }

   inheritedImplementation& simplementation()
      {  return (inheritedImplementation&) *this; }
   const inheritedImplementation& implementation() const
      {  return (const inheritedImplementation&) *this; }

   virtual bool apply(const VirtualOperation& aoperation, Scalar::EvaluationEnvironment& env) override
      {  const Operation& operation = (const Operation&) aoperation;
         if (!TypeBase::apply(operation, env)
            && !matMethodTable[operation.getType()].execute(*this, operation, env))
            {  AssumeUncalled }
         return true;
      }

   virtual typename inherited::ZeroResult queryZeroResult() const override
      {  return typename inherited::ZeroResult().setBool(inheritedImplementation::isZero()); }

   DefineDeclareMethod(CastChar)
   DefineDeclareMethod(CastSignedChar)
   DefineDeclareMethod(CastUnsignedChar)
   DefineDeclareMethod(CastShort)
   DefineDeclareMethod(CastUnsignedShort)
   DefineDeclareMethod(CastInt)
   DefineDeclareMethod(CastUnsignedInt)
   DefineDeclareMethod(CastLong)
   DefineDeclareMethod(CastUnsignedLong)
   DefineDeclareMethod(CastMultiFloat)
   DefineDeclareMethod(CastFloat)
   DefineDeclareMethod(CastDouble)
   DefineDeclareMethod(CastLongDouble)
   DefineDeclareMethod(CastMultiBit)

   DefineDeclareMethod(PlusAssign)
   DefineDeclareMethod(MinusAssign)
   
   DefineDeclareMethod(Compare)
   DefineDeclareMethod(IsInftyExponent)
   DefineDeclareMethod(IsNaN)
   DefineDeclareMethod(IsQNaN)
   DefineDeclareMethod(IsSNaN)
   DefineDeclareMethod(IsPositive)
   DefineDeclareMethod(IsZeroExponent)
   DefineDeclareMethod(IsNegative)
   DefineDeclareMethod(CastMantissa)
   
   DefineDeclareMethod(MinAssign)
   DefineDeclareMethod(MaxAssign)
   DefineDeclareMethod(TimesAssign)
   DefineDeclareMethod(DivideAssign)
   DefineDeclareMethod(OppositeAssign)
   DefineDeclareMethod(AbsAssign)

   DefineDeclareMethod(MultAddAssign)
   DefineDeclareMethod(MultSubAssign)
   DefineDeclareMethod(NegMultAddAssign)
   DefineDeclareMethod(NegMultSubAssign)
}; 

#undef DefineDeclareMethod
#undef DefineDeclareOMethod

}}}} // end of namespace Analyzer::Scalar::Floating::VirtualExact

#endif // Analyzer_Interpretation_Scalar_VirtualExact_MultiFloatElementH

