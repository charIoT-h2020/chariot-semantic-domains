/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : Top.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of a class of Top.
//

#include "Analyzer/Scalar/Approximate/Top.h"
// #include "Analyzer/Scalar/Scalar.hpp"

namespace Analyzer { namespace Scalar { namespace Approximate {

inline PPVirtualElement
Top::promoteIntegerInterval() const {
   return Details::IntOperationElement::Methods::newInterval(*this,
         Details::VirtualIntegerInterval::Init().setInterval(Details::IntOperationElement::Methods::newMin(*this),
            Details::IntOperationElement::Methods::newMax(*this)));
}

/* Définition des différentes tables de méthodes */

#define DefineInitQuery(TypeOperation) \
   elementAt(QueryOperation::T##TypeOperation).setMethod(&Top::query##TypeOperation);
#define DefineInitOQuery(TypeOperation, TypeMethod) \
   elementAt(QueryOperation::T##TypeOperation).setMethod(&Top::query##TypeMethod);

Top::FloatQueryTable::FloatQueryTable() {
#include "StandardClasses/UndefineNew.h"
   FunctionQueryTable* table = &elementAt(VirtualQueryOperation::TCompareSpecial);
   table->setSize(3);
   (*table)[0].setMethod(&Top::queryCompareSpecial);
   (*table)[1].setMethod(&Top::queryGuardAllFloat);
   (*table)[2].setMethod(&Top::queryCompareSpecial);
   
   table = &elementAt(VirtualQueryOperation::TSimplification);
   table->setSize(Details::IntOperationElement::QueryOperation::EndOfTypeSimplification);

   table = &elementAt(VirtualQueryOperation::TDuplication);
   table->setSize(3);
   (*table)[VirtualQueryOperation::TCDClone].setMethod(&VirtualElement::queryCopy);
   (*table)[VirtualQueryOperation::TCDSpecialize].setMethod(&VirtualElement::querySpecialize);
#include "StandardClasses/DefineNew.h"
}

Top::IntQueryTable::IntQueryTable() {
#include "StandardClasses/UndefineNew.h"
   FunctionQueryTable* table = &elementAt(VirtualQueryOperation::TCompareSpecial);
   table->setSize(7);
   (*table)[0].setMethod(&Top::queryCompareSpecial);
   (*table)[1].setMethod(&Top::queryGuardAllInt);
   (*table)[2].setMethod(&Top::queryCompareSpecial);
   (*table)[3].setMethod(&Top::queryCompareSpecial);
   (*table)[4].setMethod(&Top::queryCompareSpecial);
   (*table)[5].setMethod(&Top::queryCompareSpecial);
   (*table)[6].setMethod(&Top::queryCompareSpecial);
   
   table = &elementAt(VirtualQueryOperation::TSimplification);
   table->setSize(Details::IntOperationElement::QueryOperation::EndOfTypeSimplification);

   (*table)[Details::IntOperationElement::QueryOperation::TSUndefined].setMethod(&Top::querySimplificationIdentity);
   (*table)[Details::IntOperationElement::QueryOperation::TSConstant].setMethod(&Top::queryFailSimplification);
   (*table)[Details::IntOperationElement::QueryOperation::TSInterval].setMethod(&Top::querySimplificationAsInterval);
   (*table)[Details::IntOperationElement::QueryOperation::TSConstantDisjunction].setMethod(&Top::querySimplificationAsConstantDisjunction);

   table = &elementAt(VirtualQueryOperation::TDuplication);
   table->setSize(3);
   (*table)[VirtualQueryOperation::TCDClone].setMethod(&VirtualElement::queryCopy);
   (*table)[VirtualQueryOperation::TCDSpecialize].setMethod(&VirtualElement::querySpecialize);
#include "StandardClasses/DefineNew.h"
}

#undef DefineInitQuery
#undef DefineInitOQuery

Top::FloatQueryTable Top::mqtFloatMethodQueryTable;
Top::IntQueryTable Top::mqtIntMethodQueryTable;

#define DefineInitApplyTo(TypeOperation)                                                        \
   elementAt(Operation::TypeOperation).setMethod(&Top::applyTo##TypeOperation);
#define DefineInitOApplyTo(TypeOperation, TypeMethod)                                           \
   elementAt(Operation::TypeOperation).setMethod(&Top::applyTo##TypeMethod);
#define DefineInitOIFApplyTo(TypeOperation, TypeIntMethod, TypeFloatMethod)                     \
   elementAt(Operation::TypeOperation).setArray(new IntAndFloatApplyToMethod(                   \
         &Top::applyTo##TypeIntMethod, &Top::applyTo##TypeFloatMethod));
#define DefineInitIFApplyTo(TypeOperation)                                                      \
   elementAt(Operation::TypeOperation).setArray(new IntAndFloatApplyToMethod(                   \
         &Top::applyTo##TypeOperation##Int, &Top::applyTo##TypeOperation##Float));
#define DefineInitIFPApplyTo(TypeOperation, TypeMethod)                                         \
   elementAt(Operation::TypeOperation).setArray(new IntFloatAndPointerApplyToMethod(            \
         &Top::applyTo##TypeMethod##Int, &Top::applyTo##TypeMethod##Float,                      \
         &Top::applyTo##TypeMethod##Pointer));

// Top::MethodApplyToTable::MethodApplyToTable() {
// #include "StandardClasses/UndefineNew.h"
//    clear();
// DefineInitOIFApplyTo(CompareLess, CompareIntOrdered, CompareAnyResult)
// DefineInitOIFApplyTo(CompareLessOrEqual, CompareIntOrdered, CompareAnyResult)
// DefineInitOApplyTo(CompareEqual, CompareAnyResult)
// DefineInitOApplyTo(CompareDifferent, CompareAnyResult)
// DefineInitOIFApplyTo(CompareGreaterOrEqual, CompareIntOrdered, CompareAnyResult)
// DefineInitOIFApplyTo(CompareGreater, CompareIntOrdered, CompareAnyResult)

// DefineInitIFPApplyTo(PlusAssign, ArithmeticAssign)
// DefineInitIFPApplyTo(MinusAssign, ArithmeticAssign)
// DefineInitIFApplyTo(TimesAssign)
// DefineInitOIFApplyTo(DivideAssign, PromotedInteger, DivideAssignFloat)

// DefineInitOApplyTo(ModuloAssign, PromotedInteger)
// DefineInitOApplyTo(BitOrAssign, PromotedInteger)
// DefineInitOApplyTo(BitAndAssign, PromotedInteger)
// DefineInitOApplyTo(BitExclusiveOrAssign, PromotedInteger)
// DefineInitOApplyTo(LeftShiftAssign, PromotedInteger)
// DefineInitOApplyTo(RightShiftAssign, PromotedInteger)

// DefineInitOApplyTo(LogicalAnd, PromotedInteger)
// DefineInitOApplyTo(LogicalOr, PromotedInteger)

// DefineInitApplyTo(Atan2)
// DefineInitApplyTo(Fmod)
// DefineInitApplyTo(Frexp)
// DefineInitApplyTo(Ldexp)
// DefineInitApplyTo(Modf)
// DefineInitApplyTo(Pow)
// #include "StandardClasses/DefineNew.h"
// }
#undef DefineInitApplyTo
#undef DefineInitOApplyTo
#undef DefineInitOIFApplyTo
#undef DefineInitIFPApplyTo

#define DefineInitApply(TypeOperation)                                                          \
   elementAt(Operation::TypeOperation).setMethod(&Top::apply##TypeOperation);
#define DefineInitOApply(TypeOperation, TypeMethod)                                             \
   elementAt(Operation::TypeOperation).setMethod(&Top::apply##TypeMethod);
#define DefineInitBApply(TypeOperation, TypeArray, TypeFstMethod, TypeSndMethod)                \
   elementAt(Operation::TypeOperation).setArray(new TypeArray##ApplyMethod(                     \
         &Top::apply##TypeFstMethod, &Top::apply##TypeSndMethod));
#define DefineInitTApply(TypeOperation, TypeArray, TypeFstMethod, TypeSndMethod, TypeThdMethod) \
   elementAt(Operation::TypeOperation).setArray(new TypeArray##ApplyMethod(                     \
         &Top::apply##TypeFstMethod, &Top::apply##TypeSndMethod, &Top::apply##TypeThdMethod));
#define DefineInitIFPApply(TypeOperation, TypeMethod)                                           \
   elementAt(Operation::TypeOperation).setArray(new IntFloatAndPointerBinaryApplyMethod(        \
         &Top::apply##TypeMethod##Int, &Top::apply##TypeMethod##Float,          \
         &Top::apply##TypeMethod##Pointer));
#define DefineInitIFApply(TypeOperation)                                                        \
   elementAt(Operation::TypeOperation).setArray(new IntAndFloatBinaryApplyMethod(               \
         &Top::apply##TypeOperation##Int, &Top::apply##TypeOperation##Float));

// Top::MethodApplyTable::MethodApplyTable() {
// #include "StandardClasses/UndefineNew.h"
//    clear();
// DefineInitOApply(CastInt, CastInteger)
// DefineInitOApply(CastChar, CastInteger)
// DefineInitOApply(CastSignedChar, CastInteger)
// DefineInitOApply(CastUnsignedChar, CastInteger)
// DefineInitOApply(CastShort, CastInteger)
// DefineInitOApply(CastUnsignedShort, CastInteger)
// DefineInitOApply(CastUnsignedInt, CastInteger)
// DefineInitOApply(CastLong, CastInteger)
// DefineInitOApply(CastUnsignedLong, CastInteger)
// DefineInitBApply(CastFloat, IntAndPointerUnary, PromotedInteger, PromotedFloat)
// DefineInitBApply(CastDouble, IntAndPointerUnary, PromotedInteger, PromotedFloat)
// DefineInitBApply(CastLongDouble, IntAndPointerUnary, PromotedInteger, PromotedFloat)

// DefineInitOApply(PrevAssign, IncrementAssign)
// DefineInitOApply(NextAssign, IncrementAssign)
// DefineInitIFPApply(PlusAssign, ArithmeticAssign)
// DefineInitIFPApply(MinusAssign, ArithmeticAssign)
// DefineInitBApply(CompareLess, IntAndGenericCompare, CompareIntOrdered, CompareAnyResult)
// DefineInitBApply(CompareLessOrEqual, IntAndGenericCompare, CompareIntOrdered, CompareAnyResult)
// DefineInitOApply(CompareEqual, CompareAnyResult)
// DefineInitOApply(CompareDifferent, CompareAnyResult)
// DefineInitBApply(CompareGreater, IntAndGenericCompare, CompareIntOrdered, CompareAnyResult)
// DefineInitBApply(CompareGreaterOrEqual, IntAndGenericCompare, CompareIntOrdered, CompareAnyResult)
// DefineInitOApply(IsInftyExponent, QueryFloat)
// DefineInitOApply(IsNaN, QueryFloat)
// DefineInitOApply(IsQNaN, QueryFloat)
// DefineInitOApply(IsSNaN, QueryFloat)
// DefineInitOApply(IsPositive, QueryFloat)
// DefineInitOApply(IsZeroExponent, QueryFloat)
// DefineInitOApply(IsNegative, QueryFloat)
// DefineInitOApply(CastMantissa, ...)

// DefineInitIFApply(TimesAssign)
// DefineInitBApply(DivideAssign, GenericIntAndFloatBinary, PromotedInteger, DivideAssignFloat)
// DefineInitOApply(OppositeAssign, Identity)
// DefineInitApply(Opposite)

// DefineInitOApply(ModuloAssign, PromotedInteger)
// DefineInitOApply(BitOrAssign, PromotedInteger)
// DefineInitOApply(BitAndAssign, PromotedInteger)
// DefineInitOApply(BitExclusiveOrAssign, PromotedInteger)
// DefineInitOApply(BitNegateAssign, Identity)
// DefineInitOApply(LeftShiftAssign, PromotedInteger)
// DefineInitOApply(RightShiftAssign, Identity)

// DefineInitOApply(LogicalAndAssign, PromotedInteger)
// DefineInitOApply(LogicalOrAssign, PromotedInteger)
// DefineInitOApply(LogicalNegateAssign, Identity)

// DefineInitOApply(Acos, Unimplemented)
// DefineInitOApply(Asin, Unimplemented)
// DefineInitOApply(Atan, Unimplemented)
// DefineInitOApply(Atan2, Unimplemented)
// DefineInitOApply(Ceil, Unimplemented)
// DefineInitOApply(Cos, Unimplemented)
// DefineInitOApply(Cosh, Unimplemented)
// DefineInitOApply(Exp, Unimplemented)
// DefineInitOApply(Fabs, Unimplemented)
// DefineInitOApply(Floor, Unimplemented)
// DefineInitOApply(Fmod, Unimplemented)
// DefineInitOApply(Frexp, Unimplemented)
// DefineInitOApply(Ldexp, Unimplemented)
// DefineInitOApply(Log, Unimplemented)
// DefineInitOApply(Log10, Unimplemented)
// DefineInitOApply(Modf, Unimplemented)
// DefineInitOApply(Pow, Unimplemented)
// DefineInitOApply(Sin, Unimplemented)
// DefineInitOApply(Sinh, Unimplemented)
// DefineInitOApply(Sqrt, Unimplemented)
// DefineInitOApply(Tan, Unimplemented)
// DefineInitOApply(Tanh, Unimplemented)
// #include "StandardClasses/DefineNew.h"
// }

#undef DefineInitApply
#undef DefineInitOApply
#undef DefineInitIFPApply
#undef DefineInitIFApply
#undef DefineInitBIFApply

#define DefineInitUnaryConstraint(TypeOperation) \
   mcutUnaryTable.elementAt(Operation::TypeOperation).setMethod(&VirtualElement::constraint##TypeOperation);

#define DefineInitUnaryOConstraint(TypeOperation, TypeMethod) \
   mcutUnaryTable.elementAt(Operation::TypeOperation).setMethod(&VirtualElement::constraint##TypeMethod);

#define DefineInitBinaryConstraintGeneric(TypeOperation)                                           \
   mcbtBinaryTable.elementAt(Operation::TypeOperation).setArray(new GenericBinaryConstraintMethod( \
      &VirtualElement::constraint##TypeOperation));

// Top::MethodConstraintTable::MethodConstraintTable() {
// #include "StandardClasses/UndefineNew.h"
// DefineInitUnaryOConstraint(CastChar, CastIntInteger)
// DefineInitUnaryOConstraint(CastSignedChar, CastIntInteger)
// DefineInitUnaryOConstraint(CastUnsignedChar, CastIntInteger)
// DefineInitUnaryOConstraint(CastShort, CastIntInteger)
// DefineInitUnaryOConstraint(CastUnsignedShort, CastIntInteger)
// DefineInitUnaryOConstraint(CastInt, CastIntInteger)
// DefineInitUnaryOConstraint(CastUnsignedInt, CastIntInteger)
// DefineInitUnaryOConstraint(CastLong, CastIntInteger)
// DefineInitUnaryOConstraint(CastUnsignedLong, CastIntInteger)
// DefineInitUnaryOConstraint(CastFloat, CastIntDouble)
// DefineInitUnaryOConstraint(CastDouble, CastIntDouble)
// DefineInitUnaryOConstraint(CastLongDouble, CastIntDouble)

// DefineInitUnaryConstraint(PrevAssign)
// DefineInitUnaryConstraint(NextAssign)

// DefineInitBinaryConstraintGeneric(PlusAssign)
// DefineInitBinaryConstraintGeneric(MinusAssign)
// DefineInitBinaryConstraintGeneric(TimesAssign)
// DefineInitBinaryConstraintGeneric(DivideAssign)

// DefineInitUnaryConstraint(OppositeAssign)

// DefineInitBinaryConstraintGeneric(ModuloAssign)
// DefineInitBinaryConstraintGeneric(BitOrAssign)
// DefineInitBinaryConstraintGeneric(BitAndAssign)
// DefineInitBinaryConstraintGeneric(BitExclusiveOrAssign)
// DefineInitBinaryConstraintGeneric(LeftShiftAssign)
// DefineInitBinaryConstraintGeneric(RightShiftAssign)
// DefineInitUnaryConstraint(BitNegateAssign)

// DefineInitBinaryConstraintGeneric(LogicalAnd)
// DefineInitBinaryConstraintGeneric(LogicalOr)
// DefineInitUnaryConstraint(LogicalNegate)

   // unimplemented on floating points
// #include "StandardClasses/DefineNew.h"
// }

#undef DefineInitUnaryConstraint
#undef DefineInitUnaryOConstraint
#undef DefineInitBinaryConstraint

// Top::MethodConstraintTable Top::mctMethodConstraintTable;

/* Definition of the virtual methods */

// #define DefineTypeObject Top
// #include "Analyzer/Scalar/Approximate/VirtualCall.incc"
// #undef DefineTypeObject

bool
Top::apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) {
   if (inherited::apply(operation, env))
      return true;

   if (afApplyTopFunction && (*afApplyTopFunction)(*this, operation, (EvaluationEnvironment&) env))
      return true;
   PPVirtualElement copy;
   if (operation.isConst() || operation.isConstWithAssign()) {
      copy.setElement(*this);
      env.presult() = copy;
   };
   return true;
}

bool
Top::applyTo(const VirtualOperation& operation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   if (afApplyToTopFunction && (*afApplyToTopFunction)(*this, thisElement, operation, (EvaluationEnvironment&) env))
      return true;

   PPVirtualElement copy;
   copy.setElement(*this);
   env.presult() = copy;
   return true;
}

bool
Top::constraint(const VirtualOperation& operation, const VirtualElement& result, ConstraintEnvironment& env) {
   if (inherited::constraint(operation, result, env))
      return true;
      
   if (cfConstraintTopFunction && (*cfConstraintTopFunction)(*this, operation, result, env))
      return true;
   env.mergeVerdictDegradate();
   return true;
}

bool
Top::mergeWithTo(VirtualElement& source, EvaluationEnvironment& env) const {
   env.mergeVerdictExact();
   env.storeResult(createSCopy());
   env.setUnstable();
   return true;
}

bool
Top::containTo(const VirtualElement& source, EvaluationEnvironment& env) const {
   env.setPartialApplied();
   return true;
}

bool
Top::intersectWithTo(VirtualElement& source, EvaluationEnvironment& env) const {
   env.mergeVerdictExact();
   return true;
}

bool
Top::mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) {
   ((EvaluationEnvironment&) env).mergeVerdictExact();
   return true;
}

bool
Top::contain(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) const
   { return true; }

bool
Top::intersectWith(const VirtualElement& source, EvaluationEnvironment& env) {
   env.mergeVerdictExact();
   if (source.getApproxKind() > getApproxKind())
      return source.intersectWithTo(*this, env);
   if (!source.getApproxKind().isTop()) {
      env.storeResult(source.createSCopy());
      env.setUnstable();
   };
   return true;
}

/* Implementation of the query methods */

bool
Top::querySizes(const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) const {
   AssumeCondition(dynamic_cast<const Details::RealOperationElement::QueryOperation::SizesEnvironment*>(&aenv))
   Details::RealOperationElement::QueryOperation::SizesEnvironment& env = (Details::RealOperationElement::QueryOperation::SizesEnvironment&) aenv;
   env.setSizeMantissa(uSizeMantissa).setSizeExponent(uSizeExponent).setSize(uSizeInBits);
   return true;
}

bool
Top::queryCompareSpecial(const VirtualElement& thisElement, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env) {
   ((VirtualQueryOperation::CompareSpecialEnvironment&) env).fill();
   return true;
}

bool
Top::queryGuardAllFloat(const VirtualElement& thisElement, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const VirtualQueryOperation::GuardEnvironment*>(&aenv))
   VirtualQueryOperation::GuardEnvironment& env = (VirtualQueryOperation::GuardEnvironment&) aenv;
   env.presult() = Approximate::Details::RealOperationElement::Methods::newTrue(thisElement);
   return true;
}

bool
Top::queryGuardAllInt(const VirtualElement& thisElement, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const VirtualQueryOperation::GuardEnvironment*>(&aenv))
   VirtualQueryOperation::GuardEnvironment& env = (VirtualQueryOperation::GuardEnvironment&) aenv;
   env.presult() = Approximate::Details::IntOperationElement::Methods::newTrue(thisElement);
   return true;
}

bool
Top::querySimplificationIdentity(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment&)
   {  return true; }

bool
Top::queryFailSimplification(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env) {
   AssumeCondition(dynamic_cast<const SimplificationEnvironment*>(&env))
   ((SimplificationEnvironment&) env).setFail();
    return true;
}

bool
Top::querySimplificationAsInterval(const VirtualElement& thisElement, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const Details::IntOperationElement::SimplificationEnvironment*>(&aenv))
   Details::IntOperationElement::SimplificationEnvironment& env = (Details::IntOperationElement::SimplificationEnvironment&) aenv;

   bool isUnsigned = Details::IntOperationElement::Methods::isSigned(thisElement);
   PNT::TPassPointer<Details::VirtualIntegerInterval, PPAbstractElement> result = Details::IntOperationElement::
         Methods::newInterval(thisElement, Details::VirtualIntegerInterval::Init().setInterval(Details::IntOperationElement::Methods::newMin(thisElement),
            Details::IntOperationElement::Methods::newMax(thisElement)).setUnsigned(isUnsigned));
   env.presult() = result;
   return true;
}

bool
Top::querySimplificationAsConstantDisjunction(const VirtualElement& thisElement, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const Details::IntOperationElement::SimplificationEnvironment*>(&aenv))
   Details::IntOperationElement::SimplificationEnvironment& env = (Details::IntOperationElement::SimplificationEnvironment&) aenv;

   if (thisElement.getSizeInBits() <= 6) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunction = Methods::newDisjunction(thisElement);
      PPVirtualElement min = Details::IntOperationElement::Methods::newMin(thisElement);
      int size = (1 << thisElement.getSizeInBits());
      VirtualOperation incOperation = Details::IntOperationElement::
         Methods::queryNextAssign(thisElement);
      while (--size >= 0) {
         disjunction->addMay(min->createSCopy());
         min = Details::IntOperationElement::Methods::applyAssign(min, incOperation,
               EvaluationParameters());
      };
      env.presult() = disjunction;
   }
   else
      ((SimplificationEnvironment&) env).setFail();
    return true;
}

bool
Top::query(const VirtualQueryOperation& aoperation, VirtualQueryOperation::Environment& env) const {
   if (inherited::query(aoperation, env))
      return true;
   const VirtualQueryOperation& operation = (const VirtualQueryOperation&) aoperation;
   if (operation.isOperation() || operation.isDomain() || operation.isExtern()) {
      AssumeCondition(pfqtQueryOperationTable && pfqtQueryDomainTable && pfqtQueryExternTable)
      (operation.isOperation() ? pfqtQueryOperationTable : (operation.isDomain()
           ? pfqtQueryDomainTable : pfqtQueryExternTable))->execute(*this, operation, env);
   }
   else if (operation.isExternMethod()) {
      if (operation.getExtensions() == Details::RealOperationElement::QueryOperation::TEMSizes)
         assume(querySizes(operation, env));
      else
         assume(false);
   }
   else {
      if (doesSupportFloatOperation())
         assume(mqtFloatMethodQueryTable[operation.getType()].execute(*this, operation, env));
      else
         assume(mqtIntMethodQueryTable[operation.getType()].execute(*this, operation, env));
   }
   return true;
}

/* Implementation of the methods applyTo */

   // comparison methods

bool
Top::applyToCompareIntOrdered(const VirtualOperation& aoperation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   // the algorithm could be more robust
   typedef Scalar::Integer::Operation Operation;
   const Operation& operation = (const Operation&) aoperation;
   Details::IntOperationElement::QueryOperation::CompareSpecialEnvironment thisSpecial
      = Details::IntOperationElement::Methods::compareSpecial(thisElement);
   bool isAssuredResult = (operation.getType() == Operation::TCompareLessOrEqual)
      || (operation.getType() == Operation::TCompareGreaterOrEqual);
   bool isSourceMayMinimal;
   bool isSourceSureMinimal;
   if ((operation.getType() == Operation::TCompareLess)
         || (operation.getType() == Operation::TCompareGreaterOrEqual)) {
      if (Details::IntOperationElement::Methods::isSigned(*this)) {
         isSourceSureMinimal = thisSpecial.isMinint();
         isSourceMayMinimal = thisSpecial.mayBeMinint();
      }
      else {
         isSourceSureMinimal = thisSpecial.isZero();
         isSourceMayMinimal = thisSpecial.mayBeZero();
      };
   }
   else if ((operation.getType() == Operation::TCompareGreater)
         || (operation.getType() == Operation::TCompareLessOrEqual)) {
      isSourceSureMinimal = thisSpecial.isMaxint();
      isSourceMayMinimal = thisSpecial.mayBeMaxint();
   }
   else {
      AssumeUncalled
      isSourceSureMinimal = isSourceMayMinimal = false;
   };

   env.mergeVerdictExact();
   if (isSourceSureMinimal)
      env.presult() = (const PPVirtualElement&) (isAssuredResult
            ? Details::IntOperationElement::Methods::newTrue(*this)
            : Details::IntOperationElement::Methods::newFalse(*this));
   else if (isSourceMayMinimal) {
      if (env.getInformationKind().isSure())
         env.presult() = (const PPVirtualElement&) (isAssuredResult
               ? Details::IntOperationElement::Methods::newTrue(*this)
               : Details::IntOperationElement::Methods::newFalse(*this));
      else {
         PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
            disjunction = Details::IntOperationElement::Methods::newBooleanDisjunction(*this);
         if (env.getInformationKind().isMay())
            disjunction->addMay(Details::IntOperationElement::Methods::newTrue(*this))
               .addMay(Details::IntOperationElement::Methods::newFalse(*this));
         else
            disjunction->addExact((const PPVirtualElement&) (isAssuredResult
                  ? Details::IntOperationElement::Methods::newTrue(*this)
                  : Details::IntOperationElement::Methods::newFalse(*this)))
               .addMay((const PPVirtualElement&) (isAssuredResult
                  ? Details::IntOperationElement::Methods::newFalse(*this)
                  : Details::IntOperationElement::Methods::newTrue(*this)));
         env.presult() = disjunction;
      };
   }
   else {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
         disjunction = Details::IntOperationElement::Methods::newBooleanDisjunction(*this);
      disjunction->add(Details::IntOperationElement::Methods::newTrue(*this), env.getInformationKind())
         .add(Details::IntOperationElement::Methods::newFalse(*this), env.getInformationKind());
      env.presult() = disjunction;
   };
   return true;
}

bool
Top::applyToCompareAnyResult(const VirtualOperation&, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   env.mergeVerdictExact();
   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
      disjunction = Details::IntOperationElement::Methods::newBooleanDisjunction(*this);
   disjunction->add(Details::IntOperationElement::Methods::newTrue(*this), env.getInformationKind())
      .add(Details::IntOperationElement::Methods::newFalse(*this), env.getInformationKind());
   env.presult() = disjunction;
   return true;
}

   // arithmetic methods

bool
Top::applyToArithmeticAssignInt(const VirtualOperation& operation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   bool doesHaveDifferentSize = thisElement.getSizeInBits() != getSizeInBits();
   if (doesHaveDifferentSize) { // char += Tint
      env.setPositiveOverflow(env.getInformationKind());
      env.setNegativeOverflow(env.getInformationKind());
   }
   if (env.doesPropagateErrorStates())
      return applyToPromotedInteger(operation, thisElement, env);
   env.presult() = Details::IntOperationElement::Methods::newTop(thisElement);
   if (!doesHaveDifferentSize) { // char += TChar
      Details::IntOperationElement::QueryOperation::CompareSpecialEnvironment compare
         = Details::IntOperationElement::Methods::compareSpecial(thisElement);
      if (compare.isZero())
         return true;
      if (compare.mayBeGreaterZero()) {
         env.setPositiveOverflow();
         if (compare.isGreaterZero() && (!env.getInformationKind().isMay()))
            env.setSurePositiveOverflow();
      };
      if (compare.mayBeLessZero()) {
         env.setNegativeOverflow();
         if (compare.isLessZero() && (!env.getInformationKind().isMay()))
            env.setSureNegativeOverflow();
      };
   };
   return true;
}

bool
Top::applyToArithmeticAssignFloat(const VirtualOperation&, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   env.presult() = Details::RealOperationElement::Methods::newTop(thisElement);
   Details::RealOperationElement::QueryOperation::CompareSpecialEnvironment compare
      = Details::RealOperationElement::Methods::compareSpecial(thisElement);
   if (compare.isZero())
      return true;
   if (compare.mayBeGreaterZero())
      env.setPositiveOverflow(env.getInformationKind());
   if (compare.mayBeLessZero())
      env.setNegativeOverflow(env.getInformationKind());
   return true;
}

   // methods * and /

bool
Top::applyToTimesAssignInt(const VirtualOperation& operation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   if (env.doesPropagateErrorStates()) {
      if (thisElement.getSizeInBits() != getSizeInBits()
            && thisElement.queryZeroResult().mayBeDifferentZero()) { // char *= Tint
         env.setPositiveOverflow(env.getInformationKind());
         env.setNegativeOverflow(env.getInformationKind());
      };
      return applyToPromotedInteger(operation, thisElement, env);
   };
   Details::IntOperationElement::QueryOperation::CompareSpecialEnvironment
      thisElementCompare = Details::IntOperationElement::Methods::compareSpecial(thisElement);
   if (thisElementCompare.isZero()) {
      env.mergeVerdictExact();
      return true;
   };
   if (thisElementCompare.mayBeOne() || thisElementCompare.mayBeMinusOne())
      env.mergeVerdictExact();
   else
      env.mergeVerdictDegradate();
   if (thisElementCompare.mayBeGreaterOne() || thisElementCompare.mayBeLessMinusOne()) {
      env.setPositiveOverflow(env.getInformationKind());
      env.setNegativeOverflow(env.getInformationKind());
   };
   if (env.getInformationKind().isMay())
      env.presult() = Details::IntOperationElement::Methods::newTop(thisElement);
   else {
      if (thisElementCompare.isOne()
            || (Details::IntOperationElement::Methods::isSigned(thisElement) && thisElementCompare.isMinusOne()))
         env.presult() = Details::IntOperationElement::Methods::newTop(thisElement);
      else if (env.getInformationKind().isSure()) // take 1 and one value aleatory
         return true;
      else { // env.getInformationKind() == IKExact
         PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result
            = Details::IntOperationElement::Methods::newDisjunction(thisElement);
         result->addMay(Details::IntOperationElement::Methods::newTop(thisElement)).addSure(thisElement);
         env.presult() = result;
      };
   };
   return true;
}

bool
Top::applyToTimesAssignFloat(const VirtualOperation&, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   Details::RealOperationElement::QueryOperation::CompareSpecialEnvironment thisCompare
      = Details::RealOperationElement::Methods::compareSpecial(thisElement);
   if (thisCompare.isZero() || thisElement.getApproxKind().isTop()) {
      env.mergeVerdictExact();
      return true;
   };
   if (thisCompare.isContractant())
      env.mergeVerdictExact();
   else
      env.mergeVerdictDegradate();
   if (thisCompare.mayBeExpensive()) {
      env.setPositiveOverflow(env.getInformationKind());
      env.setNegativeOverflow(env.getInformationKind());
   };
   if (thisCompare.mayBeStrictContractant()) {
      env.setPositiveUnderflow(env.getInformationKind());
      env.setNegativeUnderflow(env.getInformationKind());
   };

   if (env.getInformationKind().isMay())
      env.presult() = Details::RealOperationElement::Methods::newTop(thisElement);
   else {
      if (thisCompare.isOne() || thisCompare.isMinusOne())
         env.presult() = Details::RealOperationElement::Methods::newTop(thisElement);
      else if (env.getInformationKind().isSure()) // take 1 and one value aleatory
         return true;
      else { // env.getInformationKind() == IKExact
         PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result
            = Details::IntOperationElement::Methods::newDisjunction(thisElement);
         result->addMay(Details::RealOperationElement::Methods::newTop(thisElement)).addSure(thisElement);
         env.presult() = result;
      };
   };
   return true;
}

bool
Top::applyToDivideAssignFloat(const VirtualOperation& operation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   env.setDivisionByZero(env.getInformationKind());
   return applyToTimesAssignFloat(operation, thisElement, env);
}

bool
Top::applyToPromotedInteger(const VirtualOperation& operation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   PPVirtualElement promotedThis = promoteIntegerInterval();
   TransmitEnvironment transmitEnv(env);
   transmitEnv.setFirstArgument(*promotedThis);
   bool result = thisElement.apply(operation, transmitEnv);
   AssumeCondition(result)
   env.presult() = transmitEnv.presult();
   return true;
}

/* Definition of the apply methods */

bool
Top::applyPromotedInteger(const VirtualOperation& operation, EvaluationEnvironment& env) {
   PPVirtualElement promotedThis = promoteIntegerInterval();
   TransmitEnvironment transmitEnv(env);
   transmitEnv.assignArguments(env);
   bool result = promotedThis->apply(operation, transmitEnv);
   AssumeCondition(result)
   env.presult() = transmitEnv.presult();
   return true;
}

bool
Top::applyPromotedFloat(const VirtualOperation& operation, EvaluationEnvironment& env)
   {  AssumeUnimplemented }

bool
Top::applyCastInteger(const VirtualOperation& operation, EvaluationEnvironment& env) {
   int size = 0, resultSize = 0;
   PPVirtualElement zero = Details::IntOperationElement::Methods::newZero(*this);
   zero = Details::IntOperationElement::Methods::applyAssign(zero, operation, EvaluationParameters(env));
   if ((size = getSizeInBits()) >= (resultSize = zero->getSizeInBits())) {
      env.mergeVerdictExact();
      env.presult() = Details::IntOperationElement::Methods::newTop(*zero);
      if (size > resultSize) { // + floating points and pointers
         env.setPositiveOverflow(env.getInformationKind());
         if (Details::IntOperationElement::Methods::isSigned(*zero))
            env.setNegativeOverflow(env.getInformationKind());
      }
      else if (Details::IntOperationElement::Methods::isSigned(*this)
            != Details::IntOperationElement::Methods::isSigned(*zero)) {
         if (Details::IntOperationElement::Methods::isSigned(*this))
            env.setNegativeOverflow(env.getInformationKind());
         else
            env.setPositiveOverflow(env.getInformationKind());
      };
      return true;
   }
   else // + pointers
      return applyPromotedInteger(operation, env);
}

bool
Top::applyUnimplemented(const VirtualOperation& operation, EvaluationEnvironment& env)
   {  AssumeUnimplemented }

bool
Top::applyPrevAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   env.setNegativeOverflow(env.getInformationKind());
   if (env.doesPropagateErrorStates())
      return applyPromotedInteger(operation, env);
   env.mergeVerdictExact();
   return true;
}

bool
Top::applyNextAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   env.setPositiveOverflow(env.getInformationKind());
   if (env.doesPropagateErrorStates())
      return applyPromotedInteger(operation, env);
   env.mergeVerdictExact();
   return true;
}

bool
Top::applyArithmeticAssignInt(const VirtualOperation& operation, const VirtualElement& source,
      EvaluationEnvironment& env) {
   if (env.doesPropagateErrorStates())
      return applyPromotedInteger(operation, env);
   Details::IntOperationElement::QueryOperation::CompareSpecialEnvironment
      sourceCompare = Details::IntOperationElement::Methods::compareSpecial(source);
   if (sourceCompare.isZero())
      return true;
   if (sourceCompare.mayBeGreaterZero())
      env.setPositiveOverflow(env.getInformationKind());
   if (sourceCompare.mayBeLessZero())
      env.setNegativeOverflow(env.getInformationKind());
   return true;
}

bool
Top::applyArithmeticAssignFloat(const VirtualOperation& operation, const VirtualElement& source,
      EvaluationEnvironment& env) {
   Details::RealOperationElement::QueryOperation::CompareSpecialEnvironment
      sourceCompare = Details::RealOperationElement::Methods::compareSpecial(source);
   if (sourceCompare.isZero())
      return true;
   if (sourceCompare.mayBeGreaterZero())
      env.setPositiveOverflow(env.getInformationKind());
   if (sourceCompare.mayBeLessZero())
      env.setNegativeOverflow(env.getInformationKind());
   return true;
}

bool
Top::applyCompareAnyResult(const VirtualOperation& operation, EvaluationEnvironment& env) {
   env.mergeVerdictExact();
   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
      result = Details::IntOperationElement::Methods::newBooleanDisjunction(*this);
   result->add(Details::IntOperationElement::Methods::newFalse(*this), env.getInformationKind())
      .add(Details::IntOperationElement::Methods::newTrue(*this), env.getInformationKind());
   env.presult() = result;
   return true;
}

bool
Top::applyQueryFloat(const VirtualOperation& operation, EvaluationEnvironment& env) {
   env.mergeVerdictExact();
   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
      result = Details::IntOperationElement::Methods::newBooleanDisjunction(*this);
   result->add(Details::IntOperationElement::Methods::newFalse(*this), env.getInformationKind())
      .add(Details::IntOperationElement::Methods::newTrue(*this), env.getInformationKind());
   env.presult() = result;
   return true;
}

bool
Top::applyTimesAssignInt(const VirtualOperation& operation, const VirtualElement& source,
      EvaluationEnvironment& env) {
   env.mergeVerdictExact();
   Details::IntOperationElement::QueryOperation::CompareSpecialEnvironment
      sourceCompare = Details::IntOperationElement::Methods::compareSpecial(source);
   if (sourceCompare.isZero()) {
      env.presult() = Details::IntOperationElement::Methods::newZero(*this);
      return true;
   };
   if (sourceCompare.mayBeGreaterOne() || sourceCompare.mayBeLessMinusOne()) {
      env.setPositiveOverflow(env.getInformationKind());
      env.setNegativeOverflow(env.getInformationKind());
   };
   if (source.getApproxKind().isTop() || sourceCompare.isOne()
         || (Details::IntOperationElement::Methods::isSigned(source) && sourceCompare.isMinusOne()))
      return true;

   if (env.doesPropagateErrorStates())
      return applyPromotedInteger(operation, env);
   
   env.mergeVerdictDegradate();
   if (env.getInformationKind().isMay())
      return true;

   if (env.getInformationKind().isSure()) // take 1 and one value aleatory
      env.storeResult(source.createSCopy());
   else { // env.getInformationKind().isExact()
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
         result = Details::IntOperationElement::Methods::newDisjunction(source);
      result->addMay(Details::IntOperationElement::Methods::newTop(source)).addSure(source);
      env.presult() = result;
   };
   return true;
}

bool
Top::applyTimesAssignFloat(const VirtualOperation& operation, const VirtualElement& source,
      EvaluationEnvironment& env) {
   env.mergeVerdictExact();
   Details::RealOperationElement::QueryOperation::CompareSpecialEnvironment
      sourceCompare = Details::RealOperationElement::Methods::compareSpecial(source);
   if (sourceCompare.isZero()) {
      env.presult() = Details::RealOperationElement::Methods::newZero(source);
      return true;
   };
   if (sourceCompare.isContractant())
      env.mergeVerdictExact();
   else
      env.mergeVerdictDegradate();
   if (sourceCompare.mayBeExpensive()) {
      env.setPositiveOverflow(env.getInformationKind());
      env.setNegativeOverflow(env.getInformationKind());
   };
   if (sourceCompare.mayBeStrictContractant()) {
      env.setPositiveUnderflow(env.getInformationKind());
      env.setNegativeUnderflow(env.getInformationKind());
   };
   if ((source.getApproxKind().isTop()) || sourceCompare.isOne() || sourceCompare.isMinusOne())
      return true;

   if (env.doesPropagateErrorStates())
      return applyPromotedFloat(operation, env);
   
   env.mergeVerdictDegradate();
   if (env.getInformationKind().isMay())
      return true;

   if (env.getInformationKind().isSure()) // take 1 and one value aleatory
      env.storeResult(source.createSCopy());
   else { // env.getInformationKind().isExact()
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
         result = Details::RealOperationElement::Methods::newDisjunction(source);
      result->addMay(Details::RealOperationElement::Methods::newTop(source)).addSure(source);
      env.presult() = result;
   };
   return true;
}

bool
Top::applyDivideAssignFloat(const VirtualOperation& operation, const VirtualElement& source,
      EvaluationEnvironment& env) {
   env.mergeVerdictExact();
   Details::RealOperationElement::QueryOperation::CompareSpecialEnvironment
      sourceCompare = Details::RealOperationElement::Methods::compareSpecial(source);
   if (sourceCompare.isZero()) {
      env.setEmpty();
      return true;
   };
   if (sourceCompare.mayBeZero())
      env.setDivisionByZero(env.getInformationKind());
   return applyTimesAssignFloat(operation, source, env);
}

bool
Top::applyIdentity(const VirtualOperation& operation, EvaluationEnvironment& env) {
   env.mergeVerdictExact();
   return true;
}

}}} // end of namespace Analyzer::Scalar::Approximate

