/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate native scalar elements
// File      : ConstantBit.template
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of a class of bit elements that relies on independent host analyses.
//

#include "Analyzer/Scalar/Approximate/General/ConstantBit.h"
// #include "Analyzer/Scalar/Approximate/General/ConstantBit.template"
#include "Analyzer/Scalar/Approximate/Top.h"
#include "Analyzer/Scalar/Approximate/MethodTable.template"

namespace Analyzer { namespace Scalar {

namespace Bit { namespace Approximate {

// constraint methods table

#define DefineInitConstraintCompareMethodTable(TypeOperation)                                    \
   mcctCompareTable.elementAt(Operation::T##TypeOperation)                                       \
   .setMethod(&ConstantElement::constraint##TypeOperation);

#define DefineInitConstraintCompareOMethodTable(TypeOperation, TypeMethod)                       \
   mcctCompareTable.elementAt(Operation::T##TypeOperation)                                       \
   .setMethod(&ConstantElement::constraint##TypeMethod);

#define DefineInitConstraintBinaryMethodTable(TypeOperation)                                     \
   mcbtBinaryTable.elementAt(Operation::T##TypeOperation)                                        \
   .setMethod(&ConstantElement::constraint##TypeOperation);

#define DefineInitConstraintOBinaryMethodTable(TypeOperation, TypeEquivOperation)                \
   mcbtBinaryTable.elementAt(Operation::T##TypeOperation)                                        \
   .setMethod(&ConstantElement::constraint##TypeEquivOperation);

#define DefineInitConstraintBinaryMethodTableGeneric(TypeOperation)                              \
   mcbtBinaryTable.elementAt(Operation::T##TypeOperation).setArray(                              \
      new GenericBinaryConstraintMethod(&ConstantElement::constraint##TypeOperation));

#define DefineInitConstraintBinaryMethodTableDouble(TypeOperation)                               \
   mcbtBinaryTable.elementAt(Operation::T##TypeOperation).setArray(                              \
      new GenericAndConstantResultBinaryConstraintMethod(                                        \
         &ConstantElement::constraint##TypeOperation,                                            \
         &ConstantElement::constraint##TypeOperation##ConstantResult));

#define DefineInitConstraintBinaryMethodTableTriple(TypeOperation)                               \
   mcbtBinaryTable.elementAt(Operation::T##TypeOperation).setArray(                              \
      new GenericAndConstantResultSourceBinaryConstraintMethod(                                  \
         &IntElement::constraint##TypeOperation,                                                 \
         &IntElement::constraint##TypeOperation##ConstantResultArgResult,                        \
         &IntElement::constraint##TypeOperation##ConstantResultArgSource));

ConstantElement::MethodConstraintTable::MethodConstraintTable() {
#include "StandardClasses/UndefineNew.h"
   DefineInitConstraintCompareOMethodTable(CompareLess, CompareLessOrGreater)
   DefineInitConstraintCompareOMethodTable(CompareLessOrEqual, CompareLessOrEqualOrGreater)
   DefineInitConstraintCompareMethodTable(CompareEqual)
   DefineInitConstraintCompareMethodTable(CompareDifferent)
   DefineInitConstraintCompareOMethodTable(CompareGreaterOrEqual, CompareLessOrEqualOrGreater)
   DefineInitConstraintCompareOMethodTable(CompareGreater, CompareLessOrGreater)

   DefineInitConstraintOBinaryMethodTable(MinAssign, AndAssign)
   DefineInitConstraintOBinaryMethodTable(MaxAssign, OrAssign)

   DefineInitConstraintBinaryMethodTable(PlusAssign)
   DefineInitConstraintBinaryMethodTable(MinusAssign)
   DefineInitConstraintBinaryMethodTable(OrAssign)
   DefineInitConstraintBinaryMethodTable(AndAssign)
   DefineInitConstraintBinaryMethodTable(ExclusiveOrAssign)
#include "StandardClasses/DefineNew.h"
}
#undef DefineInitConstraintBinaryMethodTableTriple
#undef DefineInitConstraintBinaryMethodTableDouble
#undef DefineInitConstraintBinaryMethodTableGeneric
#undef DefineInitConstraintCompareMethodTable
#undef DefineInitConstraintCompareOMethodTable
#undef DefineInitConstraintBinaryMethodTable
#undef DefineInitConstraintOBinaryMethodTable

ConstantElement::MethodConstraintTable ConstantElement::mctMethodConstraintTable;

// query methods table

ConstantElement::QueryTable::QueryTable() {
#include "StandardClasses/UndefineNew.h"
   FunctionQueryTable* table = &elementAt(VirtualQueryOperation::TCompareSpecial);
   table->setSize(2);
   (*table)[0].setMethod(&ConstantElement::queryCompareSpecial);
   (*table)[1].setMethod(&ConstantElement::queryGuardAll);
  
   table = &elementAt(VirtualQueryOperation::TSimplification);
   table->setSize(1);
   (*table)[0].setMethod(&ConstantElement::querySimplification);
  
   table = &elementAt(VirtualQueryOperation::TDomain);
   table->setSize(QueryOperation::EndOfTypeDomain);

   table = &elementAt(VirtualQueryOperation::TDuplication);
   table->setSize(3);
   (*table)[QueryOperation::TCDClone].setMethod(&VirtualElement::queryCopy);
   (*table)[QueryOperation::TCDSpecialize].setMethod(&VirtualElement::querySpecialize);
   
   table = &elementAt(VirtualQueryOperation::TExtern);
   table->setSize(VirtualQueryOperation::EndOfTypeExtern);
   (*table)[VirtualQueryOperation::TENativeOperation].setMethod(&ConstantElement::queryBitNativeOperation);
#include "StandardClasses/DefineNew.h"
}

/* Implementation of the class ConstantElement */

bool
ConstantElement::applyTop(VirtualElement& topElement, const VirtualOperation& aoperation,
      EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const Operation*>(&aoperation) && dynamic_cast<const Top*>(&topElement))
   const Operation& operation = (const Operation&) aoperation;

   if (operation.getType() == Operation::TCastMultiBit) {
      AssumeCondition(dynamic_cast<const CastMultiBitOperation*>(&operation))
      env.presult() = Scalar::Approximate::Details::IntOperationElement::Methods::newTop(
         *Methods::newMultiBitConstant(topElement, Init().setBitSize(((const CastMultiBitOperation&) operation).getResultSize())));
      return true;
   };
   return false;
}

bool
ConstantElement::constraintTop(VirtualElement& topElement, const VirtualOperation& aoperation,
      const VirtualElement& result, ConstraintEnvironment& env) {
   AssumeCondition(dynamic_cast<const Operation*>(&aoperation))
   // const Operation& operation = (const Operation&) aoperation;
   // return gctMethodConstraintTable.execute(topElement, operation, result, env);
   return false;
}

bool
ConstantElement::applyToTop(const VirtualElement& topElement, VirtualElement& thisElement, const VirtualOperation& operation, EvaluationEnvironment& env) {
   return false;
}

bool
ConstantElement::constraint(const VirtualOperation& operation, const VirtualElement& result,
      ConstraintEnvironment& env) {
   return inherited::constraint(operation, result, env) ? true
      :  Scalar::Approximate::Details::MethodAccess<DomainTraits>::constraintConstant(*this, (const Operation&) operation, result, env);
}

bool
ConstantElement::constraintTo(const VirtualOperation& operation, VirtualElement& source,
      const VirtualElement& result, ConstraintEnvironment& env) {
   return Scalar::Approximate::Details::MethodAccess<DomainTraits>::constraintTo(*this, (const Operation&) operation, source,
         result, env);
}

bool
ConstantElement::intersectWithTo(Approximate::VirtualElement&, EvaluationEnvironment& env) const {
   env.mergeVerdictExact();
   env.setUnstable();
   env.storeResult(createSCopy());
   return true;
}

bool
ConstantElement::mergeWith(const Scalar::VirtualElement& asource, Scalar::EvaluationEnvironment& aenv) {
   if (inherited::mergeWith(asource, aenv))
      return true;

   EvaluationEnvironment& env = (EvaluationEnvironment&) aenv;
   AssumeCondition(dynamic_cast<const ConstantElement*>(&asource))
   const ConstantElement& source = (const ConstantElement&) asource;
   if (fValue != source.fValue) {
      env.presult() = Methods::newTop(*this);
      env.mergeVerdictExact();
      env.setUnstable();
   };
   return true;
}

bool
ConstantElement::contain(const Scalar::VirtualElement& asource, Scalar::EvaluationEnvironment& aenv) const {
   if (inherited::contain(asource, aenv))
      return true;
   const VirtualElement& source = (const VirtualElement&) asource;
   EvaluationEnvironment& env = (EvaluationEnvironment&) aenv;
   if (source.getType().isConstant())
      return Scalar::Approximate::Details::MethodAccess<DomainTraits>::containConstant(*this, source, env);
   return source.containTo(*this, env);
}

bool
ConstantElement::intersectWith(const Approximate::VirtualElement& asource, EvaluationEnvironment& env) {
   if (asource.getType().isConstant()) {
      AssumeCondition(dynamic_cast<const ConstantElement*>(&asource))
      const ConstantElement& source = (const ConstantElement&) asource;
      if (fValue != source.fValue)
         env.setEmpty();
   }
   else {
      assume(asource.contain(*this, env));
      if (env.isNoneApplied())
         env.setEmpty();
      else if (env.isPartialApplied())
         return asource.intersectWithTo(*this, env);
   };
   return true;
}

/***************************************/
/* Implementation of the apply methods */
/***************************************/

bool
ConstantElement::applyCastMultiBit(const VirtualOperation& aoperation, EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const CastMultiBitOperation*>(&aoperation))
   const CastMultiBitOperation& operation = (const CastMultiBitOperation&) aoperation;
   env.presult() = Methods::newMultiBitConstant(*this, Init().setBitSize(operation.getResultSize()).setInitialValue(
         Scalar::Integer::Implementation::IntElement(fValue)));
   return true;
}

bool
ConstantElement::applyPrevAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
  if (!fValue) {
     env.setNegativeOverflow();
     if (env.doesPropagateErrorStates())
        fValue = true;
     else
        env.setEmpty();
  }
  else
     fValue = false;
  return true;
}

bool
ConstantElement::applyNextAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   if (fValue) {
      env.setPositiveOverflow();
      if (env.doesPropagateErrorStates())
         fValue = false;
      else
         env.setEmpty();
   }
   else
      fValue = true;
   return true;
}

bool
ConstantElement::applyPlusAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const ConstantElement*>(&env.getFirstArgument()))
   const ConstantElement& source = (const ConstantElement&) env.getFirstArgument();
   if (fValue && source.fValue) {
      env.setPositiveOverflow();
      if (env.doesPropagateErrorStates())
         fValue = false;
      else
         env.setEmpty();
   }
   else
      fValue |= source.fValue;
   return true;
}

bool
ConstantElement::applyMinusAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const ConstantElement*>(&env.getFirstArgument()))
   const ConstantElement& source = (const ConstantElement&) env.getFirstArgument();
   if (!fValue && source.fValue) {
      env.setNegativeOverflow();
      if (env.doesPropagateErrorStates())
         fValue = true;
      else
         env.setEmpty();
   }
   else
      fValue |= (fValue == source.fValue) ? false : true;
   return true;
}

bool
ConstantElement::applyCompareLess(const VirtualOperation& operation, EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const ConstantElement*>(&env.getFirstArgument()))
   const ConstantElement& source = (const ConstantElement&) env.getFirstArgument();
   ConstantElement* result = createSCopy();
   result->fValue = (fValue < source.fValue);
   env.storeResult(result);
   return true;
}

bool
ConstantElement::applyCompareLessOrEqual(const VirtualOperation& operation, EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const ConstantElement*>(&env.getFirstArgument()))
   const ConstantElement& source = (const ConstantElement&) env.getFirstArgument();
   ConstantElement* result = createSCopy();
   result->fValue = (fValue <= source.fValue);
   env.storeResult(result);
   return true;
}

bool
ConstantElement::applyCompareEqual(const VirtualOperation& operation, EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const ConstantElement*>(&env.getFirstArgument()))
   const ConstantElement& source = (const ConstantElement&) env.getFirstArgument();
   ConstantElement* result = createSCopy();
   result->fValue = (fValue == source.fValue);
   env.storeResult(result);
   return true;
}

bool
ConstantElement::applyCompareDifferent(const VirtualOperation& operation, EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const ConstantElement*>(&env.getFirstArgument()))
   const ConstantElement& source = (const ConstantElement&) env.getFirstArgument();
   ConstantElement* result = createSCopy();
   result->fValue = (fValue != source.fValue);
   env.storeResult(result);
   return true;
}

bool
ConstantElement::applyCompareGreater(const VirtualOperation& operation, EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const ConstantElement*>(&env.getFirstArgument()))
   const ConstantElement& source = (const ConstantElement&) env.getFirstArgument();
   ConstantElement* result = createSCopy();
   result->fValue = (fValue > source.fValue);
   env.storeResult(result);
   return true;
}

bool
ConstantElement::applyCompareGreaterOrEqual(const VirtualOperation& operation, EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const ConstantElement*>(&env.getFirstArgument()))
   const ConstantElement& source = (const ConstantElement&) env.getFirstArgument();
   ConstantElement* result = createSCopy();
   result->fValue = (fValue >= source.fValue);
   env.storeResult(result);
   return true;
}

bool
ConstantElement::applyMinAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const ConstantElement*>(&env.getFirstArgument()))
   const ConstantElement& source = (const ConstantElement&) env.getFirstArgument();
   fValue = fValue && source.fValue;
   return true;
}

bool
ConstantElement::applyMaxAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const ConstantElement*>(&env.getFirstArgument()))
   const ConstantElement& source = (const ConstantElement&) env.getFirstArgument();
   fValue = fValue || source.fValue;
   return true;
}

