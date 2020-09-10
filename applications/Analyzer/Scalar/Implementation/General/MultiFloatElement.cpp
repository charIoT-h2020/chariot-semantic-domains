/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : implementation of general scalar elements
// File      : MultiFloatElement.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of a multifloat template class providing all the operations on multifloats.
//

#include "Analyzer/Scalar/Implementation/General/MultiFloatElement.h"
// #include "Analyzer/Scalar/Scalar.hpp"

namespace Analyzer {}

#include "Analyzer/Scalar/Implementation/General/MultiFloatElement.template"

namespace Analyzer { namespace Scalar {
namespace Floating { namespace Implementation { namespace Details {

template class TMultiFloatImplementOperation<Numerics::TBuiltDouble<ExtensibleBuiltDoubleTraits> >;

MultiFloatElement::MultiFloatElement(int sizeMantissa, int sizeExponent, IntNature nature,
      const MultiBit::Implementation::Details::MultiBitElement& intValue, ReadParameters& params)
   :  inherited(sizeMantissa, sizeExponent) {
   if (nature == INPointer) {
      typedef MultiBit::Implementation::Details::MultiBitElement BigInt; // put a flag big/little endian in params
      AssumeCondition(sizeMantissa + sizeExponent + 1 == intValue.getSize())
      if (intValue.cbitArray(intValue.getSize()-1))
         inherited::setNegative();
      BigInt mantissaContent = intValue, exponentContent = intValue;
      mantissaContent.setSize(sizeMantissa);
      mantissaContent.normalize();
      exponentContent >>= sizeMantissa;
      exponentContent.setSize(sizeExponent);
      exponentContent.normalize();
      int nbCell = (sizeMantissa-1)/(8*sizeof(unsigned int))+1;
      while (--nbCell >= 0)
         mantissa()[nbCell] = mantissaContent[nbCell];
      nbCell = (sizeExponent-1)/(8*sizeof(unsigned int))+1;
      while (--nbCell >= 0)
         exponent()[nbCell] = exponentContent[nbCell];
   }
   else {
      inherited::IntConversion intConversion;
      if (nature == INSigned) {
         intConversion.setSigned();
         intConversion.assignSigned(intValue);
      }
      else {
         intConversion.setUnsigned();
         intConversion.assignUnsigned(intValue);
      };
      inherited::setInteger(intConversion, params);
   }
}

void
MultiFloatElement::assign(const Scalar::Implementation::VirtualElement& asource) {
   AssumeCondition(dynamic_cast<const Scalar::Implementation::ImplementationElement*>(&asource))
   const Scalar::Implementation::ImplementationElement& isource = (const Scalar::Implementation::ImplementationElement&) asource;
   if (isource.isConstantDouble()) {
      typedef Numerics::TDoubleElement<Numerics::TFloatingBase<Numerics::DoubleTraits> > Double;
      AssumeCondition(dynamic_cast<const Floating::Implementation::DoubleElement*>(&asource))
      const Floating::Implementation::DoubleElement& source = (const Floating::Implementation::DoubleElement&) asource;
      Double::BuiltDouble softDouble(Double(source.getElement()));
      mantissa().setSize(softDouble.getMantissa().getSize());
      exponent().setSize(softDouble.getBasicExponent().getSize());
      int nbCell = (softDouble.getMantissa().getSize()+8*sizeof(unsigned int)-1)/(8*sizeof(unsigned int));
      while (--nbCell >= 0)
         mantissa()[nbCell] = softDouble.getMantissa()[nbCell];
      nbCell = (softDouble.getBasicExponent().getSize()+8*sizeof(unsigned int)-1)/(8*sizeof(unsigned int));
      while (--nbCell >= 0)
         exponent()[nbCell] = softDouble.getBasicExponent()[nbCell];
      if (softDouble.isNegative())
         setNegative();
   }
   else if (isource.isMultiFloat()) {
      AssumeCondition(dynamic_cast<const Floating::Implementation::MultiFloatElement*>(&asource))
      operator=((const Floating::Implementation::MultiFloatElement&) asource);
   }
   else
      { AssumeCondition(false) }
}

} // end of namespace Details

}}}} // end of namespace Analyzer::Scalar::Floating::Implementation

