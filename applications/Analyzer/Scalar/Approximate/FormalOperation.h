/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : FormalOperation.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a class of formal operations.
//

#ifndef Analyzer_Scalar_Approximate_FormalOperationH
#define Analyzer_Scalar_Approximate_FormalOperationH

#include "Analyzer/Scalar/Approximate/VirtualElement.h"
#include "Analyzer/Scalar/Approximate/MethodTable.h"

namespace Analyzer { namespace Scalar { namespace Approximate {

class FormalImplementation {
  protected:
   PNT::PassPointer<VirtualOperation> ppvoOperation;
   PPVirtualElement ppveFstArg;
   PPVirtualElement ppveSndArg;

  protected:
   PPVirtualElement queryTopResult() const
      {  EvaluationEnvironment env(VirtualElement::EPMayStopErrorStates);
         env.setTopLatticeCreation();
         env.setLowLevel();
         if (ppveSndArg.isValid())
            env.setFirstArgument(*ppveSndArg);
         PNT::PassPointer<VirtualOperation> operation(ppvoOperation, PNT::Pointer::Duplicate());
         operation->setConstWithAssign();
         ppveFstArg->apply(*operation, env);
         return env.presult();
      }
   int countAtomic() const
      {  return (ppveFstArg.isValid() ? ppveFstArg->countAtomic() : 1)
                  * (ppveSndArg.isValid() ? ppveSndArg->countAtomic() : 1);
      }
   bool isAtomic() const
      {  return (!ppveFstArg.isValid() || ppveFstArg->isAtomic())
                  && (!ppveSndArg.isValid() || ppveSndArg->isAtomic());
      }

  public:
   bool hasOperation() const { return ppvoOperation.isValid(); }
   void setOperation(const VirtualOperation& operation) { ppvoOperation.setElement(operation); }
   VirtualElement& fstArg() const { return *ppveFstArg; }
   VirtualElement& sndArg() const { return *ppveSndArg; }
   PPVirtualElement& sfstArg() { return ppveFstArg; }
   PPVirtualElement& ssndArg() { return ppveSndArg; }
   bool hasSndArg() const { return ppveSndArg.isValid(); }

  public:
   class Init : public VirtualElement::Init {
     private:
      typedef VirtualElement::Init inherited;
      PNT::PassPointer<VirtualOperation> ppvoOperation;
      PPVirtualElement ppveFstArg;
      PPVirtualElement ppveSndArg;

     public:
      Init() {}
      Init(const Init& source) = default;

      Init& setOperation(const VirtualOperation& operation) { ppvoOperation.setElement(operation); return *this; }
      Init& setFstArg(PPVirtualElement fstArg) { ppveFstArg = fstArg; return *this; }
      Init& setSndArg(PPVirtualElement sndArg) { ppveSndArg = sndArg; return *this; }
      friend class FormalImplementation;
   };

   FormalImplementation(const Init& init)
      :  ppvoOperation(init.ppvoOperation), ppveFstArg(init.ppveFstArg),
         ppveSndArg(init.ppveSndArg)
      {  if (ppvoOperation.isValid())
            ppvoOperation->setConst();
      }
   FormalImplementation(const FormalImplementation& source)
      :  ppvoOperation(source.ppvoOperation, PNT::Pointer::Duplicate()),
         ppveFstArg(source.ppveFstArg, PNT::Pointer::Duplicate()),
         ppveSndArg(source.ppveSndArg, PNT::Pointer::Duplicate()) {}
   FormalImplementation& operator=(const FormalImplementation& source)
      {  ppvoOperation.fullAssign(source.ppvoOperation);
         ppveFstArg.fullAssign(source.ppveFstArg);
         ppveSndArg.fullAssign(source.ppveSndArg);
         return *this;
      }

   bool isValid() const
      {  return ppvoOperation.isValid() && ppveFstArg.isValid()
            && ((ppvoOperation->getArgumentsNumber() <= 1) || ppveSndArg.isValid());
      }
   const VirtualOperation& getOperation() const { return *ppvoOperation; }
};

template <class TypeBase>
class TFormalOperationElement : public TypeBase, public FormalImplementation {
  private:
   typedef TypeBase inherited;
   typedef TFormalOperationElement<TypeBase> thisType;

  public:
   typedef FormalImplementation::Init Init;
   typedef TFormalOperationElement<TypeBase> FormalOperationElement;
   TFormalOperationElement(const Init& init)
      :  inherited(init), FormalImplementation(init) 
      {  inherited::setFormalOperation(); }
   TFormalOperationElement(const thisType& source) = default;
   TFormalOperationElement& operator=(const thisType& source) = default;
   TemplateDefineCopy(TFormalOperationElement, TypeBase)
   DTemplateDefineAssign(TFormalOperationElement, TypeBase)
   virtual bool isValid() const override
      {  return inherited::isValid() && FormalImplementation::isValid(); }
      
   virtual const VirtualElement::FunctionQueryTable& functionQueryTable(int queryIndex) const override
      {  return queryTopResult()->functionQueryTable(queryIndex); }
   virtual FormalImplementation& _asFormal() override { return *this; }

   virtual int countAtomic() const override
      {  return FormalImplementation::countAtomic(); }
   virtual bool isAtomic() const override
      {  return FormalImplementation::isAtomic(); }
   virtual int _getSizeInBits() const override { return queryTopResult()->getSizeInBits(); }
   
