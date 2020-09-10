/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : implementation of general scalar elements
// File      : MultiBitElement.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of a multibit template class providing all the operations on multibits.
//

#include "Analyzer/Scalar/Implementation/General/MultiBitElement.h"
// #include "Analyzer/Scalar/Scalar.hpp"

namespace Analyzer {}

#include "Analyzer/Scalar/Implementation/Native/IntegerElement.h"

#include "Numerics/Numerics.template"
#include "Analyzer/Scalar/Implementation/General/MultiBitElement.template"

namespace Analyzer { namespace Scalar {
namespace MultiBit { namespace Implementation { namespace Details {

template class TMultiBitImplementOperation<BaseMultiBitElement>;

MultiBitElement::MultiBitElement(int size, IntNature nature,
      const Floating::Implementation::Details::MultiFloatElement& floatValue,
      ReadFloatParameters& params) {
   inherited::assertSize(size);
   if (nature == INPointer) {
      AssumeCondition(size == floatValue.mantissa().getSize() + floatValue.exponent().getSize() + 1)
      setBitArray(size-1, floatValue.isNegative());
      inherited mantissa, exponent;
      mantissa.assertSize(floatValue.mantissa().getSize());
      int nbCell = (floatValue.mantissa().getSize()-1)/(8*sizeof(unsigned int))+1;
      while (--nbCell >= 0)
         mantissa[nbCell] = floatValue.mantissa()[nbCell];
      mantissa.normalizeLastCell();
      nbCell = (floatValue.exponent().getSize()-1)/(8*sizeof(unsigned int))+1;
      while (--nbCell >= 0)
         exponent[nbCell] = floatValue.exponent()[nbCell];
      exponent.normalizeLastCell();
      exponent.assertSize(size);
      exponent <<= mantissa.getSize();
      mantissa.assertSize(size);
      inherited::operator|=(exponent).operator|=(mantissa); 
   }
   else {
      Floating::Implementation::Details::MultiFloatElement::IntConversion intResult;
      if (nature == INSigned)
         intResult.setSigned();
      else
         intResult.setUnsigned();
      intResult.setSize(inherited::getSize());
      floatValue.retrieveInteger(intResult, params);
      Floating::Implementation::MultiFloatElement::IntConversion::BigInt result;
      if (nature == INSigned)
         result = intResult.asInt();
      else
         result = intResult.asUnsignedInt();
      int nbCell = (result.getSize()-1)/(8*sizeof(unsigned int))+1;
      while (--nbCell >= 0)
         inherited::array(nbCell) = result[nbCell];
   };
}

void
MultiBitElement::assign(const Scalar::Implementation::VirtualElement& asource) {
   AssumeCondition(dynamic_cast<const Scalar::Implementation::ImplementationElement*>(&asource))
   const Scalar::Implementation::ImplementationElement& isource = (const Scalar::Implementation::ImplementationElement&) asource;
   if (isource.isConstantSigned()) {
      AssumeCondition(dynamic_cast<const Integer::Implementation::IntElement*>(&asource))
      const Integer::Implementation::IntElement& source = (const Integer::Implementation::IntElement&) asource;
      operator=((unsigned int) source.getElement());
   }
   else if (isource.isConstantUnsigned()) {
      AssumeCondition(dynamic_cast<const Integer::Implementation::UnsignedIntElement*>(&asource))
      const Integer::Implementation::UnsignedIntElement& source = (const Integer::Implementation::UnsignedIntElement&) asource;
      operator=(source.getElement());
   }
   else if (isource.isMultiBit()) {
      AssumeCondition(dynamic_cast<const Scalar::MultiBit::Implementation::MultiBitElement*>(&asource))
      BaseMultiBitElement::operator=((const BaseMultiBitElement&) (const Scalar::MultiBit::Implementation::MultiBitElement&) asource);
   }
   else
      { AssumeCondition(false) }
}

} // end of namespace Details

}}}} // end of namespace Analyzer::Scalar::MultiBit::Implementation

