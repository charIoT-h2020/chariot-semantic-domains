/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : Disjunction.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a class of disjunctions.
//

#ifndef Analyzer_Scalar_Approximate_DisjunctionH
#define Analyzer_Scalar_Approximate_DisjunctionH

#include "Analyzer/Scalar/Approximate/VirtualElement.h"

namespace Analyzer { namespace Scalar { namespace Approximate {

class AccessDisjunction;
class Disjunction : public VirtualDisjunction {
  private:
   typedef VirtualDisjunction inherited;
   const FunctionQueryTable* pfqtQueryDomainTable;
   const FunctionQueryTable* pfqtQueryOperationTable;
   const FunctionQueryTable* pfqtQueryExternTable;
   int uSizeInBits;
   int uSizeMantissa;
   int uSizeExponent;
   typedef VirtualDisjunction::QueryTable InheritedQueryTable;
   class QueryTable : public InheritedQueryTable {
     private:
      typedef InheritedQueryTable inherited;
     public:
      QueryTable();
   };
   friend class QueryTable;
   static QueryTable mqtMethodQueryTable;

  public:
   class ElementsList : public COL::TCopyCollection<COL::TArray<VirtualElement> > {
     private:
      typedef COL::TCopyCollection<COL::TArray<VirtualElement> > inherited;
      
     public:
      ElementsList() {}
      ElementsList(const ElementsList& source) = default;
     
      void mergeNew(VirtualElement* pveElement);
   };
   
  private:
   ElementsList selExactElements;
   ElementsList selSureElements;
   ElementsList selMayElements;

   class AddElement;
   class MergeApplyApplication;
   class MergeApplyToApplication;
   class IntersectToApplication;
   class IntersectApplication;
   class ContainApplication;
   class ContainToApplication;
   class ConstraintToApplication;
   class ConstraintUnaryApplication;
   class ConstraintBinaryApplication;
   class MergeQueryBitDomainApplication;
   class MergeQueryBoundValueApplication;
   class QueryCopyApplication;
   class QuerySpecializeApplication;
   friend class QueryCopyApplication;
   friend class QuerySpecializeApplication;

  protected:
   virtual void _add(PPVirtualElement element, const InformationKind& kind) override
      {  if (element.isValid()) {
            (kind.isExact()
               ? selExactElements : ((kind.isSure())
               ? selSureElements
               : selMayElements)).mergeNew(element.extractElement());
         };
      }

   friend class AccessDisjunction;
   const VirtualElement& getAnyExact() const
      {  return selExactElements.getElement(COL::VirtualCollection::ExtendedLocateParameters()); }
   int getExactDisjunctionNumber() const { return selExactElements.count(); }
   friend class MergeQueryBoundValueApplication;

   virtual void _write(OSBase& out, const STG::IOObject::FormatParameters& params) const override;

  public:
   template <class TypeBase>
   class TInit : public TypeBase {
     private:
      typedef TypeBase inherited;
      const FunctionQueryTable* pfqtQueryDomainTable;
      const FunctionQueryTable* pfqtQueryOperationTable;
      const FunctionQueryTable* pfqtQueryExternTable;
      
     public:
      TInit() : pfqtQueryDomainTable(nullptr), pfqtQueryOperationTable(nullptr), pfqtQueryExternTable(nullptr) {}
      TInit(const TInit<TypeBase>& source) = default;
      TInit<TypeBase>& operator=(const TInit<TypeBase>& source) = default;
      TemplateDefineCopy(TInit, TypeBase)
      DTemplateDefineAssign(TInit, TypeBase)
            
