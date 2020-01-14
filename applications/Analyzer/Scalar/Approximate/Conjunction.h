/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : Conjunction.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a class of conjunctions.
//

#ifndef Analyzer_Scalar_Approximate_ConjunctionH
#define Analyzer_Scalar_Approximate_ConjunctionH

#include "Analyzer/Scalar/Approximate/VirtualElement.h"

namespace Analyzer { namespace Scalar { namespace Approximate {

class Conjunction : public VirtualElement {
  private:
   typedef VirtualElement inherited;
   const FunctionQueryTable* pfqtQueryDomainTable;
   const FunctionQueryTable* pfqtQueryOperationTable;

   typedef inherited::QueryTable InheritedQueryTable;
   class QueryTable : public InheritedQueryTable {
     private:
      typedef InheritedQueryTable inherited;
     public:
      QueryTable() { AssumeUnimplemented }
   };
   friend class QueryTable;
   static QueryTable mqtMethodQueryTable;

  public:
   class GuardElement : public EnhancedObject {
     private:
      typedef EnhancedObject inherited;
      PPVirtualElement ppveElement;
      PPVirtualElement ppveTrueCondition;
      
     public:
      GuardElement(PPVirtualElement element) : ppveElement(element) {}
      GuardElement(const GuardElement& source) = default;
      GuardElement& operator=(const GuardElement& source) = default;
      DefineCopy(GuardElement)
      DDefineAssign(GuardElement)

      const VirtualElement& getElement() const { return *ppveElement; }
      VirtualElement& getElement() { return *ppveElement; }
      PPVirtualElement& selement() { return ppveElement; }
   };

   class ElementsList : public COL::TCopyCollection<COL::TArray<GuardElement> > {
     private:
      typedef COL::TCopyCollection<COL::TArray<GuardElement> > inherited;
      
     public:
      ElementsList() {}
      ElementsList(const ElementsList& source) = default;
     
      void mergeNew(VirtualElement* element) { AssumeUnimplemented }
   };
   
  private:
   ElementsList selSubElements;

  protected:
   virtual void _write(OSBase& out, const STG::IOObject::FormatParameters& params) const override
      {  AssumeUnimplemented }

  public:
   class Init : public VirtualElement::Init {
     private:
      typedef VirtualElement::Init inherited;
      const FunctionQueryTable* pfqtQueryDomainTable;
      const FunctionQueryTable* pfqtQueryOperationTable;
      
     public:
      Init() : pfqtQueryDomainTable(nullptr), pfqtQueryOperationTable(nullptr) {}
      Init(const Init& source) = default;
      Init& operator=(const Init& source) = default;
      DefineCopy(Init)
      DDefineAssign(Init)
      
      Init& setQueryDomainTable(const FunctionQueryTable& queryDomainTable)
         {  pfqtQueryDomainTable = &queryDomainTable; return *this; }
      Init& setQueryOperationTable(const FunctionQueryTable& queryOperationTable)
         {  pfqtQueryOperationTable = &queryOperationTable; return *this; }
      Init& setTable(const VirtualElement& element)
         {  pfqtQueryDomainTable = &element.functionQueryTable(VirtualQueryOperation::TDomain);
            pfqtQueryOperationTable = &element.functionQueryTable(VirtualQueryOperation::TOperation);
            return *this;
         }
      Init& setTable(const Conjunction& conjunction)
         {  inherited::setBitSize(conjunction.getSizeInBits());
            pfqtQueryDomainTable = conjunction.pfqtQueryDomainTable;
            pfqtQueryOperationTable = conjunction.pfqtQueryOperationTable;
            return *this;
         }
      Init& setBitSize(int bitSize) { return (Init&) inherited::setBitSize(bitSize); }

      friend class Conjunction;
   };
   friend class Init;
   