   virtual bool applyTo(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const override { AssumeUncalled return false; }
   virtual bool mergeWithTo(VirtualElement& source, EvaluationEnvironment& env) const override { AssumeUncalled return false; }
   virtual bool containTo(const VirtualElement& source, EvaluationEnvironment& env) const override { AssumeUncalled return false; }
   virtual bool intersectWithTo(VirtualElement& source, EvaluationEnvironment& env) const override { AssumeUncalled return false; }
   virtual bool constraintTo(const VirtualOperation&, VirtualElement& source, const VirtualElement&, ConstraintEnvironment&) override { AssumeUncalled return false; }
   virtual VirtualElement::ZeroResult queryZeroResult() const override { AssumeUncalled return VirtualElement::ZeroResult(); }

   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override { return inherited::apply(operation, env); }
   virtual bool mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) override { return inherited::mergeWith(source, env); }
   virtual bool contain(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) const override { return inherited::contain(source, env); }
   virtual bool intersectWith(const VirtualElement& source, EvaluationEnvironment& env) override { return inherited::intersectWith(source, env); }
   virtual bool constraint(const VirtualOperation& operation, const VirtualElement& source, ConstraintEnvironment& env) override { return inherited::constraint(operation, source, env); }
   virtual bool query(const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) const override
      {  bool result = inherited::query(operation, env);
         if (!result) {
            if (operation.isDuplication()) {
               if (operation.getExtensions() == VirtualQueryOperation::TCDClone)
                  return queryCopy(*this, operation, env);
               else if (operation.getExtensions() == VirtualQueryOperation::TCDSpecialize)
                  return querySpecialize(*this, operation, env);
            }
            else if (!operation.isSimplification())
               result = ppveFstArg->query(operation, env);
         };
         return result;
      }
   virtual bool guard(PPVirtualElement thenElement, PPVirtualElement elseElement, EvaluationEnvironment& env) override
      {  return inherited::guardFormal(thenElement, elseElement, env); }

   static bool queryCopy(const VirtualElement& source, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env);
   static bool querySpecialize(const VirtualElement& source, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env);
};

template <class TypeBase>
bool
TFormalOperationElement<TypeBase>::queryCopy(const VirtualElement& source, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const TFormalOperationElement<TypeBase>*>(&source) && dynamic_cast<const VirtualQueryOperation::DuplicationEnvironment*>(&aenv))
   VirtualQueryOperation::DuplicationEnvironment& env = (VirtualQueryOperation::DuplicationEnvironment&) aenv;
   TFormalOperationElement<TypeBase>* result = ((const TFormalOperationElement<TypeBase>&) source).createSCopy();
   env.presult().absorbElement(result);
   if (result->sfstArg().isValid())
      result->sfstArg()->query(VirtualQueryOperation().setSpecialize(), env);
   if (result->ssndArg().isValid())
      result->ssndArg()->query(VirtualQueryOperation().setSpecialize(), env);
   return true;
}

template <class TypeBase>
bool
TFormalOperationElement<TypeBase>::querySpecialize(const VirtualElement& source, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) {
   AssumeCondition(dynamic_cast<const TFormalOperationElement<TypeBase>*>(&source))
   TFormalOperationElement<TypeBase>& result = const_cast<TFormalOperationElement<TypeBase>&>((const TFormalOperationElement<TypeBase>&) source);
   if (result.sfstArg().isValid())
      result.sfstArg()->query(operation, env);
   if (result.ssndArg().isValid())
      result.ssndArg()->query(operation, env);
   return true;
}

} // end of namespace Approximate

namespace MultiBit { namespace Approximate {

class BaseOperationElement : public TFormalOperationElement<Approximate::Details::IntOperationElement> {
  private:
   typedef TFormalOperationElement<Approximate::Details::IntOperationElement> inherited;

  public:
   BaseOperationElement(const Init& init) : inherited(init) {}
   BaseOperationElement(const BaseOperationElement& source) : inherited(source) {}

   virtual bool query(const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) const
      {  bool result = Approximate::Details::IntOperationElement::query(operation, env);
         if (!result) {
            if (operation.isDuplication()) {
               if (operation.getExtensions() == VirtualQueryOperation::TCDClone)
                  return queryCopy(*this, operation, env);
               else if (operation.getExtensions() == VirtualQueryOperation::TCDSpecialize)
                  return querySpecialize(*this, operation, env);
            }
            else if (!operation.isSimplification()) {
               if (!operation.isDomain())
                  result = ppveFstArg->query(operation, env);
               else {
                  QueryOperation::TypeDomain domainType = ((const QueryOperation&) operation).getTypeDomain();
                  if ((domainType == QueryOperation::TDDisjunction)
                        || ((domainType >= QueryOperation::TDInterval)
                              && (domainType <= QueryOperation::TDIntForShift))
                        || ((domainType >= QueryOperation::TDZero)
                              && (domainType <= QueryOperation::TDOne))
                        || (domainType == QueryOperation::TDMin)
                        || (domainType == QueryOperation::TDMax))
                     result = false;
                  else
                     result = ppveFstArg->query(operation, env);
               };
            };
         };
         return result;
      }

   bool constraintCompareGeneric(const VirtualOperation&, VirtualElement& source,
      Argument arg, ConstraintEnvironment& env);
};

class ConcatOperationElement : public BaseOperationElement {
  private:
   typedef BaseOperationElement inherited;

  protected:
   virtual void _write(OSBase& out, const STG::IOObject::FormatParameters& params) const override
      {  out.put('(');
         fstArg().write(out, params);
         out.put('.');
         sndArg().write(out, params);
         out.put(')');
      }
   PPVirtualElement queryTopResult() const;

  public:
   typedef Scalar::MultiBit::Operation Operation;

   class Constants : public inherited::Constants, public Scalar::Approximate::Details::BaseConstants {
     private:
      typedef MultiBit::Operation Operation;

