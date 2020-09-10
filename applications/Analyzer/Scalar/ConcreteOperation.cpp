/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : operations
// File      : ConcreteOperation.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of all available scalar atomic operations.
//

#include "Analyzer/Scalar/ConcreteOperation.h"
// #include "Analyzer/Scalar/Scalar.hpp"

namespace Analyzer { namespace Scalar { namespace Bit {

void
Operation::write(STG::IOObject::OSBase& out, const STG::IOObject::FormatParameters& params) const {
   switch (queryOwnField()) {
      case TCastChar: out << "(char)"; break;
      case TCastInt: out << "(int)"; break;
      case TCastUnsignedInt: out << "(unsigned)"; break;
      case TCastMultiBit: out << "(multi)"; break;
      case TPrevAssign: out << "--"; break;
      case TNextAssign: out << "++"; break;
      case TPlusAssign: out << "+"; break;
      case TMinusAssign: out << "-"; break;
      case TCompareLess: out << "<"; break;
      case TCompareLessOrEqual: out << "<="; break;
      case TCompareEqual: out << "=="; break;
      case TCompareDifferent: out << "!="; break;
      case TCompareGreaterOrEqual: out << ">="; break;
      case TCompareGreater: out << ">"; break;
      case TMinAssign: out << "min"; break;
      case TMaxAssign: out << "max"; break;
      case TOrAssign: out << "|"; break;
      case TAndAssign: out << "&"; break;
      case TExclusiveOrAssign: out << "^"; break;
      case TNegateAssign: out << "~"; break;
   };
}

} // end of namespace Bit

namespace MultiBit {

Floating::Operation
Operation::getFloatOperation() const {
   Type type = getType();
   Floating::Operation result;
   if (type == TPlusFloatAssign)
      result.setPlusAssign();
   else if (type == TMinusFloatAssign)
      result.setMinusAssign();
   else if ((type >= TCompareLessFloat) && (type <= TCompareGreaterFloat))
      result.setCompare((Floating::Operation::Type) ((type - TCompareLessFloat) + Floating::Operation::StartOfCompare));
   else if (type == TMinFloatAssign)
      result.setMinAssign();
   else if (type == TMaxFloatAssign)
      result.setMaxAssign();
   else if (type == TTimesFloatAssign)
      result.setTimesAssign();
   else if (type == TDivideFloatAssign)
      result.setDivideAssign();
   else if (type == TOppositeFloatAssign)
      result.setOppositeAssign();
   else if ((type >= EndOfInteger) && (type < EndOfType))
      result.setTranscendental((Floating::Operation::Type) ((type - EndOfInteger) + Floating::Operation::EndOfNumeric));
   return result;
}

Operation&
Operation::setFloatOperation(const Floating::Operation& operation) {
   Floating::Operation::Type type = operation.getType();
   if (type == Floating::Operation::TPlusAssign)
      setPlusFloatAssign();
   else if (type == Floating::Operation::TMinusAssign)
      setMinusFloatAssign();
   else if ((type >= Floating::Operation::StartOfCompare) && (type < Floating::Operation::EndOfCompare))
      setField((Type) ((type - Floating::Operation::StartOfCompare) + TCompareLessFloat));
   else if (type == Floating::Operation::TMinAssign)
      setMinFloatAssign();
   else if (type == Floating::Operation::TMaxAssign)
      setMaxFloatAssign();
   else if (type == Floating::Operation::TTimesAssign)
      setTimesFloatAssign();
   else if (type == Floating::Operation::TDivideAssign)
      setDivideFloatAssign();
   else if (type == Floating::Operation::TOppositeAssign)
      setOppositeFloatAssign();
   else if ((type >= Floating::Operation::EndOfNumeric) && (type < Floating::Operation::EndOfType))
      setField((Type) ((type - Floating::Operation::EndOfNumeric) + EndOfType));
   return *this;
}

} // end of namespace MultiBit

namespace Integer {

void
Operation::write(STG::IOObject::OSBase& out, const STG::IOObject::FormatParameters& params) const {
   switch (queryOwnField()) {
      case TCastChar: out << "(char)"; break;
      case TCastSignedChar: out << "(signed char)"; break;
      case TCastUnsignedChar: out << "(unsigned char)"; break;
      case TCastShort: out << "(short)"; break;
      case TCastUnsignedShort: out << "(unsigned short)"; break;
      case TCastInt: out << "(int)"; break;
      case TCastUnsignedInt: out << "(unsigned)"; break;
      case TCastLong: out << "(long)"; break;
      case TCastUnsignedLong: out << "(unsigned long)"; break;
      case TCastFloat: out << "(float)"; break;
      case TCastDouble: out << "(double)"; break;
      case TCastLongDouble: out << "(long double)"; break;
      case TCastMultiBit: out << "(multi)"; break;
      case TCastBit: out << "(bit)"; break;
      case TPrevAssign: out << "--"; break;
      case TNextAssign: out << "++"; break;
      case TPlusAssign: out << "+"; break;
      case TMinusAssign: out << "-"; break;
      case TMinusSignedAssign: out << "-"; break;
      case TCompareLess: out << "<"; break;
      case TCompareLessOrEqual: out << "<="; break;
      case TCompareEqual: out << "=="; break;
      case TCompareDifferent: out << "!="; break;
      case TCompareGreaterOrEqual: out << ">="; break;
      case TCompareGreater: out << ">"; break;
      case TMinAssign: out << "min"; break;
      case TMaxAssign: out << "max"; break;
      case TTimesAssign: out << "*"; break;
      case TDivideAssign: out << "/"; break;
      case TOppositeAssign: out << "-"; break;
      case TModuloAssign: out << "%"; break;
      case TBitOrAssign: out << "|"; break;
      case TBitAndAssign: out << "&"; break;
      case TBitExclusiveOrAssign: out << "^"; break;
      case TBitNegateAssign: out << "~"; break;
      case TLeftShiftAssign: out << "<<"; break;
      case TRightShiftAssign: out << ">>"; break;
      case TLeftRotateAssign: out << "<<_rot"; break;
      case TRightRotateAssign: out << ">>_rot"; break;
      case TLogicalAndAssign: out << "&&"; break;
      case TLogicalOrAssign: out << "||"; break;
      case TLogicalNegateAssign: out << "!"; break;
   };
}

} // end of namespace Integer

namespace Floating {

void
Operation::write(STG::IOObject::OSBase& out, const STG::IOObject::FormatParameters& params) const {
   switch (queryOwnField()) {
      case TCastChar: out << "(char)"; break;
      case TCastSignedChar: out << "(signed char)"; break;
      case TCastUnsignedChar: out << "(unsigned char)"; break;
      case TCastShort: out << "(short)"; break;
      case TCastUnsignedShort: out << "(unsigned short)"; break;
      case TCastInt: out << "(int)"; break;
      case TCastUnsignedInt: out << "(unsigned int)"; break;
      case TCastLong: out << "(long)"; break;
      case TCastUnsignedLong: out << "(unsigned long)"; break;
      case TCastMultiFloat: out << "(multi float)"; break;
      case TCastFloat: out << "(float)"; break;
      case TCastDouble: out << "(double)"; break;
      case TCastLongDouble: out << "(long double)"; break;
      case TCastRational: out << "(rational)"; break;
      case TCastMultiBit: out << "(multi)"; break;
      case TCastMultiBitPointer: out << "(multi pointer)"; break;
      case TPlusAssign: isAssign() ? out << "+=" : out << '+'; break;
      case TMinusAssign: isAssign() ? out << "-=" : out << '-'; break;
      case TCompareLess: out << "<"; break;
      case TCompareLessOrEqual: out << "<="; break;
      case TCompareEqual: out << "=="; break;
      case TCompareDifferent: out << "!="; break;
      case TCompareGreaterOrEqual: out << ">="; break;
      case TCompareGreater: out << ">"; break;
      case TIsInftyExponent: out << "is_infty"; break;
      case TIsNaN: out << "is_nan"; break;
      case TIsQNaN: out << "is_qnan"; break;
      case TIsSNaN: out << "is_snan"; break;
      case TIsPositive: out << "is_sign > 0"; break;
      case TIsZeroExponent: out << "is_exponent == 0"; break;
      case TIsNegative: out << "is_sign < 0"; break;
      case TCastMantissa: out << "mantissa"; break;
      case TMinAssign: out << "min"; break;
      case TMaxAssign: out << "max"; break;
      case TTimesAssign: isAssign() ? out << "*=" : out << '*'; break;
      case TDivideAssign: isAssign() ? out << "/=" : out << '/'; break;
      case TOppositeAssign: out << "-unary"; break;
      case TAbsAssign: out << "abs"; break;
      case TMultAddAssign: out << "*+"; break;
      case TMultSubAssign: out << "*-"; break;
      case TNegMultAddAssign: out << "-*+"; break;
      case TNegMultSubAssign: out << "-*-"; break;
      case TAcos: out << "acos"; break;
      case TAsin: out << "asin"; break;
      case TAtan: out << "atan"; break;
      case TAtan2: out << "atan2"; break;
      case TCeil: out << "ceil"; break;
      case TCos: out << "cos"; break;
      case TCosh: out << "cosh"; break;
      case TExp: out << "exp"; break;
      case TFabs: out << "fabs"; break;
      case TFloor: out << "floor"; break;
      case TFmod: out << "fmod"; break;
      case TFrexp: out << "frexp"; break;
      case TLdexp: out << "ldexp"; break;
      case TLog: out << "log"; break;
      case TLog10: out << "log10"; break;
      case TModf: out << "modf"; break;
      case TPow: out << "pow"; break;
      case TSin: out << "sin"; break;
      case TSinh: out << "sinh"; break;
      case TSqrt: out << "sqrt"; break;
      case TTan: out << "tan"; break;
      case TTanh: out << "tanh"; break;
   };
}

} // end of namespace Floating

}} // end of namespace Analyzer::Scalar

