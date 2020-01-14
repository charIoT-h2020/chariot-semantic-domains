/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : scalar elements common to compilation constants and symbolic execution
// File      : MultiBitElement.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a multi-bit class used in simulation/symbolic execution.
//   All this stuff is rather defined for host independent code analysis.
//

#ifndef Analyzer_Scalar_VirtualExact_MultiBitElementH
#define Analyzer_Scalar_VirtualExact_MultiBitElementH

#include "Analyzer/Scalar/Implementation/General/MultiBitElement.h"
#include "Pointer/TreeMethods.h"

namespace Analyzer { namespace Scalar { namespace MultiBit { namespace VirtualExact {

#define DefineDeclareMethod(TypeOperation)                                                   \
bool apply##TypeOperation(const Scalar::MultiBit::Operation& operation, EvaluationEnvironment& env);

/*****************************************************/
/* Definition of the template class TMultiBitElement */
/*****************************************************/

template <class TypeBase, class TypeImplementation>
class TMultiBitElement : public TypeBase, protected TypeImplementation {
  public:
   typedef Scalar::MultiBit::Operation Operation;

  private:
   typedef TypeImplementation inheritedImplementation;
   typedef TypeBase inherited;
   typedef TMultiBitElement<TypeBase, TypeImplementation> thisType;

   class MethodApplyTable;
   #include "Analyzer/Scalar/VirtualExact/SimpleMethodTable.inch"

   static MethodApplyTable matMethodTable;

  protected:
   virtual ComparisonResult _compare(const EnhancedObject& asource) const override
      {  ComparisonResult result = inherited::_compare(asource);
         return (result == CREqual)
            ? inheritedImplementation::_compare((const inheritedImplementation&) (const thisType&) castFromCopyHandler(asource))
            : result;
      }
   virtual TypeBase* _createMultiBitElement(const typename TypeBase::Init& init) const override
      {  return new thisType(init); }
   virtual TypeBase* _createMultiFloatElement(const typename TypeBase::InitFloat& init) const override;
   virtual int _getSizeInBits() const override { return inheritedImplementation::getSize(); }

   TMultiBitElement(const typename TypeBase::Init& init)
      :  inherited(init), inheritedImplementation(init.getBitSize())
      {  if (init.hasInitialValue())
            inheritedImplementation::assign(init.getInitialValue());
      }
   TMultiBitElement(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   Template2DefineCopy(TMultiBitElement, TypeBase, TypeImplementation)
   DTemplate2DefineAssign(TMultiBitElement, TypeBase, TypeImplementation)
   DTemplate2Compare(TMultiBitElement, TypeBase, TypeImplementation)
   Template2StaticInheritConversions(TMultiBitElement, inherited, TypeBase, TypeImplementation)

   const inheritedImplementation& implementation() const
      {  return (const inheritedImplementation&) *this; }
   inheritedImplementation& implementation() { return (inheritedImplementation&) *this; }

   virtual bool apply(const VirtualOperation& aoperation, Scalar::EvaluationEnvironment& env) override
      {  const Operation& operation = (const Operation&) aoperation;
         if (!TypeBase::apply(operation, env)
            && !matMethodTable[operation.getType()].execute(*this, operation, env))
            {  AssumeUncalled }
         return true;
      }

   virtual typename inherited::ZeroResult queryZeroResult() const override
      {  return typename inherited::ZeroResult().setBool(inheritedImplementation::isZero()); }

   DefineDeclareMethod(Extend)
   DefineDeclareMethod(Reduce)
   DefineDeclareMethod(BitSet)
   DefineDeclareMethod(Concat)

   DefineDeclareMethod(CastChar)
   DefineDeclareMethod(CastInt)
   DefineDeclareMethod(CastUnsignedInt)
   DefineDeclareMethod(CastMultiBit)
   DefineDeclareMethod(CastBit)
   DefineDeclareMethod(CastShiftBit)
   DefineDeclareMethod(CastMultiFloat)

   DefineDeclareMethod(PrevSignedAssign)
   DefineDeclareMethod(PrevUnsignedAssign)
   DefineDeclareMethod(NextSignedAssign)
   DefineDeclareMethod(NextUnsignedAssign)

   DefineDeclareMethod(UnaryFloatAssign)
   DefineDeclareMethod(BinaryFloatAssign)
   DefineDeclareMethod(PlusSignedAssign)
   DefineDeclareMethod(PlusUnsignedAssign)
   DefineDeclareMethod(PlusUnsignedWithSignedAssign)
   DefineDeclareMethod(MinusSignedAssign)
   DefineDeclareMethod(MinusUnsignedAssign)

   DefineDeclareMethod(CompareSigned)
   DefineDeclareMethod(CompareUnsigned)
   DefineDeclareMethod(CompareFloat)
   DefineDeclareMethod(MinSignedAssign)
   DefineDeclareMethod(MinUnsignedAssign)
   DefineDeclareMethod(MaxSignedAssign)
   DefineDeclareMethod(MaxUnsignedAssign)

   DefineDeclareMethod(TimesSignedAssign)
   DefineDeclareMethod(TimesUnsignedAssign)
   DefineDeclareMethod(DivideSignedAssign)
   DefineDeclareMethod(DivideUnsignedAssign)
   DefineDeclareMethod(OppositeSignedAssign)

   DefineDeclareMethod(ModuloSignedAssign)
   DefineDeclareMethod(ModuloUnsignedAssign)
   DefineDeclareMethod(BitOrAssign)
   DefineDeclareMethod(BitAndAssign)
   DefineDeclareMethod(BitExclusiveOrAssign)
   DefineDeclareMethod(BitNegateAssign)
   DefineDeclareMethod(LeftShiftAssign)
   DefineDeclareMethod(LogicalRightShiftAssign)
   DefineDeclareMethod(ArithmeticRightShiftAssign)
   DefineDeclareMethod(RotateAssign)
}; 

#undef DefineDeclareMethod

}}}} // end of namespace Analyzer::Scalar::MultiBit::VirtualExact

#endif // Analyzer_Interpretation_Scalar_VirtualExact_MultiBitElementH