     public:
      static const int NBUnaryConstraintMethods = Operation::EndOfUnary + 3; // BitNegate & LogicalNegate
      static int convertUnaryConstraintToOperationToIndex(Operation::Type type)
         {  return (type < Operation::EndOfUnary) ? type
               : ((type == Operation::TOppositeSignedAssign) ? (Operation::EndOfUnary)
               : ((type == Operation::TOppositeFloatAssign) ? (Operation::EndOfUnary+1)
               : ((type == Operation::TBitNegateAssign) ? (Operation::EndOfUnary+2) : -1)));
         }
      static const int NBBinaryConstraintMethods
         = Operation::EndOfType - Operation::EndOfCompare + Operation::StartOfCompare;
      static int convertBinaryConstraintOperationToIndex(Operation::Type type)
         {  return (type < Operation::EndOfExtension) ? (type == Operation::TConcat ? 0 : 1)
               : ((type < Operation::EndOfBinary) ? (2 + type - Operation::StartOfBinary)
               : (2 + type - Operation::EndOfCompare + Operation::EndOfBinary - Operation::StartOfBinary));
         }

      static const int NBConstraintCompareMethods
         = Operation::EndOfCompare - Operation::TCompareLessSigned;
      static int convertCompareConstraintOperationToIndex(Operation::Type type)
         {  return type - Operation::TCompareLessSigned; }

      static const int NBApplyMethods = Operation::EndOfType;
      static int convertApplyOperationToIndex(Operation::Type type) { return type; }
      static const int NBApplyToMethods = Operation::EndOfType - Operation::EndOfUnary;
      static int convertApplyToOperationToIndex(Operation::Type type)
         { return (type >= Operation::EndOfExtension) ? (type - Operation::EndOfUnary + 2)
               : (type == Operation::TConcat ? 0 : 1);
         }
   };
   typedef Scalar::Approximate::Details::TDomainTraits<ConcatOperationElement> DomainTraits;
   
  protected:
   class ConstantAndGenericApplyMethod : public Scalar::Approximate::Details::MultiApplyMethod<DomainTraits> {
     private:
      typedef Scalar::Approximate::Details::MultiApplyMethod<DomainTraits> inherited;
      typedef bool (ConcatOperationElement::*PointerApplyConstantMethod)
         (const VirtualOperation&, const VirtualElement&, EvaluationEnvironment&);
      typedef bool (ConcatOperationElement::*PointerApplyGenericMethod)
         (const VirtualOperation&, const VirtualElement&, EvaluationEnvironment&);

      PointerApplyConstantMethod pacmConstantMethod;
      PointerApplyGenericMethod paimGenericMethod;

     public:
      ConstantAndGenericApplyMethod(PointerApplyConstantMethod constantMethod,
            PointerApplyGenericMethod genericMethod)
         :  pacmConstantMethod(constantMethod), paimGenericMethod(genericMethod) {}
      ConstantAndGenericApplyMethod(const ConstantAndGenericApplyMethod& source) = default;
      DefineCopy(ConstantAndGenericApplyMethod)
      DDefineAssign(ConstantAndGenericApplyMethod)
         
      virtual bool execute(ConcatOperationElement& object, const VirtualOperation& operation, EvaluationEnvironment& env) const override
         {  VirtualElement::ApproxKind kind = env.getFirstArgument().getApproxKind();
            return (kind.isConstant())
               ? (object.*pacmConstantMethod)((const Operation&) operation, env.getFirstArgument(), env)
               : (object.*paimGenericMethod)((const Operation&) operation, env.getFirstArgument(), env);
         }
   };

   class ConcatenationCursor {
     private:
      typedef COL::TArray<ConcatOperationElement, HandlerCopyCast<ConcatOperationElement> > Path;
      const ConcatOperationElement& ceSupport;
      Path acePathTo;
      VirtualElement* pveElement;

     public:
      ConcatenationCursor(const ConcatOperationElement& support) : ceSupport(support), pveElement(nullptr) {}
      ConcatenationCursor(const ConcatenationCursor& source) = default;

      bool setToFirst();
      bool setToNext();
      bool setToLast();
      bool setToPrevious();

      VirtualElement* freeBeforeAndReplaceRoot();
      VirtualElement* freeNextAndReplaceRoot();
      const VirtualElement& elementAt() const { AssumeCondition(pveElement) return *pveElement; }
      void replaceElementAtWith(VirtualElement* newElement);
   };
   friend class ConcatenationCursor;

  private:
   class MethodApplyTable : public Scalar::Approximate::Details::MethodApplyTable<DomainTraits> {
     public:
      MethodApplyTable();
   };
   class MethodApplyToTable : public Scalar::Approximate::Details::MethodApplyToTable<DomainTraits> {
     public:
      MethodApplyToTable();
   };
   class MethodConstraintTable : public Scalar::Approximate::Details::MethodConstraintTable<DomainTraits> {
     public:
      MethodConstraintTable();
   };

   static MethodApplyTable matMethodApplyTable;
   static MethodApplyToTable mattMethodApplyToTable;
   static MethodConstraintTable mctMethodConstraintTable;

  public:
   class QueryTable : public inherited::QueryTable {
     public:
      QueryTable();
   };
   friend class QueryTable;
   static QueryTable mqtMethodQueryTable;

  public:
   ConcatOperationElement(const Init& init) : inherited(init)
      {  if (!hasOperation())
            setOperation(ConcatOperation());
      }
   ConcatOperationElement(const ConcatOperationElement& source) = default;
   DefineCopy(ConcatOperationElement)
   virtual bool isValid() const override
      {  return inherited::isValid() && fstArg().getSizeInBits() > 0 && sndArg().getSizeInBits() > 0; }

