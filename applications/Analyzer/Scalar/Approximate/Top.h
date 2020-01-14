/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : Top.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a class of Top.
//

#ifndef Analyzer_Scalar_Approximate_TopH
#define Analyzer_Scalar_Approximate_TopH

#include "Analyzer/Scalar/Approximate/VirtualElement.h"

namespace Analyzer { namespace Scalar { namespace Approximate {

class Top : public VirtualDisjunction {
  public:
   typedef bool (*ApplyFunction)(VirtualElement&, const VirtualOperation&, EvaluationEnvironment&);
   typedef bool (*ApplyToFunction)(const VirtualElement&, VirtualElement&, const VirtualOperation&, EvaluationEnvironment&);
   typedef bool (*ConstraintFunction)(VirtualElement&, const VirtualOperation&, const VirtualElement&,  ConstraintEnvironment&);
     
  private:
   typedef VirtualDisjunction inherited;
   int uSizeInBits;
   int uSizeMantissa;
   int uSizeExponent;
   
   class MethodApplyTable;
   class MethodApplyToTable;
   class MethodQueryTable;

   PPVirtualElement promoteIntegerInterval() const;

   class FloatQueryTable : public inherited::QueryTable {
     private:
      typedef MultiBit::Approximate::TVirtualElement<Approximate::Details::RealOperationElement>::QueryTable inherited;
     public:
      FloatQueryTable();
   };
   class IntQueryTable : public inherited::QueryTable {
     private:
      typedef MultiBit::Approximate::TVirtualElement<Approximate::Details::IntOperationElement>::QueryTable inherited;
     public:
      IntQueryTable();
   };
   
   const FunctionQueryTable* pfqtQueryOperationTable;
   const FunctionQueryTable* pfqtQueryDomainTable;
   const FunctionQueryTable* pfqtQueryExternTable;
   ApplyFunction afApplyTopFunction;
   ApplyToFunction afApplyToTopFunction;
   ConstraintFunction cfConstraintTopFunction;
   static FloatQueryTable mqtFloatMethodQueryTable;
   static IntQueryTable mqtIntMethodQueryTable;
   bool doesSupportFloatOperation() const { return uSizeMantissa > 0; }
   bool doesSupportIntOperation() const { return uSizeMantissa == 0; }
   
  protected:
   virtual void _add(PPVirtualElement element, const InformationKind& kind) override {}
   virtual void _write(OSBase& out, const STG::IOObject::FormatParameters& params) const override
      {  out << "Top " << getSizeInBits(); }
   
  public:
   template <class TypeBase>
   class TInit : public TypeBase {
     private:
      typedef TypeBase inherited;

      const FunctionQueryTable* pfqtQueryOperationTable;
      const FunctionQueryTable* pfqtQueryDomainTable;
      const FunctionQueryTable* pfqtQueryExternTable;
      ApplyFunction afApplyTopFunction;
      ApplyToFunction afApplyToTopFunction;
      ConstraintFunction cfConstraintTopFunction;
      
     public:
      TInit()
         :  pfqtQueryOperationTable(nullptr), pfqtQueryDomainTable(nullptr),
            pfqtQueryExternTable(nullptr), afApplyTopFunction(nullptr),
            afApplyToTopFunction(nullptr), cfConstraintTopFunction(nullptr) {}
      TInit(const TInit<TypeBase>& init) = default;
      TInit<TypeBase>& operator=(const TInit<TypeBase>& init) = default;
      TemplateDefineCopy(TInit, TypeBase)
      DTemplateDefineAssign(TInit, TypeBase)

      TInit<TypeBase>& set1()   { return (TInit<TypeBase>&) inherited::set1(); }
      TInit<TypeBase>& set2()   { return (TInit<TypeBase>&) inherited::set2(); }
      TInit<TypeBase>& set4()   { return (TInit<TypeBase>&) inherited::set4(); }
      TInit<TypeBase>& set8()   { return (TInit<TypeBase>&) inherited::set8(); }
      TInit<TypeBase>& set16()  { return (TInit<TypeBase>&) inherited::set16(); }
      TInit<TypeBase>& set32()  { return (TInit<TypeBase>&) inherited::set32(); }
      TInit<TypeBase>& set64()  { return (TInit<TypeBase>&) inherited::set64(); }
      TInit<TypeBase>& set128() { return (TInit<TypeBase>&) inherited::set128(); }
      TInit<TypeBase>& setBitSize(int bitSize) { return (TInit<TypeBase>&) inherited::setBitSize(bitSize); }

