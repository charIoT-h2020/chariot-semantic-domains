/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate native scalar elements
// File      : ConstantNativeElement.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a class of elements that rely on native host implementation.
//   This introduces template classes that close the integer and floating point classes.
//   All this stuff is rather defined for source code analysis.
//

#ifndef Analyzer_Scalar_Approximate_ConstantNativeElementH
#define Analyzer_Scalar_Approximate_ConstantNativeElementH

#include "Analyzer/Scalar/Implementation/Native/IntegerElement.h"
#include "Analyzer/Scalar/Approximate/BitFieldMask.h"

namespace Analyzer { namespace Scalar { namespace Integer { namespace Approximate {

class CharElement;
class SignedCharElement;
class UnsignedCharElement;
class ShortElement;
class UnsignedShortElement;
class IntElement;
class UnsignedIntElement;
class LongElement;
class UnsignedLongElement;

}} // end of namespace Integer::Approximate

namespace Floating { namespace Approximate {

class FloatElement;
class DoubleElement;
class LongDoubleElement;

}} // end of namespace Floating::Approximate

namespace Approximate { namespace Details {

template <class TypeBase, class TypeDerived>
class TBaseElement : public TypeBase {
  private:
   typedef TypeBase inherited;
   typedef TBaseElement<TypeBase, TypeDerived> thisType;