bool
ConstantElement::applyOrAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const ConstantElement*>(&env.getFirstArgument()))
   const ConstantElement& source = (const ConstantElement&) env.getFirstArgument();
   fValue |= source.fValue;
   return true;
}

bool
ConstantElement::applyAndAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const ConstantElement*>(&env.getFirstArgument()))
   const ConstantElement& source = (const ConstantElement&) env.getFirstArgument();
   fValue &= source.fValue;
   return true;
}

bool
ConstantElement::applyExclusiveOrAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const ConstantElement*>(&env.getFirstArgument()))
   const ConstantElement& source = (const ConstantElement&) env.getFirstArgument();
   fValue ^= source.fValue;
   return true;
}

bool
ConstantElement::applyNegateAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   fValue = !fValue;
   return true;
}

/********************************************/
/* Implementation of the constraint methods */
/********************************************/

bool
ConstantElement::constraintPlusAssign(const VirtualOperation&, VirtualElement& source,
      const VirtualElement& result, Argument arg, ConstraintEnvironment& env) {
   if (!fValue) {
      IntersectEnvironment intersectionEnv(env, arg.queryInverse());
      source.intersectWith(result, intersectionEnv);
   }
   else {
      QueryOperation::CompareSpecialEnvironment compare = Methods::compareSpecial(result);
      if (env.doesStopErrorStates() || !env.isPositiveOverflow()) {
         if (!compare.mayBeTrue()) {
            env.setUnstable();
            env.setEmpty();
         }
         else {
            PNT::TPassPointer<ConstantElement, PPAbstractElement> intersectElement(*this);
            intersectElement->fValue = false;
            IntersectEnvironment intersectionEnv(env, arg.queryInverse());
            source.intersectWith(*intersectElement, intersectionEnv);
         };
      }
      else {
         if (compare.mayBeTrue() && compare.mayBeFalse())
            return true;
         else if (compare.isTrue()) {
            PNT::TPassPointer<ConstantElement, PPAbstractElement> intersectElement(*this);
            intersectElement->fValue = false;
            IntersectEnvironment intersectionEnv(env, arg.queryInverse());
            source.intersectWith(*intersectElement, intersectionEnv);
         }
         else if (compare.isFalse()) {
            PNT::TPassPointer<ConstantElement, PPAbstractElement> intersectElement(*this);
            intersectElement->fValue = true;
            IntersectEnvironment intersectionEnv(env, arg.queryInverse());
            source.intersectWith(*intersectElement, intersectionEnv);
         }
         else {
            env.setUnstable();
            env.setEmpty();
         };
      };
   };
   return true;
}