      TInit<TypeBase>& setTable(const VirtualElement& element)
         {  pfqtQueryOperationTable = &element.functionQueryTable(VirtualQueryOperation::TOperation);
            pfqtQueryDomainTable = &element.functionQueryTable(VirtualQueryOperation::TDomain);
            pfqtQueryExternTable = &element.functionQueryTable(VirtualQueryOperation::TExtern);
            return *this;
         }
      TInit<TypeBase>& setApplyTop(ApplyFunction applyTopFunction)
         {  afApplyTopFunction = applyTopFunction; return *this; }
      TInit<TypeBase>& setApplyToTop(ApplyToFunction applyToTopFunction)
         {  afApplyToTopFunction = applyToTopFunction; return *this; }
      TInit<TypeBase>& setConstraintTop(ConstraintFunction constraintTopFunction)
         {  cfConstraintTopFunction = constraintTopFunction; return *this; }

      friend class Top;
   };
   typedef TInit<VirtualDisjunction::Init> Init;
   typedef TInit<VirtualDisjunction::InitFloat> InitFloat;
   
   Top(const Init& init)
      :  inherited(init), uSizeInBits(init.getBitSize()), uSizeMantissa(0), uSizeExponent(0),
         pfqtQueryOperationTable(init.pfqtQueryOperationTable),
         pfqtQueryDomainTable(init.pfqtQueryDomainTable), pfqtQueryExternTable(init.pfqtQueryExternTable),
         afApplyTopFunction(init.afApplyTopFunction), afApplyToTopFunction(init.afApplyToTopFunction),
         cfConstraintTopFunction(init.cfConstraintTopFunction)
      {  setTop(); }
   Top(const InitFloat& init)
      :  inherited(init), uSizeInBits(init.getBitSize()), uSizeMantissa(init.sizeMantissa()),
         uSizeExponent(init.sizeExponent()), pfqtQueryOperationTable(init.pfqtQueryOperationTable),
         pfqtQueryDomainTable(init.pfqtQueryDomainTable), pfqtQueryExternTable(init.pfqtQueryExternTable),
         afApplyTopFunction(init.afApplyTopFunction), afApplyToTopFunction(init.afApplyToTopFunction),
         cfConstraintTopFunction(init.cfConstraintTopFunction)
      {  setTop(); }
   Top(const Top& source) = default;
   Top& operator=(const Top& source)
      {  inherited::operator=(source);
         pfqtQueryDomainTable = source.pfqtQueryDomainTable;
         return *this;
      }
   
   DefineCopy(Top)
   DDefineAssign(Top)
   virtual const FunctionQueryTable& functionQueryTable(int queryIndex) const override
      {  return (queryIndex == VirtualQueryOperation::TOperation)
            ? *pfqtQueryOperationTable : ((queryIndex == VirtualQueryOperation::TDomain)
            ? *pfqtQueryDomainTable : ((queryIndex == VirtualQueryOperation::TExtern)
            ? *pfqtQueryExternTable : (doesSupportFloatOperation()
               ? mqtFloatMethodQueryTable[queryIndex] : mqtIntMethodQueryTable[queryIndex])));
      }
      
   void setTable(const Init& init)
      {  uSizeInBits = init.getBitSize();
         uSizeMantissa = 0;
         uSizeExponent = 0;
         pfqtQueryOperationTable = init.pfqtQueryOperationTable;
         pfqtQueryDomainTable = init.pfqtQueryDomainTable;
         pfqtQueryExternTable = init.pfqtQueryExternTable;
         afApplyTopFunction = init.afApplyTopFunction;
         afApplyToTopFunction = init.afApplyToTopFunction;
         cfConstraintTopFunction = init.cfConstraintTopFunction;
      }
   void setTable(const InitFloat& init)
      {  uSizeInBits = init.getBitSize();
         uSizeMantissa = init.sizeMantissa();
         uSizeExponent = init.sizeExponent();
         pfqtQueryOperationTable = init.pfqtQueryOperationTable;
         pfqtQueryDomainTable = init.pfqtQueryDomainTable;
         pfqtQueryExternTable = init.pfqtQueryExternTable;
         afApplyTopFunction = init.afApplyTopFunction;
         afApplyToTopFunction = init.afApplyToTopFunction;
         cfConstraintTopFunction = init.cfConstraintTopFunction;
      }
   void setSizeInBits(int value) { uSizeInBits = value; }
   void addToSizeInBits(int value) { uSizeInBits += value; }
   const ApplyFunction& getApplyFunction() const { return afApplyTopFunction; }
   const ApplyToFunction& getApplyToFunction() const { return afApplyToTopFunction; }
   const ConstraintFunction& getConstraintFunction() const { return cfConstraintTopFunction; }
   virtual bool isInt() const override { return uSizeMantissa == 0 && uSizeExponent == 0; }
   virtual bool isMultiBit() const override { return uSizeMantissa == 0 && uSizeExponent == 0; }
   virtual bool isFloat() const override { return uSizeMantissa > 0 && uSizeExponent > 0; }
   virtual bool isBoolean() const override { return false; }

