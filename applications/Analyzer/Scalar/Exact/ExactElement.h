/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : scalar elements common to compilation constants and symbolic execution
// File      : ExactElement.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of scalar elements used in simulation/symbolic execution.
//

#ifndef Analyzer_Scalar_Exact_ExactElementH
#define Analyzer_Scalar_Exact_ExactElementH

#include "Analyzer/Scalar/VirtualExact/NativeVirtualExact.hpp"
#include "Analyzer/Scalar/VirtualExact/GeneralVirtualExact.hpp"

namespace Analyzer { namespace Scalar { namespace Exact {

/*************************************************/
/* Definition of the class Exact::VirtualElement */
/*************************************************/

class VirtualElement : public Scalar::VirtualElement {
  public:
   typedef TEvaluationEnvironment<VirtualElement, EvaluationEnvironment> ApplyEnvironment;
   typedef PNT::TPassPointer<VirtualElement, PPAbstractElement> PPVirtualElement;

  private:
   typedef Scalar::VirtualElement inherited;
   typedef inherited::Init InheritedInit;
   
  protected:
   enum Type { TUndefined, TScalar, TPointer, TTop };
   DefineExtendedParameters(2, inherited)
   void setScalar() { AssumeCondition(!hasOwnField()) mergeOwnField(TScalar); }
   void setPointer() { AssumeCondition(!hasOwnField()) mergeOwnField(TPointer); }
   void setTop() { AssumeCondition(!hasOwnField()) mergeOwnField(TTop); }

  protected:
   Type type() const { return (Type) queryOwnField(); }
   void setType(Type type) { AssumeCondition(!hasOwnField()) mergeOwnField(type); }

  public:
   bool isScalar() const { return queryOwnField() == TScalar; }
   bool isPointer() const { return queryOwnField() == TPointer; }
   bool isTop() const { return queryOwnField() == TTop; }

  public:
   class Init : public InheritedInit {
     private:
      typedef InheritedInit inherited;
      Type tType;
      
     public:
      Init() : tType(TUndefined) {}
      Init(const InheritedInit& source) : inherited(source), tType(TUndefined) {}
      Init(const Init& source) = default;
      Init& operator=(const Init& source) = default;
      DefineCopy(Init)
      DDefineAssign(Init)

      Init& setScalar()    { tType = TScalar; return *this; }
      Init& setPointer()   { tType = TPointer; return *this; }
      Init& setTop()       { tType = TTop; return *this; }
      Init& setInitialValue(const Scalar::Implementation::VirtualElement& iveConstant) { return (Init&) inherited::setInitialValue(iveConstant); }
      friend class Exact::VirtualElement;
   };
   class InitFloat : public Init {
     private:
      typedef Init inherited;
      int uSizeMantissa, uSizeExponent;
      
     public:
      InitFloat() {}
      InitFloat(const InitFloat& source) = default;
      InitFloat& operator=(const InitFloat& source) = default;
      DefineCopy(InitFloat)
      DDefineAssign(InitFloat)
      
      InitFloat& setSizeMantissa(int sizeMantissa) { uSizeMantissa = sizeMantissa; return *this; }
      InitFloat& setSizeExponent(int sizeExponent) { uSizeExponent = sizeExponent; return *this; }
      const int& sizeMantissa() const { return uSizeMantissa; }
      const int& sizeExponent() const { return uSizeExponent; }
   };
   friend class Init;

  protected:
   VirtualElement(const Init& init)
      :  inherited(init) { mergeOwnField(init.tType); }
   VirtualElement(const VirtualElement& source) = default;

   virtual VirtualElement* _createMultiBitElement(const Init&) const { AssumeUncalled return nullptr; }
   virtual VirtualElement* _createMultiFloatElement(const InitFloat&) const { AssumeUncalled return nullptr; }

  public:
   DefineCopy(VirtualElement)
   virtual bool isValid() const override { return inherited::isValid() && hasOwnField(); }

   int getDependanceLevel() const { AssumeCondition(hasOwnField()) return queryOwnField(); }

   virtual bool apply(const VirtualOperation& operation, EvaluationEnvironment& aenv) override
      {  bool result = false;
         if ((result = AbstractElement::apply(operation, aenv)) == false) {
            ApplyEnvironment& env = (ApplyEnvironment&) aenv;
            int args = operation.getArgumentsNumber();
            if ((args > 0) && (env.getFirstArgument().type() > type())) {
               result = env.getFirstArgument().applyTo(operation, *this, env);
               AssumeCondition(result)
            };
         };
         return result;
      }
   virtual bool applyTo(const VirtualOperation& operation, VirtualElement& source, ApplyEnvironment& env) const
      {  return false; }

   static VirtualElement* createBitElement(bool value);
   static VirtualElement* createIntElement(int value);
   static VirtualElement* createIntElement(const Init& init);
   static VirtualElement* createBoolElement(bool value);
   static VirtualElement* createUndefined(const Init& init);

