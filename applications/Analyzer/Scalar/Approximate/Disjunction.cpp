/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : Disjunction.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of a class of disjunctions.
//

#include "Analyzer/Scalar/Approximate/Disjunction.h"
#include "Analyzer/Scalar/Approximate/Top.h"
// #include "Analyzer/Scalar/Scalar.hpp"

namespace Analyzer {}

#include "Collection/Collection.template"

namespace Analyzer { namespace Scalar { namespace Approximate {

void
Disjunction::ElementsList::mergeNew(VirtualElement* newElement) {
   EvaluationEnvironment env = EvaluationEnvironment(EvaluationParameters());
   AssumeAllocation(newElement)
   PPVirtualElement element(newElement, PNT::Pointer::Init());
   ElementsList::Cursor cursor(*this);
   bool isNext = false;
   if (cursor.setToFirst()) {
      do {
         isNext = false;
         if (Methods::contain(cursor.elementAt(), *newElement, EvaluationParameters(env)))
            return;
         if (Methods::contain(*newElement, cursor.elementAt(), EvaluationParameters(env))) {
            cursor.setToNext();
            freePrevious(cursor);
            isNext = true;
         };
      } while (isNext ? cursor.isValid() : cursor.setToNext());
   };
   insertNewAtEnd(element.extractElement());
}

/***********************************************/
/* Definition of the dispatch of apply methods */
/***********************************************/

/* Definition of the class Disjunction::AddElement */

class Disjunction::AddElement {
  private:
   ElementsList& elReceiver;
   
  public:
   AddElement(ElementsList& receiver) : elReceiver(receiver) {}
   bool operator()(const VirtualElement& source) const
      {  elReceiver.mergeNew(source.createSCopy()); return true; }
};

/* Definition of the class Disjunction::MergeApplyApplication */

class Disjunction::MergeApplyApplication {
  private:
   PNT::AutoPointer<VirtualOperation> apvoOperation;
   EvaluationEnvironment& seeEnv;
   PPVirtualElement ppseResult;
   InformationKind ikInformation;

  public:
   MergeApplyApplication(const VirtualOperation& operation, EvaluationEnvironment& env,
         InformationKind information)
      :  apvoOperation(operation), seeEnv(env), ikInformation(information) {}

   bool operator()(VirtualElement& element);
   PPVirtualElement extractResult() { return ppseResult; }
};

bool
Disjunction::MergeApplyApplication::operator()(VirtualElement& element) {
   TransmitEnvironment
      localEnv(seeEnv, EvaluationParameters(seeEnv).setInformationKind(ikInformation));
   localEnv.assignArguments(seeEnv);

   bool applicationResult = element.apply(*apvoOperation, localEnv);
   AssumeCondition(applicationResult)
   if (localEnv.isEmpty()) {
      localEnv.clear();
      return true;
   };
   localEnv.merge();

   EvaluationEnvironment::LatticeCreation lattice = seeEnv.getLatticeCreation();
   seeEnv.setDisjunctionLatticeCreation();
   ppseResult = Methods::merge(ppseResult,
         localEnv.hasResult() ? localEnv.getResult() : element, seeEnv);
   seeEnv.setLatticeCreation(lattice);

   localEnv.clear();
   return true;
}

/* Definition of the class Disjunction::MergeApplyToApplication */

class Disjunction::MergeApplyToApplication {
  private:
   PNT::AutoPointer<VirtualOperation> apvoOperation;
   VirtualElement& seArgument;
   EvaluationEnvironment& seeEnv;
   PPVirtualElement ppseResult;
   InformationKind ikInformation;

  public:
   MergeApplyToApplication(const VirtualOperation& operation, VirtualElement& argument,
         EvaluationEnvironment& env, InformationKind information)
      :  apvoOperation(operation), seArgument(argument), seeEnv(env),
         ikInformation(information) {}

   bool operator()(const VirtualElement& element);
   PPVirtualElement extractResult() { return ppseResult; }
};

bool
Disjunction::MergeApplyToApplication::operator()(const VirtualElement& element) {
   TransmitEnvironment
      localEnv(seeEnv, EvaluationParameters(seeEnv).setInformationKind(ikInformation));
   localEnv.assignArguments(seeEnv);
   localEnv.setFirstArgument(element);
   PPVirtualElement argument;
   if (!apvoOperation->isConst())
      argument.setElement(seArgument);

   bool applicationResult = (argument.isValid() ? *argument : seArgument).apply(*apvoOperation, localEnv);
   AssumeCondition(applicationResult)
   if (localEnv.isEmpty()) {
      localEnv.clear();
      return true;
   };
   if (localEnv.hasResult())
      argument = localEnv.presult();
   localEnv.merge().clear();

   EvaluationEnvironment::LatticeCreation lattice = seeEnv.getLatticeCreation();
   seeEnv.setDisjunctionLatticeCreation();
   ppseResult = Methods::merge(ppseResult, (argument.isValid() ? *argument : seArgument), seeEnv);
   seeEnv.setLatticeCreation(lattice);

   return true;
}

/**********************************************************/
/* Definition of the dispatch of the intersection methods */
/**********************************************************/

/* Definition of the class Disjunction::IntersectApplication */

class Disjunction::IntersectApplication {
  private:
   const VirtualElement& seArgument;
   EvaluationEnvironment& seeEnv;
   PPVirtualElement ppseResult;
   InformationKind ikInformation;
   
  public:
   IntersectApplication(const VirtualElement& argument, EvaluationEnvironment& env,
         InformationKind information)
      :  seArgument(argument), seeEnv(env), ikInformation(information) {}

   bool operator()(VirtualElement& thisElement);
   PPVirtualElement extractResult() { return ppseResult; }
};

bool
Disjunction::IntersectApplication::operator()(VirtualElement& thisElement) {
   TransmitEnvironment
      localEnv(seeEnv, EvaluationParameters(seeEnv).setInformationKind(ikInformation));

   bool applicationResult = thisElement.intersectWith(seArgument, localEnv);
   AssumeCondition(applicationResult)
   if (localEnv.isEmpty()) {
      localEnv.clear();
      return true;
   };
   PPVirtualElement thisArgument = localEnv.presult();
   localEnv.merge().clear();

   EvaluationEnvironment::LatticeCreation lattice = seeEnv.getLatticeCreation();
   seeEnv.setDisjunctionLatticeCreation();
   ppseResult = Methods::merge(ppseResult, thisArgument.isValid() ? *thisArgument : thisElement, seeEnv);
   seeEnv.setLatticeCreation(lattice);

   return true;
}

/* Definition of the class Disjunction::IntersectToApplication */

class Disjunction::IntersectToApplication {
  private:
   VirtualElement& seThis;
   EvaluationEnvironment& seeEnv;
   PPVirtualElement ppseResult;
   InformationKind ikInformation;
   