      TInit<TypeBase>& setSizeMantissa(int sizeMantissa) { return (TInit<TypeBase>&) TypeBase::setSizeMantissa(sizeMantissa); }
      TInit<TypeBase>& setSizeExponent(int sizeExponent) { return (TInit<TypeBase>&) TypeBase::setSizeExponent(sizeExponent); }
      TInit<TypeBase>& setQueryDomainTable(const FunctionQueryTable& queryDomainTable)
         {  pfqtQueryDomainTable = &queryDomainTable; return *this; }
      TInit<TypeBase>& setQueryOperationTable(const FunctionQueryTable& queryOperationTable)
         {  pfqtQueryOperationTable = &queryOperationTable; return *this; }
      TInit<TypeBase>& setQueryExternTable(const FunctionQueryTable& queryExternTable)
         {  pfqtQueryExternTable = &queryExternTable; return *this; }
      TInit<TypeBase>& setTable(const VirtualElement& element)
         {  pfqtQueryDomainTable = &element.functionQueryTable(VirtualQueryOperation::TDomain);
            pfqtQueryOperationTable = &element.functionQueryTable(VirtualQueryOperation::TOperation);
            pfqtQueryExternTable = &element.functionQueryTable(VirtualQueryOperation::TExtern);
            return *this;
         }
      TInit<TypeBase>& setTable(const Disjunction& disjunction)
         {  inherited::setBitSize(disjunction.getSizeInBits());
            pfqtQueryDomainTable = disjunction.pfqtQueryDomainTable;
            pfqtQueryOperationTable = disjunction.pfqtQueryOperationTable;
            pfqtQueryExternTable = disjunction.pfqtQueryExternTable;
            return *this;
         }
      TInit<TypeBase>& setPureTable(const Disjunction& disjunction)
         {  pfqtQueryDomainTable = disjunction.pfqtQueryDomainTable;
            pfqtQueryOperationTable = disjunction.pfqtQueryOperationTable;
            pfqtQueryExternTable = disjunction.pfqtQueryExternTable;
            return *this;
         }
      TInit<TypeBase>& setBitSize(int bitSize) { return (TInit<TypeBase>&) inherited::setBitSize(bitSize); }

      friend class Disjunction;
   };
   typedef TInit<VirtualElement::Init> Init;
   typedef TInit<VirtualElement::InitFloat> InitFloat;
   friend class TInit<VirtualElement::Init>;
   friend class TInit<VirtualElement::InitFloat>;
   
   Disjunction(const Init& init)
      :  inherited(init), pfqtQueryDomainTable(init.pfqtQueryDomainTable),
         pfqtQueryOperationTable(init.pfqtQueryOperationTable), pfqtQueryExternTable(init.pfqtQueryExternTable),
         uSizeInBits(init.getBitSize()), uSizeMantissa(0), uSizeExponent(0)
      {  AssumeCondition(pfqtQueryDomainTable) setDisjunction(); }
   Disjunction(const InitFloat& init)
      :  inherited(init), pfqtQueryDomainTable(init.pfqtQueryDomainTable),
         pfqtQueryOperationTable(init.pfqtQueryOperationTable), pfqtQueryExternTable(init.pfqtQueryExternTable),
         uSizeInBits(init.getBitSize()),
         uSizeMantissa(init.sizeMantissa()), uSizeExponent(init.sizeExponent())
      {  AssumeCondition(pfqtQueryDomainTable) setDisjunction(); }

      
   Disjunction(const Disjunction& source) = default;
   Disjunction& operator=(const Disjunction& source)
      {  EnhancedObject::operator=(source);
         selExactElements = source.selExactElements;
         selSureElements = source.selSureElements;
         selMayElements = source.selMayElements;
         return *this;
      }

   DefineCopy(Disjunction)
   DDefineAssign(Disjunction)
   virtual const FunctionQueryTable& functionQueryTable(int queryIndex) const override
      {  return (queryIndex == VirtualQueryOperation::TOperation)
            ? *pfqtQueryOperationTable : ((queryIndex == VirtualQueryOperation::TDomain)
            ? *pfqtQueryDomainTable : ((queryIndex == VirtualQueryOperation::TExtern)
            ? *pfqtQueryExternTable : mqtMethodQueryTable[queryIndex]));
      }
   virtual Disjunction* createClearCopy() const
      {  return new Disjunction(Disjunction::Init().setPureTable(*this)); }

   // construction methods
   void add(PPVirtualElement element, const InformationKind& kind) { _add(element, kind); }

   virtual int countAtomic() const override
      {  int result = 0;
         std::function<bool (const VirtualElement&)> countAtomic = [&result](const VirtualElement& source)
            {  result += source.countAtomic(); return true; };
         selExactElements.foreachDo(countAtomic);
         selMayElements.foreachDo(countAtomic);
         selSureElements.foreachDo(countAtomic);
         return result;
      }