bool
ConstantElement::constraintMinusAssign(const VirtualOperation&, VirtualElement& source,
      const VirtualElement& result, Argument arg, ConstraintEnvironment& env) {
   if (!fValue) {
      if (arg.isFst()) {
         IntersectEnvironment intersectionEnv(env, arg.queryInverse());
         source.intersectWith(result, intersectionEnv);
      }
      else {
         QueryOperation::CompareSpecialEnvironment compare = Methods::compareSpecial(result);
         if (compare.mayBeTrue() && compare.mayBeFalse())
            return true;
         else if (compare.isTrue()) {
            PNT::TPassPointer<ConstantElement, PPAbstractElement> intersectElement(*this);
            intersectElement->fValue = false;
            IntersectEnvironment intersectionEnv(env, arg.queryInverse());
            source.intersectWith(*intersectElement, intersectionEnv);
         }
         else if (compare.isFalse()) {
            PNT::TPassPointer<ConstantElement, PPAbstractElement> intersectElement(*this);
            intersectElement->fValue = true;
            IntersectEnvironment intersectionEnv(env, arg.queryInverse());
            source.intersectWith(*intersectElement, intersectionEnv);
         }
         else {
            env.setUnstable();
            env.setEmpty();
         };
      };
   }
   else {
      if (arg.isResult() && (env.doesStopErrorStates() || !env.isNegativeOverflow())) {
         QueryOperation::CompareSpecialEnvironment compare = Methods::compareSpecial(result);
         if (!compare.mayBeFalse()) {
            env.setUnstable();
            env.setEmpty();
            return true;
         };
         if (!compare.isTrue()) {
            IntersectEnvironment intersectionEnv(env, arg.queryInverse());
            source.intersectWith(*this, intersectionEnv);
            return true;
         };
      };
      IntersectEnvironment intersectionEnv(env, arg.queryInverse());
      source.intersectWith(result, intersectionEnv);
   };
   return true;
}