  public:
   IntersectToApplication(VirtualElement& thisElement, EvaluationEnvironment& env,
         InformationKind information)
      :  seThis(thisElement), seeEnv(env), ikInformation(information) {}

   bool operator()(const VirtualElement& source);
   PPVirtualElement extractResult() { return ppseResult; }
};

bool
Disjunction::IntersectToApplication::operator()(const VirtualElement& source) {
   TransmitEnvironment
      localEnv(seeEnv, EvaluationParameters(seeEnv).setInformationKind(ikInformation));

   PPVirtualElement thisArgument(seThis);
   bool applicationResult = thisArgument->intersectWith(source, localEnv);
   AssumeCondition(applicationResult)
   if (localEnv.isEmpty()) {
      localEnv.clear();
      return true;
   };
   if (localEnv.hasResult())
      thisArgument = localEnv.presult();
   localEnv.merge().clear();

   EvaluationEnvironment::LatticeCreation lattice = seeEnv.getLatticeCreation();
   seeEnv.setDisjunctionLatticeCreation();
   if (!ppseResult.isValid())
      ppseResult = thisArgument;
   else
      ppseResult = Methods::merge(ppseResult, *thisArgument, seeEnv);
   seeEnv.setLatticeCreation(lattice);

   return true;
}

/*************************************************/
/* Definition of the dispatch of contain methods */
/*************************************************/

/* Definition of the class Disjunction::ContainApplication */

class Disjunction::ContainApplication {
  private:
   const VirtualElement& seArgument;
   EvaluationEnvironment& seeEnv;
   enum Result { RNone, RPartial, RTotal };
   Result rResult;
   InformationKind ikInformation;
   
  public:
   ContainApplication(const VirtualElement& argument, EvaluationEnvironment& env,
         InformationKind information)
      :  seArgument(argument), seeEnv(env), rResult(RNone), ikInformation(information) {}

   bool operator()(VirtualElement& thisElement);
   bool isTotal() const { return rResult == RTotal; }
   bool isPartial() const { return rResult == RPartial; }
   bool isNone() const { return rResult == RNone; }
};

bool
Disjunction::ContainApplication::operator()(VirtualElement& thisElement) {
   EvaluationParameters params(seeEnv);
   params.setInformationKind(ikInformation);
   AssumeCondition(rResult != RTotal)
   EvaluationEnvironment env(params);
   assume(thisElement.contain(seArgument, env));
   if (env.isTotalApplied())
      rResult = RTotal;
   else if (env.isPartialApplied())
      rResult = RPartial;
   return rResult != RTotal;
}

/* Definition of the class Disjunction::ContainToApplication */

class Disjunction::ContainToApplication {
  private:
   VirtualElement& seThis;
   EvaluationEnvironment& seeEnv;
   enum Result { RNone, RPartial, RTotal };
   Result rResult;
   InformationKind ikInformation;
   
  public:
   ContainToApplication(VirtualElement& thisElement, EvaluationEnvironment& env,
         InformationKind information)
      :  seThis(thisElement), seeEnv(env), rResult(RTotal), ikInformation(information) {}

   bool operator()(const VirtualElement& source);
   bool isTotal() const { return rResult == RTotal; }
   bool isPartial() const { return rResult == RPartial; }
   bool isNone() const { return rResult == RNone; }
};

bool
Disjunction::ContainToApplication::operator()(const VirtualElement& source) {
   EvaluationParameters params(seeEnv);
   params.setInformationKind(ikInformation);
   AssumeCondition(rResult != RNone)
   EvaluationEnvironment env(params);
   assume(seThis.contain(source, env));
   if (env.isTotalApplied()) {
      if (rResult != RTotal)
         rResult = RPartial;
   }
   else if (env.isPartialApplied())
      rResult = RPartial;
   else { // env.isNoneApplied()
      if (rResult != RNone)
         rResult = RPartial;
   }
   return true;
}

/********************************************************/
/* Definition of the dispatch of the constraint methods */
/********************************************************/

/* Implementation of the class Disjunction::ConstraintToApplication */

class Disjunction::ConstraintToApplication {
  private:
   PNT::AutoPointer<VirtualOperation> apvoOperation;
   const VirtualElement& seThis;
   const VirtualElement& seResult;
   ConstraintEnvironment& ceeEnv;
   PPVirtualElement ppseThisResult;
   PPVirtualElement ppseSourceResult;
   InformationKind ikInformation;

  public:
   ConstraintToApplication(const VirtualOperation& operation, VirtualElement& thisElement,
      const VirtualElement& resultElement, ConstraintEnvironment& env,
      InformationKind information)
      :  apvoOperation(operation), seThis(thisElement), seResult(resultElement),
         ceeEnv(env), ikInformation(information) {}
   bool operator()(VirtualElement& source);

   PPVirtualElement extractThisResult() { return ppseThisResult; }
   PPVirtualElement extractArgument() { return ppseSourceResult; }
};

bool
Disjunction::ConstraintToApplication::operator()(VirtualElement& source) {
   PPVirtualElement thisConstraint(seThis);
   ConstraintTransmitEnvironment localEnv(ceeEnv, PPVirtualElement(source),
         ConstraintParameters(ceeEnv).setInformationKind(ikInformation));

   bool applicationResult = thisConstraint->constraint(*apvoOperation, seResult, localEnv);
   if (localEnv.isEmpty()) {
      localEnv.clear();
      return true;
   };
   AssumeCondition(applicationResult)
   if (localEnv.hasResult())
      thisConstraint = localEnv.presult();

   EvaluationEnvironment::LatticeCreation lattice = ceeEnv.getLatticeCreation();
   ceeEnv.setDisjunctionLatticeCreation();
   ppseThisResult = Methods::merge(ppseThisResult, *thisConstraint, ceeEnv);
   ceeEnv.setLatticeCreation(lattice);
   if (!ppseSourceResult.isValid())
      ppseSourceResult.absorbElement((VirtualElement*) localEnv.extractFirstArgument());
   else
      ppseSourceResult = Methods::merge(ppseSourceResult, localEnv.getFirstArgument(), ceeEnv);
   localEnv.merge().clear();

   return true;
}

/* Definition of the class Disjunction::ConstraintUnaryApplication */

class Disjunction::ConstraintUnaryApplication {
  private:
   PNT::AutoPointer<VirtualOperation> apvoOperation;
   const VirtualElement& seResult;
   ConstraintEnvironment& ceeEnv;
   PPVirtualElement ppseResult;
   InformationKind ikInformation;

  public:
   ConstraintUnaryApplication(const VirtualOperation& operation, const VirtualElement& resultElement,
      ConstraintEnvironment& env, InformationKind information)
      :  apvoOperation(operation), seResult(resultElement),
         ceeEnv(env), ikInformation(information) {}