   virtual const FunctionQueryTable& functionQueryTable(int queryIndex) const override
      {  return ((queryIndex == VirtualQueryOperation::TOperation)
               || (queryIndex == VirtualQueryOperation::TDomain) || (queryIndex == VirtualQueryOperation::TExtern))
            ? fstArg().functionQueryTable(queryIndex)
            : mqtMethodQueryTable[queryIndex];
      }
   virtual int _getSizeInBits() const override
      {  return fstArg().getSizeInBits() + sndArg().getSizeInBits(); }
   virtual bool applyTo(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool mergeWithTo(VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool containTo(const VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool intersectWithTo(VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool constraintTo(const VirtualOperation&, VirtualElement& source, const VirtualElement&, ConstraintEnvironment&) override;
   virtual ZeroResult queryZeroResult() const override
      {  ZeroResult fstResult = fstArg().queryZeroResult(), sndResult = sndArg().queryZeroResult();
         fstResult.intersectWithOnZero(sndResult);
         return fstResult;
      }

   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override;
   virtual bool mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) override;
   virtual bool contain(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) const override;
   virtual bool intersectWith(const VirtualElement& source, EvaluationEnvironment& env) override;
   virtual bool constraint(const VirtualOperation& operation, const VirtualElement& source, ConstraintEnvironment& env) override;
   virtual bool query(const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) const override;
   // virtual bool guard(PPVirtualElement thenElement, PPVirtualElement elseElement, EvaluationEnvironment& env) override
   //    {  return guardFormal(thenElement, elseElement, env); }

     // apply methods
#define DefineDeclareApply(TypeOperation) \
bool apply##TypeOperation(const VirtualOperation& operation, EvaluationEnvironment& env);
#define DefineDeclareBiApply(TypeOperation) \
bool apply##TypeOperation##Constant(const VirtualOperation& operation, const VirtualElement& source, EvaluationEnvironment& env); \
bool apply##TypeOperation(const VirtualOperation& operation, const VirtualElement& source, EvaluationEnvironment& env);

   DefineDeclareApply(Extend)
   DefineDeclareApply(Concat)
   DefineDeclareApply(Reduce)
   DefineDeclareApply(BitSet)
   DefineDeclareApply(CastShiftBit)
   DefineDeclareApply(CastIntegerSigned)
   DefineDeclareApply(CastIntegerUnsigned)
   DefineDeclareApply(CastDouble)
   DefineDeclareApply(PrevAssign)
   DefineDeclareApply(NextAssign)
   DefineDeclareApply(PlusAssign)
   DefineDeclareApply(Float)
   DefineDeclareApply(MinusAssign)
   DefineDeclareApply(CompareLessOrGreater)
   DefineDeclareApply(CompareLessOrGreaterOrEqual)
   DefineDeclareApply(CompareEqual)
   DefineDeclareApply(CompareDifferent)
   DefineDeclareApply(MinMaxAssign)
   DefineDeclareApply(TimesAssign)
   DefineDeclareApply(DivideAssign)
   DefineDeclareApply(OppositeSignedAssign)
   DefineDeclareApply(ModuloAssign)
   DefineDeclareApply(BitBinaryAssign)
   DefineDeclareApply(BitUnaryAssign)
   DefineDeclareBiApply(LeftShiftAssign)
   DefineDeclareBiApply(LogicalRightShiftAssign)
   DefineDeclareBiApply(ArithmeticRightShiftAssign)
   DefineDeclareBiApply(LeftRotateAssign)
   DefineDeclareBiApply(RightRotateAssign)

#undef DefineDeclareApply
#undef DefineDeclareBiApply

#define DefineDeclareApplyTo(TypeVirtualOperation) \
   bool applyTo##TypeVirtualOperation(const VirtualOperation&, VirtualElement&, EvaluationEnvironment&) const;
   
   DefineDeclareApplyTo(BitSet)
   DefineDeclareApplyTo(Concat)
   DefineDeclareApplyTo(CommutativeAssign)
   DefineDeclareApplyTo(Commutative)
   DefineDeclareApplyTo(MinusAssign)
   DefineDeclareApplyTo(DivideAssign)
   DefineDeclareApplyTo(ModuloAssign)
   DefineDeclareApplyTo(Float)
   DefineDeclareApplyTo(CompareLessOrGreater)
   DefineDeclareApplyTo(CompareLessOrGreaterOrEqual)
   DefineDeclareApplyTo(ShiftAssign)
   DefineDeclareApplyTo(RotateAssign)

#undef DefineDeclareApplyTo
      
     // query methods
#define DefineDeclareQuery(TypeOperation) \
static bool query##TypeOperation(const VirtualElement&, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env);

   DefineDeclareQuery(CompareSpecial)
   DefineDeclareQuery(Simplification)
   DefineDeclareQuery(SimplificationAsInterval)
   DefineDeclareQuery(SimplificationAsConstantDisjunction)
   DefineDeclareQuery(DispatchOnTopResult)
   DefineDeclareQuery(DispatchOnBothPart)

#undef DefineDeclareQuery

   /* constraint methods */
#define DefineDeclareConstraint(TypeOperation)                                                   \
bool constraint##TypeOperation(const VirtualOperation&, VirtualElement& source,                  \
      Argument arg, ConstraintEnvironment& env);

#define DefineDeclareUnaryConstraint(TypeOperation)                                              \
bool constraint##TypeOperation(const VirtualOperation&, VirtualElement& source,                  \
      Argument arg, ConstraintEnvironment& env);

   // DefineDeclareConstraint(CompareMultiple)
   // DefineDeclareConstraint(CompareLess)
   // DefineDeclareConstraint(CompareLessOrEqual)
   // DefineDeclareConstraint(CompareEqual)
   // DefineDeclareConstraint(CompareDifferent)
   // DefineDeclareConstraint(CompareGreaterOrEqual)
   // DefineDeclareConstraint(CompareGreater)
   // DefineDeclareConstraint(MinAssign)
   // DefineDeclareConstraint(MaxAssign)
   // DefineDeclareConstraint(LeftShiftAssign)
   // DefineDeclareConstraint(LogicalRightShiftAssign)
   // DefineDeclareConstraint(ArithmeticRightShiftAssign)
   // DefineDeclareConstraint(LeftRotateAssign)
   // DefineDeclareConstraint(RightRotateAssign)
#undef DefineDeclareConstraint
#undef DefineDeclareUnaryConstraint
};

class ExtendOperationElement : public BaseOperationElement {
  private:
   typedef BaseOperationElement inherited;