  protected:
#define DefineDeclareVirtualCast(TypeCast)                                                         \
   virtual VirtualElement* _create##TypeCast##Element(const VirtualElement::Init& init) const { AssumeUncalled return nullptr; }\
   VirtualElement* create##TypeCast##Element(const VirtualElement::Init& init) const { return _create##TypeCast##Element(init); }

#define DefineDeclareVirtualIntCast(TypeCast) DefineDeclareVirtualCast(TypeCast)                 \
   virtual void _castAssign(const Integer::Approximate::TypeCast##Element& source, EvaluationEnvironment& env) { AssumeUncalled } \
  public:                                                                                        \
   thisType& castAssign(const Integer::Approximate::TypeCast##Element& source, EvaluationEnvironment& env) { _castAssign(source, env); return *this; }\
  protected:

#define DefineDeclareVirtualFloatCast(TypeCast) DefineDeclareVirtualCast(TypeCast)               \
   virtual void _castAssign(const Floating::Approximate::TypeCast##Element& source, EvaluationEnvironment& env) { AssumeUncalled } \
  public:                                                                                        \
   thisType& castAssign(const Floating::Approximate::TypeCast##Element& source, EvaluationEnvironment& env) { _castAssign(source, env); return *this; }\
  protected:

   DefineDeclareVirtualIntCast(Char)
   DefineDeclareVirtualIntCast(SignedChar)
   DefineDeclareVirtualIntCast(UnsignedChar)
   DefineDeclareVirtualIntCast(Short)
   DefineDeclareVirtualIntCast(UnsignedShort)
   DefineDeclareVirtualIntCast(Int)
   DefineDeclareVirtualIntCast(UnsignedInt)
   DefineDeclareVirtualIntCast(Long)
   DefineDeclareVirtualIntCast(UnsignedLong)
   DefineDeclareVirtualFloatCast(Float)
   DefineDeclareVirtualFloatCast(Double)
   DefineDeclareVirtualFloatCast(LongDouble)
   DefineDeclareVirtualCast(Bit)
   DefineDeclareVirtualCast(MultiBit)
   DefineDeclareVirtualCast(MultiFloat)
   
#undef  DefineDeclareVirtualIntCast
#undef  DefineDeclareVirtualFloatCast
#undef  DefineDeclareVirtualCast

  public:
   TBaseElement(const typename TypeBase::Init& init) : inherited(init) {}
   TBaseElement(const TBaseElement<TypeBase, TypeDerived>& source) : inherited(source) {}
   bool queryBound(const typename TypeBase::QueryOperation&, typename TypeBase::QueryOperation::Environment& env) const;

#define DefineDeclareMethod(TypeOperation)                                                         \
bool apply##TypeOperation(const VirtualOperation& operation, EvaluationEnvironment& env);

#define DefineDeclareConstraintCompareMethod(TypeOperation)                                        \
bool constraint##TypeOperation(const VirtualOperation&, VirtualElement& source,                    \
      ConstraintEnvironment::Argument arg, ConstraintEnvironment& env);

#define DefineDeclareConstraintBinaryMethod(TypeOperation)                                         \
bool constraint##TypeOperation(const VirtualOperation&, VirtualElement& source, const VirtualElement& result,\
   ConstraintEnvironment::Argument arg, ConstraintEnvironment& env);

   DefineDeclareMethod(MinAssign)
   DefineDeclareMethod(MaxAssign)
   
   DefineDeclareConstraintCompareMethod(CompareLessOrGreater)
   DefineDeclareConstraintCompareMethod(CompareLessOrEqualOrGreater)
   DefineDeclareConstraintCompareMethod(CompareEqual)
   DefineDeclareConstraintCompareMethod(CompareDifferent)

   DefineDeclareConstraintBinaryMethod(MinAssignWithConstantResult)
   DefineDeclareConstraintBinaryMethod(MaxAssignWithConstantResult)
};

template <class TypeRingTraits>
class TTopCast {
  public:
   static bool applyIntegerTop(VirtualElement& top, const VirtualOperation& operation, EvaluationEnvironment& env);
   static bool applyFloatingTop(VirtualElement& top, const VirtualOperation& operation, EvaluationEnvironment& env);
   static bool applyTo(const VirtualElement& top, VirtualElement& thisElement, const VirtualOperation& operation, EvaluationEnvironment& env);
   static bool applyToFloatingTop(const VirtualElement& top, VirtualElement& thisElement, const VirtualOperation& operation, EvaluationEnvironment& env);
   static bool constraintIntegerTop(VirtualElement& top, const VirtualOperation& operation, const VirtualElement& result, ConstraintEnvironment& env);
   static bool constraintFloatingTop(VirtualElement& top, const VirtualOperation& operation, const VirtualElement& result, ConstraintEnvironment& env);
};

#define DefineDeclareQueryFunction(TypeOperation)                                                  \
   static bool queryNew##TypeOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);

template <class TypeBase>
class TCloseCastElement : public TypeBase {
  private:
   typedef TypeBase inherited;
   typedef TCloseCastElement<TypeBase> thisType;

  protected:
   typedef Details::TDomainTraits<thisType> DomainTraits;
   typedef typename TypeBase::template MethodConstraintTable<DomainTraits> InheritedMethodConstraintTable;
   class MethodConstraintTable : public InheritedMethodConstraintTable {
     private:
      typedef InheritedMethodConstraintTable inherited;
     public:
      MethodConstraintTable();
   };
   friend class Details::MethodAccess<DomainTraits>;
   static MethodConstraintTable mctMethodConstraintTable;

  public:
   TCloseCastElement(const typename TypeBase::Init& init) : inherited(init) {}
   TCloseCastElement(const thisType& source) : inherited(source) {}
   TemplateDefineCopy(TCloseCastElement, TypeBase)

   typedef typename TypeBase::inheritedImplementation inheritedImplementation;
   typedef typename inheritedImplementation::SimulatedType SimulatedType;

   static const thisType& getSameArgument(const EvaluationEnvironment& env)
      {  AssumeCondition(dynamic_cast<const thisType*>(&env.getFirstArgument()))
         return (const thisType&) env.getFirstArgument();
      }
   
   virtual bool applyTo(const VirtualOperation&, Approximate::VirtualElement&, EvaluationEnvironment&) const override { AssumeUncalled return false; }
   virtual bool mergeWithTo(Approximate::VirtualElement&, EvaluationEnvironment&) const override { AssumeUncalled return false; }
   virtual bool intersectWithTo(Approximate::VirtualElement&, EvaluationEnvironment&) const override;
   virtual bool constraintTo(const VirtualOperation&, Approximate::VirtualElement& source, const Approximate::VirtualElement&, ConstraintEnvironment&) override;
   virtual VirtualElement::ZeroResult queryZeroResult() const override { return VirtualElement::ZeroResult().setBool((bool) inheritedImplementation::getElement()); }

   virtual bool contain(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) const override;
   virtual bool containTo(const VirtualElement&, EvaluationEnvironment&) const override { AssumeUncalled return false; }
   virtual bool intersectWith(const Approximate::VirtualElement& source, EvaluationEnvironment& env) override;
   virtual bool constraint(const VirtualOperation&, const Approximate::VirtualElement&, ConstraintEnvironment& env) override;
};

template <class TypeBased, class TypeRingTraits>
class TCloseCastIntegerBasedElement : public TCloseCastElement<TypeBased> {
  private:
   typedef TCloseCastElement<TypeBased> inherited;
   typedef TCloseCastIntegerBasedElement<TypeBased, TypeRingTraits> thisType;

  protected:
   typedef Details::TDomainTraits<thisType> DomainTraits;
   typedef typename TypeBased::template MethodApplyTable<DomainTraits> InheritedMethodApplyTable;
   class MethodApplyTable : public InheritedMethodApplyTable {
     private:
      typedef InheritedMethodApplyTable inherited;
     public:
      MethodApplyTable();
   };
   friend class MethodApplyTable;
   typedef typename TypeBased::template MethodQueryTable<DomainTraits> InheritedMethodQueryTable;
   class MethodQueryTable : public InheritedMethodQueryTable {
     private:
      typedef InheritedMethodQueryTable inherited;
     public:
      MethodQueryTable();
   };
   friend class MethodQueryTable;
   static MethodApplyTable matMethodApplyTable;
   static MethodQueryTable mqtMethodQueryTable;
   friend class Details::MethodAccess<DomainTraits>;
   typedef typename inherited::inheritedImplementation inheritedImplementation;
   typedef Approximate::Details::IntOperationElement::Methods Methods;

#define DefineDeclareVirtualCast(TypeCast)                                                       \
   virtual VirtualElement* _create##TypeCast##Element(const VirtualElement::Init& init) const override\
      {  return TypeRingTraits::create##TypeCast##Element(init); }
   
#define DefineDeclareVirtualIntCast(TypeCast) DefineDeclareVirtualCast(TypeCast)                 \
   virtual void _castAssign(const Integer::Approximate::TypeCast##Element& source, EvaluationEnvironment& env) override;

#define DefineDeclareVirtualFloatCast(TypeCast)                                                  \
   virtual VirtualElement* _create##TypeCast##Element(const VirtualElement::Init& init) const override\
      {  return TypeRingTraits::create##TypeCast##Element((const VirtualElement::InitFloat&) init); }\
   virtual void _castAssign(const Floating::Approximate::TypeCast##Element& source, EvaluationEnvironment& env) override;
   
   DefineDeclareVirtualIntCast(Char)
   DefineDeclareVirtualIntCast(SignedChar)
   DefineDeclareVirtualIntCast(UnsignedChar)
   DefineDeclareVirtualIntCast(Short)
   DefineDeclareVirtualIntCast(UnsignedShort)
   DefineDeclareVirtualIntCast(Int)
   DefineDeclareVirtualIntCast(UnsignedInt)
   DefineDeclareVirtualIntCast(Long)
   DefineDeclareVirtualIntCast(UnsignedLong)
   DefineDeclareVirtualFloatCast(Float)
   DefineDeclareVirtualFloatCast(Double)
   DefineDeclareVirtualFloatCast(LongDouble)
   DefineDeclareVirtualCast(Bit)
   DefineDeclareVirtualCast(MultiBit)
   
#undef DefineDeclareVirtualCast
#undef DefineDeclareVirtualIntCast
#undef DefineDeclareVirtualFloatCast
   
  public:
   TCloseCastIntegerBasedElement(const VirtualElement::Init& init) : inherited(init) {}
   TCloseCastIntegerBasedElement(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   Template2DefineCopy(TCloseCastIntegerBasedElement, TypeBased, TypeRingTraits)
   DTemplate2DefineAssign(TCloseCastIntegerBasedElement, TypeBased, TypeRingTraits)
      
   typedef Approximate::Details::IntOperationElement::QueryOperation QueryOperation;
   
   DefineDeclareMethod(CastChar)
   DefineDeclareMethod(CastSignedChar)
   DefineDeclareMethod(CastUnsignedChar)
   DefineDeclareMethod(CastShort)
   DefineDeclareMethod(CastUnsignedShort)
   DefineDeclareMethod(CastInt)
   DefineDeclareMethod(CastUnsignedInt)
   DefineDeclareMethod(CastLong)
   DefineDeclareMethod(CastUnsignedLong)
   DefineDeclareMethod(CastFloat)
   DefineDeclareMethod(CastDouble)
   DefineDeclareMethod(CastLongDouble)
   DefineDeclareMethod(CastMultiBit)
   DefineDeclareMethod(CastBit)
   DefineDeclareMethod(Compare)

   virtual const VirtualElement::FunctionQueryTable& functionQueryTable(int queryIndex) const override
      {  return mqtMethodQueryTable.getFunctionTable((VirtualQueryOperation::Type) queryIndex); }

   virtual bool query(const VirtualQueryOperation&, VirtualQueryOperation::Environment& env) const override;
   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override;
   virtual bool mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) override;
   virtual bool guard(PPVirtualElement thenElement, PPVirtualElement elseElement, EvaluationEnvironment& env) override
      {  return inherited::guardConstant((bool) inheritedImplementation::getElement(), thenElement, elseElement, env); }

   DefineDeclareQueryFunction(Disjunction)
   DefineDeclareQueryFunction(MultiBitConstant)
   DefineDeclareQueryFunction(MultiFloatConstant)
   DefineDeclareQueryFunction(Interval)
   DefineDeclareQueryFunction(BooleanDisjunction)
   DefineDeclareQueryFunction(Top)
   DefineDeclareQueryFunction(False)
   DefineDeclareQueryFunction(True)
   DefineDeclareQueryFunction(IntForShift)
   DefineDeclareQueryFunction(Constant)
   DefineDeclareQueryFunction(Zero)
   DefineDeclareQueryFunction(MinusOne)
   DefineDeclareQueryFunction(One)
   DefineDeclareQueryFunction(Min)
   DefineDeclareQueryFunction(Max)
};

template <class TypeBased, class TypeRingTraits>
class TCloseCastFloatingBasedElement : public TCloseCastElement<TypeBased> {
  private:
   typedef TCloseCastElement<TypeBased> inherited;
   typedef TCloseCastFloatingBasedElement<TypeBased, TypeRingTraits> thisType;

  protected:
   typedef Details::TDomainTraits<thisType> DomainTraits;
   typedef typename TypeBased::template MethodApplyTable<DomainTraits> InheritedMethodApplyTable;
   typedef typename TypeBased::template MethodQueryTable<DomainTraits> InheritedMethodQueryTable;
   class MethodApplyTable : public InheritedMethodApplyTable {
     private:
      typedef InheritedMethodApplyTable inherited;
     public:
      MethodApplyTable();
   };
   friend class MethodApplyTable;
   class MethodQueryTable : public InheritedMethodQueryTable {
     private:
      typedef InheritedMethodQueryTable inherited;
     public:
      MethodQueryTable();
   };
   friend class MethodQueryTable;
   static MethodApplyTable matMethodApplyTable;
   static MethodQueryTable mqtMethodQueryTable;
   friend class Details::MethodAccess<DomainTraits>;
   typedef typename inherited::inheritedImplementation inheritedImplementation;
   typedef Approximate::Details::RealOperationElement::Methods Methods;
   
#define DefineDeclareVirtualCast(TypeCast)                                                       \
   virtual VirtualElement* _create##TypeCast##Element(const VirtualElement::Init& init) const override\
      {  return TypeRingTraits::create##TypeCast##Element(init); }

#define DefineDeclareVirtualIntCast(TypeCast) DefineDeclareVirtualCast(TypeCast)                 \
   virtual void _castAssign(const Integer::Approximate::TypeCast##Element& source, EvaluationEnvironment& env) override;

#define DefineDeclareVirtualFloatCast(TypeCast)                                                  \
   virtual VirtualElement* _create##TypeCast##Element(const VirtualElement::Init& init) const override\
      {  return TypeRingTraits::create##TypeCast##Element((const VirtualElement::InitFloat&) init); } \
   virtual void _castAssign(const Floating::Approximate::TypeCast##Element& source, EvaluationEnvironment& env) override;
   
   DefineDeclareVirtualIntCast(Char)
   DefineDeclareVirtualIntCast(SignedChar)
   DefineDeclareVirtualIntCast(UnsignedChar)
   DefineDeclareVirtualIntCast(Short)
   DefineDeclareVirtualIntCast(UnsignedShort)
   DefineDeclareVirtualIntCast(Int)
   DefineDeclareVirtualIntCast(UnsignedInt)
   DefineDeclareVirtualIntCast(Long)
   DefineDeclareVirtualIntCast(UnsignedLong)
   DefineDeclareVirtualFloatCast(Float)
   DefineDeclareVirtualFloatCast(Double)
   DefineDeclareVirtualFloatCast(LongDouble)
   
#undef DefineDeclareVirtualCast
#undef DefineDeclareVirtualIntCast
#undef DefineDeclareVirtualFloatCast

   virtual VirtualElement* _createMultiFloatElement(const VirtualElement::Init& init) const override
      {  return TypeRingTraits::createMultiFloatElement((const VirtualElement::InitFloat&) init); }
   
  public:
   TCloseCastFloatingBasedElement(const VirtualElement::InitFloat& init) : inherited(init) {}
   TCloseCastFloatingBasedElement(const thisType& source) : inherited(source) {}
   Template2DefineCopy(TCloseCastFloatingBasedElement, TypeBased, TypeRingTraits)

   typedef Approximate::Details::RealOperationElement::QueryOperation QueryOperation;
   
   DefineDeclareMethod(CastChar)
   DefineDeclareMethod(CastSignedChar)
   DefineDeclareMethod(CastUnsignedChar)
   DefineDeclareMethod(CastShort)
   DefineDeclareMethod(CastUnsignedShort)
   DefineDeclareMethod(CastInt)
   DefineDeclareMethod(CastUnsignedInt)
   DefineDeclareMethod(CastLong)
   DefineDeclareMethod(CastUnsignedLong)
   DefineDeclareMethod(CastFloat)
   DefineDeclareMethod(CastDouble)
   DefineDeclareMethod(CastLongDouble)

   DefineDeclareMethod(CastMultiFloat)
   DefineDeclareMethod(CastViaMultiFloat)
   DefineDeclareMethod(Compare)

   virtual const VirtualElement::FunctionQueryTable& functionQueryTable(int queryIndex) const override
      {  return mqtMethodQueryTable.getFunctionTable((VirtualQueryOperation::Type) queryIndex); }

   virtual bool query(const VirtualQueryOperation&, VirtualQueryOperation::Environment& env) const override;
   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override;
   virtual bool mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) override;

   DefineDeclareQueryFunction(Disjunction)
   DefineDeclareQueryFunction(BooleanDisjunction)
   DefineDeclareQueryFunction(MultiBitConstant)
   DefineDeclareQueryFunction(MultiFloatConstant)
   DefineDeclareQueryFunction(Interval)
   DefineDeclareQueryFunction(Top)
   DefineDeclareQueryFunction(Zero)
   DefineDeclareQueryFunction(Min)
   DefineDeclareQueryFunction(Max)
   DefineDeclareQueryFunction(True)
   DefineDeclareQueryFunction(False)
};

#undef DefineDeclareMethod
#undef DefineDeclareQueryMethod
#undef DefineDeclareConstraintCompareMethod
#undef DefineDeclareConstraintBinaryMethod

} // end of namespace Details

}}} // end of namespace Analyzer::Scalar::Approximate

#endif // Analyzer_Scalar_Approximate_ConstantNativeElementH