   static VirtualElement* createCharElement(const Init& init);
   static VirtualElement* createSignedCharElement(const Init& init);
   static VirtualElement* createUnsignedCharElement(const Init& init);
   static VirtualElement* createShortElement(const Init& init);
   static VirtualElement* createUnsignedShortElement(const Init& init);
   static VirtualElement* createUnsignedIntElement(const Init& init);
   static VirtualElement* createLongElement(const Init& init);
   static VirtualElement* createUnsignedLongElement(const Init& init);
   VirtualElement* createMultiBitElement(const Init& init) const { return _createMultiBitElement(init); }
   static VirtualElement* createFloatElement(const Init& init);
   static VirtualElement* createDoubleElement(const Init& init);
   static VirtualElement* createLongDoubleElement(const Init& init);
   VirtualElement* createMultiFloatElement(const InitFloat& init) const { return _createMultiFloatElement(init); }

   bool applyComparison(const VirtualOperation& operation, const VirtualElement& source) const
      {  EvaluationEnvironment env = EvaluationEnvironment(EvaluationEnvironment::Init());
         env.setFirstArgument(source);
         const_cast<VirtualElement&>(*this).apply(operation, env);
         return ((const VirtualElement&) env.getResult()).queryZeroResult().isTrue();
      }
};

class PPVirtualElement : public Scalar::PPVirtualElement {
  private:
   typedef Scalar::PPVirtualElement inherited;
   PPVirtualElement(const PPAbstractElement& source) : inherited(source) {}

  public:
   PPVirtualElement() {}
   PPVirtualElement(int value) : inherited(VirtualElement::createIntElement(value), Init()) {}
   PPVirtualElement(const VirtualElement& element) : inherited(element) {}
   PPVirtualElement(VirtualElement* element, Init init) : inherited(element, init) {}
   PPVirtualElement(const PPVirtualElement& source) = default;

   void fullAssign(const PPVirtualElement& source) { inherited::fullAssign(source); }
   void applyAssign(const VirtualOperation& assignOperation, const VirtualElement& source);
};

/* Inline methods of the class PPVirtualElement */

inline void
PPVirtualElement::applyAssign(const VirtualOperation& assignOperation, const VirtualElement& source) {
   EvaluationEnvironment env = EvaluationEnvironment(EvaluationEnvironment::Init());
   env.setFirstArgument(source);
   getElement().apply(assignOperation, env);
   if (env.hasResult())
      assign(env.presult());
}

template <class TypeMultiTraits>
class TMultiBitElement : public MultiBit::VirtualExact::TMultiBitElement<VirtualElement,
      MultiBit::Implementation::Details::TMultiBitElement<TypeMultiTraits> > {
  private:
   typedef Exact::VirtualElement VirtualElement;
   typedef MultiBit::VirtualExact::TMultiBitElement<VirtualElement,
         MultiBit::Implementation::Details::TMultiBitElement<TypeMultiTraits> > inherited;
   typedef TMultiBitElement<TypeMultiTraits> thisType;

  public:
   TMultiBitElement(const typename inherited::Init& init) : inherited(init) {}
   TMultiBitElement(const thisType& source) = default;
};

class MultiBitElement : public MultiBit::VirtualExact::TMultiBitElement<VirtualElement,
      MultiBit::Implementation::Details::MultiBitElement> {
  private:
   typedef Analyzer::Scalar::Exact::VirtualElement VirtualElement;
   typedef MultiBit::VirtualExact::TMultiBitElement<VirtualElement,
         MultiBit::Implementation::Details::MultiBitElement> inherited;

  public:
   MultiBitElement(const Init& init) : inherited(init) {}
   MultiBitElement(const MultiBitElement& source) = default;
};

typedef Bit::VirtualExact::TBitElement<VirtualElement>                BitElement;
typedef Integer::VirtualExact::TCharElement<VirtualElement>           CharElement;
typedef Integer::VirtualExact::TSignedCharElement<VirtualElement>     SignedCharElement;
typedef Integer::VirtualExact::TUnsignedCharElement<VirtualElement>   UnsignedCharElement;
typedef Integer::VirtualExact::TShortElement<VirtualElement>          ShortElement;
typedef Integer::VirtualExact::TUnsignedShortElement<VirtualElement>  UnsignedShortElement;
typedef Integer::VirtualExact::TIntElement<VirtualElement>            IntElement;
typedef Integer::VirtualExact::TUnsignedIntElement<VirtualElement>    UnsignedIntElement;
typedef Integer::VirtualExact::TLongElement<VirtualElement>           LongElement;
typedef Integer::VirtualExact::TUnsignedLongElement<VirtualElement>   UnsignedLongElement;
typedef Floating::VirtualExact::TFloatElement<VirtualElement>         FloatElement;
typedef Floating::VirtualExact::TDoubleElement<VirtualElement>        DoubleElement;
typedef Floating::VirtualExact::TLongDoubleElement<VirtualElement>    LongDoubleElement;

typedef VirtualExact::TUndefElement<VirtualElement> UndefElement;

} // end of namespace Exact

extern template MultiBit::VirtualExact::TMultiBitElement<Exact::VirtualElement, MultiBit::Implementation::Details::MultiBitElement>::MethodApplyTable
Analyzer::Scalar::MultiBit::VirtualExact::TMultiBitElement<Exact::VirtualElement, MultiBit::Implementation::Details::MultiBitElement>::matMethodTable;

}} // end of namespace Analyzer::Scalar

#endif // Analyzer_Scalar_Exact_ExactElementH