  protected:
   virtual void _write(OSBase& out, const STG::IOObject::FormatParameters& params) const override
      {  out.write(getOperation().getExtension(), params.isRaw());
         out << "<-";
         if (getOperation().getType() == Operation::TExtendWithSign)
            out.put('s');
         out.put('[');
         fstArg().write(out, params);
         out.put(']');
      }
   PPVirtualElement queryTopResult() const;

  public:
   typedef Scalar::MultiBit::Operation Operation;

   class Init : public BaseOperationElement::Init {
     private:
      typedef BaseOperationElement::Init inherited;
      int uExtension;
      bool fSigned;

     public:
      Init() : uExtension(0), fSigned(false) {}
      Init(const Init& source) = default;
      Init& operator=(const Init& source) = default;
      DefineCopy(Init)
      DDefineAssign(Init)

      Init& setBitSize(int bitSize) { return (Init&) inherited::setBitSize(bitSize); }
      Init& setSignedExtension(int extension) { uExtension = extension; fSigned = true; return *this; }
      Init& setUnsignedExtension(int extension) { uExtension = extension; fSigned = false; return *this; }
      friend class ExtendOperationElement;
   };

   class Constants : public inherited::Constants, public Scalar::Approximate::Details::BaseConstants {
     private:
      typedef MultiBit::Operation Operation;

     public:
      static const int NBUnaryConstraintMethods = Operation::EndOfUnary + 3; // BitNegate & LogicalNegate
      static int convertUnaryConstraintToOperationToIndex(Operation::Type type)
         {  return (type < Operation::EndOfUnary) ? type
               : ((type == Operation::TOppositeSignedAssign) ? (Operation::EndOfUnary)
               : ((type == Operation::TOppositeFloatAssign) ? (Operation::EndOfUnary+1)
               : ((type == Operation::TBitNegateAssign) ? (Operation::EndOfUnary+2) : -1)));
         }
      static const int NBBinaryConstraintMethods
         = Operation::EndOfType - Operation::EndOfCompare + Operation::StartOfCompare;
      static int convertBinaryConstraintOperationToIndex(Operation::Type type)
         {  return (type < Operation::EndOfExtension) ? (type == Operation::TConcat ? 0 : 1)
               : ((type < Operation::EndOfBinary) ? (2 + type - Operation::StartOfBinary)
               : (2 + type - Operation::EndOfCompare + Operation::EndOfBinary - Operation::StartOfBinary));
         }

      static const int NBConstraintCompareMethods
         = Operation::EndOfCompare - Operation::TCompareLessSigned;
      static int convertCompareConstraintOperationToIndex(Operation::Type type)
         {  return type - Operation::TCompareLessSigned; }

      static const int NBApplyMethods = Operation::EndOfType;
      static int convertApplyOperationToIndex(Operation::Type type) { return type; }
      static const int NBApplyToMethods = Operation::EndOfType - Operation::EndOfUnary;
      static int convertApplyToOperationToIndex(Operation::Type type)
         { return (type >= Operation::EndOfExtension) ? (type - Operation::EndOfUnary + 2)
               : (type == Operation::TConcat ? 0 : 1);
         }
   };
   typedef Scalar::Approximate::Details::TDomainTraits<ExtendOperationElement> DomainTraits;
   
  protected:
   class ConstantAndGenericApplyMethod : public Scalar::Approximate::Details::MultiApplyMethod<DomainTraits> {
     private:
      typedef Scalar::Approximate::Details::MultiApplyMethod<DomainTraits> inherited;
      typedef bool (ExtendOperationElement::*PointerApplyConstantMethod)
         (const VirtualOperation&, const VirtualElement&, EvaluationEnvironment&);
      typedef bool (ExtendOperationElement::*PointerApplyGenericMethod)
         (const VirtualOperation&, EvaluationEnvironment&);

      PointerApplyConstantMethod pacmConstantMethod;
      PointerApplyGenericMethod paimGenericMethod;

     public:
      ConstantAndGenericApplyMethod(PointerApplyConstantMethod constantMethod,
            PointerApplyGenericMethod genericMethod)
         :  pacmConstantMethod(constantMethod), paimGenericMethod(genericMethod) {}
      ConstantAndGenericApplyMethod(const ConstantAndGenericApplyMethod& source) = default;
      ConstantAndGenericApplyMethod& operator=(const ConstantAndGenericApplyMethod& source) = default;
      DefineCopy(ConstantAndGenericApplyMethod)
      DDefineAssign(ConstantAndGenericApplyMethod)
         
      virtual bool execute(ExtendOperationElement& object, const VirtualOperation& operation, EvaluationEnvironment& env) const override
         {  VirtualElement::ApproxKind kind = env.getFirstArgument().getApproxKind();
            return (kind.isConstant())
               ? (object.*pacmConstantMethod)((const Operation&) operation, env.getFirstArgument(), env)
               : (object.*paimGenericMethod)((const Operation&) operation, env);
         }
   };

   class ExtendAndGenericApplyMethod : public Scalar::Approximate::Details::MultiApplyMethod<DomainTraits> {
     private:
      typedef Scalar::Approximate::Details::MultiApplyMethod<DomainTraits> inherited;
      typedef bool (ExtendOperationElement::*PointerApplyExtendMethod)
         (const VirtualOperation&, const ExtendOperationElement&, EvaluationEnvironment&);
      typedef bool (ExtendOperationElement::*PointerApplyGenericMethod)
         (const VirtualOperation&, EvaluationEnvironment&);

      PointerApplyExtendMethod pacmExtendMethod;
      PointerApplyGenericMethod paimGenericMethod;

