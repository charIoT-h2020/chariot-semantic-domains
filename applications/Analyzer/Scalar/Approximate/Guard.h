/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : Guard.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a class of guarded elements.
//

#ifndef Analyzer_Scalar_Approximate_GuardH
#define Analyzer_Scalar_Approximate_GuardH

#include "Analyzer/Scalar/Approximate/VirtualElement.h"

namespace Analyzer { namespace Scalar { namespace Approximate {

class Guard : public VirtualElement {
  private:
   typedef VirtualElement inherited;
   PPVirtualElement ppveCondition, ppveThen, ppveElse;
   const FunctionQueryTable* pfqtQueryDomainTable;
   const FunctionQueryTable* pfqtQueryOperationTable;
   const FunctionQueryTable* pfqtQueryExternTable;
   int uSizeInBits;
   int uSizeMantissa;
   int uSizeExponent;

   typedef VirtualElement::QueryTable InheritedQueryTable;
   class QueryTable : public InheritedQueryTable {
     private:
      typedef InheritedQueryTable inherited;
     public:
      QueryTable();
   };
   friend class QueryTable;
   static QueryTable mqtMethodQueryTable;

  protected:
   virtual void _write(OSBase& out, const STG::IOObject::FormatParameters& params) const override
      {  out.put('(');
         ppveCondition->write(out, params);
         out.writesome(") ? (");
         if (ppveThen.isValid())
            ppveThen->write(out, params);
         else
            out.writesome("nil");
         out.writesome(") : (");
         if (ppveElse.isValid())
            ppveElse->write(out, params);
         else
            out.writesome("nil");
         out.put(')');
      }

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
      TInit<TypeBase>& setTable(const Guard& guard)
         {  inherited::setBitSize(guard.getSizeInBits());
            pfqtQueryDomainTable = guard.pfqtQueryDomainTable;
            pfqtQueryOperationTable = guard.pfqtQueryOperationTable;
            pfqtQueryExternTable = guard.pfqtQueryExternTable;
            return *this;
         }
      TInit<TypeBase>& setBitSize(int bitSize) { return (TInit<TypeBase>&) inherited::setBitSize(bitSize); }

      friend class Guard;
   };
   typedef TInit<VirtualElement::Init> Init;
   typedef TInit<VirtualElement::InitFloat> InitFloat;
   friend class TInit<VirtualElement::Init>;
   friend class TInit<VirtualElement::InitFloat>;

   Guard(const Init& init)
      :  inherited(init), pfqtQueryDomainTable(init.pfqtQueryDomainTable),
         pfqtQueryOperationTable(init.pfqtQueryOperationTable), pfqtQueryExternTable(init.pfqtQueryExternTable),
         uSizeInBits(init.getBitSize()), uSizeMantissa(0), uSizeExponent(0)
      {  AssumeCondition(pfqtQueryDomainTable) setGuard(); }
   Guard(const InitFloat& init)
      :  inherited(init), pfqtQueryDomainTable(init.pfqtQueryDomainTable),
         pfqtQueryOperationTable(init.pfqtQueryOperationTable), pfqtQueryExternTable(init.pfqtQueryExternTable),
         uSizeInBits(init.getBitSize()), uSizeMantissa(init.sizeMantissa()), uSizeExponent(init.sizeExponent())
      {  AssumeCondition(pfqtQueryDomainTable) setGuard(); }
   Guard(const Guard& source)
      :  inherited(source), ppveCondition(source.ppveCondition, PNT::Pointer::Duplicate()),
         ppveThen(source.ppveThen, PNT::Pointer::Duplicate()),
         ppveElse(source.ppveElse, PNT::Pointer::Duplicate()),
         pfqtQueryDomainTable(source.pfqtQueryDomainTable),
         pfqtQueryOperationTable(source.pfqtQueryOperationTable),
         pfqtQueryExternTable(source.pfqtQueryExternTable),
         uSizeInBits(source.uSizeInBits), uSizeMantissa(source.uSizeMantissa),
         uSizeExponent(source.uSizeExponent) {}
   Guard& operator=(const Guard& source)
      {  inherited::operator=(source);
         ppveCondition.fullAssign(source.ppveCondition);
         ppveThen.fullAssign(source.ppveThen);
         ppveElse.fullAssign(source.ppveElse);
         return *this;
      }
   DefineCopy(Guard)
   DDefineAssign(Guard)

   void setCondition(PPVirtualElement condition, PPVirtualElement thenElement, PPVirtualElement elseElement)
      {  ppveCondition = condition;
         ppveThen = thenElement;
         ppveElse = elseElement;
      }

   virtual const FunctionQueryTable& functionQueryTable(int queryIndex) const override
      {  return (queryIndex == VirtualQueryOperation::TOperation)
            ? *pfqtQueryOperationTable : ((queryIndex == VirtualQueryOperation::TDomain)
            ? *pfqtQueryDomainTable : ((queryIndex == VirtualQueryOperation::TExtern)
            ? *pfqtQueryExternTable : mqtMethodQueryTable[queryIndex]));
      }
   virtual int countAtomic() const override
      {  return ppveCondition->countAtomic() * (ppveThen.isValid() ? ppveThen->countAtomic() : 1)
                  * (ppveElse.isValid() ? ppveElse->countAtomic() : 1);
      }
   virtual bool isAtomic() const override
      {  bool result = false;
         if (ppveCondition->isAtomic())
            result = (!ppveThen.isValid() || ppveThen->isAtomic())
                  && (!ppveElse.isValid() || ppveElse->isAtomic());
         else
            result = (!ppveThen.isValid() || !ppveElse->isAtomic())
                  && (!ppveThen.isValid() || ppveThen->isAtomic())
                  && (!ppveElse.isValid() || ppveElse->isAtomic());
         return result;
      }

   virtual int _getSizeInBits() const override { return uSizeInBits; }
   virtual bool applyTo(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool mergeWithTo(VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool containTo(const VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool intersectWithTo(VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool constraintTo(const VirtualOperation&, VirtualElement& source, const VirtualElement&, ConstraintEnvironment&) override;
   virtual ZeroResult queryZeroResult() const override;

   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override;
   virtual bool mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) override;
   virtual bool contain(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) const override;
   virtual bool intersectWith(const VirtualElement& source, EvaluationEnvironment& env) override;
   virtual bool constraint(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&) override;
   virtual bool query(const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) const override;

   bool applyToNative(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const;
   bool applyNative(const VirtualOperation& operation, EvaluationEnvironment& env);
   
   static bool queryGuard(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareSpecial(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool querySimplification(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool querySimplificationAsInterval(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool querySimplificationAsConstantDisjunction(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCopy(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool querySpecialize(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   
   virtual bool isValid() const override
      {  return VirtualElement::isValid()
            && (ppveCondition.isValid() && (ppveThen.isValid() || ppveElse.isValid()));
      }
   VirtualElement& getCondition() const { return *ppveCondition; }
   bool hasThen() const { return ppveThen.isValid(); }
   VirtualElement& getThen() const { return *ppveThen; }
   bool hasElse() const { return ppveElse.isValid(); }
   VirtualElement& getElse() const { return *ppveElse; }
 
   class QueryOperation : public VirtualQueryOperation {
     private:
      typedef VirtualQueryOperation inherited;
      
     public:
      QueryOperation() {}
      QueryOperation(const QueryOperation& source) = default;
   };
};

}}} // end of namespace Analyzer::Scalar::Approximate


#endif // Analyzer_Scalar_Approximate_GuardH

