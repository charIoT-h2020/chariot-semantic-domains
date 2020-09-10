/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : ConstantMultiBitShare.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of an approximated integer class for multibit operations
//     but existentially quantified. It represents only one value but
//     still unknown. This value may be shared with other values which expresses
//     relational information.
//   This definition relies on independent host analyses.
//

#ifndef Analyzer_Scalar_Approximate_ConstantMultiBit_SHAREH
#define Analyzer_Scalar_Approximate_ConstantMultiBit_SHAREH

#include "Analyzer/Scalar/Approximate/General/ConstantMultiBit.h"
#include "Analyzer/Scalar/Approximate/Disjunction.h"
#include "Analyzer/Scalar/Approximate/FormalOperation.h"

namespace Analyzer { namespace Scalar { namespace Approximate {

class VirtualExactToApproximateElement : public VirtualElement {
  public:
   class SharedApproximate : public PNT::SharedElement {
     private:
      typedef PNT::SharedElement inherited;
      PPVirtualElement ppveShared;
      long int uIdentifier;
      static int uSharedIdentifier; // [TODO] not reentrant

     protected:
      void notifyUpdate(PNT::SharedPointer::Notification& notification) const
         {  inherited::notifyUpdate(notification); }

     public:
      SharedApproximate(PPVirtualElement shared)
         : ppveShared(shared), uIdentifier(++uSharedIdentifier) {}
      SharedApproximate(const SharedApproximate& source)
         : inherited(source), ppveShared(source.ppveShared, PNT::Pointer::Duplicate()),
           uIdentifier(++uSharedIdentifier) {}
      SharedApproximate& operator=(const SharedApproximate& source)
         {  inherited::operator=(source);
            ppveShared.fullAssign(source.ppveShared);
            return *this;
         }
      DefineCopy(SharedApproximate)
      DDefineAssign(SharedApproximate)

      const VirtualElement& shared() const { return *ppveShared; }
      long int getIdentifier() const { return uIdentifier; }
      VirtualElement& sshared() { return *ppveShared; }
      void setShared(PPVirtualElement shared) { ppveShared = shared; }
      PPVirtualElement extractShared() { return ppveShared; }
      void moveAllCallsTo(SharedApproximate& destination)
         {  inherited::moveAllCallsTo(destination); }
      friend class VirtualExactToApproximateElement;
   };

   class SpecializationNotification;
   class SharedApproximatePointer : public PNT::TSharedPointer<SharedApproximate> {
     private:
      typedef PNT::TSharedPointer<SharedApproximate> inherited;
      PNT::AutoPointer<Memory::VirtualMemoryModifier> apvmcParent;

     protected:
      virtual ComparisonResult _compare(const EnhancedObject& asource) const override
         {  ComparisonResult result = inherited::_compare(asource);
            if (result == CREqual) {
               const SharedApproximatePointer& source = (const SharedApproximatePointer&) castFromCopyHandler(asource);
               result = tcompare(key(), source.key());
            };
            return result;
         }
      friend class VirtualExactToApproximateElement::SpecializationNotification;

     public:
      SharedApproximatePointer() {}
      SharedApproximatePointer(SharedApproximate* sharedApproximate, Init init)
         :  inherited(sharedApproximate, init) {}
      SharedApproximatePointer(const SharedApproximatePointer& source) : inherited(source) {}
      SharedApproximatePointer& operator=(const SharedApproximatePointer& source)
         {  inherited::fullAssign(source); return *this; }
      virtual ~SharedApproximatePointer() { inherited::release(); }
      DefineCopy(SharedApproximatePointer)
      DDefineAssign(SharedApproximatePointer)

      bool isEqual(const SharedApproximatePointer& source) const
         {  return key() == source.key(); }
      SharedApproximatePointer& assign(SharedApproximate* domain, Init init)
         {  if (domain != key())
               inherited::release();
            return (SharedApproximatePointer&) inherited::assign(domain, init);
         }
      void setParent(const Memory::VirtualMemoryModifier& parent) { apvmcParent.setElement(parent); }
      bool hasParent() const { return apvmcParent.isValid(); }
      Memory::VirtualMemoryModifier& parent() const { return *apvmcParent; }
      void clear() { inherited::release(); }
      void release() { inherited::release(); }
   };

   class SpecializationNotification : public PNT::SharedPointer::Notification {
     private:
      typedef PNT::SharedPointer::Notification inherited;
      SharedApproximate* sapSpecializedDomain;
      Memory::VirtualModificationDate* pvmdModificationDate;
      
