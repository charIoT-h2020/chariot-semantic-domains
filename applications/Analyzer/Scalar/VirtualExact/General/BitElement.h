/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : scalar elements common to compilation constants and symbolic execution
// File      : BitElement.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a bit class used in simulation/symbolic execution.
//   All this stuff is rather defined for host independent code analysis.
//

#ifndef Analyzer_Scalar_VirtualExact_BitElementH
#define Analyzer_Scalar_VirtualExact_BitElementH

#include "Analyzer/Virtual/Scalar/AtomicElement.h"
#include "Analyzer/Scalar/ConcreteOperation.h"
#include "Pointer/TreeMethods.h"

namespace Analyzer { namespace Scalar { namespace Bit { namespace VirtualExact {

template <class TypeBase>
class TBitElement : public TypeBase {
  private:
   typedef TBitElement<TypeBase> thisType;
   typedef TypeBase inherited;
   bool fValue;

  protected:
   virtual ComparisonResult _compare(const EnhancedObject& asource) const override
      {  ComparisonResult result = inherited::_compare(asource);
         return (result == CREqual)
            ? fcompare((int) fValue, (int) ((const thisType&) asource).fValue) : result;
      }
   virtual int _getSizeInBits() const override { return 1; }

  public:
   TBitElement(bool value)
      :  inherited(typename TypeBase::Init().set1()), fValue(value) {}
   TBitElement(const typename TypeBase::Init& init)
      :  inherited(init), fValue(false) {}
   TBitElement(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   TemplateDefineCopy(TBitElement, TypeBase)
   DTemplateDefineAssign(TBitElement, TypeBase)

   virtual bool isValid() const override { return inherited::isValid(); }

   // atomic methods
   typedef Bit::Operation Operation;
   const thisType& implementation() const { return *this; }
   bool getElement() const { return fValue; }

   virtual bool apply(const VirtualOperation& operation, EvaluationEnvironment& env) override;
   virtual typename inherited::ZeroResult queryZeroResult() const override
      {  return typename inherited::ZeroResult().setBool(fValue); }
};

}}}} // end of namespace Analyzer::Scalar::Bit::VirtualExact

#endif // Analyzer_Scalar_VirtualExact_BitElementH

