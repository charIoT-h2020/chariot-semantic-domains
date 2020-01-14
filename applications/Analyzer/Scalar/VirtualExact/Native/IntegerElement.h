/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : scalar elements common to compilation constants and symbolic execution
// File      : IntegerElement.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of integer classes that are very helpful to define
//     the compilation constants and the scalar elements used in
//     simulation/symbolic execution.
//   All this stuff is rather defined for source code analysis.
//

#ifndef Analyzer_Scalar_VirtualExact_IntegerElementH
#define Analyzer_Scalar_VirtualExact_IntegerElementH

#include "Analyzer/Scalar/Implementation/Native/IntegerElement.h"
#include "Pointer/TreeMethods.h"

namespace Analyzer { namespace Scalar { namespace Integer { namespace VirtualExact {

#define DefineDeclareUnaryMethod(TypeOperation)                                                   \
bool apply##TypeOperation(const Operation& operation, EvaluationEnvironment& env);

#define DefineDeclareBinaryMethod(TypeOperation)                                                  \
bool apply##TypeOperation(const Operation& operation, EvaluationEnvironment& env);

#define DefineDeclareOBinaryMethod(TypeOperation, TypeArg)                                        \
bool apply##TypeOperation##TypeArg(const Operation& operation, EvaluationEnvironment& env);

/************************************************/
/* Definition of the template class TIntElement */
/************************************************/

template <class TypeBase>
class TIntElement : public TypeBase, private Implementation::IntElement {
  private:
   typedef Implementation::IntElement inheritedImplementation;
   typedef TypeBase inherited;
   typedef TIntElement<TypeBase> thisType;

   class MethodApplyTable;
   #include "Analyzer/Scalar/VirtualExact/Native/IntMethodTable.inch"

   static MethodApplyTable matMethodTable;

  protected:
   virtual ComparisonResult _compare(const EnhancedObject& asource) const override
      {  ComparisonResult result = inherited::_compare(asource);
         return (result == CREqual)
            ? inheritedImplementation::_compare((const inheritedImplementation&) castFromCopyHandler(asource))
            : result;
      }

  public:
   TIntElement(const typename TypeBase::Init& init) :  inherited(init)
      {  if (init.hasInitialValue()) {
            AssumeCondition(dynamic_cast<const inheritedImplementation*>(&init.getInitialValue()))
            inheritedImplementation::assign(((const inheritedImplementation&) init.getInitialValue()).getElement());
         };
      }
   TIntElement(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   TemplateDefineCopy(TIntElement, TypeBase)
   DTemplateDefineAssign(TIntElement, TypeBase)
   DTemplateCompare(TIntElement, TypeBase)
   StaticInheritConversions(TIntElement, inherited)

   virtual bool isValid() const override { return inherited::isValid() && inheritedImplementation::isValid(); }
   virtual bool isInt() const override { return true; }
   
   int getInt() const { return inheritedImplementation::getElement(); }
   const inheritedImplementation& implementation() const
      { return (const inheritedImplementation&) *this; }

   virtual bool apply(const VirtualOperation& operation, EvaluationEnvironment& env) override
      {  if (!TypeBase::apply(operation, env)
            && !matMethodTable[((const Operation&) operation).getType()].execute(*this,
               (const Operation&) operation, env))
            {  AssumeUncalled }
         return true;
      }

   #include "Analyzer/Scalar/VirtualExact/Native/CastInteger.inch"

   virtual typename inherited::ZeroResult queryZeroResult() const override
      {  return typename inherited::ZeroResult().setBool(inheritedImplementation::getElement()); }
   virtual int _getSizeInBits() const override
      {  return sizeof(typename inheritedImplementation::SimulatedType)*8; }

   DefineDeclareUnaryMethod(PrevAssign)
   DefineDeclareUnaryMethod(NextAssign)
   DefineDeclareBinaryMethod(PlusAssign)
   DefineDeclareBinaryMethod(MinusAssign)
   DefineDeclareBinaryMethod(TimesAssign)
   DefineDeclareBinaryMethod(DivideAssign)
   DefineDeclareUnaryMethod(OppositeAssign)
   DefineDeclareBinaryMethod(ModuloAssign)
   DefineDeclareBinaryMethod(BitOrAssign)
   DefineDeclareBinaryMethod(BitAndAssign)
   DefineDeclareBinaryMethod(BitExclusiveOrAssign)
   DefineDeclareUnaryMethod(BitNegateAssign)
   DefineDeclareBinaryMethod(LeftShiftAssign)
   DefineDeclareBinaryMethod(RightShiftAssign)
   DefineDeclareBinaryMethod(LogicalAndAssign)
   DefineDeclareBinaryMethod(LogicalOrAssign)
   DefineDeclareBinaryMethod(LogicalNegateAssign)
}; 

/********************************************************/
/* Definition of the template class TUnsignedIntElement */
/********************************************************/

template <class TypeBase>
class TUnsignedIntElement : public TypeBase, private Implementation::UnsignedIntElement {
  private:
   typedef TUnsignedIntElement<TypeBase> thisType;
   typedef Implementation::UnsignedIntElement inheritedImplementation;
   typedef TypeBase inherited;