     protected:
      const SharedApproximatePointer& getOrigin() const
         {  const PNT::SharedPointer* result = inherited::getOrigin();
            AssumeCondition(dynamic_cast<const SharedApproximatePointer*>(result))
            return *(const SharedApproximatePointer*) result;
         }

     protected:
      virtual void _update(PNT::SharedPointer& call) override;

     public:
      SpecializationNotification(const SharedApproximatePointer& origin, Memory::VirtualModificationDate& modificationDate)
         :  inherited(origin), sapSpecializedDomain(nullptr), pvmdModificationDate(&modificationDate) {}
      SpecializationNotification(const SpecializationNotification& source) = default;
      SpecializationNotification& operator=(const SpecializationNotification& source) = default;
      DefineCopy(SpecializationNotification)
      DDefineAssign(SpecializationNotification)
      friend class VirtualExactToApproximateElement;
   };

  private:
   typedef VirtualElement inherited;
   SharedApproximatePointer sapDomain;

   void assumeSingle()
      {  if (!sapDomain->isSingleReferenced())
            sapDomain.assign(new SharedApproximate(*sapDomain), PNT::Pointer::Init());
      }
   SharedApproximate* specializeOthers(Memory::VirtualModificationDate* modificationDate, SharedApproximate* result=nullptr)
      {  if (!sapDomain->isSingleReferenced()) {
            if (modificationDate) {
               SpecializationNotification specializationNotification(sapDomain, *modificationDate);
               specializationNotification.sapSpecializedDomain = result;
               sapDomain->notifyUpdate(specializationNotification);
               result = specializationNotification.sapSpecializedDomain;
            }
            else
               result = new SharedApproximate(*sapDomain);
         };
         return result;
      }
   PPVirtualElement simplify();

  protected:
   virtual void _write(OSBase& out, const STG::IOObject::FormatParameters& params) const override
      {  out.writesome("(exist t_").write(sapDomain.isValid()
               ? sapDomain->getIdentifier() : 0, false).writesome(" in ");
         sapDomain->shared().write(out, params);
         out.put(')');
      }
  public:
   typedef Memory::VirtualMemoryModifier VirtualMemoryModifier;
   VirtualExactToApproximateElement(const Init& init, PPVirtualElement shared /* , VirtualMemoryModifier* parent=nullptr */);
   VirtualExactToApproximateElement(const VirtualExactToApproximateElement& source);
   virtual ~VirtualExactToApproximateElement() { sapDomain.release(); }
   VirtualExactToApproximateElement& operator=(const VirtualExactToApproximateElement& source)
      {  inherited::operator=(source);
         sapDomain = source.sapDomain;
         return *this;
      }
   DefineCopy(VirtualExactToApproximateElement)
   DDefineAssign(VirtualExactToApproximateElement)

   VirtualElement& domain() const { return sapDomain->sshared(); }
   bool doesPointsTo(const PNT::SharedPointer* caller) const { return sapDomain.key() == caller->key(); }

   bool isEqual(const VirtualExactToApproximateElement& source) const
      {  return sapDomain.isEqual(source.sapDomain); }
   void setParent(const VirtualMemoryModifier& parent) { sapDomain.setParent(parent); }
   virtual bool mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) override;
   virtual bool mergeWithTo(VirtualElement& source, EvaluationEnvironment& env) const override
      {  return source.mergeWith(sapDomain->sshared(), env); }
   bool sapply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env)
      {  if (!operation.isConst())
            assumeSingle();
         bool result = sapDomain->sshared().apply(operation, env);
         if (result && !operation.isConst() && env.hasResult())
            sapDomain->setShared(env.presult());
         if (result && !operation.isConst()) {
            PPVirtualElement simplification = simplify();
            if (simplification.isValid())
               env.presult() = simplification;
         };
         return result;
      }
   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override
      {  bool result = inherited::apply(operation, env);
         if (!result) {
            if (!operation.isConst())
               assumeSingle();
            result = sapDomain->sshared().apply(operation, env);
            if (result && !operation.isConst() && env.hasResult())
               sapDomain->setShared(env.presult());
            if (result && !operation.isConst()) {
               PPVirtualElement simplification = simplify();
               if (simplification.isValid())
                  env.presult() = simplification;
            };
         };
         return result;
      }

