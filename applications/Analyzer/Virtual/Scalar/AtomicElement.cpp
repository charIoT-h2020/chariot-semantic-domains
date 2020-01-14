/////////////////////////////////
//
// Library   : Analyzer/Virtual
// Unit      : Scalar
// File      : AtomicElement.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of the base hierarchy of scalar elements.
//

#include "Analyzer/Virtual/Scalar/AtomicElement.h"
#include "Analyzer/Virtual/Control/AbstractControl.h"

namespace Analyzer {}

#include "Collection/Collection.template"

namespace Analyzer { namespace Scalar {

Control::AbstractType*
VirtualElement::createType() const { return new Control::GenericTypeSize(_getSizeInBits()); }

}} // end of namespace Analyzer::Scalar

template class COL::TreeParent<Analyzer::Scalar::ScopedVariableDeclaration, Analyzer::Scalar::ScopedVariableDeclaration::RegistrationCast>;