   bool operator()(VirtualElement& thisElement);

   PPVirtualElement extractResult() { return ppseResult; }
};

bool
Disjunction::ConstraintUnaryApplication::operator()(VirtualElement& thisElement) {
   ConstraintTransmitEnvironment
      localEnv(ceeEnv, EvaluationParameters(ceeEnv).setInformationKind(ikInformation));

   bool applicationResult = thisElement.constraint(*apvoOperation, seResult, localEnv);
   AssumeCondition(applicationResult)
   if (localEnv.isEmpty()) {
      localEnv.clear();
      return true;
   };

   EvaluationEnvironment::LatticeCreation lattice = ceeEnv.getLatticeCreation();
   ceeEnv.setDisjunctionLatticeCreation();
   ppseResult = Methods::merge(ppseResult,
         localEnv.hasResult() ? localEnv.getResult() : thisElement, ceeEnv);
   ceeEnv.setLatticeCreation(lattice);

   return true;
}

/* Definition of the class Disjunction::ConstraintBinaryApplication */

class Disjunction::ConstraintBinaryApplication {
  private:
   PNT::AutoPointer<VirtualOperation> apvoOperation;
   PPVirtualElement ppseArgument;
   const VirtualElement& seResult;
   ConstraintEnvironment& ceeEnv;
   PPVirtualElement ppseThisResult;
   PPVirtualElement ppseArgumentResult;
   InformationKind ikInformation;

  public:
   ConstraintBinaryApplication(const VirtualOperation& operation, PPVirtualElement argumentElement,
      const VirtualElement& resultElement, ConstraintEnvironment& env,
      InformationKind information)
      :  apvoOperation(operation), ppseArgument(argumentElement), seResult(resultElement),
         ceeEnv(env), ikInformation(information) {}

   bool operator()(VirtualElement& thisElement);