   virtual int _getSizeInBits() const override { return uSizeInBits; }
   virtual bool applyTo(const VirtualOperation& operation, VirtualElement&, EvaluationEnvironment& env) const override;
   virtual bool mergeWithTo(VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool containTo(const VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool intersectWithTo(VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool constraintTo(const VirtualOperation&, VirtualElement& source, const VirtualElement&, ConstraintEnvironment&) override
      {  AssumeUncalled return false; }
   virtual ZeroResult queryZeroResult() const override { return ZeroResult(); }
   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override;
   virtual bool mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) override;
   virtual bool contain(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) const override;
   virtual bool intersectWith(const VirtualElement& source, EvaluationEnvironment& env) override;
   virtual bool constraint(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&) override;
   virtual bool query(const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) const override;
   virtual bool guard(PPVirtualElement thenElement, PPVirtualElement elseElement, EvaluationEnvironment& env) override
      {  return guardApproximate(thenElement, elseElement, env); }

   /* query methods */
   static bool queryGuardAllFloat(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryGuardAllInt(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareSpecial(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryFailSimplification(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool querySimplificationIdentity(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool querySimplificationAsConstantDisjunction(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool querySimplificationAsInterval(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   bool querySizes(const VirtualQueryOperation&, VirtualQueryOperation::Environment& env) const;
   
   // all these methods are implemented by a method table for the error propagation
   /* apply methods */
      // applyTo methods
#define DefineDeclareApplyTo(TypeOperation) \
   bool applyTo##TypeOperation(const VirtualOperation&, VirtualElement&, EvaluationEnvironment&) const;
   
   DefineDeclareApplyTo(CompareIntOrdered)
   DefineDeclareApplyTo(CompareAnyResult)

   DefineDeclareApplyTo(ArithmeticAssignInt)
   DefineDeclareApplyTo(ArithmeticAssignFloat)
   DefineDeclareApplyTo(ArithmeticAssignPointer)
   DefineDeclareApplyTo(TimesAssignInt)
   DefineDeclareApplyTo(TimesAssignFloat)
   DefineDeclareApplyTo(DivideAssignFloat)

   DefineDeclareApplyTo(PromotedInteger)
   DefineDeclareApplyTo(PromotedFloat)

   DefineDeclareApplyTo(Atan2)
   DefineDeclareApplyTo(Fmod)
   DefineDeclareApplyTo(Frexp)
   DefineDeclareApplyTo(Ldexp)
   DefineDeclareApplyTo(Modf)
   DefineDeclareApplyTo(Pow)
#undef DefineDeclareApplyTo

#define DefineDeclareUnaryApply(TypeOperation) \
   bool apply##TypeOperation(const VirtualOperation&, EvaluationEnvironment&);
#define DefineDeclareBinaryApply(TypeOperation) \
   bool apply##TypeOperation(const VirtualOperation&, const VirtualElement&, EvaluationEnvironment&);
      // apply methods
   DefineDeclareUnaryApply(PromotedInteger)
   DefineDeclareUnaryApply(PromotedFloat)
   DefineDeclareUnaryApply(CastInteger)
   DefineDeclareUnaryApply(Extend)
   DefineDeclareUnaryApply(Reduce)

   DefineDeclareUnaryApply(PrevAssign)
   DefineDeclareUnaryApply(NextAssign)
   DefineDeclareBinaryApply(ArithmeticAssignInt)
   DefineDeclareBinaryApply(ArithmeticAssignFloat)
   DefineDeclareBinaryApply(ArithmeticAssignPointer)
   DefineDeclareBinaryApply(CompareIntOrdered)
   DefineDeclareUnaryApply(CompareAnyResult)
   DefineDeclareUnaryApply(QueryFloat)

   DefineDeclareBinaryApply(TimesAssignInt)
   DefineDeclareBinaryApply(TimesAssignFloat)
   DefineDeclareBinaryApply(DivideAssignFloat)
   DefineDeclareUnaryApply(Identity)
   DefineDeclareUnaryApply(Unimplemented)
#undef DefineDeclareUnaryApply
#undef DefineDeclareBinaryApply
};

}}} // end of namespace Analyzer::Scalar::Approximate

#endif // Analyzer_Scalar_Approximate_TopH
