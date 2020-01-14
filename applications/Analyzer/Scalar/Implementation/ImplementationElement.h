/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : scalar elements
// File      : ImplementationElement.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a hierarchy for implementation scalar elements.
//   The constant, symbolic values and approximate values should use
//     them in their constructors.
//

#ifndef Analyzer_Scalar_Implementation_ImplementationElementH
#define Analyzer_Scalar_Implementation_ImplementationElementH

#include "Analyzer/Virtual/Scalar/AtomicElement.h"
#include "Analyzer/Scalar/ConcreteOperation.h"
#include "Numerics/Integer.h"

namespace Analyzer { namespace Scalar { namespace Implementation {

class ImplementationElement : public Implementation::VirtualElement {
  public:
   enum Type
      {  TUndefined, TConstantSigned, TConstantUnsigned, TConstantDouble, TMultiBit, TMultiFloat };

  private:
   typedef Implementation::VirtualElement inherited;

  protected:
   DefineExtendedParameters(3, inherited)
   void setConstantSigned() { AssumeCondition(!hasOwnField()) mergeOwnField(TConstantSigned); }
   void setConstantUnsigned() { AssumeCondition(!hasOwnField()) mergeOwnField(TConstantUnsigned); }
   void setConstantDouble() { AssumeCondition(!hasOwnField()) mergeOwnField(TConstantDouble); }
   void setMultiBit() { AssumeCondition(!hasOwnField()) mergeOwnField(TMultiBit); }
   void setMultiFloat() { AssumeCondition(!hasOwnField()) mergeOwnField(TMultiFloat); }

  public:
   ImplementationElement() {}
   ImplementationElement(const ImplementationElement& source) = default;
   DefineCopy(ImplementationElement)
   DCompare(ImplementationElement)

   bool isConstantSigned() const { return queryOwnField() == TConstantSigned; }
   bool isConstantUnsigned() const { return queryOwnField() == TConstantUnsigned; }
   bool isConstantDouble() const { return queryOwnField() == TConstantDouble; }
   bool isMultiBit() const { return queryOwnField() == TMultiBit; }
   bool isMultiFloat() const { return queryOwnField() == TMultiFloat; }
};

}}} // end of namespace Analyzer::Scalar::Implementation

#endif