   bool sapplyTo(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const
      {  const VirtualElement& argument = (const VirtualElement&) env.getFirstArgument();
         env.setFirstArgument(sapDomain->shared());
         bool result = source.apply(operation, env);
         env.setFirstArgument(argument);
         if (env.hasResult() && ((const VirtualElement&) *env.presult()).isAtomic()) {
            PNT::TPassPointer<VirtualExactToApproximateElement, PPAbstractElement> resultElement(*this);
            resultElement->assumeSingle();
            resultElement->sapDomain->setShared(env.presult());
            PPVirtualElement simplification = resultElement->simplify();
            if (simplification.isValid())
               env.presult() = simplification;
            else
               env.presult() = resultElement;
         };
         return result;
      }
   virtual bool applyTo(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const override
      {  const VirtualElement& argument = (const VirtualElement&) env.getFirstArgument();
         env.setFirstArgument(sapDomain->shared());
         bool result = source.apply(operation, env);
         env.setFirstArgument(argument);
         if (env.hasResult() && ((const VirtualElement&) *env.presult()).isAtomic()) {
            PNT::TPassPointer<VirtualExactToApproximateElement, PPAbstractElement> resultElement(*this);
            resultElement->assumeSingle();
            resultElement->sapDomain->setShared(env.presult());
            PPVirtualElement simplification = resultElement->simplify();
            if (simplification.isValid())
               env.presult() = simplification;
            else
               env.presult() = resultElement;
         };
         return result;
      }
   virtual int _getSizeInBits() const override { return sapDomain->shared().getSizeInBits(); }
   virtual bool contain(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) const override;
   virtual Control::AbstractType* createType() const override { return sapDomain->shared().createType(); }
   virtual int countAtomic() const override { return 1; }
   virtual bool query(const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) const override
      {  bool result = inherited::query(operation, env);
         if (!result) {
            if (operation.isDuplication()) {
               if (operation.getExtensions() == VirtualQueryOperation::TCDClone)
                  return queryCopy(*this, operation, env);
               else if (operation.getExtensions() == VirtualQueryOperation::TCDSpecialize)
                  return querySpecialize(*this, operation, env);
            }
            else {
               if (!operation.isSimplification())
                  result = sapDomain->shared().query(operation, env);
               else {
                  AssumeCondition(dynamic_cast<const SimplificationEnvironment*>(&env))
                  SimplificationEnvironment& resultEnv = (SimplificationEnvironment&) env;
                  if (operation.getExtensions() == 0) {
                     PPVirtualElement simplification = const_cast<VirtualExactToApproximateElement&>(*this).simplify();
                     if (simplification.isValid())
                         resultEnv.presult() = simplification;
                  }
                  else {
                     PPVirtualElement copy(sapDomain->shared());
                     result = copy->query(operation, env);
                     if (!resultEnv.hasResult() && !resultEnv.hasFailed() && !resultEnv.isEmpty())
                        resultEnv.presult() = copy;
                  };
               };
            }
         };
         return result;
      }
   virtual bool containTo(const VirtualElement& source, EvaluationEnvironment& env) const override
      {  bool result = inherited::containTo(source, env);
         if (!result) {
            result = source.contain(sapDomain->shared(), env);
            if (env.isTotalApplied())
               env.setPartialApplied();
         };
         return result;
      }
   virtual bool intersectWith(const VirtualElement& source, EvaluationEnvironment& env) override;
   virtual bool intersectWithTo(VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual ZeroResult queryZeroResult() const override { return sapDomain->shared().queryZeroResult(); }
   virtual bool constraint(const VirtualOperation& operation, const VirtualElement& result, ConstraintEnvironment& env) override;
   virtual bool constraintTo(const VirtualOperation& operation, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment& env) override;
   virtual bool guard(PPVirtualElement thenResult, PPVirtualElement elseResult, EvaluationEnvironment& env) override
      {  bool result = inherited::guard(thenResult, elseResult, env);
         if (!result) {
            assumeSingle();
            result = sapDomain->sshared().guard(thenResult, elseResult, env);
            if (result && env.hasResult() && !((const VirtualElement&) *env.presult()).getType().isConstant())
               sapDomain->setShared(env.presult());
         };
         return result;
      }
   virtual const FunctionQueryTable& functionQueryTable(int queryIndex) const override
      {  return sapDomain->shared().functionQueryTable(queryIndex); }
   virtual bool isBoolean() const override { return sapDomain->shared().isBoolean(); }
   virtual bool isMultiBit() const override { return sapDomain->shared().isMultiBit(); }
   virtual bool isInt() const override { return sapDomain->shared().isInt(); }
   virtual bool isFloat() const override { return sapDomain->shared().isFloat(); }
   
   static bool queryCopy(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment&);
   static bool querySpecialize(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment&);
};

namespace Details {

template <class TypeBase>
class TCompareOperationElement : public TFormalOperationElement<TypeBase> {
  public:
   typedef Scalar::MultiBit::Operation MultiBitOperation;
   typedef Scalar::Approximate::Details::IntOperationElement IntOperationElement;

  private:
   typedef TFormalOperationElement<TypeBase> inherited;
   typedef TCompareOperationElement<TypeBase> thisType;

  protected:
   virtual void _write(STG::IOObject::OSBase& out, const STG::IOObject::FormatParameters& params) const override;

  protected:
   MultiBitOperation::Type getOperationType() const
      {  return ((const MultiBitOperation&) inherited::getOperation()).getType(); }

   virtual PPVirtualElement convertArgumentTopToResult(PPVirtualElement multiBitTop) const { return multiBitTop; }

  public:
   TCompareOperationElement(const typename inherited::Init& iInit, MultiBitOperation::Type compareType) : inherited(iInit)
      {  AssumeCondition(compareType >= MultiBitOperation::StartOfCompare && compareType < MultiBitOperation::EndOfCompare)
         if (!inherited::hasOperation()) {
            MultiBitOperation comparisonOperation;
            comparisonOperation.setType(compareType);
            comparisonOperation.setArgumentsNumber(1);
            comparisonOperation.setConst();
            inherited::setOperation(comparisonOperation);
         };
      }
   TCompareOperationElement(const thisType& source) : inherited(source) {}
   TemplateDefineCopy(TCompareOperationElement, TypeBase)
   
   VirtualElement& fstArg() const { return inherited::fstArg(); }
   VirtualElement& sndArg() const { return inherited::sndArg(); }
   PPVirtualElement& sfstArg() { return inherited::sfstArg(); }
   PPVirtualElement& ssndArg() { return inherited::ssndArg(); }
   const VirtualOperation& getOperation() const { return inherited::getOperation(); }
   
   virtual int countAtomic() const override { return fstArg().countAtomic() * sndArg().countAtomic(); }
   virtual bool isAtomic() const override { return fstArg().isAtomic() && sndArg().isAtomic(); }
   virtual bool isValid() const override
      {  return inherited::isValid() && fstArg().getSizeInBits() > 0 && sndArg().getSizeInBits() > 0; }

   virtual int _getSizeInBits() const override { return 1; }
   virtual bool mergeWithTo(VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool containTo(const VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool intersectWithTo(VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual AbstractElement::ZeroResult queryZeroResult() const override;

   virtual bool mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) override;
   virtual bool contain(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) const override;
   virtual bool intersectWith(const VirtualElement& source, EvaluationEnvironment& env) override;

     // Méthodes apply
#define DefineDeclareApply(TypeOperation) \
bool apply##TypeOperation(const VirtualOperation& operation, EvaluationEnvironment& env);
#define DefineDeclareBiApply(TypeOperation) \
bool apply##TypeOperation##Constant(const VirtualOperation& operation, const VirtualElement& source, EvaluationEnvironment& env); \
bool apply##TypeOperation(const VirtualOperation& operation, const VirtualElement& source, EvaluationEnvironment& env);

   DefineDeclareApply(Top)
   DefineDeclareApply(True)
   DefineDeclareApply(MultiBitTrue)
   DefineDeclareApply(False)
   DefineDeclareApply(MultiBitFalse)
   DefineDeclareApply(Identity)
   DefineDeclareApply(CastIntegerSigned)
   DefineDeclareApply(CastIntegerUnsigned)
   DefineDeclareApply(NegateAssign)
   DefineDeclareApply(PlusAssign)
   DefineDeclareApply(MinMaxAssign)
   DefineDeclareApply(OrAssign)
   DefineDeclareApply(AndAssign)
#undef DefineDeclareApply
#undef DefineDeclareBiApply

#define DefineDeclareApplyTo(TypeVirtualOperation) \
   bool applyTo##TypeVirtualOperation(const VirtualOperation&, VirtualElement&, EvaluationEnvironment&) const;
   
   DefineDeclareApplyTo(CommutativeAssign)
   DefineDeclareApplyTo(Commutative)

#undef DefineDeclareApplyTo
      
     // Méthodes query
#define DefineDeclareQuery(TypeOperation) \
static bool query##TypeOperation(const VirtualElement&, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env);

   DefineDeclareQuery(CompareSpecial)
   DefineDeclareQuery(Simplification)
   DefineDeclareQuery(DispatchOnTopResult)

#undef DefineDeclareQuery

   /* Méthodes de contraintes */
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

}} // end of namespace Approximate::Details

namespace MultiBit { namespace Approximate {

class CompareOperationElement;

}} // end of namespace MultiBit::Approximate

namespace Bit { namespace Approximate {

class BaseCompareOperationElement : public Scalar::Approximate::Details::TCompareOperationElement<Details::BitElement> {
  private:
   typedef Scalar::Approximate::Details::TCompareOperationElement<Details::BitElement> inherited;

  public:
   typedef Scalar::MultiBit::Operation MultiBitOperation;
   BaseCompareOperationElement(const Init& init, MultiBitOperation::Type compareType) : inherited(init, compareType) {}
   BaseCompareOperationElement(const BaseCompareOperationElement& source) : inherited(source) {}

   virtual bool query(const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) const
      {  bool result = Details::BitElement::query(operation, env);
         if (!result) {
            if (operation.isDuplication()) {
               if (operation.getExtensions() == VirtualQueryOperation::TCDClone)
                  return queryCopy(*this, operation, env);
               else if (operation.getExtensions() == VirtualQueryOperation::TCDSpecialize)
                  return querySpecialize(*this, operation, env);
            }
            else if (!operation.isSimplification() && !operation.isDomain())
               result = ppveFstArg->query(operation, env);
         };
         return result;
      }
};

class CompareOperationElement : public BaseCompareOperationElement {
  private:
   typedef BaseCompareOperationElement inherited;

  public:
   typedef Bit::Operation Operation;

   class Constants : public Scalar::Approximate::Details::BaseConstants {
     private:
      typedef Bit::Operation Operation;

     public:
      static const int NBApplyMethods = Operation::EndOfType;
      static int convertApplyOperationToIndex(Operation::Type type) { return type; }
      static const int NBApplyToMethods = Operation::EndOfType-1-Operation::EndOfUnary;
      static int convertApplyToOperationToIndex(Operation::Type type) { return type-Operation::EndOfUnary; }
      
      static const int NBConstraintCompareMethods
         = Operation::EndOfCompare - Operation::StartOfCompare;
      static int convertCompareConstraintOperationToIndex(Operation::Type type)
         {  return type - Operation::StartOfCompare; }
      
      static const int NBBinaryConstraintMethods
         = Operation::EndOfType - Operation::EndOfUnary - NBConstraintCompareMethods;
      static int convertBinaryConstraintOperationToIndex(Operation::Type type)
         {  return (type < Operation::EndOfBinary) ? (type - Operation::EndOfUnary)
               : (type - Operation::EndOfCompare
                     + (Operation::EndOfBinary - Operation::EndOfUnary));
         }

      static const int NBUnaryConstraintMethods = Operation::EndOfUnary + 1;
      static int convertUnaryConstraintToOperationToIndex(Operation::Type type)
         {  return (type < Operation::EndOfUnary) ? type
               : ((type == Operation::TNegateAssign) ? Operation::EndOfUnary : -1);
         }
   };
   typedef Scalar::Approximate::Details::TDomainTraits<CompareOperationElement> DomainTraits;
   
  protected:
   class ConstantAndGenericApplyMethod : public Scalar::Approximate::Details::MultiApplyMethod<DomainTraits> {
     private:
      typedef Scalar::Approximate::Details::MultiApplyMethod<DomainTraits> inherited;
      typedef bool (CompareOperationElement::*PointerApplyConstantMethod)
         (const VirtualOperation&, const VirtualElement&, EvaluationEnvironment&);
      typedef bool (CompareOperationElement::*PointerApplyGenericMethod)
         (const VirtualOperation&, const VirtualElement&, EvaluationEnvironment&);

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
         
      virtual bool execute(CompareOperationElement& object, const VirtualOperation& operation, EvaluationEnvironment& env) const override
         {  VirtualElement::ApproxKind kind = env.getFirstArgument().getType();
            return (kind.isConstant())
               ? (object.*pacmConstantMethod)((const Operation&) operation, env.getFirstArgument(), env)
               : (object.*paimGenericMethod)((const Operation&) operation, env.getFirstArgument(), env);
         }
   };

   virtual PPVirtualElement convertArgumentTopToResult(PPVirtualElement multiBitTop) const override
      {  EvaluationEnvironment env = EvaluationEnvironment(EvaluationEnvironment::Init());
         inherited::assume(multiBitTop->apply(Scalar::MultiBit::CastBitOperation(), env));
         return env.hasResult() ? env.presult() : multiBitTop;
      }

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
   typedef inherited::Init Init;
   CompareOperationElement(const Init& init, MultiBitOperation::Type compareType) : inherited(init, compareType) {}
   CompareOperationElement(const CompareOperationElement& source) : inherited(source) {}
   DefineCopy(CompareOperationElement)

   virtual const FunctionQueryTable& functionQueryTable(int queryIndex) const override
      {  return ((queryIndex == VirtualQueryOperation::TOperation)
               || (queryIndex == VirtualQueryOperation::TDomain) || (queryIndex == VirtualQueryOperation::TExtern))
            ? fstArg().functionQueryTable(queryIndex)
            : mqtMethodQueryTable[queryIndex];
      }
   virtual bool applyTo(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool constraintTo(const VirtualOperation&, VirtualElement& source, const VirtualElement&, ConstraintEnvironment&) override;
   virtual bool intersectWith(const VirtualElement& source, EvaluationEnvironment& env) override;

   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override;
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

   DefineDeclareApply(CastMultiBit)
   DefineDeclareApply(CompareLessOrGreater)
   DefineDeclareApply(CompareLessOrGreaterOrEqual)
   DefineDeclareApply(CompareEqual)
   DefineDeclareApply(CompareDifferent)
   DefineDeclareApply(ExclusiveOrAssign)
#undef DefineDeclareApply
#undef DefineDeclareBiApply

#define DefineDeclareApplyTo(TypeVirtualOperation) \
   bool applyTo##TypeVirtualOperation(const VirtualOperation&, VirtualElement&, EvaluationEnvironment&) const;
   
   DefineDeclareApplyTo(MinusAssign)
   DefineDeclareApplyTo(CompareLessOrGreater)
   DefineDeclareApplyTo(CompareLessOrGreaterOrEqual)

#undef DefineDeclareApplyTo
      
   /* constraint methods */
#define DefineDeclareConstraint(TypeOperation)                                                   \
bool constraint##TypeOperation(const VirtualOperation&, VirtualElement& source,                  \
      const VirtualElement& result, Argument arg, ConstraintEnvironment& env);

#define DefineDeclareUnaryConstraint(TypeOperation)                                              \
bool constraint##TypeOperation(const VirtualOperation&, const VirtualElement& result, ConstraintEnvironment& env);

   DefineDeclareUnaryConstraint(CastMultiBit)
   DefineDeclareConstraint(PlusAssign)
   DefineDeclareConstraint(MinusAssign)
   DefineDeclareConstraint(OrAssign)
   DefineDeclareConstraint(AndAssign)
   DefineDeclareConstraint(ExclusiveOrAssign)
   DefineDeclareUnaryConstraint(NegateAssign)
#undef DefineDeclareConstraint
#undef DefineDeclareUnaryConstraint

     // query methods
#define DefineDeclareQuery(TypeOperation) \
static bool query##TypeOperation(const VirtualElement&, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env);

   DefineDeclareQuery(Simplification)
   DefineDeclareQuery(SimplificationAsConstantDisjunction)
   DefineDeclareQuery(DispatchOnTopResult)

#undef DefineDeclareQuery

   /* constraint methods */
};

class ExactToApproximateElement : public VirtualExactToApproximateElement {
  private:
   typedef VirtualExactToApproximateElement inherited;

  public:
   typedef Bit::Operation Operation;

   ExactToApproximateElement(const Init& init, PPVirtualElement shared/* , VirtualMemoryModifier* parent=nullptr*/)
      :  inherited(init, shared /* , parent */) {}
   ExactToApproximateElement(const ExactToApproximateElement& source) = default;
   DefineCopy(ExactToApproximateElement)
};

}} // end of namespace Bit::Approximate

namespace MultiBit { namespace Approximate {

class BaseCompareOperationElement : public Scalar::Approximate::Details::TCompareOperationElement<Scalar::Approximate::Details::IntOperationElement> {
  private:
   typedef Scalar::Approximate::Details::TCompareOperationElement<Scalar::Approximate::Details::IntOperationElement> inherited;

  public:
   typedef MultiBit::Operation Operation;
   typedef inherited::Init Init;
   BaseCompareOperationElement(const Init& init, Operation::Type compareType) : inherited(init, compareType) {}
   BaseCompareOperationElement(const BaseCompareOperationElement& source) = default;

   virtual bool query(const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) const
      {  bool result = IntOperationElement::query(operation, env);
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
};

class CompareOperationElement : public BaseCompareOperationElement {
  private:
   typedef BaseCompareOperationElement inherited;

  public:
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
   typedef Scalar::Approximate::Details::TDomainTraits<CompareOperationElement> DomainTraits;
   
  protected:
   class ConstantAndGenericApplyMethod : public Scalar::Approximate::Details::MultiApplyMethod<DomainTraits> {
     private:
      typedef Scalar::Approximate::Details::MultiApplyMethod<DomainTraits> inherited;
      typedef bool (CompareOperationElement::*PointerApplyConstantMethod)
         (const VirtualOperation&, const VirtualElement&, EvaluationEnvironment&);
      typedef bool (CompareOperationElement::*PointerApplyGenericMethod)
         (const VirtualOperation&, const VirtualElement&, EvaluationEnvironment&);

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

      virtual bool execute(CompareOperationElement& object, const VirtualOperation& operation, EvaluationEnvironment& env) const override
         {  VirtualElement::ApproxKind kind = env.getFirstArgument().getType();
            return (kind.isConstant())
               ? (object.*pacmConstantMethod)((const Operation&) operation, env.getFirstArgument(), env)
               : (object.*paimGenericMethod)((const Operation&) operation, env.getFirstArgument(), env);
         }
   };

   virtual PPVirtualElement convertArgumentTopToResult(PPVirtualElement multiBitTop) const override
      {  EvaluationEnvironment env = EvaluationEnvironment(EvaluationEnvironment::Init());
         inherited::assume(multiBitTop->apply(Scalar::MultiBit::ReduceOperation().setLowBit(0).setHighBit(0), env));
         return env.hasResult() ? env.presult() : multiBitTop;
      }
      
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
   CompareOperationElement(const Init& init, Operation::Type compareType) : inherited(init, compareType) {}
   CompareOperationElement(const CompareOperationElement& source) = default;
   DefineCopy(CompareOperationElement)

   virtual const FunctionQueryTable& functionQueryTable(int queryIndex) const override
      {  return ((queryIndex == VirtualQueryOperation::TOperation)
               || (queryIndex == VirtualQueryOperation::TDomain) || (queryIndex == VirtualQueryOperation::TExtern))
            ? fstArg().functionQueryTable(queryIndex)
            : mqtMethodQueryTable[queryIndex];
      }
   virtual bool applyTo(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const override;
   virtual bool constraintTo(const VirtualOperation&, VirtualElement& source, const VirtualElement&, ConstraintEnvironment&) override;
   virtual bool intersectWith(const VirtualElement& source, EvaluationEnvironment& env) override;

   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override;
   virtual bool constraint(const VirtualOperation& operation, const VirtualElement& source, ConstraintEnvironment& env) override;
   virtual bool query(const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) const override;
   // virtual bool guard(PPVirtualElement thenElement, PPVirtualElement elseElement, EvaluationEnvironment& env) override
   //    {  return guardFormal(thenElement, elseElement, env); }

     // Méthodes apply
#define DefineDeclareApply(TypeOperation) \
bool apply##TypeOperation(const VirtualOperation& operation, EvaluationEnvironment& env);
#define DefineDeclareBiApply(TypeOperation) \
bool apply##TypeOperation##Constant(const VirtualOperation& operation, const VirtualElement& source, EvaluationEnvironment& env); \
bool apply##TypeOperation(const VirtualOperation& operation, const VirtualElement& source, EvaluationEnvironment& env);

   DefineDeclareApply(Extend)
   DefineDeclareApply(Concat)
   DefineDeclareApply(Reduce)
   DefineDeclareApply(BitSet)
   DefineDeclareApply(CastBit)
   DefineDeclareApply(CastShiftBit)
   DefineDeclareApply(CompareLessOrGreater)
   DefineDeclareApply(CompareLessOrGreaterOrEqual)
   DefineDeclareApply(CompareEqual)
   DefineDeclareApply(CompareDifferent)
   DefineDeclareApply(Float)
   DefineDeclareApply(TimesAssign)
   DefineDeclareApply(DivideAssign)
   DefineDeclareApply(OppositeSignedAssign)
   DefineDeclareApply(ModuloAssign)
   DefineDeclareApply(ExclusiveOrAssign)
   DefineDeclareBiApply(LeftShiftAssign)
   DefineDeclareBiApply(RightShiftAssign)
#undef DefineDeclareApply
#undef DefineDeclareBiApply

#define DefineDeclareApplyTo(TypeVirtualOperation) \
   bool applyTo##TypeVirtualOperation(const VirtualOperation&, VirtualElement&, EvaluationEnvironment&) const;
   
   DefineDeclareApplyTo(BitSet)
   DefineDeclareApplyTo(Concat)
   DefineDeclareApplyTo(MinusAssign)
   DefineDeclareApplyTo(DivideAssign)
   DefineDeclareApplyTo(ModuloAssign)
   DefineDeclareApplyTo(Float)
   DefineDeclareApplyTo(CompareLessOrGreater)
   DefineDeclareApplyTo(CompareLessOrGreaterOrEqual)

#undef DefineDeclareApplyTo
      
     // query methods
#define DefineDeclareQuery(TypeOperation) \
static bool query##TypeOperation(const VirtualElement&, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env);

   DefineDeclareQuery(CompareSpecial)
   DefineDeclareQuery(Simplification)
   DefineDeclareQuery(SimplificationAsInterval)
   DefineDeclareQuery(SimplificationAsConstantDisjunction)
   DefineDeclareQuery(DispatchOnTopResult)

#undef DefineDeclareQuery

   /* contraint methods */
#define DefineDeclareConstraint(TypeOperation)                                                   \
bool constraint##TypeOperation(const VirtualOperation&, VirtualElement& source,                  \
      const VirtualElement& result, Argument arg, ConstraintEnvironment& env);

#define DefineDeclareUnaryConstraint(TypeOperation)                                              \
bool constraint##TypeOperation(const VirtualOperation&, const VirtualElement& result, ConstraintEnvironment& env);

   DefineDeclareUnaryConstraint(CastBit)
   DefineDeclareUnaryConstraint(BitNegateAssign)
   DefineDeclareConstraint(PlusAssign)
   DefineDeclareConstraint(MinusAssign)
   DefineDeclareConstraint(BitOrAssign)
   DefineDeclareConstraint(BitAndAssign)
   DefineDeclareConstraint(BitExclusiveOrAssign)

#undef DefineDeclareConstraint
#undef DefineDeclareUnaryConstraint
};

class ExactToApproximateElement : public VirtualExactToApproximateElement {
  private:
   typedef VirtualExactToApproximateElement inherited;

  public:
   typedef MultiBit::Operation Operation;

   ExactToApproximateElement(const Init& init, PPVirtualElement shared/*, VirtualMemoryModifier* parent=nullptr */)
      :  inherited(init, shared /*, parent */) {}
   ExactToApproximateElement(const ExactToApproximateElement& source) = default;
   DefineCopy(ExactToApproximateElement)

   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override;
   virtual bool applyTo(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const override;

#define DefineDeclareApply(TypeOperation) \
bool apply##TypeOperation(const Operation& operation, EvaluationEnvironment& env);

   DefineDeclareApply(Compare)
   DefineDeclareApply(Extend)
   DefineDeclareApply(Concat)
   DefineDeclareApply(Reduce)
   DefineDeclareApply(MinusAssign)
   // DefineDeclareApply(CompareLessOrGreater)
   // DefineDeclareApply(CompareLessOrGreaterOrEqual)
   // DefineDeclareApply(CompareEqual)
   // DefineDeclareApply(CompareDifferent)

#undef DefineDeclareApply

#define DefineDeclareApplyTo(TypeVirtualOperation) \
   bool applyTo##TypeVirtualOperation(const Operation&, VirtualElement&, EvaluationEnvironment&) const;
   
   DefineDeclareApplyTo(Compare)
   // DefineDeclareApplyTo(CompareLessOrGreater)
   // DefineDeclareApplyTo(CompareLessOrGreaterOrEqual)

#undef DefineDeclareApplyTo

};

}} // end of namespace MultiBit::Approximate

}} // end of namespace Analyzer::Scalar

#endif // Analyzer_Scalar_Approximate_ConstantMultiBit_SHAREH