   PPVirtualElement extractThisResult() { return ppseThisResult; }
   PPVirtualElement extractArgument() { return ppseArgumentResult; }
};

bool
Disjunction::ConstraintBinaryApplication::operator()(VirtualElement& thisElement) {
   ConstraintTransmitEnvironment
      localEnv(ceeEnv, EvaluationParameters(ceeEnv).setInformationKind(ikInformation));
   localEnv.absorbFirstArgument(ppseArgument->createSCopy());

   bool applicationResult = thisElement.constraint(*apvoOperation, seResult, localEnv);
   AssumeCondition(applicationResult)
   if (localEnv.isEmpty()) {
      localEnv.clear();
      return true;
   };

   EvaluationEnvironment::LatticeCreation lattice = ceeEnv.getLatticeCreation();
   ceeEnv.setDisjunctionLatticeCreation();
   ppseThisResult = Methods::merge(ppseThisResult,
         localEnv.hasResult() ? localEnv.getResult() : thisElement, ceeEnv);
   ppseArgumentResult = Methods::merge(ppseArgumentResult, localEnv.getFirstArgument(), ceeEnv);
   ceeEnv.setLatticeCreation(lattice);

   return true;
}

/***********************************************/
/* Definition of the dispatch of query methods */
/***********************************************/

#define DefineInitQuery(TypeOperation) \
   elementAt(QueryOperation::T##TypeOperation).setMethod(&Top::query##TypeOperation);
#define DefineInitOQuery(TypeOperation, TypeMethod) \
   elementAt(QueryOperation::T##TypeOperation).setMethod(&Top::query##TypeMethod);

Disjunction::QueryTable::QueryTable() {
#include "StandardClasses/UndefineNew.h"
   FunctionQueryTable* table = &elementAt(VirtualQueryOperation::TCompareSpecial);
   table->setSize(6);
   (*table)[0].setMethod(&Disjunction::queryCompareSpecial);
   (*table)[1].setMethod(&Disjunction::queryCompareSpecial);
   (*table)[2].setMethod(&Disjunction::queryCompareSpecial);
   (*table)[3].setMethod(&Disjunction::queryCompareSpecial);
   (*table)[4].setMethod(&Disjunction::queryCompareSpecial);
   (*table)[5].setMethod(&Disjunction::queryCompareSpecial);
   
   table = &elementAt(VirtualQueryOperation::TSimplification);
   table->setSize(Details::IntOperationElement::QueryOperation::EndOfTypeSimplification);
   // use a double function pointer dependint on the type of Top (integer class or not)
   (*table)[Details::IntOperationElement::QueryOperation::TSUndefined].setMethod(&Disjunction::querySimplification); // Disjunction::queryIdentity
   (*table)[Details::IntOperationElement::QueryOperation::TSConstant].setMethod(&Top::queryFailSimplification);
   (*table)[Details::IntOperationElement::QueryOperation::TSInterval].setMethod(&Disjunction::querySimplificationAsInterval);
   (*table)[Details::IntOperationElement::QueryOperation::TSConstantDisjunction].setMethod(&Disjunction::querySimplificationAsConstantDisjunction);

   table = &elementAt(VirtualQueryOperation::TDuplication);
   table->setSize(3);
   (*table)[QueryOperation::TCDClone].setMethod(&Disjunction::queryCopy);
   (*table)[QueryOperation::TCDSpecialize].setMethod(&Disjunction::querySpecialize);
#include "StandardClasses/DefineNew.h"
}

#undef DefineInitQuery
#undef DefineInitOQuery

Disjunction::QueryTable Disjunction::mqtMethodQueryTable;

/* Definition of the class MergeQueryBitDomainApplication */

/* Definition of the class MergeQueryBoundValueApplication */

class Disjunction::QueryCopyApplication {
  private:
   VirtualQueryOperation::DuplicationEnvironment& deEnv;
   const VirtualQueryOperation& qoCopyOperation;
   enum Destination { DUndefined, DSure, DExact, DMay };
   Destination dDestination;

  public:
   QueryCopyApplication(const VirtualQueryOperation& copyOperation, VirtualQueryOperation::DuplicationEnvironment& env)
      :  deEnv(env), qoCopyOperation(copyOperation), dDestination(DUndefined) {}

   QueryCopyApplication& setSure() { dDestination = DSure; return *this; }
   QueryCopyApplication& setExact() { dDestination = DExact; return *this; }
   QueryCopyApplication& setMay() { dDestination = DMay; return *this; }
   bool operator()(const VirtualElement& source)
      {  AssumeCondition(dDestination != DUndefined)
         VirtualQueryOperation::DuplicationEnvironment envCopy(deEnv);
         source.query(qoCopyOperation, envCopy);
         Disjunction& result = (Disjunction&) *deEnv.presult();
         ((dDestination == DSure) ? result.selSureElements : ((dDestination == DExact) ? result.selExactElements : result.selMayElements))
            .insertNewAtEnd(envCopy.presult().extractElement());
         return true;
      }
};

class Disjunction::QuerySpecializeApplication {
  private:
   VirtualQueryOperation::DuplicationEnvironment& deEnv;
   const VirtualQueryOperation& qoCopyOperation;

  public:
   QuerySpecializeApplication(const VirtualQueryOperation& copyOperation, VirtualQueryOperation::DuplicationEnvironment& env)
      :  deEnv(env), qoCopyOperation(copyOperation) {}

   bool operator()(VirtualElement& source)
      {  source.query(qoCopyOperation, deEnv);
         return true;
      }
};

/*******************************************/
/* Implementation of the class Disjunction */
/*******************************************/

void
Disjunction::_write(OSBase& out, const STG::IOObject::FormatParameters& aparams) const {
   const FormatParameters& params = (const FormatParameters&) aparams;
   std::function<bool (const VirtualElement&)> writeAction = [&out, &params](const VirtualElement& source)
      {  out.put(' ');
         source.write(out, params);
         return true;
      };
   out << "Disjunction(";
   if (!params.isOneLine())
      out << "\n\t\t\t\t";
   out << "May: ";
   selMayElements.foreachDo(writeAction);
   if (!params.isOneLine())
      out << "\n\t\t\t\t";
   else
      out.put(' ');
   out << "Exact: ";
   selExactElements.foreachDo(writeAction);
   if (!params.isOneLine())
      out << "\n\t\t\t\t";
   else
      out.put(' ');
   out << "Sure: ";
   selSureElements.foreachDo(writeAction);
   if (!params.isOneLine())
      out << "\n\t\t\t\t";
   out.put(')');
}

/* Definition of the apply methods */

bool
Disjunction::applyToNative(const VirtualOperation& operation, VirtualElement& argument, EvaluationEnvironment& env) const {
   PPVirtualElement result;
   if (env.getInformationKind().isMay()) {
      MergeApplyToApplication application(operation, argument, env, env.getInformationKind());
      selExactElements.foreachDo(application);
      selMayElements.foreachDo(application);
      result = application.extractResult();
   }
   else if (env.getInformationKind().isSure()) {
      MergeApplyToApplication application(operation, argument, env, env.getInformationKind());
      selExactElements.foreachDo(application);
      selSureElements.foreachDo(application);
      result = application.extractResult();
   }
   else {
      Disjunction* disjunctionResult = nullptr;
      if (uSizeInBits > 1) {
         result = Methods::newDisjunction(*this);
         AssumeCondition(dynamic_cast<const Disjunction*>(result.key()))
         disjunctionResult = (Disjunction*) &*result;
      }
      else
         result.absorbElement(disjunctionResult = createClearCopy());

      MergeApplyToApplication exactApplication(operation, argument, env, env.getInformationKind());
      selExactElements.foreachDo(exactApplication);
      PPVirtualElement exactResult = exactApplication.extractResult();

      MergeApplyToApplication mayApplication(operation, argument, env, InformationKind().setMay());
      selMayElements.foreachDo(mayApplication);
      PPVirtualElement mayResult = mayApplication.extractResult();

      MergeApplyToApplication sureApplication(operation, argument, env, InformationKind().setSure());
      selSureElements.foreachDo(sureApplication);
      PPVirtualElement sureResult = sureApplication.extractResult();

      disjunctionResult->addExact(exactResult).addMay(mayResult).addSure(sureResult);
      result = Methods::simplify(result);
   };
   if (!result.isValid())
      env.setEmpty();
   else
      env.presult() = result;
   return true;
}

Scalar::VirtualElement::ZeroResult
Disjunction::queryZeroResult() const {
   ZeroResult result;
   ElementsList::Cursor mayCursor(selMayElements);
   bool hasValue = false;
   if (mayCursor.setToFirst()) {
     if (!hasValue) {
       result = mayCursor.elementAt().queryZeroResult();
       hasValue = true;
     };
     while (mayCursor.setToNext()) {
       result.mergeWith(mayCursor.elementAt().queryZeroResult());
     };
   };
   ElementsList::Cursor exactCursor(selExactElements);
   if (exactCursor.setToFirst()) {
     if (!hasValue) {
       result = exactCursor.elementAt().queryZeroResult();
       hasValue = true;
     }
     else
       result.mergeWith(exactCursor.elementAt().queryZeroResult());
     while (exactCursor.setToNext()) {
       result.mergeWith(exactCursor.elementAt().queryZeroResult());
     };
   };
   return result;
}

bool
Disjunction::applyNative(const VirtualOperation& operation, EvaluationEnvironment& env) {
   PPVirtualElement result;
   if (env.getInformationKind().isMay()) {
      MergeApplyApplication application(operation, env, env.getInformationKind());
      selExactElements.foreachDo(application);
      selMayElements.foreachDo(application);
      result = application.extractResult();
   }
   else if (env.getInformationKind().isSure()) {
      MergeApplyApplication application(operation, env, env.getInformationKind());
      selExactElements.foreachDo(application);
      selSureElements.foreachDo(application);
      result = application.extractResult();
   }
   else {
      MergeApplyApplication exactApplication(operation, env, InformationKind().setExact());
      selExactElements.foreachDo(exactApplication);
      PPVirtualElement exactResult = exactApplication.extractResult();

      MergeApplyApplication mayApplication(operation, env, InformationKind().setMay());
      selMayElements.foreachDo(mayApplication);
      PPVirtualElement mayResult = mayApplication.extractResult();

      MergeApplyApplication sureApplication(operation, env, InformationKind().setSure());
      selSureElements.foreachDo(sureApplication);
      PPVirtualElement sureResult = sureApplication.extractResult();

      VirtualElement* reference = exactResult.isValid() ? &*exactResult : &*mayResult;
      Disjunction* disjunctionResult = nullptr;
      if (reference->getSizeInBits() > 1) {
         result = Methods::newDisjunction(*reference);
         AssumeCondition(dynamic_cast<const Disjunction*>(result.key()))
         disjunctionResult = (Disjunction*) &*result;
      }
      else {
         result.absorbElement(disjunctionResult = createClearCopy());
         Init init;
         init.setTable(*reference).setBitSize(1);
         disjunctionResult->uSizeInBits = 1;
         disjunctionResult->pfqtQueryDomainTable = init.pfqtQueryDomainTable;
         disjunctionResult->pfqtQueryOperationTable = init.pfqtQueryOperationTable;
         disjunctionResult->pfqtQueryExternTable = init.pfqtQueryExternTable;
      };
      disjunctionResult->addExact(exactResult).addMay(mayResult).addSure(sureResult);
      result = Methods::simplify(result);
   };
   if (!result.isValid())
      env.setEmpty();
   else
      env.presult() = result;
   return true;
}

/* Definition of the merge and intersection methods */

bool
Disjunction::mergeWithTo(VirtualElement& source, EvaluationEnvironment& env) const {
   AssumeCondition(source.getApproxKind().compareApply(ApproxKind().setDisjunction()) == CRLess)
   EvaluationEnvironment::LatticeCreation latticeCreation = env.getLatticeCreation();
   if ((latticeCreation.isTop() || latticeCreation.isShareTop()) && !Methods::contain(source, *this, EvaluationParameters(env))) {
      env.presult() = Details::IntOperationElement::Methods::newTop(*this);
      return true;
   };
   Disjunction* result = nullptr;
   if (uSizeInBits > 1) {
      env.presult() = Methods::newDisjunction(*this);
      AssumeCondition(dynamic_cast<const Disjunction*>(&env.getResult()))
      result = (Disjunction*) &env.getResult();
   }
   else
      env.storeResult(result = createClearCopy());
   result->addExact(source);
   if (env.getInformationKind().isMay()) {
      result->selMayElements.addCopyAll(selExactElements);
      result->selMayElements.addCopyAll(selMayElements);
   }
   else if (env.getInformationKind().isSure()) {
      result->selSureElements.addCopyAll(selExactElements);
      result->selSureElements.addCopyAll(selSureElements);
   }
   else {
      result->selExactElements.addCopyAll(selExactElements);
      result->selMayElements.addCopyAll(selMayElements);
      result->selSureElements.addCopyAll(selSureElements);
   };
   env.setUnstable();
   env.mergeVerdictExact();
   return true;
}

bool
Disjunction::containTo(const VirtualElement& source, EvaluationEnvironment& env) const
   {  env.setPartialApplied();
      return true;
   }

bool
Disjunction::mergeWith(const Scalar::VirtualElement& asource, Scalar::EvaluationEnvironment& aenv) {
   if (VirtualElement::mergeWith(asource, aenv))
      return true;
   EvaluationEnvironment& env = (EvaluationEnvironment&) aenv;
   const VirtualElement& source = (const VirtualElement&) asource;
   AssumeCondition(source.getApproxKind().compareApply(ApproxKind().setDisjunction()) <= CREqual)

   EvaluationEnvironment::LatticeCreation latticeCreation = env.getLatticeCreation();
   if ((latticeCreation.isTop() || latticeCreation.isShareTop()) && !Methods::contain(*this, (const VirtualElement&) source, EvaluationParameters(env))) {
      env.presult() = Details::IntOperationElement::Methods::newTop(*this);
      return true;
   };
   if (source.getApproxKind().compareApply(ApproxKind().setDisjunction()) == CRLess)
      add(source, env.getInformationKind());
   else {
      const Disjunction& disjunctionSource = (const Disjunction&) source;
      if (env.getInformationKind().isMay()) {
         AddElement addElement(selMayElements);
         disjunctionSource.selExactElements.foreachDo(addElement);
         // selMayElements.addCopyAll(disjunctionSource.selExactElements);
         disjunctionSource.selMayElements.foreachDo(addElement);
         // selMayElements.addCopyAll(disjunctionSource.selMayElements);
      }
      else if (env.getInformationKind().isSure()) {
         AddElement addElement(selSureElements);
         disjunctionSource.selExactElements.foreachDo(addElement);
         // selSureElements.addCopyAll(disjunctionSource.selExactElements);
         disjunctionSource.selSureElements.foreachDo(addElement);
         // selSureElements.addCopyAll(disjunctionSource.selSureElements);
      }
      else {
         AddElement addExactElement(selExactElements);
         disjunctionSource.selExactElements.foreachDo(addExactElement);
         // selExactElements.addCopyAll(disjunctionSource.selExactElements);
         AddElement addMayElement(selMayElements);
         disjunctionSource.selMayElements.foreachDo(addMayElement);
         // selMayElements.addCopyAll(disjunctionSource.selMayElements);
         AddElement addSureElement(selSureElements);
         disjunctionSource.selSureElements.foreachDo(addSureElement);
         // selSureElements.addCopyAll(disjunctionSource.selSureElements);
      };
   };
   env.setUnstable();
   env.mergeVerdictExact();
   return true;
}

bool
Disjunction::contain(const Scalar::VirtualElement& asource, Scalar::EvaluationEnvironment& aenv) const {
   EvaluationEnvironment& env = (EvaluationEnvironment&) aenv;
   const VirtualElement& source = (const VirtualElement&) asource;

   if (source.getApproxKind().compareApply(ApproxKind().setDisjunction()) == CRGreater)
     return source.containTo(*this, env);

   if (source.getApproxKind().compareApply(ApproxKind().setDisjunction()) == CRLess) {
      if (source.getApproxKind().isFormalConstruction())
         return source.containTo(*this, env);
      ContainApplication contain(source, env, InformationKind::IKMay);
      selMayElements.foreachDo(contain);
      if (contain.isTotal())
         return true;
      selExactElements.foreachDo(contain);
      if (contain.isTotal())
         return true;
      if (contain.isNone())
         env.setNoneApplied();
      else
         env.setPartialApplied();
   }
   else {
      AssumeCondition(dynamic_cast<const Disjunction*>(&source))
      const Disjunction& disjunctionSource = (const Disjunction&) source;
      ElementsList::Cursor mayCursor(disjunctionSource.selMayElements);
      for (bool doesContinue = mayCursor.setToFirst(); doesContinue; doesContinue = mayCursor.setToNext()) {
         ContainApplication contain(mayCursor.elementAt(), env, InformationKind::IKMay);
         selMayElements.foreachDo(contain);
         if (contain.isTotal())
           continue;
         selExactElements.foreachDo(contain);
         if (contain.isTotal())
           continue;
         if (contain.isNone())
            env.setNoneApplied();
         else {
            env.setPartialApplied();
            return true;
         };
      };
      ElementsList::Cursor exactCursor(disjunctionSource.selExactElements);
      for (bool doesContinue = exactCursor.setToFirst(); doesContinue; doesContinue = exactCursor.setToNext()) {
         ContainApplication contain(exactCursor.elementAt(), env, InformationKind::IKMay);
         selMayElements.foreachDo(contain);
         if (contain.isTotal())
           continue;
         selExactElements.foreachDo(contain);
         if (contain.isTotal())
           continue;
         if (contain.isNone())
            env.setNoneApplied();
         else
            env.setPartialApplied();
         return true;
      };
   };
   return true;
}

bool
Disjunction::intersectWithTo(VirtualElement& source, EvaluationEnvironment& env) const {
   // x /\ (a U b U c) = (x /\ a) U ...
   AssumeCondition(source.getApproxKind().compareConstraint(ApproxKind().setDisjunction()) == CRGreater)
   env.mergeVerdictExact();
   env.setUnstable();
   PPVirtualElement result;
   if (env.getInformationKind().isMay()) {
      IntersectToApplication intersection(source, env, env.getInformationKind());
      selExactElements.foreachDo(intersection);
      selMayElements.foreachDo(intersection);
      result = intersection.extractResult();
   }
   else if (env.getInformationKind().isSure()) {
      IntersectToApplication intersection(source, env, env.getInformationKind());
      selExactElements.foreachDo(intersection);
      selSureElements.foreachDo(intersection);
      result = intersection.extractResult();
   }
   else {
      Disjunction* disjunctionResult = nullptr;
      if (uSizeInBits > 1) {
         result = Methods::newDisjunction(*this);
         AssumeCondition(dynamic_cast<const Disjunction*>(result.key()))
         disjunctionResult = (Disjunction*) &*result;
      }
      else
         result.absorbElement(disjunctionResult = createClearCopy());

      PPVirtualElement maySource(source), sureSource(source);
      IntersectToApplication
         exactIntersection(source, env, env.getInformationKind()),
         mayIntersection(*maySource, env, InformationKind().setMay()),
         sureIntersection(*sureSource, env, InformationKind().setSure());

      selExactElements.foreachDo(exactIntersection);
      PPVirtualElement exactResult = exactIntersection.extractResult();

      selMayElements.foreachDo(mayIntersection);
      PPVirtualElement mayResult = mayIntersection.extractResult();

      selSureElements.foreachDo(sureIntersection);
      PPVirtualElement sureResult = sureIntersection.extractResult();

      disjunctionResult->addExact(exactResult).addMay(mayResult).addSure(sureResult);
      result = Methods::simplify(result);
   };
   if (!result.isValid())
      env.setEmpty();
   else
      env.presult() = result;
   return true;
}

bool
Disjunction::intersectWith(const VirtualElement& source, EvaluationEnvironment& env) {
   if (VirtualElement::intersectWith(source, env))
      return true;
   PPVirtualElement result;
   if (env.getInformationKind().isMay()) {
      IntersectApplication application(source, env, env.getInformationKind());
      selExactElements.foreachDo(application);
      selMayElements.foreachDo(application);
      result = application.extractResult();
   }
   else if (env.getInformationKind().isSure()) {
      IntersectApplication application(source, env, env.getInformationKind());
      selExactElements.foreachDo(application);
      selSureElements.foreachDo(application);
      result = application.extractResult();
   }
   else {
      Disjunction* disjunctionResult = nullptr;
      if (uSizeInBits > 1) {
         result = Methods::newDisjunction(*this);
         AssumeCondition(dynamic_cast<const Disjunction*>(result.key()))
         disjunctionResult = (Disjunction*) &*result;
      }
      else
         result.absorbElement(disjunctionResult = createClearCopy());

      IntersectApplication exactApplication(source, env, env.getInformationKind());
      selExactElements.foreachDo(exactApplication);
      PPVirtualElement exactResult = exactApplication.extractResult();

      IntersectApplication mayApplication(source, env, InformationKind().setMay());
      selMayElements.foreachDo(mayApplication);
      PPVirtualElement mayResult = mayApplication.extractResult();

      IntersectApplication sureApplication(source, env, InformationKind().setSure());
      selSureElements.foreachDo(sureApplication);
      PPVirtualElement sureResult = sureApplication.extractResult();

      disjunctionResult->addExact(exactResult).addMay(mayResult).addSure(sureResult);
      result = Methods::simplify(result);
   };
   if (!result.isValid())
      env.setEmpty();
   else
      env.presult() = result;
   return true;
}

/* Definition of the constraint methods */

bool
Disjunction::constraintTo(const VirtualOperation& operation, VirtualElement& thisElement,
      const VirtualElement& resultElement, ConstraintEnvironment& env) {
   AssumeCondition(thisElement.getApproxKind().compareConstraint(ApproxKind().setDisjunction()) == CRGreater)
   env.mergeVerdictExact();
   env.setUnstable();
   PPVirtualElement result, argument;
   if (env.getInformationKind().isMay()) {
      ConstraintToApplication constraint(operation, thisElement, resultElement, env, env.getInformationKind());
      selExactElements.foreachDo(constraint);
      selMayElements.foreachDo(constraint);
      result = constraint.extractThisResult();
      argument = constraint.extractArgument();
   }
   else if (env.getInformationKind().isSure()) {
      ConstraintToApplication constraint(operation, thisElement, resultElement, env, env.getInformationKind());
      selExactElements.foreachDo(constraint);
      selSureElements.foreachDo(constraint);
      result = constraint.extractThisResult();
      argument = constraint.extractArgument();
   }
   else {
      Disjunction* disjunctionResult = nullptr;
      if (uSizeInBits > 1) {
         result = Methods::newDisjunction(*this);
         AssumeCondition(dynamic_cast<const Disjunction*>(result.key()))
         disjunctionResult = (Disjunction*) &*result;
      }
      else
         result.absorbElement(disjunctionResult = createClearCopy());

      ConstraintToApplication
         exactConstraint(operation, thisElement, resultElement, env, env.getInformationKind()),
         mayConstraint(operation, thisElement, resultElement, env, InformationKind().setMay()),
         sureConstraint(operation, thisElement, resultElement, env, InformationKind().setSure());

      selExactElements.foreachDo(exactConstraint);
      PPVirtualElement exactResult = exactConstraint.extractThisResult();
      PPVirtualElement exactArgument = exactConstraint.extractArgument();

      selMayElements.foreachDo(mayConstraint);
      PPVirtualElement mayResult = mayConstraint.extractThisResult();
      PPVirtualElement mayArgument = mayConstraint.extractArgument();

      selSureElements.foreachDo(sureConstraint);
      PPVirtualElement sureResult = sureConstraint.extractThisResult();
      PPVirtualElement sureArgument = sureConstraint.extractArgument();

      disjunctionResult->addExact(exactResult).addMay(mayResult).addSure(sureResult);
      result = Methods::simplify(result);
   };
   if (!result.isValid())
      env.setEmpty();
   else
      env.presult() = result;
   return true;
}

bool
Disjunction::constraint(const VirtualOperation& operation, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   if (VirtualElement::constraint(operation, resultElement, env))
      return true;
   if (operation.getArgumentsNumber() == 0) {
      PPVirtualElement result;
      if (env.getInformationKind().isMay()) {
         ConstraintUnaryApplication application(operation, resultElement, env, env.getInformationKind());
         selExactElements.foreachDo(application);
         selMayElements.foreachDo(application);
         result = application.extractResult();
      }
      else if (env.getInformationKind().isSure()) {
         ConstraintUnaryApplication application(operation, resultElement, env, env.getInformationKind());
         selExactElements.foreachDo(application);
         selSureElements.foreachDo(application);
         result = application.extractResult();
      }
      else {
         Disjunction* disjunctionResult = nullptr;
         if (uSizeInBits > 1) {
            result = Methods::newDisjunction(*this);
            AssumeCondition(dynamic_cast<const Disjunction*>(result.key()))
            disjunctionResult = (Disjunction*) &*result;
         }
         else
            result.absorbElement(disjunctionResult = createClearCopy());

         ConstraintUnaryApplication exactApplication(operation, resultElement, env, env.getInformationKind());
         selExactElements.foreachDo(exactApplication);
         PPVirtualElement exactResult = exactApplication.extractResult();

         ConstraintUnaryApplication mayApplication(operation, resultElement, env, InformationKind().setMay());
         selMayElements.foreachDo(mayApplication);
         PPVirtualElement mayResult = mayApplication.extractResult();

         ConstraintUnaryApplication sureApplication(operation, resultElement, env, InformationKind().setSure());
         selSureElements.foreachDo(sureApplication);
         PPVirtualElement sureResult = sureApplication.extractResult();

         disjunctionResult->addExact(exactResult).addMay(mayResult).addSure(sureResult);
         result = Methods::simplify(result);
      };
      if (!result.isValid())
         env.setEmpty();
      else
         env.presult() = result;
   }
   else if (operation.getArgumentsNumber() == 1) {
      PPVirtualElement result;
      PPVirtualElement source((VirtualElement*) env.extractFirstArgument(), PNT::Pointer::Init());
      if (env.getInformationKind().isMay()) {
         ConstraintBinaryApplication application(operation, source, resultElement, env, env.getInformationKind());
         selExactElements.foreachDo(application);
         selMayElements.foreachDo(application);
         result = application.extractThisResult();
         source = application.extractArgument();
      }
      else if (env.getInformationKind().isSure()) {
         ConstraintBinaryApplication application(operation, source, resultElement, env, env.getInformationKind());
         selExactElements.foreachDo(application);
         selSureElements.foreachDo(application);
         result = application.extractThisResult();
         source = application.extractArgument();
      }
      else {
         ConstraintBinaryApplication
            mayConstraint(operation, *source, resultElement, env, InformationKind().setMay()),
            sureConstraint(operation, *source, resultElement, env, InformationKind().setSure()),
            exactConstraint(operation, source, resultElement, env, InformationKind().setExact());

         Disjunction *disjunctionResult = nullptr, *disjunctionSource = nullptr;
         if (uSizeInBits > 1) {
            result = Methods::newDisjunction(*this);
            AssumeCondition(dynamic_cast<const Disjunction*>(result.key()))
            disjunctionResult = (Disjunction*) &*result;
         }
         else
            result.absorbElement(disjunctionResult = createClearCopy());

         if (uSizeInBits > 1) {
            source = Methods::newDisjunction(*this);
            AssumeCondition(dynamic_cast<const Disjunction*>(source.key()))
            disjunctionSource = (Disjunction*) &*source;
         }
         else
            source.absorbElement(disjunctionSource = createClearCopy());

         selExactElements.foreachDo(exactConstraint);
         PPVirtualElement exactResult = exactConstraint.extractThisResult();
         PPVirtualElement exactSource = exactConstraint.extractArgument();

         selMayElements.foreachDo(mayConstraint);
         PPVirtualElement mayResult = mayConstraint.extractThisResult();
         PPVirtualElement maySource = mayConstraint.extractArgument();

         selSureElements.foreachDo(sureConstraint);
         PPVirtualElement sureResult = sureConstraint.extractThisResult();
         PPVirtualElement sureSource = sureConstraint.extractArgument();

         disjunctionResult->addExact(exactResult).addMay(mayResult).addSure(sureResult);
         disjunctionSource->addExact(exactSource).addMay(maySource).addSure(sureSource);
         result = Methods::simplify(result);
         source = Methods::simplify(source);
      };
      if (!result.isValid() || !source.isValid())
         env.setEmpty();
      else {
         env.presult() = result;
         env.absorbFirstArgument(source.extractElement());
      };
   }
   else {
      AssumeUnimplemented
   };
   return true;
}

/* Definition of the query methods */

bool
Disjunction::queryCompareSpecial(const VirtualElement& athisElement,
      const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const Disjunction*>(&athisElement))
   const Disjunction& thisElement = (const Disjunction&) athisElement;
   auto& env = (QueryOperation::CompareSpecialEnvironment&) aenv;
   std::function<bool (const VirtualElement&)> application = [&env, &operation]
         (const VirtualElement& thisElement)
      {  thisElement.query(operation, env);
         return true;
      };
   thisElement.selExactElements.foreachDo(application);
   thisElement.selMayElements.foreachDo(application);
   return true;
}

bool
Disjunction::querySimplification(const VirtualElement& athisElement, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const Disjunction*>(&athisElement))
   Disjunction& thisElement = const_cast<Disjunction&>((const Disjunction&) athisElement);
   QueryOperation::SimplificationEnvironment& env = (QueryOperation::SimplificationEnvironment&) aenv;
   if (thisElement.selExactElements.isEmpty() && thisElement.selMayElements.isEmpty() && thisElement.selSureElements.isEmpty()) {
      env.setEmpty();
      return true;
   };
   if ((thisElement.selExactElements.count() == 1) && thisElement.selMayElements.isEmpty() && thisElement.selSureElements.isEmpty()) {
      PPVirtualElement result(&thisElement.selExactElements.extractFirst(), PNT::Pointer::Init());
      if (result->getApproxKind().isDisjunction())
         env.presult() = Methods::simplify(result);
      else
         env.presult() = result;
      return true;
   };

   if (!thisElement.selExactElements.isEmpty()) {
      ElementsList::Cursor previousCursor(thisElement.selExactElements);
      do {
         VirtualElement& element = thisElement.selExactElements.getSNextElement(previousCursor);
         if (element.getApproxKind().isDisjunction()) {
            thisElement.selExactElements.deleteNext(previousCursor);
            PNT::TPassPointer<Disjunction, PPAbstractElement> removedElement((Disjunction*) &element, PNT::Pointer::Init());

            removedElement->selExactElements.moveAllTo(thisElement.selExactElements); // at the end -> verification
            removedElement->selMayElements.moveAllTo(thisElement.selMayElements);
            removedElement->selSureElements.moveAllTo(thisElement.selSureElements);
         }
         else if (element.getApproxKind().isTop()) {
            thisElement.selExactElements.deleteNext(previousCursor);
            env.presult().absorbElement(&element);
            return true;
         };
      } while (previousCursor.setToNext() && !previousCursor.isLast());
   };

   if (!thisElement.selMayElements.isEmpty()) {
      ElementsList::Cursor previousCursor(thisElement.selMayElements);
      do {
         VirtualElement& element = thisElement.selMayElements.getSNextElement(previousCursor);
         if (element.getApproxKind().isDisjunction()) {
            thisElement.selMayElements.deleteNext(previousCursor);
            PNT::TPassPointer<Disjunction, PPAbstractElement> removedElement((Disjunction*) &element, PNT::Pointer::Init());

            removedElement->selExactElements.moveAllTo(thisElement.selMayElements);
            removedElement->selMayElements.moveAllTo(thisElement.selMayElements);
         }
         else if (element.getApproxKind().isTop()) {
            thisElement.selMayElements.deleteNext(previousCursor);
            thisElement.selMayElements.freeAll();
            thisElement.selMayElements.insertNewAt(previousCursor, &element, COL::VirtualCollection::RPAfter);
         };
      } while (previousCursor.setToNext() && !previousCursor.isLast());
   };

   if (!thisElement.selSureElements.isEmpty()) {
      ElementsList::Cursor previousCursor(thisElement.selSureElements);
      do {
         VirtualElement& element = thisElement.selSureElements.getSNextElement(previousCursor);
         if (element.getApproxKind().isDisjunction()) {
            thisElement.selSureElements.deleteNext(previousCursor);
            PNT::TPassPointer<Disjunction, PPAbstractElement> removedElement((Disjunction*) &element, PNT::Pointer::Init());

            removedElement->selExactElements.moveAllTo(thisElement.selSureElements);
            removedElement->selSureElements.moveAllTo(thisElement.selSureElements);
         }
         else if (element.getApproxKind().isTop()) {
            thisElement.selSureElements.deleteNext(previousCursor);
            env.presult().absorbElement(&element);
            return true;
         };
      } while (previousCursor.setToNext() && !previousCursor.isLast());
   };

   return true;
}

bool
Disjunction::querySimplificationAsInterval(const VirtualElement& thisElement, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env) {
   AssumeCondition(dynamic_cast<const SimplificationEnvironment*>(&env))
   ((SimplificationEnvironment&) env).setFail();
   return true;
}

bool
Disjunction::querySimplificationAsConstantDisjunction(const VirtualElement& athisElement,
        const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const Disjunction*>(&athisElement))
   AssumeCondition(dynamic_cast<const Details::IntOperationElement::SimplificationEnvironment*>(&aenv))
   // should add a query method asking whether the simplification is possible and whether it is not soon performed
   