   Conjunction(const Init& init)
      :  inherited(init), pfqtQueryDomainTable(init.pfqtQueryDomainTable), pfqtQueryOperationTable(init.pfqtQueryOperationTable)
      {  AssumeCondition(pfqtQueryDomainTable) setConjunction(); }
   Conjunction(const Conjunction& source) = default;
   Conjunction& operator=(const Conjunction& source) = default;
   DefineCopy(Conjunction)
   DDefineAssign(Conjunction)
   virtual const FunctionQueryTable& functionQueryTable(int queryIndex) const override
      {  return (queryIndex == VirtualQueryOperation::TOperation)
            ? *pfqtQueryOperationTable : ((queryIndex == VirtualQueryOperation::TDomain)
            ? *pfqtQueryOperationTable : mqtMethodQueryTable[queryIndex]);
      }
   virtual bool isBoolean() const override { return selSubElements.getFirst().getElement().isBoolean(); }
   virtual bool isInt() const override { return selSubElements.getFirst().getElement().isInt(); }
   virtual bool isMultiBit() const override { return selSubElements.getFirst().getElement().isMultiBit(); }
   virtual bool isFloat() const override { return selSubElements.getFirst().getElement().isFloat(); }

   // construction methods
   void add(PPVirtualElement element) { AssumeUnimplemented }
   virtual int _getSizeInBits() const override
      {  return selSubElements.getFirst().getElement().getSizeInBits(); }
   
   // inherited methods from VirtualElement
   virtual bool applyTo(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const override
      {  AssumeCondition(getApproxKind().compareApply(source.getApproxKind()) >= CREqual)
         return applyToNative(operation, source, env);
      }
   virtual bool mergeWithTo(VirtualElement& source, EvaluationEnvironment& env) const override
      {  AssumeUnimplemented }
   virtual bool containTo(const VirtualElement& source, EvaluationEnvironment& env) const override
      {  AssumeUnimplemented }
   virtual bool intersectWithTo(VirtualElement& source, EvaluationEnvironment& env) const override
      {  AssumeUnimplemented }
   virtual bool constraintTo(const VirtualOperation&, VirtualElement& source, const VirtualElement&, ConstraintEnvironment&) override
      {  AssumeUnimplemented }
   virtual ZeroResult queryZeroResult() const override
      {  AssumeUnimplemented }

   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override
      { return (VirtualElement::apply(operation, env)) ? true : applyNative(operation, (EvaluationEnvironment&) env); }
   virtual bool mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) override
      {  AssumeUnimplemented }
   virtual bool contain(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) const override
      {  AssumeUnimplemented }
   virtual bool intersectWith(const VirtualElement& source, EvaluationEnvironment& env) override
      {  AssumeUnimplemented }
   virtual bool constraint(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&) override
      {  AssumeUnimplemented }
   virtual bool query(const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) const override
      {  AssumeUnimplemented }

   bool applyToNative(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const
      {  AssumeUnimplemented }
   bool applyNative(const VirtualOperation& operation, EvaluationEnvironment& env)
      {  AssumeUnimplemented }
   
   ElementsList& subElements() { return selSubElements; }
   const ElementsList& subElements() const { return selSubElements; }
   
   static bool queryCompareSpecial(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env)
      {  AssumeUnimplemented }
   static bool querySimplification(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env)
      {  AssumeUnimplemented }
   static bool querySimplificationAsInterval(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env)
      {  AssumeUnimplemented }
   static bool querySimplificationAsConstantDisjunction(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env)
      {  AssumeUnimplemented }
   
   static bool queryCopy(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env)
      {  AssumeUnimplemented }
   static bool querySpecialize(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env)
      {  AssumeUnimplemented }
   bool isValid() const override { return VirtualElement::isValid() && (selSubElements.count() >= 2); }
      
   class QueryOperation : public VirtualQueryOperation {
     private:
      typedef VirtualQueryOperation inherited;
      
     public:
      QueryOperation() {}
      QueryOperation(const QueryOperation& source) = default;
   };
};

}}} // end of namespace Analyzer::Scalar::Approximate


#endif // Analyzer_Scalar_Approximate_ConjunctionH