bool
ConstantElement::constraintCompareLessOrGreater(const VirtualOperation& operation, VirtualElement& source,
      Argument arg, ConstraintEnvironment& env) {
   bool isLess = (((const Operation&) operation).getType() == Operation::TCompareLess)
      ?  arg.isResult() : arg.isFst();
   if (fValue ? isLess : !isLess) {
      env.setEmpty();
      env.setUnstable();
   }
   else {
      PNT::TPassPointer<ConstantElement, PPAbstractElement> intersectElement(*this);
      intersectElement->fValue = isLess;
      IntersectEnvironment intersectionEnv(env, arg.queryInverse());
      source.intersectWith(*intersectElement, intersectionEnv);
   };
   return true;
}

bool
ConstantElement::constraintCompareLessOrEqualOrGreater(const VirtualOperation& operation, VirtualElement& source,
      Argument arg, ConstraintEnvironment& env) {
   bool isLess = (((const Operation&) operation).getType() == Operation::TCompareLessOrEqual)
      ?  arg.isResult() : arg.isFst();
   if (fValue ? isLess : !isLess) {
      IntersectEnvironment intersectionEnv(env, arg.queryInverse());
      source.intersectWith(*this, intersectionEnv);
   };
   return true;
}

bool
ConstantElement::constraintCompareEqual(const VirtualOperation&, VirtualElement& source,
      Argument arg, ConstraintEnvironment& env) {
   IntersectEnvironment intersectionEnv(env, arg.queryInverse());
   source.intersectWith(*this, intersectionEnv);
   return true;
}