     public:
      ExtendAndGenericApplyMethod(PointerApplyExtendMethod extendMethod,
            PointerApplyGenericMethod genericMethod)
         :  pacmExtendMethod(extendMethod), paimGenericMethod(genericMethod) {}
      ExtendAndGenericApplyMethod(const ExtendAndGenericApplyMethod& source) = default;
      ExtendAndGenericApplyMethod& operator=(const ExtendAndGenericApplyMethod& source) = default;
      DefineCopy(ExtendAndGenericApplyMethod)
      DDefineAssign(ExtendAndGenericApplyMethod)
         
      virtual bool execute(ExtendOperationElement& object, const VirtualOperation& operation, EvaluationEnvironment& env) const override
         {  VirtualElement::ApproxKind kind = env.getFirstArgument().getApproxKind();
            bool result;
            if (kind.isFormalOperation()) {
               typedef Scalar::Approximate::Details::IntOperationElement IntOperationElement;
               AssumeCondition(dynamic_cast<const TFormalOperationElement<IntOperationElement>*>(&env.getFirstArgument()))
               const auto& argument = (const TFormalOperationElement<IntOperationElement>&) env.getFirstArgument();
               AssumeCondition(dynamic_cast<const Operation*>(&argument.getOperation()))
               Operation::Type type = ((const Operation&) argument.getOperation()).getType();
               if (type == Operation::TExtendWithZero || type == Operation::TExtendWithSign) {
                  AssumeCondition(dynamic_cast<const ExtendOperationElement*>(&env.getFirstArgument()))
                  result = (object.*pacmExtendMethod)((const Operation&) operation, (const ExtendOperationElement&) env.getFirstArgument(), env);
               }
               else
                  result = (object.*paimGenericMethod)((const Operation&) operation, env);
            }
            else
               result = (object.*paimGenericMethod)((const Operation&) operation, env);
            return result;
         }
   };

  private:
   class MethodApplyTable : public Scalar::Approximate::Details::MethodApplyTable<DomainTraits> {
     public:
      MethodApplyTable();
   };
   class MethodApplyToTable : public Scalar::Approximate::Details::MethodApplyToTable<DomainTraits> {
     public:
      MethodApplyToTable();
   };
   class MethodConstraintTable : public Scalar::Approximate::Details::MethodConstraintTable<DomainTraits> {
     public:
      MethodConstraintTable();
   };

   static MethodApplyTable matMethodApplyTable;
   static MethodApplyToTable mattMethodApplyToTable;
   static MethodConstraintTable mctMethodConstraintTable;

  public:
   class QueryTable : public inherited::QueryTable {
     public:
      QueryTable();
   };
   friend class QueryTable;
   static QueryTable mqtMethodQueryTable;

   PPVirtualElement simplify(EvaluationEnvironment& env);
   PPVirtualElement simplifyAsConcatenation(EvaluationEnvironment& env);
   PPVirtualElement simplify(EvaluationEnvironment& env) const;

  public:
   ExtendOperationElement(const Init& init) : inherited(init)
      {  if (!hasOperation()) {
            ExtendOperation operation;
            if (init.fSigned)
               operation.setExtendWithSign(init.uExtension);
            else
               operation.setExtendWithZero(init.uExtension);
            setOperation(operation);
         };
      }
   ExtendOperationElement(const ExtendOperationElement& source) = default;
   DefineCopy(ExtendOperationElement)
   DDefineAssign(ExtendOperationElement)
   virtual bool isValid() const override
      {  return inherited::isValid() && fstArg().getSizeInBits() > 0 && !hasSndArg(); }
   const ExtendOperation& getOperation() const
      {  return (const ExtendOperation&) inherited::getOperation(); }