   virtual int _getSizeInBits() const override
      {  return !selExactElements.isEmpty() ? selExactElements.getFirst().getSizeInBits()
            : (!selSureElements.isEmpty() ? selSureElements.getFirst().getSizeInBits()
            : (!selMayElements.isEmpty() ? selMayElements.getFirst().getSizeInBits() : uSizeInBits));
      }
   virtual bool isBoolean() const override
      {  return !selExactElements.isEmpty() ? selExactElements.getFirst().isBoolean()
	      : !selSureElements.isEmpty() ? selSureElements.getFirst().isBoolean() : selMayElements.getFirst().isBoolean();
      }
   virtual bool isInt() const override
      {  return !selExactElements.isEmpty() ? selExactElements.getFirst().isInt()
	      : !selSureElements.isEmpty() ? selSureElements.getFirst().isInt() : selMayElements.getFirst().isInt();
      }
   virtual bool isFloat() const override
      {  return !selExactElements.isEmpty() ? selExactElements.getFirst().isFloat()
	      : !selSureElements.isEmpty() ? selSureElements.getFirst().isFloat() : selMayElements.getFirst().isFloat();
      }
   virtual bool isMultiBit() const override
      {  return !selExactElements.isEmpty() ? selExactElements.getFirst().isMultiBit()
	      : !selSureElements.isEmpty() ? selSureElements.getFirst().isMultiBit() : selMayElements.getFirst().isMultiBit();
      }
   
   // inherited methods of VirtualElement
   virtual bool applyTo(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const override
      {  AssumeCondition(/* isValid() && */ getApproxKind().compareApply(source.getApproxKind()) >= CREqual)
         return applyToNative(operation, source, env);
      }
   virtual bool mergeWithTo(VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool containTo(const VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool intersectWithTo(VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool constraintTo(const VirtualOperation&, VirtualElement& source, const VirtualElement&, ConstraintEnvironment&) override;
   virtual ZeroResult queryZeroResult() const override;

   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override
      {  AssumeCondition(isLightValid()) 
         return (VirtualElement::apply(operation, env)) ? true : applyNative(operation, (EvaluationEnvironment&) env);
      }
   virtual bool mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) override;
   virtual bool contain(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) const override;
   virtual bool intersectWith(const VirtualElement& source, EvaluationEnvironment& env) override;
   virtual bool constraint(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&) override;
   virtual bool query(const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) const override;
   virtual bool guard(PPVirtualElement thenElement, PPVirtualElement elseElement, EvaluationEnvironment& env) override
      {  return guardApproximate(thenElement, elseElement, env); }

   bool applyToNative(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const;
   bool applyNative(const VirtualOperation& operation, EvaluationEnvironment& env);
   
   ElementsList& exacts() { return selExactElements; }
   ElementsList& sures() { return selSureElements; }
   ElementsList& mays() { return selMayElements; }
   const ElementsList& exacts() const { return selExactElements; }
   const ElementsList& sures() const { return selSureElements; }
   const ElementsList& mays() const { return selMayElements; }
   
   static bool queryCompareSpecial(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& qoeEnv);
   static bool querySimplification(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& qoeEnv);
   static bool querySimplificationAsInterval(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& qoeEnv);
   static bool querySimplificationAsConstantDisjunction(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& qoeEnv);
   static bool queryCopy(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& qoeEnv);
   static bool querySpecialize(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& qoeEnv);
   
   virtual bool isValid() const override
      {  return VirtualElement::isValid()
            && ((selExactElements.count() >= 2) || !selSureElements.isEmpty() || !selMayElements.isEmpty());
      }
   bool isLightValid() const
      {  return VirtualElement::isValid()
            && ((selExactElements.count() >= 1) || !selSureElements.isEmpty() || !selMayElements.isEmpty());
      }

   class QueryOperation : public VirtualQueryOperation {
     private:
      typedef VirtualQueryOperation inherited;
      
     public:
      QueryOperation() {}
      QueryOperation(const QueryOperation& source) = default;
   };
};

class AccessDisjunction {
  public:
   static const VirtualElement& getAnyExact(const Disjunction& source)
      {  return source.getAnyExact(); }
   static int getExactDisjunctionNumber(const Disjunction& source) 
      { return source.getExactDisjunctionNumber(); }
};

}}} // end of namespace Analyzer::Scalar::Approximate::Disjunction


#endif // Analyzer_Scalar_Approximate_DisjunctionH