bool
ConstantElement::constraintCompareDifferent(const VirtualOperation&, VirtualElement& source,
      Argument arg, ConstraintEnvironment& env) {
   PNT::TPassPointer<ConstantElement, PPAbstractElement> intersectElement(*this);
   intersectElement->fValue = !fValue;
   IntersectEnvironment intersectionEnv(env, arg.queryInverse());
   source.intersectWith(*intersectElement, intersectionEnv);
   return true;
}

bool
ConstantElement::constraintAndAssign(const VirtualOperation&, VirtualElement& source,
      const VirtualElement& result, Argument arg, ConstraintEnvironment& env) {
   if (!fValue) {
      QueryOperation::CompareSpecialEnvironment compare = Methods::compareSpecial(result);
      if (!compare.mayBeFalse())
         env.setEmpty();
   }
   else {
      IntersectEnvironment intersectionEnv(env, arg.queryInverse());
      source.intersectWith(result, intersectionEnv);
   };
   return true;
}

bool
ConstantElement::constraintOrAssign(const VirtualOperation&, VirtualElement& source,
      const VirtualElement& result, Argument arg, ConstraintEnvironment& env) {
   if (fValue) {
      QueryOperation::CompareSpecialEnvironment compare = Methods::compareSpecial(result);
      if (!compare.mayBeTrue())
         env.setEmpty();
   }
   else {
      IntersectEnvironment intersectionEnv(env, arg.queryInverse());
      source.intersectWith(result, intersectionEnv);
   };
   return true;
}