   ElementsList result;
   
   Disjunction& thisElement = const_cast<Disjunction&>((const Disjunction&) athisElement);
   Details::IntOperationElement::SimplificationEnvironment& env = (Details::IntOperationElement::SimplificationEnvironment&) aenv;

   while (!thisElement.selMayElements.isEmpty()) {
      PPVirtualElement element(&thisElement.selMayElements.extractFirst(), PNT::Pointer::Init());
      element = Details::IntOperationElement::Methods::simplifyAsConstantDisjunction(element);
      if (!element.isValid()) {
         env.setFail();
         return true;
      };
      if (element->getApproxKind().isConstant())
         result.addNew(element.extractElement());
      else {
         AssumeCondition(dynamic_cast<const Disjunction*>(element.key()))
         Disjunction& source = (Disjunction&) *element;
         source.selMayElements.moveAllTo(result);
      };
   };
         
   while (!thisElement.selExactElements.isEmpty()) {
      PPVirtualElement element(&thisElement.selExactElements.extractFirst(), PNT::Pointer::Init());
      element = Details::IntOperationElement::Methods::simplifyAsConstantDisjunction(element);
      if (!element.isValid()) {
         env.setFail();
         return true;
      };
      if (element->getApproxKind().isConstant())
         result.addNew(element.extractElement());
      else {
         AssumeCondition(dynamic_cast<const Disjunction*>(element.key()))
         Disjunction& source = (Disjunction&) *element;
         source.selMayElements.moveAllTo(result);
      };
   };