   virtual const FunctionQueryTable& functionQueryTable(int queryIndex) const override
      {  return ((queryIndex == VirtualQueryOperation::TOperation)
               || (queryIndex == VirtualQueryOperation::TDomain) || (queryIndex == VirtualQueryOperation::TExtern))
            ? fstArg().functionQueryTable(queryIndex)
            : mqtMethodQueryTable[queryIndex];
      }
   virtual int _getSizeInBits() const override
      {  return fstArg().getSizeInBits() + getOperation().getExtension(); }
   virtual bool applyTo(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool mergeWithTo(VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool containTo(const VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool intersectWithTo(VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool constraintTo(const VirtualOperation&, VirtualElement& source, const VirtualElement&, ConstraintEnvironment&) override;
   virtual ZeroResult queryZeroResult() const override
      {  return fstArg().queryZeroResult(); }

   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override;
   virtual bool mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) override;
   virtual bool contain(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) const override;
   virtual bool intersectWith(const VirtualElement& source, EvaluationEnvironment& env) override;
   virtual bool constraint(const VirtualOperation& operation, const VirtualElement& source, ConstraintEnvironment& env) override;
   virtual bool query(const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) const override;
   // virtual bool guard(PPVirtualElement thenElement, PPVirtualElement elseElement, EvaluationEnvironment& env) override
   //    {  return guardFormal(thenElement, elseElement, env); }

     // apply methods
#define DefineDeclareApply(TypeOperation) \
bool apply##TypeOperation(const VirtualOperation& operation, EvaluationEnvironment& env);
#define DefineDeclareApplyExtend(TypeOperation) \
bool apply##TypeOperation##Extend(const VirtualOperation& operation, const ExtendOperationElement& source, EvaluationEnvironment& env);
#define DefineDeclareApplyConstant(TypeOperation) \
bool apply##TypeOperation##Constant(const VirtualOperation& operation, const VirtualElement& source, EvaluationEnvironment& env);

   DefineDeclareApply(DefaultUnaryAssign)
   DefineDeclareApply(DefaultUnary)
   DefineDeclareApply(DefaultBinaryAssign)
   DefineDeclareApply(DefaultBinary)
   DefineDeclareApply(Extend)
   DefineDeclareApply(Reduce)
   DefineDeclareApply(BitSet)
   DefineDeclareApply(AsConcatenationAssign)
   DefineDeclareApply(MinusUnsignedAssign)
   DefineDeclareApply(CastShiftBit)
   DefineDeclareApplyExtend(Compare)
   DefineDeclareApplyExtend(MinMaxAssign)
   DefineDeclareApply(OppositeSignedAssign)
   DefineDeclareApplyExtend(BitBinaryAssign)
   DefineDeclareApplyExtend(BitExclusiveOrAssign)
   DefineDeclareApply(BitNegateAssign)
   DefineDeclareApplyConstant(LeftShiftAssign)
   DefineDeclareApplyConstant(RightShiftAssign)

#undef DefineDeclareApply
#undef DefineDeclareApplyExtend
#undef DefineDeclareApplyConstant

#define DefineDeclareApplyTo(TypeVirtualOperation) \
   bool applyTo##TypeVirtualOperation(const VirtualOperation&, VirtualElement&, EvaluationEnvironment&) const;
   
   DefineDeclareApplyTo(DefaultBinary)
   DefineDeclareApplyTo(CommutativeAssign)
   DefineDeclareApplyTo(Commutative)
   DefineDeclareApplyTo(Compare)

#undef DefineDeclareApplyTo
      
     // query methods
#define DefineDeclareQuery(TypeOperation) \
static bool query##TypeOperation(const VirtualElement&, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env);

   DefineDeclareQuery(CompareSpecial)
   DefineDeclareQuery(Simplification)
   DefineDeclareQuery(SimplificationAsConstantDisjunction)
   DefineDeclareQuery(DispatchOnTopResult)

#undef DefineDeclareQuery

   /* constraint methods */
#define DefineDeclareConstraint(TypeOperation)                                                   \
bool constraint##TypeOperation(const VirtualOperation&, VirtualElement& source,                                          \
      Argument arg, ConstraintEnvironment& env);

#define DefineDeclareUnaryConstraint(TypeOperation)                                                   \
bool constraint##TypeOperation(const VirtualOperation&, VirtualElement& source,                                          \
      Argument arg, ConstraintEnvironment& env);

   // DefineDeclareConstraint(CompareMultiple)
   // DefineDeclareConstraint(CompareLess)
   // DefineDeclareConstraint(CompareLessOrEqual)
   // DefineDeclareConstraint(CompareEqual)
   // DefineDeclareConstraint(CompareDifferent)
   // DefineDeclareConstraint(CompareGreaterOrEqual)
   // DefineDeclareConstraint(CompareGreater)
   // DefineDeclareConstraint(MinAssign)
   // DefineDeclareConstraint(MaxAssign)
   // DefineDeclareConstraint(LeftShiftAssign)
   // DefineDeclareConstraint(LogicalRightShiftAssign)
   // DefineDeclareConstraint(ArithmeticRightShiftAssign)
   // DefineDeclareConstraint(LeftRotateAssign)
   // DefineDeclareConstraint(RightRotateAssign)
#undef DefineDeclareConstraint
#undef DefineDeclareUnaryConstraint
};

class MinusOperationElement : public BaseOperationElement {
  private:
   typedef BaseOperationElement inherited;

  protected:
   virtual void _write(OSBase& out, const STG::IOObject::FormatParameters& params) const override
      {  out.put('(');
         fstArg().write(out, params);
         out << " -";
         if (((const Operation&) getOperation()).getType() == Operation::TMinusSignedAssign)
            out << "signed ";
         else
            out << "unsigned ";
         sndArg().write(out, params);
         out.put(')');
      }

  public:
   typedef MultiBit::Operation Operation;

   class Constants : public inherited::Constants, public Scalar::Approximate::Details::BaseConstants {
     private:
      typedef MultiBit::Operation Operation;

     public:
      static const int NBUnaryConstraintMethods = Operation::EndOfUnary + 3; // BitNegate & LogicalNegate
      static int convertUnaryConstraintToOperationToIndex(Operation::Type type)
         {  return (type < Operation::EndOfUnary) ? type
               : ((type == Operation::TOppositeSignedAssign) ? (Operation::EndOfUnary)
               : ((type == Operation::TOppositeFloatAssign) ? (Operation::EndOfUnary+1)
               : ((type == Operation::TBitNegateAssign) ? (Operation::EndOfUnary+2) : -1)));
         }
      static const int NBBinaryConstraintMethods
         = Operation::EndOfType - Operation::EndOfCompare + Operation::StartOfCompare;
      static int convertBinaryConstraintOperationToIndex(Operation::Type type)
         {  return (type < Operation::EndOfExtension) ? (type == Operation::TConcat ? 0 : 1)
               : ((type < Operation::EndOfBinary) ? (2 + type - Operation::StartOfBinary)
               : (2 + type - Operation::EndOfCompare + Operation::EndOfBinary - Operation::StartOfBinary));
         }

      static const int NBConstraintCompareMethods
         = Scalar::MultiBit::Operation::EndOfCompare - Scalar::MultiBit::Operation::TCompareLessSigned;
      static int convertCompareConstraintOperationToIndex(Scalar::MultiBit::Operation::Type type)
         {  return type - Scalar::MultiBit::Operation::TCompareLessSigned; }

      static const int NBApplyMethods = Operation::EndOfType;
      static int convertApplyOperationToIndex(Operation::Type type) { return type; }
      static const int NBApplyToMethods = Operation::EndOfType - Operation::EndOfUnary;
      static int convertApplyToOperationToIndex(Operation::Type type)
         { return (type >= Operation::EndOfExtension) ? (type - Operation::EndOfUnary + 2)
               : (type == Operation::TConcat ? 0 : 1);
         }
   };
   typedef Scalar::Approximate::Details::TDomainTraits<MinusOperationElement> DomainTraits;
   