bool
ConstantElement::constraintExclusiveOrAssign(const VirtualOperation&, VirtualElement& source,
      const VirtualElement& result, Argument arg, ConstraintEnvironment& env) {
   if (!fValue) {
      IntersectEnvironment intersectionEnv(env, arg.queryInverse());
      source.intersectWith(result, intersectionEnv);
   }
   else {
      QueryOperation::CompareSpecialEnvironment compare = Methods::compareSpecial(result);
      if (compare.mayBeTrue() && compare.mayBeFalse())
         return true;
      else if (compare.isTrue()) {
         PNT::TPassPointer<ConstantElement, PPAbstractElement> intersectElement(*this);
         intersectElement->fValue = false;
         IntersectEnvironment intersectionEnv(env, arg.queryInverse());
         source.intersectWith(*intersectElement, intersectionEnv);
      }
      else if (compare.isFalse()) {
         PNT::TPassPointer<ConstantElement, PPAbstractElement> intersectElement(*this);
         intersectElement->fValue = true;
         IntersectEnvironment intersectionEnv(env, arg.queryInverse());
         source.intersectWith(*intersectElement, intersectionEnv);
      }
      else {
         env.setUnstable();
         env.setEmpty();
      };
   };
   return true;
}

/***************************************/
/* Implementation of the query methods */
/***************************************/

bool
ConstantElement::queryCompareSpecial(const VirtualElement& aelement, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const QueryOperation::CompareSpecialEnvironment*>(&aenv)
     && dynamic_cast<const ConstantElement*>(&aelement))
   const ConstantElement& thisElement = (const ConstantElement&) aelement;
   QueryOperation::CompareSpecialEnvironment& env = (QueryOperation::CompareSpecialEnvironment&) aenv;
   if (thisElement.fValue)
      env.setTrue();
   else
      env.setFalse();
   return true;
}

bool
ConstantElement::querySimplification(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env) {
   return true;
}

}}}} // end of namespace Analyzer::Scalar::Bit::Approximate

/*
#include "Analyzer/Interpretation/Scalar/Approximate/ConstantBit.template"

namespace Analyzer { namespace Scalar { namespace Bit { namespace Approximate {

class CastRingTraits {
  public:
   static VirtualElement* createIntElement(const VirtualElement::Init& init);
   static VirtualElement* createBitElement(const VirtualElement::Init& init);
   static VirtualElement* createMultiBitElement(const VirtualElement::Init& init);
};

template class Bit::TCloseConstantElement<CastRingTraits>;

VirtualElement*
CastRingTraits::createIntElement(const VirtualElement::Init& init)
   {  return new Integer::IntElement(init); }
// {  return new Approximate::Details::TCloseCastIntegerBasedElement<Integer::IntElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createMultiBitElement(const VirtualElement::Init& init)
   {  return nullptr; }
   
VirtualElement*
CastRingTraits::createBitElement(const VirtualElement::Init& init)
   {  return new Bit::TCloseConstantElement<CastRingTraits>(init); }


}}}} // end of namespace Analyzer::Scalar::Bit::Approximate

*/