   while (!thisElement.selSureElements.isEmpty()) {
      PPVirtualElement element(&thisElement.selSureElements.extractFirst(), PNT::Pointer::Init());
      element = Details::IntOperationElement::Methods::simplifyAsConstantDisjunction(element);
      if (!element.isValid()) {
         env.setFail();
         return true;
      };
      if (element->getApproxKind().isConstant())
         result.addNew(element.extractElement());
      else {
         AssumeCondition(dynamic_cast<const Disjunction*>(element.key()))
         Disjunction& source = (Disjunction&) *element;
         source.selMayElements.moveAllTo(result);
      };
   };
   thisElement.selMayElements.swap(result);
   thisElement.selExactElements.freeAll();
   thisElement.selSureElements.freeAll();
   return true;
}

bool
Disjunction::queryCopy(const VirtualElement& athisElement, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const Disjunction*>(&athisElement))
   const Disjunction& thisElement = (const Disjunction&) athisElement;
   QueryOperation::DuplicationEnvironment& env = (QueryOperation::DuplicationEnvironment&) aenv;
   env.presult().absorbElement(thisElement.createClearCopy());
   QueryCopyApplication duplication(operation, env);

   thisElement.selSureElements.foreachDo(duplication.setSure());
   thisElement.selExactElements.foreachDo(duplication.setExact());
   thisElement.selMayElements.foreachDo(duplication.setMay());
   return true;
}

bool
Disjunction::querySpecialize(const VirtualElement& athisElement, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) {
   AssumeCondition(dynamic_cast<const Disjunction*>(&athisElement))
   const Disjunction& thisElement = (const Disjunction&) athisElement;
   QuerySpecializeApplication specialization(operation, (QueryOperation::DuplicationEnvironment&) env);
   thisElement.selSureElements.foreachDo(specialization);
   thisElement.selExactElements.foreachDo(specialization);
   thisElement.selMayElements.foreachDo(specialization);
   return true;
}

bool
Disjunction::query(const VirtualQueryOperation& aoperation, VirtualQueryOperation::Environment& env) const {
   if (inherited::query(aoperation, env))
      return true;
   const VirtualQueryOperation& operation = (const VirtualQueryOperation&) aoperation;
   if (operation.isOperation() || operation.isDomain() || operation.isExtern()) {
      AssumeCondition(pfqtQueryOperationTable && pfqtQueryDomainTable && pfqtQueryExternTable)
      (operation.isOperation() ? pfqtQueryOperationTable
         : (operation.isDomain() ? pfqtQueryDomainTable : pfqtQueryExternTable))->execute(*this, operation, env);
   }
   else if (!operation.isExternMethod())
      assume(mqtMethodQueryTable[operation.getType()].execute(*this, operation, env));
   else {
      if (!selExactElements.isEmpty())
         return selExactElements.getFirst().query(operation, env);
      if (!selMayElements.isEmpty())
         return selMayElements.getFirst().query(operation, env);
      if (!selSureElements.isEmpty())
         return selSureElements.getFirst().query(operation, env);
      return false;
   };
   return true;
}

}}} // end of namespace Analyzer::Scalar::Approximate