  private:
   class MethodApplyTable : public Scalar::Approximate::Details::MethodApplyTable<DomainTraits> {
     public:
      MethodApplyTable();
   };
   class MethodApplyToTable : public Scalar::Approximate::Details::MethodApplyToTable<DomainTraits> {
     public:
      MethodApplyToTable();
   };
   class MethodConstraintTable : public Scalar::Approximate::Details::MethodConstraintTable<DomainTraits> {
     public:
      MethodConstraintTable();
   };

   static MethodApplyTable matMethodApplyTable;
   static MethodApplyToTable mattMethodApplyToTable;
   static MethodConstraintTable mctMethodConstraintTable;

  public:
   class QueryTable : public inherited::QueryTable {
     public:
      QueryTable();
   };
   friend class QueryTable;
   static QueryTable mqtMethodQueryTable;

  public:
   MinusOperationElement(const Init& init) : inherited(init) {}
   MinusOperationElement(const MinusOperationElement& source) = default;
   DefineCopy(MinusOperationElement)
   virtual bool isValid() const override
      {  return inherited::isValid() && fstArg().getSizeInBits() > 0 && sndArg().getSizeInBits() > 0; }

   virtual const FunctionQueryTable& functionQueryTable(int queryIndex) const override
      {  return ((queryIndex == VirtualQueryOperation::TOperation)
               || (queryIndex == VirtualQueryOperation::TDomain) || (queryIndex == VirtualQueryOperation::TExtern))
            ? fstArg().functionQueryTable(queryIndex)
            : mqtMethodQueryTable[queryIndex];
      }
   virtual int _getSizeInBits() const override { return fstArg().getSizeInBits(); }
   virtual bool applyTo(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool mergeWithTo(VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool containTo(const VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool intersectWithTo(VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool constraintTo(const VirtualOperation&, VirtualElement& source, const VirtualElement&, ConstraintEnvironment&) override;
   virtual ZeroResult queryZeroResult() const override
      {  ZeroResult fstResult = fstArg().queryZeroResult(), sndResult = sndArg().queryZeroResult();
         fstResult.intersectWithOnZero(sndResult);
         return fstResult;
      }

   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override;
   virtual bool mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) override;
   virtual bool contain(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) const override;
   virtual bool intersectWith(const VirtualElement& source, EvaluationEnvironment& env) override;
   virtual bool constraint(const VirtualOperation& operation, const VirtualElement& source, ConstraintEnvironment& env) override;
   virtual bool query(const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) const override;
   // virtual bool guard(PPVirtualElement thenElement, PPVirtualElement elseElement, EvaluationEnvironment& env) override
   //    {  return guardFormal(thenElement, elseElement, env); }

     // apply methods
#define DefineDeclareApply(TypeOperation) \
bool apply##TypeOperation(const VirtualOperation& operation, EvaluationEnvironment& env);
#define DefineDeclareBiApply(TypeOperation) \
bool apply##TypeOperation##Constant(const VirtualOperation& operation, const VirtualElement& source, EvaluationEnvironment& env); \
bool apply##TypeOperation(const VirtualOperation& operation, const VirtualElement& source, EvaluationEnvironment& env);

   DefineDeclareApply(DefaultAssign)
   DefineDeclareApply(Default)
   DefineDeclareApply(Extend)
   DefineDeclareApply(Concat)
   DefineDeclareApply(Reduce)
   DefineDeclareApply(NextAssign)
   DefineDeclareApply(PrevAssign)
   DefineDeclareApply(MinusAssign)
   DefineDeclareApply(PlusAssign)
   DefineDeclareApply(Compare)
   DefineDeclareApply(OppositeSignedAssign)   

#undef DefineDeclareApply
#undef DefineDeclareBiApply

#define DefineDeclareApplyTo(TypeVirtualOperation) \
   bool applyTo##TypeVirtualOperation(const VirtualOperation&, VirtualElement&, EvaluationEnvironment&) const;
   
   DefineDeclareApplyTo(Default)
   DefineDeclareApplyTo(MinusAssign)
   DefineDeclareApplyTo(Concat)
   DefineDeclareApplyTo(CommutativeAssign)
   DefineDeclareApplyTo(Commutative)
   DefineDeclareApplyTo(Compare)

#undef DefineDeclareApplyTo
      
     // query methods
#define DefineDeclareQuery(TypeOperation) \
static bool query##TypeOperation(const VirtualElement&, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env);

   DefineDeclareQuery(CompareSpecial)
   DefineDeclareQuery(Simplification)
   DefineDeclareQuery(SimplificationAsConstantDisjunction)
   DefineDeclareQuery(DispatchOnTopResult)
   DefineDeclareQuery(DispatchOnBothPart)

#undef DefineDeclareQuery

   /* constraint methods */
#define DefineDeclareCompareConstraint(TypeOperation)                                            \
bool constraintCompare##TypeOperation(const VirtualOperation&, VirtualElement& source,           \
      Argument arg, ConstraintEnvironment& env);

   // DefineDeclareCompareConstraint(Generic)
   // DefineDeclareConstraint(MinAssign)
   // DefineDeclareConstraint(MaxAssign)
   // DefineDeclareConstraint(LeftShiftAssign)
   // DefineDeclareConstraint(LogicalRightShiftAssign)
   // DefineDeclareConstraint(ArithmeticRightShiftAssign)
   // DefineDeclareConstraint(LeftRotateAssign)
   // DefineDeclareConstraint(RightRotateAssign)
#undef DefineDeclareCompareConstraint
};

}} // end of namespace MultiBit::Approximate

}} // end of namespace Analyzer::Scalar


#endif // Analyzer_Scalar_Approximate_FormalOperationH

