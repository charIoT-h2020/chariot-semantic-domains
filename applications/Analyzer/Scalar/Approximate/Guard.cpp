/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : Guard.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of a class of guarded elements.
//

#include "Analyzer/Scalar/Approximate/Guard.h"
#include "Analyzer/Scalar/Approximate/Top.h"
// #include "Analyzer/Scalar/Scalar.hpp"

namespace Analyzer {}

#include "Collection/Collection.template"

namespace Analyzer { namespace Scalar { namespace Approximate {

/******************************************************/
/* Definition of dispatch table for the query methods */
/******************************************************/

Guard::QueryTable::QueryTable() {
#include "StandardClasses/UndefineNew.h"
   FunctionQueryTable* table = &elementAt(VirtualQueryOperation::TCompareSpecial);
   table->setSize(2);
   (*table)[0].setMethod(&Guard::queryGuard);
   (*table)[1].setMethod(&Guard::queryCompareSpecial);
   
   table = &elementAt(VirtualQueryOperation::TSimplification);
   table->setSize(Details::IntOperationElement::QueryOperation::EndOfTypeSimplification);
   // use a double function pointer depending of the type of Top (integer type or not)
   (*table)[Details::IntOperationElement::QueryOperation::TSUndefined].setMethod(&Guard::queryIdentity);
   (*table)[Details::IntOperationElement::QueryOperation::TSConstant].setMethod(&Top::queryFailSimplification);
   (*table)[Details::IntOperationElement::QueryOperation::TSInterval].setMethod(&Top::queryFailSimplification);
   (*table)[Details::IntOperationElement::QueryOperation::TSConstantDisjunction].setMethod(&Top::queryFailSimplification);

   table = &elementAt(VirtualQueryOperation::TDuplication);
   table->setSize(3);
   (*table)[QueryOperation::TCDClone].setMethod(&Guard::queryCopy);
   (*table)[QueryOperation::TCDSpecialize].setMethod(&Guard::querySpecialize);
#include "StandardClasses/DefineNew.h"
}

Guard::QueryTable Guard::mqtMethodQueryTable;

bool
Guard::query(const VirtualQueryOperation& aoperation, VirtualQueryOperation::Environment& env) const {
   if (inherited::query(aoperation, env))
      return true;
   const VirtualQueryOperation& operation = (const VirtualQueryOperation&) aoperation;
   if (operation.isOperation() || operation.isDomain() || operation.isExtern()) {
      AssumeCondition(pfqtQueryOperationTable && pfqtQueryDomainTable && pfqtQueryExternTable)
      (operation.isOperation() ? pfqtQueryOperationTable
         : (operation.isDomain() ? pfqtQueryDomainTable : pfqtQueryExternTable))->execute(*this, operation, env);
   }
   else
      assume(mqtMethodQueryTable[operation.getType()].execute(*this, operation, env));
   return true;
}

bool
Guard::applyTo(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const
   {  AssumeCondition(isValid() && getApproxKind().compareApply(source.getApproxKind()) >= CREqual)
      return applyToNative(operation, source, env);
   }

bool
Guard::mergeWithTo(VirtualElement& source, EvaluationEnvironment& env) const {
   PPVirtualElement merge;
   if (ppveThen.isValid())
      merge.setElement(*ppveThen);
   if (ppveElse.isValid()) {
      TransmitEnvironment transmitEnv(env);
      merge->mergeWith(*ppveElse, transmitEnv);
      if (transmitEnv.hasResult())
         merge = transmitEnv.presult();
   };
   if (!merge.isValid()) {
      env.setEmpty();
      return true;
   };
   return source.mergeWith(*merge, env);
}

bool
Guard::containTo(const VirtualElement& source, EvaluationEnvironment& env) const {
   PPVirtualElement merge;
   if (ppveThen.isValid())
      merge.setElement(*ppveThen);
   if (ppveElse.isValid()) {
      TransmitEnvironment transmitEnv(env);
      merge->mergeWith(*ppveElse, transmitEnv);
      if (transmitEnv.hasResult())
         merge = transmitEnv.presult();
   };
   if (!merge.isValid()) {
      env.setEmpty();
      return true;
   };
   return source.contain(*merge, env);
}

bool
Guard::intersectWithTo(VirtualElement& source, EvaluationEnvironment& env) const {
   PPVirtualElement merge;
   if (ppveThen.isValid())
      merge.setElement(*ppveThen);
   if (ppveElse.isValid()) {
      TransmitEnvironment transmitEnv(env);
      merge->mergeWith(*ppveElse, transmitEnv);
      if (transmitEnv.hasResult())
         merge = transmitEnv.presult();
   };
   if (!merge.isValid()) {
      env.setEmpty();
      return true;
   };
   return source.intersectWith(*merge, env);
}

bool
Guard::constraintTo(const VirtualOperation& operation, VirtualElement& source,
      const VirtualElement& result, ConstraintEnvironment& env) {
   PPVirtualElement merge;
   if (ppveThen.isValid())
      merge.setElement(*ppveThen);
   if (ppveElse.isValid()) {
      TransmitEnvironment transmitEnv(env);
      merge->mergeWith(*ppveElse, transmitEnv);
      if (transmitEnv.hasResult())
         merge = transmitEnv.presult();
   };
   if (!merge.isValid()) {
      env.setEmpty();
      return true;
   };
   env.absorbFirstArgument(merge.extractElement());
   return source.constraint(operation, result, env);
}

AbstractElement::ZeroResult
Guard::queryZeroResult() const {
   PPVirtualElement merge;
   if (ppveThen.isValid())
      merge.setElement(*ppveThen);
   if (ppveElse.isValid()) {
      EvaluationEnvironment env(EPMayStopErrorStates);
      merge->mergeWith(*ppveElse, env);
      if (env.hasResult())
         merge = env.presult();
   };
   if (!merge.isValid())
      return ZeroResult();
   return merge->queryZeroResult();
}

bool
Guard::apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env)
   {  AssumeCondition(isValid()) 
      return (VirtualElement::apply(operation, env)) ? true : applyNative(operation, (EvaluationEnvironment&) env);
   }

bool
Guard::mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) {
   if (VirtualElement::mergeWith(source, env))
      return true;
   PPVirtualElement merge;
   if (ppveThen.isValid())
      merge.setElement(*ppveThen);
   if (ppveElse.isValid()) {
      TransmitEnvironment transmitEnv((EvaluationEnvironment&) env);
      merge->mergeWith(*ppveElse, transmitEnv);
      if (transmitEnv.hasResult())
         merge = transmitEnv.presult();
   };
   if (!merge.isValid()) {
      env.setResult(source);
      return true;
   };
   return merge->mergeWith(source, env);
}

bool
Guard::contain(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) const {
   if (VirtualElement::contain(source, env))
      return true;
   PPVirtualElement merge;
   if (ppveThen.isValid())
      merge.setElement(*ppveThen);
   if (ppveElse.isValid()) {
      TransmitEnvironment transmitEnv((EvaluationEnvironment&) env);
      merge->mergeWith(*ppveElse, transmitEnv);
      if (transmitEnv.hasResult())
         merge = transmitEnv.presult();
   };
   if (!merge.isValid()) {
      env.setEmpty();
      return true;
   }
   return merge->contain(source, env);
}

bool
Guard::intersectWith(const VirtualElement& source, EvaluationEnvironment& env) {
   if (VirtualElement::intersectWith(source, env))
      return true;
   PPVirtualElement merge;
   if (ppveThen.isValid())
      merge.setElement(*ppveThen);
   if (ppveElse.isValid()) {
      TransmitEnvironment transmitEnv(env);
      merge->mergeWith(*ppveElse, transmitEnv);
      if (transmitEnv.hasResult())
         merge = transmitEnv.presult();
   };
   if (!merge.isValid()) {
      env.setEmpty();
      return true;
   };
   return merge->intersectWith(source, env);
}

bool
Guard::constraint(const VirtualOperation& operation, const VirtualElement& result,
      ConstraintEnvironment& env) {
   if (VirtualElement::constraint(operation, result, env))
      return true;
   PPVirtualElement merge;
   if (ppveThen.isValid())
      merge.setElement(*ppveThen);
   if (ppveElse.isValid()) {
      TransmitEnvironment transmitEnv(env);
      merge->mergeWith(*ppveElse, transmitEnv);
      if (transmitEnv.hasResult())
         merge = transmitEnv.presult();
   };
   if (!merge.isValid()) {
      env.setEmpty();
      return true;
   };
   return merge->constraint(operation, result, env);
}

bool
Guard::applyToNative(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const {
   PPVirtualElement merge;
   if (ppveThen.isValid())
      merge.setElement(*ppveThen);
   if (ppveElse.isValid()) {
      TransmitEnvironment transmitEnv(env);
      merge->mergeWith(*ppveElse, transmitEnv);
      if (transmitEnv.hasResult())
         merge = transmitEnv.presult();
   };
   if (!merge.isValid()) {
      env.setEmpty();
      return true;
   };
   env.setFirstArgument(*merge);
   return source.apply(operation, env);
}

bool
Guard::applyNative(const VirtualOperation& operation, EvaluationEnvironment& env) {
   PPVirtualElement merge;
   if (ppveThen.isValid())
      merge.setElement(*ppveThen);
   if (ppveElse.isValid()) {
      TransmitEnvironment transmitEnv(env);
      merge->mergeWith(*ppveElse, transmitEnv);
      if (transmitEnv.hasResult())
         merge = transmitEnv.presult();
   };
   if (!merge.isValid()) {
      env.setEmpty();
      return true;
   };
   return merge->apply(operation, env);
}

bool
Guard::queryCompareSpecial(const VirtualElement& athisElement,
      const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) {
   if (athisElement.getApproxKind().isGuard()) {
      AssumeCondition(dynamic_cast<const Guard*>(&athisElement))
      const Guard& thisElement = (const Guard&) athisElement;
      PPVirtualElement merge;
      if (thisElement.ppveThen.isValid())
         merge.setElement(*thisElement.ppveThen);
      if (thisElement.ppveElse.isValid()) {
         EvaluationEnvironment mergeEnv(EPMayStopErrorStates);
         merge->mergeWith(*thisElement.ppveElse, mergeEnv);
         if (mergeEnv.hasResult())
            merge = mergeEnv.presult();
      };
      if (!merge.isValid())
         return true;
      return merge->query(operation, env);
   };
   return athisElement.query(operation, env);
}

bool
Guard::queryCopy(const VirtualElement& athisElement, const VirtualQueryOperation& operation,
      VirtualQueryOperation::Environment& env) {
   if (athisElement.getApproxKind().isGuard()) {
      AssumeCondition(dynamic_cast<const Guard*>(&athisElement))
      const Guard& thisElement = (const Guard&) athisElement;
      PPVirtualElement merge;
      if (thisElement.ppveThen.isValid())
         merge.setElement(*thisElement.ppveThen);
      if (thisElement.ppveElse.isValid()) {
         EvaluationEnvironment mergeEnv(EPMayStopErrorStates);
         merge->mergeWith(*thisElement.ppveElse, mergeEnv);
         if (mergeEnv.hasResult())
            merge = mergeEnv.presult();
      };
      if (!merge.isValid())
         return true;
      return merge->query(operation, env);
   };
   return athisElement.query(operation, env);
}

bool
Guard::querySpecialize(const VirtualElement& athisElement, const VirtualQueryOperation& operation,
      VirtualQueryOperation::Environment& env) {
   if (athisElement.getApproxKind().isGuard()) {
      AssumeCondition(dynamic_cast<const Guard*>(&athisElement))
      const Guard& thisElement = (const Guard&) athisElement;
      PPVirtualElement merge;
      if (thisElement.ppveThen.isValid())
         merge.setElement(*thisElement.ppveThen);
      if (thisElement.ppveElse.isValid()) {
         EvaluationEnvironment mergeEnv(EPMayStopErrorStates);
         merge->mergeWith(*thisElement.ppveElse, mergeEnv);
         if (mergeEnv.hasResult())
            merge = mergeEnv.presult();
      };
      if (!merge.isValid())
         return true;
      return merge->query(operation, env);
   };
   return athisElement.query(operation, env);
}

bool
Guard::queryGuard(const VirtualElement& asource, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const Guard*>(&asource) && dynamic_cast<const VirtualQueryOperation::GuardEnvironment*>(&aenv))
   const Guard& source = (const Guard&) asource;
   VirtualQueryOperation::GuardEnvironment& env = (VirtualQueryOperation::GuardEnvironment&) aenv;
   if (source.ppveThen.isValid()) {
      if (source.ppveElse.isValid())
         return source.ppveCondition->query(operation, env);
      else
         env.presult().fullAssign(source.ppveCondition);
   }
   else if (source.ppveElse.isValid())
      env.presult() = VirtualElement::Methods::apply(*source.ppveCondition, Bit::Operation().setNegate(), EvaluationEnvironment::Init());
   else
      env.presult() = Bit::Approximate::Details::BitElement::Methods::newFalse(*source.ppveCondition);
   return true;
}

bool
Guard::querySimplification(const VirtualElement& athisElement, const VirtualQueryOperation& operation,
      VirtualQueryOperation::Environment& env) {
   if (athisElement.getApproxKind().isGuard()) {
      AssumeCondition(dynamic_cast<const Guard*>(&athisElement))
      const Guard& thisElement = (const Guard&) athisElement;
      PPVirtualElement merge;
      if (thisElement.ppveThen.isValid())
         merge.setElement(*thisElement.ppveThen);
      if (thisElement.ppveElse.isValid()) {
         EvaluationEnvironment mergeEnv(EPMayStopErrorStates);
         merge->mergeWith(*thisElement.ppveElse, mergeEnv);
         if (mergeEnv.hasResult())
            merge = mergeEnv.presult();
      };
      if (!merge.isValid())
         return true;
      return merge->query(operation, env);
   };
   return athisElement.query(operation, env);
}

bool
Guard::querySimplificationAsInterval(const VirtualElement& athisElement, const VirtualQueryOperation& operation,
      VirtualQueryOperation::Environment& env) {
   if (athisElement.getApproxKind().isGuard()) {
      AssumeCondition(dynamic_cast<const Guard*>(&athisElement))
      const Guard& thisElement = (const Guard&) athisElement;
      PPVirtualElement merge;
      if (thisElement.ppveThen.isValid())
         merge.setElement(*thisElement.ppveThen);
      if (thisElement.ppveElse.isValid()) {
         EvaluationEnvironment mergeEnv(EPMayStopErrorStates);
         merge->mergeWith(*thisElement.ppveElse, mergeEnv);
         if (mergeEnv.hasResult())
            merge = mergeEnv.presult();
      };
      if (!merge.isValid())
         return true;
      return merge->query(operation, env);
   };
   return athisElement.query(operation, env);
}

bool
Guard::querySimplificationAsConstantDisjunction(const VirtualElement& athisElement, const VirtualQueryOperation& operation,
      VirtualQueryOperation::Environment& env) {
   if (athisElement.getApproxKind().isGuard()) {
      AssumeCondition(dynamic_cast<const Guard*>(&athisElement))
      const Guard& thisElement = (const Guard&) athisElement;
      PPVirtualElement merge;
      if (thisElement.ppveThen.isValid())
         merge.setElement(*thisElement.ppveThen);
      if (thisElement.ppveElse.isValid()) {
         EvaluationEnvironment mergeEnv(EPMayStopErrorStates);
         merge->mergeWith(*thisElement.ppveElse, mergeEnv);
         if (mergeEnv.hasResult())
            merge = mergeEnv.presult();
      };
      if (!merge.isValid())
         return true;
      return merge->query(operation, env);
   };
   return athisElement.query(operation, env);
}

}}} // end of namespace Analyzer::Scalar::Approximate