   class MethodApplyTable;
   #include "Analyzer/Scalar/VirtualExact/SimpleMethodTable.inch"

   static MethodApplyTable matMethodTable;

  protected:
   virtual ComparisonResult _compare(const EnhancedObject& asource) const override
      {  ComparisonResult result = inherited::_compare(asource);
         return (result == CREqual)
            ? inheritedImplementation::_compare((const inheritedImplementation&) castFromCopyHandler(asource))
            : result;
      }

  public:
   TUnsignedIntElement(const typename TypeBase::Init& init)
      :  inherited(init), inheritedImplementation((const inheritedImplementation&) init.getInitialValue())
      {  AssumeCondition(dynamic_cast<const inheritedImplementation*>(&init.getInitialValue())) }
   TUnsignedIntElement(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   TemplateDefineCopy(TUnsignedIntElement, TypeBase)
   DTemplateDefineAssign(TUnsignedIntElement, TypeBase)
   DTemplateCompare(TUnsignedIntElement, TypeBase)
   StaticInheritConversions(TUnsignedIntElement, inherited)

   virtual bool isInt() const override { return true; }
   const inheritedImplementation& implementation() const
      {  return (const inheritedImplementation&) *this; }

   virtual bool apply(const VirtualOperation& operation, EvaluationEnvironment& env) override
      {  if (!inherited::apply(operation, env)
            && !matMethodTable[((const Operation&) operation).getType()].execute(*this,
               (const Operation&) operation, env))
            { AssumeUncalled }
         return true;
      }

   virtual typename inherited::ZeroResult queryZeroResult() const override
      {  return typename inherited::ZeroResult().setBool(inheritedImplementation::getElement()); }
   virtual int _getSizeInBits() const override
      {  return sizeof(typename inheritedImplementation::SimulatedType)*8; }

   #include "Analyzer/Scalar/VirtualExact/Native/CastInteger.inch"

   DefineDeclareUnaryMethod(PrevAssign)
   DefineDeclareUnaryMethod(NextAssign)
   DefineDeclareBinaryMethod(PlusAssign)
   DefineDeclareBinaryMethod(MinusAssign)
   DefineDeclareBinaryMethod(TimesAssign)
   DefineDeclareBinaryMethod(DivideAssign)
   DefineDeclareUnaryMethod(OppositeAssign)
   DefineDeclareBinaryMethod(ModuloAssign)
   DefineDeclareBinaryMethod(BitOrAssign)
   DefineDeclareBinaryMethod(BitAndAssign)
   DefineDeclareBinaryMethod(BitExclusiveOrAssign)
   DefineDeclareUnaryMethod(BitNegateAssign)
   DefineDeclareOBinaryMethod(LeftShiftAssign, IntElement)
   DefineDeclareOBinaryMethod(RightShiftAssign, IntElement)
}; 

/***************************************/
/* Definition of the class CharElement */
/***************************************/

#define DefineTypeObject TCharElement
#define DefineTypeInheritedImplementation Implementation::CharElement
#define DefineTypeIntCastPromotion CastIntOperation
#define DefineTypeCastDegradation CastCharOperation
#include "Analyzer/Scalar/VirtualExact/Native/IntegerSubElement.inch"
#undef DefineTypeCastDegradation
#undef DefineTypeIntCastPromotion
#undef DefineTypeInheritedImplementation
#undef DefineTypeObject

/*********************************************/
/* Definition of the class SignedCharElement */
/*********************************************/

#define DefineTypeObject TSignedCharElement
#define DefineTypeInheritedImplementation Implementation::SignedCharElement
#define DefineTypeIntCastPromotion CastIntOperation
#define DefineTypeCastDegradation CastSignedCharOperation
#include "Analyzer/Scalar/VirtualExact/Native/IntegerSubElement.inch"
#undef DefineTypeCastDegradation
#undef DefineTypeIntCastPromotion
#undef DefineTypeInheritedImplementation
#undef DefineTypeObject

/***********************************************/
/* Definition of the class UnsignedCharElement */
/***********************************************/

#define DefineTypeObject TUnsignedCharElement
#define DefineTypeInheritedImplementation Implementation::UnsignedCharElement
#define DefineTypeIntCastPromotion CastUnsignedIntOperation
#define DefineTypeCastDegradation CastUnsignedCharOperation
#include "Analyzer/Scalar/VirtualExact/Native/IntegerSubElement.inch"
#undef DefineTypeCastDegradation
#undef DefineTypeIntCastPromotion
#undef DefineTypeInheritedImplementation
#undef DefineTypeObject

/****************************************/
/* Definition of the class ShortElement */
/****************************************/

#define DefineTypeObject TShortElement
#define DefineTypeInheritedImplementation Implementation::ShortElement
#define DefineTypeIntCastPromotion CastIntOperation
#define DefineTypeCastDegradation CastShortOperation
#include "Analyzer/Scalar/VirtualExact/Native/IntegerSubElement.inch"
#undef DefineTypeCastDegradation
#undef DefineTypeIntCastPromotion
#undef DefineTypeInheritedImplementation
#undef DefineTypeObject

/************************************************/
/* Definition of the class UnsignedShortElement */
/************************************************/

#define DefineTypeObject TUnsignedShortElement
#define DefineTypeInheritedImplementation Implementation::UnsignedShortElement
#define DefineTypeIntCastPromotion CastUnsignedIntOperation
#define DefineTypeCastDegradation CastUnsignedShortOperation
#include "Analyzer/Scalar/VirtualExact/Native/IntegerSubElement.inch"
#undef DefineTypeCastDegradation
#undef DefineTypeIntCastPromotion
#undef DefineTypeInheritedImplementation
#undef DefineTypeObject

/***************************************/
/* Definition of the class LongElement */
/***************************************/

#define DefineTypeObject TLongElement
#define DefineTypeInheritedImplementation Implementation::LongElement
#include "Analyzer/Scalar/VirtualExact/Native/IntegerLongElement.inch"
#undef DefineTypeObject
#undef DefineTypeInheritedImplementation

/***********************************************/
/* Definition of the class UnsignedLongElement */
/***********************************************/

#define DefineTypeObject TUnsignedLongElement
#define DefineTypeInheritedImplementation Implementation::UnsignedLongElement
#include "Analyzer/Scalar/VirtualExact/Native/IntegerLongElement.inch"
#undef DefineTypeObject
#undef DefineTypeInheritedImplementation

#undef DefineDeclareUnaryMethod
#undef DefineDeclareBinaryMethod
#undef DefineDeclareOBinaryMethod

}}}} // end of namespace Analyzer::Scalar::Integer::VirtualExact

#endif // Analyzer_Scalar_VirtualExact_IntegerElementH

