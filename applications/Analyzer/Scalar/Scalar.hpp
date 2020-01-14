/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : scalar elements
// File      : Scalar.hpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of all the available scalar elements.
//

#ifndef Analyzer_Scalar_ScalarHPP
#define Analyzer_Scalar_ScalarHPP

#include "Analyzer/Virtual/Scalar/AtomicElement.h"
#include "Analyzer/Scalar/ConcreteOperation.h"
#include "Analyzer/Scalar/Implementation/Native/IntegerElement.h"
#include "Analyzer/Scalar/Implementation/Native/FloatElement.h"
#include "Analyzer/Scalar/Implementation/General/MultiBitElement.h"
#include "Analyzer/Scalar/Implementation/General/MultiFloatElement.h"
#include "Analyzer/Scalar/Constant/ConstantElement.h"
#include "Analyzer/Scalar/Exact/ExactElement.h"
#include "Analyzer/Scalar/Approximate/VirtualElement.h"
#include "Analyzer/Scalar/Approximate/General/ConstantBit.h"
#include "Analyzer/Scalar/Approximate/General/ConstantMultiBit.h"
#include "Analyzer/Scalar/Approximate/Native/ConstantIntegerElement.h"
#include "Analyzer/Scalar/Approximate/Native/ConstantFloatElement.h"
#include "Analyzer/Scalar/Approximate/General/ConstantMultiFloat.h"
#include "Analyzer/Scalar/Approximate/General/ConstantMultiBitShare.h"
#include "Analyzer/Scalar/Approximate/IntervalInteger.h"
#include "Analyzer/Scalar/Approximate/IntervalFloat.h"
#include "Analyzer/Scalar/Approximate/Disjunction.h"
#include "Analyzer/Scalar/Approximate/FormalOperation.h"
#include "Analyzer/Scalar/Approximate/Guard.h"
#include "Analyzer/Scalar/Approximate/Conjunction.h"
#include "Analyzer/Scalar/Approximate/Top.h"
#include "Analyzer/Scalar/Approximate/PrecisionLattice.h"

#endif // Analyzer_Scalar_ScalarHPP

