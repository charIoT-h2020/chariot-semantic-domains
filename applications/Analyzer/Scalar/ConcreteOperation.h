/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : operations
// File      : ConcreteOperation.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of all available scalar atomic operations.
//

#ifndef Analyzer_Scalar_ConcreteOperationH
#define Analyzer_Scalar_ConcreteOperationH

#include "Analyzer/Virtual/Scalar/OperationElement.h"

namespace Analyzer { namespace Scalar {

#define DefineBasicOperation(Kind)                                                                \
class Kind##Operation : public Operation {                                                        \
  public:                                                                                         \
   Kind##Operation() { set##Kind(); }                                                             \
   Kind##Operation(const Kind##Operation& source) = default;                                      \
};

namespace DInt {
   
template <class TypeBase>
class TCastMultiBitOperation : public TypeBase {
  private:
   typedef TCastMultiBitOperation<TypeBase> thisType;
   typedef TypeBase inherited;
   int uResultSize;

  protected:
   TemplateDefineExtendedParameters(1, TypeBase)

  public:
   TCastMultiBitOperation() : uResultSize(0) {}
   TCastMultiBitOperation(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   TemplateDefineCopy(TCastMultiBitOperation, TypeBase)
   DTemplateDefineAssign(TCastMultiBitOperation, TypeBase)

   virtual bool isSimple() const override { return false; }
   thisType& setSize(int resultSize) { uResultSize = resultSize; return *this; }
   thisType& setSigned() { mergeOwnField(1); return *this; }
   thisType& setSigned(bool isSigned) { mergeOwnField(isSigned ? 1 : 0); return *this; }
   bool isSigned() const { return hasOwnField(); }
   const int& getResultSize() const { return uResultSize; }
};

} // end of namespace DInt

namespace DReal {

template <class TypeBase>
class TCastMultiFloatOperation : public TypeBase {
  private:
   typedef TCastMultiFloatOperation<TypeBase> thisType;
   typedef TypeBase inherited;
   int uSizeMantissa;
   int uSizeExponent;

  protected:
   TemplateDefineExtendedParameters(1, TypeBase)

  public:
   TCastMultiFloatOperation() : uSizeMantissa(0), uSizeExponent(0) {}
   TCastMultiFloatOperation(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   TemplateDefineCopy(TCastMultiFloatOperation, TypeBase)
   DTemplateDefineAssign(TCastMultiFloatOperation, TypeBase)
   
   virtual bool isSimple() const override { return false; }
   thisType& setSizeExponent(int sizeExponent) { uSizeExponent = sizeExponent; return *this; }
   thisType& setSizeMantissa(int sizeMantissa) { uSizeMantissa = sizeMantissa; return *this; }
   const int& getSizeExponent() const { return uSizeExponent; }
   const int& getSizeMantissa() const { return uSizeMantissa; }
   
   thisType& setSigned() { mergeOwnField(1); return *this; }
   thisType& setSigned(bool fSigned) { mergeOwnField(fSigned ? 1 : 0); return *this; }
   bool isSigned() const { return hasOwnField(); }
   thisType& setPart(const thisType& source)
      {  uSizeMantissa = source.uSizeMantissa;
         uSizeExponent = source.uSizeExponent;
         setOwnField(source.queryOwnField());
         return *this;
      };
};

} // end of namespace DReal

/*************************************/
/* Definition of the class Operation */
/*************************************/

namespace Bit {

class Operation : public VirtualOperation {
  public:
   enum Type {
      TUndefined, TCastChar, TCastInt, TCastUnsignedInt, TCastMultiBit, EndOfCast,
      TPrevAssign = EndOfCast, TNextAssign, EndOfUnary,
      TPlusAssign = EndOfUnary, TMinusAssign, EndOfBinary,
      StartOfCompare = EndOfBinary, TCompareLess = StartOfCompare, TCompareLessOrEqual,
         TCompareEqual, TCompareDifferent, TCompareGreaterOrEqual, TCompareGreater, EndOfCompare,
      TMinAssign = EndOfCompare, TMaxAssign, EndOfAtomic,
      TOrAssign = EndOfAtomic, TAndAssign, TExclusiveOrAssign, TNegateAssign, EndOfType
   };

  protected:
   DefineExtendedParameters(5, VirtualOperation)

  private:
   typedef VirtualOperation inherited;
   Operation& setField(Type type)
      {  AssumeCondition(!hasOwnField()) mergeOwnField(type); return *this; }

  protected:
   Operation& setCastMultiBit()  { setConst(); setArgumentsNumber(0); return setField(TCastMultiBit); }

  public:
   Operation() {}
   Operation(const Operation& source) = default;
   DefineCopy(Operation)
   virtual bool isValid() const override { return hasOwnField(); }
      
   Type getType() const { return (Type) queryOwnField(); }
   Operation& setType(Type type) { setOwnField(type); return *this; }

   Operation& setCastChar()      { setConst(); setArgumentsNumber(0); return setField(TCastChar); }
   Operation& setCastInt()       { setConst(); setArgumentsNumber(0); return setField(TCastInt); }
   Operation& setCastUnsignedInt() { setConst(); setArgumentsNumber(0); return setField(TCastUnsignedInt); }
   Operation& setPrevAssign()    { setArgumentsNumber(0); return setField(TPrevAssign); }
   Operation& setNextAssign()    { setArgumentsNumber(0); return setField(TNextAssign); }
   Operation& setPlusAssign()    { setArgumentsNumber(1); return setField(TPlusAssign); }
   Operation& setMinusAssign()   { setArgumentsNumber(1); return setField(TMinusAssign); }
   Operation& setMinAssign()     { setArgumentsNumber(1); return setField(TMinAssign); }
   Operation& setMaxAssign()     { setArgumentsNumber(1); return setField(TMaxAssign); }
   Operation& setOrAssign()      { setArgumentsNumber(1); return setField(TOrAssign); }
   Operation& setAndAssign()     { setArgumentsNumber(1); return setField(TAndAssign); }
   Operation& setExclusiveOrAssign(){ setArgumentsNumber(1); return setField(TExclusiveOrAssign); }
   Operation& setNegateAssign()  { setArgumentsNumber(0); return setField(TNegateAssign); }
   
   Operation& setPlus()          { setConstWithAssign(); setArgumentsNumber(1); return setField(TPlusAssign); }
   Operation& setMinus()         { setConstWithAssign(); setArgumentsNumber(1); return setField(TMinusAssign); }
   Operation& setMin()           { setConstWithAssign(); setArgumentsNumber(1); return setField(TMinAssign); }
   Operation& setMax()           { setConstWithAssign(); setArgumentsNumber(1); return setField(TMaxAssign); }
   Operation& setOr()            { setConstWithAssign(); setArgumentsNumber(1); return setField(TOrAssign); }
   Operation& setAnd()           { setConstWithAssign(); setArgumentsNumber(1); return setField(TAndAssign); }
   Operation& setExclusiveOr()   { setConstWithAssign(); setArgumentsNumber(1); return setField(TExclusiveOrAssign); }
   Operation& setNegate()        { setConstWithAssign(); setArgumentsNumber(0); return setField(TNegateAssign); }

   Operation& setCompareLess()         { setConst(); setArgumentsNumber(1); return setField(TCompareLess); }
   Operation& setCompareLessOrEqual()  { setConst(); setArgumentsNumber(1); return setField(TCompareLessOrEqual); }
   Operation& setCompareEqual()        { setConst(); setArgumentsNumber(1); return setField(TCompareEqual); }
   Operation& setCompareDifferent()    { setConst(); setArgumentsNumber(1); return setField(TCompareDifferent); }
   Operation& setCompareGreater()      { setConst(); setArgumentsNumber(1); return setField(TCompareGreater); }
   Operation& setCompareGreaterOrEqual() { setConst(); setArgumentsNumber(1); return setField(TCompareGreaterOrEqual); }

   void write(STG::IOObject::OSBase& out, const STG::IOObject::FormatParameters& params) const;
};

DefineBasicOperation(CastChar)
DefineBasicOperation(CastInt)
DefineBasicOperation(CastUnsignedInt)
DefineBasicOperation(PrevAssign)
DefineBasicOperation(NextAssign)
DefineBasicOperation(PlusAssign)
DefineBasicOperation(MinusAssign)
DefineBasicOperation(CompareLess)
DefineBasicOperation(CompareLessOrEqual)
DefineBasicOperation(CompareEqual)
DefineBasicOperation(CompareDifferent)
DefineBasicOperation(CompareGreater)
DefineBasicOperation(CompareGreaterOrEqual)
DefineBasicOperation(MinAssign)
DefineBasicOperation(MaxAssign)
DefineBasicOperation(OrAssign)
DefineBasicOperation(AndAssign)
DefineBasicOperation(ExclusiveOrAssign)
DefineBasicOperation(NegateAssign)

class CastMultiBitOperation : public DInt::TCastMultiBitOperation<Operation> {
  public:
   CastMultiBitOperation() { setCastMultiBit(); }
   CastMultiBitOperation(const CastMultiBitOperation& source) = default;
   DefineCopy(CastMultiBitOperation)
};

} // end of namespace Bit

namespace Floating { class Operation; }

namespace MultiBit {

class Operation : public VirtualOperation {
  public:
   enum Type {
      TUndefined, TExtendWithZero, TExtendWithSign, TConcat, TReduce, TBitSet, EndOfExtension,
      TCastChar = EndOfExtension, TCastInt, TCastUnsignedInt, TCastMultiBit, TCastBit,
         TCastShiftBit, TCastMultiFloat, TCastMultiFloatPointer, TCastReal, TCastRational, EndOfCast,
      TPrevSignedAssign = EndOfCast, TPrevUnsignedAssign, TNextSignedAssign, TNextUnsignedAssign,
         EndOfUnary,
      StartOfBinary = EndOfUnary, TPlusSignedAssign = StartOfBinary, TPlusUnsignedAssign,
         TPlusUnsignedWithSignedAssign, TPlusFloatAssign, TMinusSignedAssign, TMinusUnsignedAssign,
         TMinusFloatAssign, EndOfBinary,
      StartOfCompare = EndOfBinary, TCompareLessSigned = StartOfCompare, TCompareLessOrEqualSigned,
         TCompareLessUnsigned, TCompareLessOrEqualUnsigned, TCompareLessFloat, TCompareLessOrEqualFloat,
         TCompareEqual, TCompareEqualFloat, TCompareDifferentFloat, TCompareDifferent,
         TCompareGreaterOrEqualFloat, TCompareGreaterFloat, TCompareGreaterOrEqualUnsigned, TCompareGreaterUnsigned,
         TCompareGreaterOrEqualSigned, TCompareGreaterSigned, EndOfCompare,
      TMinSignedAssign = EndOfCompare, TMinUnsignedAssign,
         TMinFloatAssign, TMaxSignedAssign, TMaxUnsignedAssign, TMaxFloatAssign, EndOfAtomic,
      TTimesSignedAssign = EndOfAtomic, TTimesUnsignedAssign, TTimesFloatAssign,
         TDivideSignedAssign, TDivideUnsignedAssign, TDivideFloatAssign, TOppositeSignedAssign,
         TOppositeFloatAssign, EndOfNumeric,
      TModuloSignedAssign = EndOfNumeric, TModuloUnsignedAssign, TBitOrAssign, TBitAndAssign,
         TBitExclusiveOrAssign, TBitNegateAssign, TLeftShiftAssign, TLogicalRightShiftAssign,
         TArithmeticRightShiftAssign, TLeftRotateAssign, TRightRotateAssign, EndOfInteger,
      TAcos = EndOfInteger, TAsin, TAtan, TAtan2, TCeil, TCos, TCosh, TExp, TFabs, TFloor, TFmod,
         TFrexp, TLdexp, TLog, TLog10, TModf, TPow, TSin, TSinh, TSqrt, TTan, TTanh, EndOfType
   };
   
   bool isLessCompare() const { return (queryOwnField() >= TCompareLessSigned) && (queryOwnField() <= TCompareLessOrEqualFloat); }
   bool isGreaterCompare() const { return (queryOwnField() >= TCompareGreaterOrEqualFloat) && (queryOwnField() <= TCompareGreaterSigned); }
   bool isEqualCompare() const { return (queryOwnField() == TCompareEqual) || (queryOwnField() == TCompareEqualFloat); }
   bool isDifferentCompare() const { return (queryOwnField() == TCompareDifferent) || (queryOwnField() == TCompareDifferentFloat); }

  protected:
   DefineExtendedParameters(7, VirtualOperation)

  private:
   typedef VirtualOperation inherited;
   Operation& setField(Type type)
      {  AssumeCondition(!hasOwnField()) mergeOwnField(type); return *this; }

  protected:
   Operation& setCastMultiBit()   { setConstWithAssign(); setArgumentsNumber(0); return setField(TCastMultiBit); }
   Operation& setCastMultiBitAssign() { setArgumentsNumber(0); return setField(TCastMultiBit); }
   Operation& setExtendWithZero() { setArgumentsNumber(0); return setField(TExtendWithZero); }
   Operation& setExtendWithSign() { setArgumentsNumber(0); return setField(TExtendWithSign); }
   Operation& setReduce() { setArgumentsNumber(0); return setField(TReduce); }
   Operation& setBitSet() { setArgumentsNumber(1); return setField(TBitSet); }

   Operation& setCastShiftBit()        { setConst(); setArgumentsNumber(0); return setField(TCastShiftBit); }
   Operation& setCastMultiFloat()      { setConst(); setArgumentsNumber(0); return setField(TCastMultiFloat); }
   Operation& setCastMultiFloatPointer() { setConst(); setArgumentsNumber(0); return setField(TCastMultiFloatPointer); }
   Operation& setPlusFloatAssign()     { setArgumentsNumber(1); return setField(TPlusFloatAssign); }
   Operation& setMinusFloatAssign()    { setArgumentsNumber(1); return setField(TMinusFloatAssign); }
   Operation& setMinFloatAssign()      { setArgumentsNumber(1); return setField(TMinFloatAssign); }
   Operation& setMaxFloatAssign()      { setArgumentsNumber(1); return setField(TMaxFloatAssign); }
   Operation& setTimesFloatAssign()    { setArgumentsNumber(1); return setField(TTimesFloatAssign); }
   Operation& setDivideFloatAssign()   { setArgumentsNumber(1); return setField(TDivideFloatAssign); }
   Operation& setOppositeFloatAssign() { setArgumentsNumber(0); return setField(TOppositeFloatAssign); }
   Operation& setPlusFloat()     { setConstWithAssign(); setArgumentsNumber(1); return setField(TPlusFloatAssign); }
   Operation& setMinusFloat()    { setConstWithAssign(); setArgumentsNumber(1); return setField(TMinusFloatAssign); }
   Operation& setMinFloat()      { setConstWithAssign(); setArgumentsNumber(1); return setField(TMinFloatAssign); }
   Operation& setMaxFloat()      { setConstWithAssign(); setArgumentsNumber(1); return setField(TMaxFloatAssign); }
   Operation& setTimesFloat()    { setConstWithAssign(); setArgumentsNumber(1); return setField(TTimesFloatAssign); }
   Operation& setDivideFloat()   { setConstWithAssign(); setArgumentsNumber(1); return setField(TDivideFloatAssign); }
   Operation& setOppositeFloat() { setConstWithAssign(); setArgumentsNumber(0); return setField(TOppositeFloatAssign); }
   Operation& setCompareLessFloat()          { setConst(); setArgumentsNumber(1); return setField(TCompareLessFloat); }
   Operation& setCompareLessOrEqualFloat()   { setConst(); setArgumentsNumber(1); return setField(TCompareLessOrEqualFloat); }
   Operation& setCompareEqualFloat()         { setConst(); setArgumentsNumber(1); return setField(TCompareEqualFloat); }
   Operation& setCompareDifferentFloat()     { setConst(); setArgumentsNumber(1); return setField(TCompareDifferentFloat); }
   Operation& setCompareGreaterFloat()       { setConst(); setArgumentsNumber(1); return setField(TCompareGreaterFloat); }
   Operation& setCompareGreaterOrEqualFloat(){ setConst(); setArgumentsNumber(1); return setField(TCompareGreaterOrEqualFloat); }

  public:
   Operation() {}
   Operation(const Operation& source) = default;
   DefineCopy(Operation)
   virtual bool isValid() const override { return hasOwnField(); }

   Type getType() const { return (Type) queryOwnField(); }
   Operation& setType(Type type) { setOwnField(type); return *this; }

   Floating::Operation getFloatOperation() const;
   Operation& setFloatOperation(const Floating::Operation& operation);
   
   Operation& setCastChar()      { setConst(); setArgumentsNumber(0); return setField(TCastChar); }
   Operation& setCastInt()       { setConst(); setArgumentsNumber(0); return setField(TCastInt); }
   Operation& setCastUnsignedInt() { setConst(); setArgumentsNumber(0); return setField(TCastUnsignedInt); }
   Operation& setCastBit()       { setConst(); setArgumentsNumber(0); return setField(TCastBit); }
   Operation& setCastReal()      { setConst(); setArgumentsNumber(0); return setField(TCastReal); }
   Operation& setCastRational()  { setConst(); setArgumentsNumber(0); return setField(TCastRational); }
   Operation& setPrevSignedAssign()    { setArgumentsNumber(0); return setField(TPrevSignedAssign); }
   Operation& setPrevUnsignedAssign()  { setArgumentsNumber(0); return setField(TPrevUnsignedAssign); }
   Operation& setNextSignedAssign()    { setArgumentsNumber(0); return setField(TNextSignedAssign); }
   Operation& setNextUnsignedAssign()  { setArgumentsNumber(0); return setField(TNextUnsignedAssign); }
   Operation& setPlusSignedAssign()    { setArgumentsNumber(1); return setField(TPlusSignedAssign); }
   Operation& setPlusUnsignedAssign()  { setArgumentsNumber(1); return setField(TPlusUnsignedAssign); }
   Operation& setPlusUnsignedWithSignedAssign()  { setArgumentsNumber(1); return setField(TPlusUnsignedWithSignedAssign); }
   Operation& setMinusSignedAssign()   { setArgumentsNumber(1); return setField(TMinusSignedAssign); }
   Operation& setMinusUnsignedAssign() { setArgumentsNumber(1); return setField(TMinusUnsignedAssign); }
   Operation& setMinSignedAssign()     { setArgumentsNumber(1); return setField(TMinSignedAssign); }
   Operation& setMinUnsignedAssign()   { setArgumentsNumber(1); return setField(TMinUnsignedAssign); }
   Operation& setMaxSignedAssign()     { setArgumentsNumber(1); return setField(TMaxSignedAssign); }
   Operation& setMaxUnsignedAssign()   { setArgumentsNumber(1); return setField(TMaxUnsignedAssign); }
   Operation& setTimesSignedAssign()   { setArgumentsNumber(1); return setField(TTimesSignedAssign); }
   Operation& setTimesUnsignedAssign() { setArgumentsNumber(1); return setField(TTimesUnsignedAssign); }
   Operation& setDivideSignedAssign()  { setArgumentsNumber(1); return setField(TDivideSignedAssign); }
   Operation& setDivideUnsignedAssign(){ setArgumentsNumber(1); return setField(TDivideUnsignedAssign); }
   Operation& setOppositeSignedAssign(){ setArgumentsNumber(0); return setField(TOppositeSignedAssign); }
   Operation& setModuloSignedAssign()  { setArgumentsNumber(1); return setField(TModuloSignedAssign); }
   Operation& setModuloUnsignedAssign(){ setArgumentsNumber(1); return setField(TModuloUnsignedAssign); }
   Operation& setBitOrAssign()         { setArgumentsNumber(1); return setField(TBitOrAssign); }
   Operation& setBitAndAssign()        { setArgumentsNumber(1); return setField(TBitAndAssign); }
   Operation& setBitExclusiveOrAssign(){ setArgumentsNumber(1); return setField(TBitExclusiveOrAssign); }
   Operation& setBitNegateAssign()     { setArgumentsNumber(0); return setField(TBitNegateAssign); }
   Operation& setLeftShiftAssign()     { setArgumentsNumber(1); return setField(TLeftShiftAssign); }
   Operation& setLogicalRightShiftAssign()    { setArgumentsNumber(1); return setField(TLogicalRightShiftAssign); }
   Operation& setArithmeticRightShiftAssign() { setArgumentsNumber(1); return setField(TArithmeticRightShiftAssign); }
   Operation& setLeftRotateAssign()    { setArgumentsNumber(1); return setField(TLeftRotateAssign); }
   Operation& setRightRotateAssign()   { setArgumentsNumber(1); return setField(TRightRotateAssign); }

   Operation& setConcat() { setArgumentsNumber(1); return setField(TConcat); }
   
   Operation& setPlusSigned()    { setConstWithAssign(); setArgumentsNumber(1); return setField(TPlusSignedAssign); }
   Operation& setPlusUnsigned()  { setConstWithAssign(); setArgumentsNumber(1); return setField(TPlusUnsignedAssign); }
   Operation& setPlusUnsignedWithSigned()  { setConstWithAssign(); setArgumentsNumber(1); return setField(TPlusUnsignedWithSignedAssign); }
   Operation& setMinusSigned()   { setConstWithAssign(); setArgumentsNumber(1); return setField(TMinusSignedAssign); }
   Operation& setMinusUnsigned() { setConstWithAssign(); setArgumentsNumber(1); return setField(TMinusUnsignedAssign); }
   Operation& setMinSigned()     { setConstWithAssign(); setArgumentsNumber(1); return setField(TMinSignedAssign); }
   Operation& setMinUnsigned()   { setConstWithAssign(); setArgumentsNumber(1); return setField(TMinUnsignedAssign); }
   Operation& setMaxSigned()     { setConstWithAssign(); setArgumentsNumber(1); return setField(TMaxSignedAssign); }
   Operation& setMaxUnsigned()   { setConstWithAssign(); setArgumentsNumber(1); return setField(TMaxUnsignedAssign); }
   Operation& setTimesSigned()   { setConstWithAssign(); setArgumentsNumber(1); return setField(TTimesSignedAssign); }
   Operation& setTimesUnsigned() { setConstWithAssign(); setArgumentsNumber(1); return setField(TTimesUnsignedAssign); }
   Operation& setDivideSigned()  { setConstWithAssign(); setArgumentsNumber(1); return setField(TDivideSignedAssign); }
   Operation& setDivideUnsigned(){ setConstWithAssign(); setArgumentsNumber(1); return setField(TDivideUnsignedAssign); }
   Operation& setOppositeSigned(){ setConstWithAssign(); setArgumentsNumber(0); return setField(TOppositeSignedAssign); }
   Operation& setModuloSigned()  { setConstWithAssign(); setArgumentsNumber(1); return setField(TModuloSignedAssign); }
   Operation& setModuloUnsigned(){ setConstWithAssign(); setArgumentsNumber(1); return setField(TModuloUnsignedAssign); }
   Operation& setBitOr()         { setConstWithAssign(); setArgumentsNumber(1); return setField(TBitOrAssign); }
   Operation& setBitAnd()        { setConstWithAssign(); setArgumentsNumber(1); return setField(TBitAndAssign); }
   Operation& setBitExclusiveOr(){ setConstWithAssign(); setArgumentsNumber(1); return setField(TBitExclusiveOrAssign); }
   Operation& setBitNegate()     { setConstWithAssign(); setArgumentsNumber(0); return setField(TBitNegateAssign); }
   Operation& setLeftShift()     { setConstWithAssign(); setArgumentsNumber(1); return setField(TLeftShiftAssign); }
   Operation& setLogicalRightShift() { setConstWithAssign(); setArgumentsNumber(1); return setField(TLogicalRightShiftAssign); }
   Operation& setArithmeticRightShift() { setConstWithAssign(); setArgumentsNumber(1); return setField(TArithmeticRightShiftAssign); }
   Operation& setLeftRotate()    { setConstWithAssign(); setArgumentsNumber(1); return setField(TLeftRotateAssign); }
   Operation& setRightRotate()   { setConstWithAssign(); setArgumentsNumber(1); return setField(TRightRotateAssign); }

   Operation& setAcos()    { setConst(); setArgumentsNumber(0); return setField(TAcos); }
   Operation& setAsin()    { setConst(); setArgumentsNumber(0); return setField(TAsin); }
   Operation& setAtan()    { setConst(); setArgumentsNumber(0); return setField(TAtan); }
   Operation& setAtan2()   { setConst(); setArgumentsNumber(1); return setField(TAtan2); }
   Operation& setCeil()    { setConst(); setArgumentsNumber(0); return setField(TCeil); }
   Operation& setCos()     { setConst(); setArgumentsNumber(0); return setField(TCos); }
   Operation& setCosh()    { setConst(); setArgumentsNumber(0); return setField(TCosh); }
   Operation& setExp()     { setConst(); setArgumentsNumber(0); return setField(TExp); }
   Operation& setFabs()    { setConst(); setArgumentsNumber(0); return setField(TFabs); }
   Operation& setFloor()   { setConst(); setArgumentsNumber(0); return setField(TFloor); }
   Operation& setFmod()    { setConst(); setArgumentsNumber(1); return setField(TFmod); }
   Operation& setFrexp()   { setConst(); setArgumentsNumber(1); return setField(TFrexp); }
   Operation& setLdexp()   { setConst(); setArgumentsNumber(1); return setField(TLdexp); }
   Operation& setLog()     { setConst(); setArgumentsNumber(0); return setField(TLog); }
   Operation& setLog10()   { setConst(); setArgumentsNumber(0); return setField(TLog10); }
   Operation& setModf()    { setConst(); setArgumentsNumber(1); return setField(TModf); }
   Operation& setPow()     { setConst(); setArgumentsNumber(1); return setField(TPow); }
   Operation& setSin()     { setConst(); setArgumentsNumber(0); return setField(TSin); }
   Operation& setSinh()    { setConst(); setArgumentsNumber(0); return setField(TSinh); }
   Operation& setSqrt()    { setConst(); setArgumentsNumber(0); return setField(TSqrt); }
   Operation& setTan()     { setConst(); setArgumentsNumber(0); return setField(TTan); }
   Operation& setTanh()    { setConst(); setArgumentsNumber(0); return setField(TTanh); }
   
   Operation& setCompareLessSigned()         { setConst(); setArgumentsNumber(1); return setField(TCompareLessSigned); }
   Operation& setCompareLessOrEqualSigned()  { setConst(); setArgumentsNumber(1); return setField(TCompareLessOrEqualSigned); }
   Operation& setCompareLessUnsigned()       { setConst(); setArgumentsNumber(1); return setField(TCompareLessUnsigned); }
   Operation& setCompareLessOrEqualUnsigned(){ setConst(); setArgumentsNumber(1); return setField(TCompareLessOrEqualUnsigned); }
   Operation& setCompareEqual()              { setConst(); setArgumentsNumber(1); return setField(TCompareEqual); }
   Operation& setCompareDifferent()          { setConst(); setArgumentsNumber(1); return setField(TCompareDifferent); }
   Operation& setCompareGreaterSigned()      { setConst(); setArgumentsNumber(1); return setField(TCompareGreaterSigned); }
   Operation& setCompareGreaterUnsigned()    { setConst(); setArgumentsNumber(1); return setField(TCompareGreaterUnsigned); }
   Operation& setCompareGreaterOrEqualSigned() { setConst(); setArgumentsNumber(1); return setField(TCompareGreaterOrEqualSigned); }
   Operation& setCompareGreaterOrEqualUnsigned() { setConst(); setArgumentsNumber(1); return setField(TCompareGreaterOrEqualUnsigned); }
};

#define DefineBasicFloatOperation(Kind)                                                            \
class Kind##Operation : public DReal::TCastMultiFloatOperation<Operation> {                        \
  public:                                                                                          \
   Kind##Operation() { set##Kind(); }                                                              \
   Kind##Operation(const Kind##Operation& source) = default;                                       \
};

DefineBasicOperation(CastChar)
DefineBasicOperation(CastInt)
DefineBasicOperation(CastUnsignedInt)
DefineBasicOperation(CastBit)
DefineBasicFloatOperation(CastMultiFloat)
DefineBasicFloatOperation(CastMultiFloatPointer)
DefineBasicOperation(CastReal)
DefineBasicOperation(CastRational)
DefineBasicOperation(Concat)
DefineBasicOperation(PrevSignedAssign)
DefineBasicOperation(PrevUnsignedAssign)
DefineBasicOperation(NextSignedAssign)
DefineBasicOperation(NextUnsignedAssign)
DefineBasicOperation(PlusSignedAssign)
DefineBasicOperation(PlusUnsignedAssign)
DefineBasicOperation(PlusUnsignedWithSignedAssign)
DefineBasicFloatOperation(PlusFloatAssign)
DefineBasicOperation(MinusSignedAssign)
DefineBasicOperation(MinusUnsignedAssign)
DefineBasicFloatOperation(MinusFloatAssign)
DefineBasicOperation(CompareLessSigned)
DefineBasicOperation(CompareLessOrEqualSigned)
DefineBasicOperation(CompareLessUnsigned)
DefineBasicOperation(CompareLessOrEqualUnsigned)
DefineBasicFloatOperation(CompareLessFloat)
DefineBasicFloatOperation(CompareLessOrEqualFloat)
DefineBasicOperation(CompareEqual)
DefineBasicFloatOperation(CompareEqualFloat)
DefineBasicOperation(CompareDifferent)
DefineBasicFloatOperation(CompareDifferentFloat)
DefineBasicOperation(CompareGreaterSigned)
DefineBasicOperation(CompareGreaterUnsigned)
DefineBasicFloatOperation(CompareGreaterFloat)
DefineBasicOperation(CompareGreaterOrEqualSigned)
DefineBasicOperation(CompareGreaterOrEqualUnsigned)
DefineBasicFloatOperation(CompareGreaterOrEqualFloat)
DefineBasicOperation(MinSignedAssign)
DefineBasicOperation(MinUnsignedAssign)
DefineBasicFloatOperation(MinFloatAssign)
DefineBasicOperation(MaxSignedAssign)
DefineBasicOperation(MaxUnsignedAssign)
DefineBasicFloatOperation(MaxFloatAssign)
DefineBasicOperation(TimesSignedAssign)
DefineBasicOperation(TimesUnsignedAssign)
DefineBasicFloatOperation(TimesFloatAssign)
DefineBasicOperation(DivideSignedAssign)
DefineBasicOperation(DivideUnsignedAssign)
DefineBasicFloatOperation(DivideFloatAssign)
DefineBasicOperation(OppositeSignedAssign)
DefineBasicFloatOperation(OppositeFloatAssign)
DefineBasicOperation(ModuloSignedAssign)
DefineBasicOperation(ModuloUnsignedAssign)
DefineBasicOperation(BitOrAssign)
DefineBasicOperation(BitAndAssign)
DefineBasicOperation(BitExclusiveOrAssign)
DefineBasicOperation(BitNegateAssign)
DefineBasicOperation(LeftShiftAssign)
DefineBasicOperation(LogicalRightShiftAssign)
DefineBasicOperation(ArithmeticRightShiftAssign)
DefineBasicOperation(LeftRotateAssign)
DefineBasicOperation(RightRotateAssign)
DefineBasicFloatOperation(Acos)
DefineBasicFloatOperation(Asin)
DefineBasicFloatOperation(Atan)
DefineBasicFloatOperation(Atan2)
DefineBasicFloatOperation(Ceil)
DefineBasicFloatOperation(Cos)
DefineBasicFloatOperation(Cosh)
DefineBasicFloatOperation(Exp)
DefineBasicFloatOperation(Fabs)
DefineBasicFloatOperation(Floor)
DefineBasicFloatOperation(Fmod)
DefineBasicFloatOperation(Frexp)
DefineBasicFloatOperation(Ldexp)
DefineBasicFloatOperation(Log)
DefineBasicFloatOperation(Log10)
DefineBasicFloatOperation(Modf)
DefineBasicFloatOperation(Pow)
DefineBasicFloatOperation(Sin)
DefineBasicFloatOperation(Sinh)
DefineBasicFloatOperation(Sqrt)
DefineBasicFloatOperation(Tan)
DefineBasicFloatOperation(Tanh)

#undef DefineBasicFloatOperation

class CastMultiBitOperation : public Scalar::DInt::TCastMultiBitOperation<Operation> {
  public:
   CastMultiBitOperation() { setCastMultiBit(); }
};

class CastMultiBitAssignOperation : public Scalar::DInt::TCastMultiBitOperation<Operation> {
  public:
   CastMultiBitAssignOperation() { setCastMultiBitAssign(); }
};

class CastShiftBitOperation : public Operation {
  private:
   int uShift;
   
  public:
   CastShiftBitOperation(int shift) : uShift(shift) { setCastShiftBit(); }
   CastShiftBitOperation(const CastShiftBitOperation& source) = default;
   DefineCopy(CastShiftBitOperation)
   DDefineAssign(CastShiftBitOperation)
   
   int& getSShift() { return uShift; }
   int getShift() const { return uShift; }
};

class ExtendOperation : public Operation {
  private:
   typedef Operation inherited;
   int uExtension;

  public:
   ExtendOperation() : uExtension(0) {}
   ExtendOperation(const ExtendOperation& source) = default;
   DefineCopy(ExtendOperation)
   DDefineAssign(ExtendOperation)

   virtual bool isSimple() const override { return false; }
   ExtendOperation& setExtendWithZero(int extension)
      {  AssumeCondition(uExtension == 0)
         inherited::setExtendWithZero();
         uExtension = extension;
         return *this;
      }
   ExtendOperation& setExtendWithSign(int extension)
      {  AssumeCondition(uExtension == 0)
         inherited::setExtendWithSign();
         uExtension = extension;
         return *this;
      }
   const int& getExtension() const { return uExtension; }
   int& sextension() { return uExtension; }
};

class ReduceOperation : public Operation {
  private:
   typedef Operation inherited;
   int uLowBit, uHighBit;

  public:
   ReduceOperation() : uLowBit(0), uHighBit(0) { inherited::setReduce(); }
   ReduceOperation(const ReduceOperation& source) = default;
   DefineCopy(ReduceOperation)
   DDefineAssign(ReduceOperation)

   virtual bool isSimple() const override { return false; }
   ReduceOperation& setLowBit(int lowBit)
      {  AssumeCondition(uLowBit == 0)
         uLowBit = lowBit;
         return *this;
      }
   ReduceOperation& setHighBit(int highBit)
      {  AssumeCondition(uHighBit == 0)
         uHighBit = highBit;
         return *this;
      }
   int& getSLowBit() { return uLowBit; }
   int& getSHighBit() { return uHighBit; }
   const int& getLowBit() const { return uLowBit; }
   const int& getHighBit() const { return uHighBit; }
};

class BitSetOperation : public Operation {
  private:
   typedef Operation inherited;
   int uLowBit, uHighBit;
   
  public:
   BitSetOperation() : uLowBit(0), uHighBit(0) { inherited::setBitSet(); }
   BitSetOperation(const BitSetOperation& source) = default;
   DefineCopy(BitSetOperation)
   DDefineAssign(BitSetOperation)

   virtual bool isSimple() const override { return false; }
   int& getSLowBit() { return uLowBit; }
   int& getSHighBit() { return uHighBit; }
   const int& getLowBit() const { return uLowBit; }
   const int& getHighBit() const { return uHighBit; }

   BitSetOperation& setLowBit(int lowBit) { uLowBit = lowBit; return *this; }
   BitSetOperation& setHighBit(int highBit) { uHighBit = highBit; return *this; }
   bool isValid() const override { return inherited::isValid() && (uLowBit < uHighBit); }
};

} // end of namespace MultiBit

namespace Integer {

class Operation : public VirtualOperation {
  private:
   typedef VirtualOperation inherited;

  public:
   enum Type {
      TUndefined, TCastChar, TCastSignedChar, TCastUnsignedChar, TCastShort, TCastUnsignedShort,
         TCastInt, TCastUnsignedInt, TCastLong, TCastUnsignedLong, TCastFloat, TCastDouble,
         TCastLongDouble, TCastMultiBit, TCastBit, EndOfCast,
      
      TPrevAssign = EndOfCast, TNextAssign, EndOfUnary,
      StartOfBinary = EndOfUnary, TPlusAssign = StartOfBinary, TMinusAssign, TMinusSignedAssign, EndOfBinary,
      StartOfCompare = EndOfBinary, TCompareLess = StartOfCompare, TCompareLessOrEqual, TCompareEqual,
         TCompareDifferent, TCompareGreaterOrEqual, TCompareGreater, EndOfCompare,
      TMinAssign = EndOfCompare, TMaxAssign, EndOfAtomic,
      TTimesAssign = EndOfAtomic, TDivideAssign, TOppositeAssign, EndOfNumeric,
      TModuloAssign = EndOfNumeric, TBitOrAssign, TBitAndAssign, TBitExclusiveOrAssign,
         TBitNegateAssign, TLeftShiftAssign, TRightShiftAssign, TLeftRotateAssign,
         TRightRotateAssign, EndOfInteger,
      TLogicalAndAssign = EndOfInteger, TLogicalOrAssign, TLogicalNegateAssign, EndOfType
   };

  protected:
   DefineExtendedParameters(6, VirtualOperation)

  private:
   Operation& setField(Type type)
      {  AssumeCondition(!hasOwnField()) mergeOwnField(type); return *this; }

  protected:
   Operation& setCastMultiBit()  { setConst(); setArgumentsNumber(0); return setField(TCastMultiBit); }
   
  public:
   Operation() {}
   Operation(const Operation& source) = default;
   DefineCopy(Operation)
   virtual bool isValid() const override { return hasOwnField(); }

   Type getType() const { return (Type) queryOwnField(); }
   Operation& setType(Type type) { setOwnField(type); return *this; }

   Operation& setCastChar() { setConst(); setArgumentsNumber(0); return setField(TCastChar); }
   Operation& setCastSignedChar() { setConst(); setArgumentsNumber(0); return setField(TCastSignedChar); }
   Operation& setCastUnsignedChar() { setConst(); setArgumentsNumber(0); return setField(TCastUnsignedChar); }
   Operation& setCastShort() { setConst(); setArgumentsNumber(0); return setField(TCastShort); }
   Operation& setCastUnsignedShort() { setConst(); setArgumentsNumber(0); return setField(TCastUnsignedShort); }
   Operation& setCastInt() { setConst(); setArgumentsNumber(0); return setField(TCastInt); }
   Operation& setCastUnsignedInt() { setConst(); setArgumentsNumber(0); return setField(TCastUnsignedInt); }
   Operation& setCastLong() { setConst(); setArgumentsNumber(0); return setField(TCastLong); }
   Operation& setCastUnsignedLong() { setConst(); setArgumentsNumber(0); return setField(TCastUnsignedLong); }
   Operation& setCastBit() { setConst(); setArgumentsNumber(0); return setField(TCastBit); }
   Operation& setCastFloat() { setConst(); setArgumentsNumber(0); return setField(TCastFloat); }
   Operation& setCastDouble() { setConst(); setArgumentsNumber(0); return setField(TCastDouble); }
   Operation& setCastLongDouble() { setConst(); setArgumentsNumber(0); return setField(TCastLongDouble); }

   Operation& setPrevAssign()    { setArgumentsNumber(0); return setField(TPrevAssign); }
   Operation& setNextAssign()    { setArgumentsNumber(0); return setField(TNextAssign); }

   Operation& setPlusAssign()    { setArgumentsNumber(1); return setField(TPlusAssign); }
   Operation& setMinusAssign()   { setArgumentsNumber(1); return setField(TMinusAssign); }
   Operation& setMinusSignedAssign() { setArgumentsNumber(1); return setField(TMinusSignedAssign); }
   Operation& setMinAssign()     { setArgumentsNumber(1); return setField(TMinAssign); }
   Operation& setMaxAssign()     { setArgumentsNumber(1); return setField(TMaxAssign); }
   Operation& setTimesAssign()   { setArgumentsNumber(1); return setField(TTimesAssign); }
   Operation& setDivideAssign()  { setArgumentsNumber(1); return setField(TDivideAssign); }
   Operation& setOppositeAssign(){ setArgumentsNumber(0); return setField(TOppositeAssign); }
   Operation& setModuloAssign()  { setArgumentsNumber(1); return setField(TModuloAssign); }
   Operation& setBitOrAssign()   { setArgumentsNumber(1); return setField(TBitOrAssign); }
   Operation& setBitAndAssign()  { setArgumentsNumber(1); return setField(TBitAndAssign); }
   Operation& setBitExclusiveOrAssign() { setArgumentsNumber(1); return setField(TBitExclusiveOrAssign); }
   Operation& setBitNegateAssign()  { setArgumentsNumber(0); return setField(TBitNegateAssign); }
   Operation& setLeftShiftAssign()  { setArgumentsNumber(1); return setField(TLeftShiftAssign); }
   Operation& setRightShiftAssign() { setArgumentsNumber(1); return setField(TRightShiftAssign); }
   Operation& setLeftRotateAssign() { setArgumentsNumber(1); return setField(TLeftRotateAssign); }
   Operation& setRightRotateAssign(){ setArgumentsNumber(1); return setField(TRightRotateAssign); }
   Operation& setLogicalAndAssign() { setArgumentsNumber(1); return setField(TLogicalAndAssign); }
   Operation& setLogicalOrAssign()  { setArgumentsNumber(1); return setField(TLogicalOrAssign); }
   Operation& setLogicalNegateAssign()  { setArgumentsNumber(0); return setField(TLogicalNegateAssign); }

   Operation& setPlus()    { setConstWithAssign(); setArgumentsNumber(1); return setField(TPlusAssign); }
   Operation& setMinus()   { setConstWithAssign(); setArgumentsNumber(1); return setField(TMinusAssign); }
   Operation& setMinusSigned() { setConstWithAssign(); setArgumentsNumber(1); return setField(TMinusSignedAssign); }
   Operation& setMin()     { setConstWithAssign(); setArgumentsNumber(1); return setField(TMinAssign); }
   Operation& setMax()     { setConstWithAssign(); setArgumentsNumber(1); return setField(TMaxAssign); }
   Operation& setTimes()   { setConstWithAssign(); setArgumentsNumber(1); return setField(TTimesAssign); }
   Operation& setDivide()  { setConstWithAssign(); setArgumentsNumber(1); return setField(TDivideAssign); }
   Operation& setOpposite(){ setConstWithAssign(); setArgumentsNumber(0); return setField(TOppositeAssign); }
   Operation& setModulo()  { setConstWithAssign(); setArgumentsNumber(1); return setField(TModuloAssign); }
   Operation& setBitOr()   { setConstWithAssign(); setArgumentsNumber(1); return setField(TBitOrAssign); }
   Operation& setBitAnd()  { setConstWithAssign(); setArgumentsNumber(1); return setField(TBitAndAssign); }
   Operation& setBitExclusiveOr(){ setConstWithAssign(); setArgumentsNumber(1); return setField(TBitExclusiveOrAssign); }
   Operation& setBitNegate()     { setConstWithAssign(); setArgumentsNumber(0); return setField(TBitNegateAssign); }
   Operation& setLeftShift()     { setConstWithAssign(); setArgumentsNumber(1); return setField(TLeftShiftAssign); }
   Operation& setRightShift()    { setConstWithAssign(); setArgumentsNumber(1); return setField(TRightShiftAssign); }
   Operation& setLeftRotate()    { setConstWithAssign(); setArgumentsNumber(1); return setField(TLeftRotateAssign); }
   Operation& setRightRotate()   { setConstWithAssign(); setArgumentsNumber(1); return setField(TRightRotateAssign); }
   Operation& setLogicalAnd()    { setConstWithAssign(); setArgumentsNumber(1); return setField(TLogicalAndAssign); }
   Operation& setLogicalOr()     { setConstWithAssign(); setArgumentsNumber(1); return setField(TLogicalOrAssign); }
   Operation& setLogicalNegate() { setConstWithAssign(); setArgumentsNumber(0); return setField(TLogicalNegateAssign); }

   Operation& setCompareLess()         { setConst(); setArgumentsNumber(1); return setField(TCompareLess); }
   Operation& setCompareLessOrEqual()  { setConst(); setArgumentsNumber(1); return setField(TCompareLessOrEqual); }
   Operation& setCompareEqual()        { setConst(); setArgumentsNumber(1); return setField(TCompareEqual); }
   Operation& setCompareDifferent()    { setConst(); setArgumentsNumber(1); return setField(TCompareDifferent); }
   Operation& setCompareGreater()      { setConst(); setArgumentsNumber(1); return setField(TCompareGreater); }
   Operation& setCompareGreaterOrEqual() { setConst(); setArgumentsNumber(1); return setField(TCompareGreaterOrEqual); }
   
   void write(STG::IOObject::OSBase& out, const STG::IOObject::FormatParameters& params) const;
};

class CastMultiBitOperation : public Scalar::DInt::TCastMultiBitOperation<Operation> {
  public:
   CastMultiBitOperation() { setCastMultiBit(); }
};

DefineBasicOperation(CastChar)
DefineBasicOperation(CastSignedChar)
DefineBasicOperation(CastUnsignedChar)
DefineBasicOperation(CastShort)
DefineBasicOperation(CastUnsignedShort)
DefineBasicOperation(CastInt)
DefineBasicOperation(CastUnsignedInt)
DefineBasicOperation(CastLong)
DefineBasicOperation(CastUnsignedLong)
DefineBasicOperation(CastBit)
DefineBasicOperation(CastFloat)
DefineBasicOperation(CastDouble)
DefineBasicOperation(CastLongDouble)
DefineBasicOperation(PrevAssign)
DefineBasicOperation(NextAssign)
DefineBasicOperation(PlusAssign)
DefineBasicOperation(MinusAssign)
DefineBasicOperation(MinusSignedAssign)
DefineBasicOperation(CompareLess)
DefineBasicOperation(CompareLessOrEqual)
DefineBasicOperation(CompareEqual)
DefineBasicOperation(CompareDifferent)
DefineBasicOperation(CompareGreater)
DefineBasicOperation(CompareGreaterOrEqual)
DefineBasicOperation(MinAssign)
DefineBasicOperation(MaxAssign)
DefineBasicOperation(TimesAssign)
DefineBasicOperation(DivideAssign)
DefineBasicOperation(OppositeAssign)
DefineBasicOperation(ModuloAssign)
DefineBasicOperation(BitOrAssign)
DefineBasicOperation(BitAndAssign)
DefineBasicOperation(BitExclusiveOrAssign)
DefineBasicOperation(BitNegateAssign)
DefineBasicOperation(LeftShiftAssign)
DefineBasicOperation(RightShiftAssign)
DefineBasicOperation(LeftRotateAssign)
DefineBasicOperation(RightRotateAssign)
DefineBasicOperation(LogicalAndAssign)
DefineBasicOperation(LogicalOrAssign)
DefineBasicOperation(LogicalNegateAssign)

} // end of namespace Integer

namespace Rational {

class Operation : public VirtualOperation {
  private:
   typedef VirtualOperation inherited;
   
  public:
   enum Type {
      TUndefined, TCastChar, TCastSignedChar, TCastUnsignedChar, TCastShort, TCastUnsignedShort,
         TCastInt, TCastUnsignedInt, TCastLong, TCastUnsignedLong, TCastFloat, TCastDouble,
         TCastLongDouble, TCastReal, TCastMultiBit, TCastMultiFloat, EndOfCast,
      TPlusAssign = EndOfCast, TMinusAssign, EndOfBinary,
      StartOfCompare = EndOfBinary, TCompareLess = StartOfCompare, TCompareLessOrEqual, TCompareEqual,
         TCompareDifferent, TCompareGreaterOrEqual, TCompareGreater, EndOfCompare,
      TMinAssign = EndOfCompare, TMaxAssign, EndOfAtomic,
      TTimesAssign = EndOfAtomic, TDivideAssign, TOppositeAssign, EndOfNumeric, EndOfType=EndOfNumeric
   };

  protected:
   DefineExtendedParameters(5, VirtualOperation)

  private:
   Operation& setField(Type type)
      {  AssumeCondition(!hasOwnField()) mergeOwnField(type); return *this; }

  protected:
   Operation& setCastMultiBit()  { setConst(); setArgumentsNumber(0); return setField(TCastMultiBit); }
   Operation& setCastMultiFloat()  { setConst(); setArgumentsNumber(0); return setField(TCastMultiFloat); }
   
  public:
   Operation() {}
   Operation(const Operation& source) = default;
   DefineCopy(Operation)
   virtual bool isValid() const override { return hasOwnField(); }

   Type getType() const { return (Type) queryOwnField(); }
   Operation& setType(Type type) { setOwnField(type); return *this; }

   Operation& setCastChar()         { setConst(); setArgumentsNumber(0); return setField(TCastChar); }
   Operation& setCastSignedChar()   { setConst(); setArgumentsNumber(0); return setField(TCastSignedChar); }
   Operation& setCastUnsignedChar() { setConst(); setArgumentsNumber(0); return setField(TCastUnsignedChar); }
   Operation& setCastShort()        { setConst(); setArgumentsNumber(0); return setField(TCastShort); }
   Operation& setCastUnsignedShort(){ setConst(); setArgumentsNumber(0); return setField(TCastUnsignedShort); }
   Operation& setCastInt()          { setConst(); setArgumentsNumber(0); return setField(TCastInt); }
   Operation& setCastUnsignedInt()  { setConst(); setArgumentsNumber(0); return setField(TCastUnsignedInt); }
   Operation& setCastLong()         { setConst(); setArgumentsNumber(0); return setField(TCastLong); }
   Operation& setCastUnsignedLong() { setConst(); setArgumentsNumber(0); return setField(TCastUnsignedLong); }
   Operation& setCastFloat()        { setConst(); setArgumentsNumber(0); return setField(TCastFloat); }
   Operation& setCastDouble()       { setConst(); setArgumentsNumber(0); return setField(TCastDouble); }
   Operation& setCastLongDouble()   { setConst(); setArgumentsNumber(0); return setField(TCastLongDouble); }
   Operation& setCastReal()         { setConst(); setArgumentsNumber(0); return setField(TCastReal); }

   Operation& setPlusAssign()       { setArgumentsNumber(1); return setField(TPlusAssign); }
   Operation& setMinusAssign()      { setArgumentsNumber(1); return setField(TMinusAssign); }
   Operation& setMinAssign()        { setArgumentsNumber(1); return setField(TMinAssign); }
   Operation& setMaxAssign()        { setArgumentsNumber(1); return setField(TMaxAssign); }
   Operation& setTimesAssign()      { setArgumentsNumber(1); return setField(TTimesAssign); }
   Operation& setDivideAssign()     { setArgumentsNumber(1); return setField(TDivideAssign); }
   Operation& setOppositeAssign()   { setArgumentsNumber(0); return setField(TOppositeAssign); }

   Operation& setPlus()    { setConstWithAssign(); setArgumentsNumber(1); return setField(TPlusAssign); }
   Operation& setMinus()   { setConstWithAssign(); setArgumentsNumber(1); return setField(TMinusAssign); }
   Operation& setMin()     { setConstWithAssign(); setArgumentsNumber(1); return setField(TMinAssign); }
   Operation& setMax()     { setConstWithAssign(); setArgumentsNumber(1); return setField(TMaxAssign); }
   Operation& setTimes()   { setConstWithAssign(); setArgumentsNumber(1); return setField(TTimesAssign); }
   Operation& setDivide()  { setConstWithAssign(); setArgumentsNumber(1); return setField(TDivideAssign); }
   Operation& setOpposite(){ setConstWithAssign(); setArgumentsNumber(0); return setField(TOppositeAssign); }

   Operation& setCompareLess()      { setConst(); setArgumentsNumber(1); return setField(TCompareLess); }
   Operation& setCompareLessOrEqual() { setConst(); setArgumentsNumber(1); return setField(TCompareLessOrEqual); }
   Operation& setCompareEqual()     { setConst(); setArgumentsNumber(1); return setField(TCompareEqual); }
   Operation& setCompareDifferent() { setConst(); setArgumentsNumber(1); return setField(TCompareDifferent); }
   Operation& setCompareGreater()   { setConst(); setArgumentsNumber(1); return setField(TCompareGreater); }
   Operation& setCompareGreaterOrEqual() { setConst(); setArgumentsNumber(1); return setField(TCompareGreaterOrEqual); }
};

class CastMultiBitOperation : public DInt::TCastMultiBitOperation<Operation> {
  public:
   CastMultiBitOperation() { DInt::TCastMultiBitOperation<Operation>::setCastMultiBit(); }
};

class CastMultiFloatOperation : public DReal::TCastMultiFloatOperation<Operation> {
  public:
   CastMultiFloatOperation() { DReal::TCastMultiFloatOperation<Operation>::setCastMultiFloat(); }
};

DefineBasicOperation(CastChar)
DefineBasicOperation(CastSignedChar)
DefineBasicOperation(CastUnsignedChar)
DefineBasicOperation(CastShort)
DefineBasicOperation(CastUnsignedShort)
DefineBasicOperation(CastInt)
DefineBasicOperation(CastUnsignedInt)
DefineBasicOperation(CastLong)
DefineBasicOperation(CastUnsignedLong)
DefineBasicOperation(CastFloat)
DefineBasicOperation(CastDouble)
DefineBasicOperation(CastLongDouble)
DefineBasicOperation(CastReal)
DefineBasicOperation(PlusAssign)
DefineBasicOperation(MinusAssign)
DefineBasicOperation(CompareLess)
DefineBasicOperation(CompareLessOrEqual)
DefineBasicOperation(CompareEqual)
DefineBasicOperation(CompareDifferent)
DefineBasicOperation(CompareGreater)
DefineBasicOperation(CompareGreaterOrEqual)
DefineBasicOperation(MinAssign)
DefineBasicOperation(MaxAssign)
DefineBasicOperation(TimesAssign)
DefineBasicOperation(DivideAssign)
DefineBasicOperation(OppositeAssign)

} // end of namespace Rational

namespace Floating {

class Operation : public VirtualOperation {
  private:
   typedef VirtualOperation inherited;
   
  public:
   enum Type {
      TUndefined, TCastChar, TCastSignedChar, TCastUnsignedChar, TCastShort, TCastUnsignedShort,
         TCastInt, TCastUnsignedInt, TCastLong, TCastUnsignedLong, TCastMultiFloat, TCastFloat,
         TCastDouble, TCastLongDouble, TCastRational, TCastMultiBit, TCastMultiBitPointer, EndOfCast,
      TPlusAssign = EndOfCast, TMinusAssign, EndOfBinary,
      StartOfCompare = EndOfBinary, TCompareLess = StartOfCompare, TCompareLessOrEqual, TCompareEqual,
         TCompareDifferent, TCompareGreaterOrEqual, TCompareGreater, EndOfCompare,
      StartOfQuery = EndOfCompare, TIsInftyExponent = StartOfQuery, TIsNaN, TIsQNaN, TIsSNaN,
         TIsPositive, TIsZeroExponent, TIsNegative, TCastMantissa, EndOfQuery,
      TMinAssign = EndOfQuery, TMaxAssign, EndOfAtomic,
      TTimesAssign = EndOfAtomic, TDivideAssign, EndOfSecondBinary, TOppositeAssign = EndOfSecondBinary,
      TAbsAssign, EndOfSimple,
      StartOfTernary = EndOfSimple, TMultAddAssign = StartOfTernary, TMultSubAssign, TNegMultAddAssign, TNegMultSubAssign, EndOfNumeric,
      TAcos = EndOfNumeric, TAsin, TAtan, TAtan2, TCeil, TCos, TCosh, TExp, TFabs, TFloor, TFmod,
      TFrexp, TLdexp, TLog, TLog10, TModf, TPow, TSin, TSinh, TSqrt, TTan, TTanh, EndOfType
   };

  protected:
   DefineExtendedParameters(7, VirtualOperation)

  private:
   Operation& setField(Type type)
      {  AssumeCondition(!hasOwnField()) mergeOwnField(type); return *this; }

  protected:
   Operation& setCastMultiBit() { setConst(); setArgumentsNumber(0); return setField(TCastMultiBit); }
   Operation& setCastMultiBitPointer() { setConst(); setArgumentsNumber(0); return setField(TCastMultiBitPointer); }
   Operation& setCastMultiFloat() { setConst(); setArgumentsNumber(0); return setField(TCastMultiFloat); }
   
  public:
   Operation() {}
   Operation(const Operation& source) = default;
   DefineCopy(Operation)
   virtual bool isValid() const override { return hasOwnField(); }

   Type getType() const { return (Type) queryOwnField(); }
   Operation& setType(Type type) { setOwnField(type); return *this; }

   Operation& setCompare(Type type)
      {  AssumeCondition((type >= StartOfCompare) && (type < EndOfCompare))
         setArgumentsNumber(1);
         setConst();
         return setField(type);
      }
   Operation& setTranscendental(Type type)
      {  AssumeCondition((type >= EndOfNumeric) && (type < EndOfType))
         setArgumentsNumber(0);
         setConst();
         return setField(type);
      }
   Operation& setCastChar()         { setArgumentsNumber(0); setConst(); return setField(TCastChar); }
   Operation& setCastSignedChar()   { setArgumentsNumber(0); setConst(); return setField(TCastSignedChar); }
   Operation& setCastUnsignedChar() { setArgumentsNumber(0); setConst(); return setField(TCastUnsignedChar); }
   Operation& setCastShort()        { setArgumentsNumber(0); setConst(); return setField(TCastShort); }
   Operation& setCastUnsignedShort(){ setArgumentsNumber(0); setConst(); return setField(TCastUnsignedShort); }
   Operation& setCastInt()          { setArgumentsNumber(0); setConst(); return setField(TCastInt); }
   Operation& setCastUnsignedInt()  { setArgumentsNumber(0); setConst(); return setField(TCastUnsignedInt); }
   Operation& setCastLong()         { setArgumentsNumber(0); setConst(); return setField(TCastLong); }
   Operation& setCastUnsignedLong() { setArgumentsNumber(0); setConst(); return setField(TCastUnsignedLong); }
   Operation& setCastFloat()        { setArgumentsNumber(0); setConst(); return setField(TCastFloat); }
   Operation& setCastDouble()       { setArgumentsNumber(0); setConst(); return setField(TCastDouble); }
   Operation& setCastLongDouble()   { setArgumentsNumber(0); setConst(); return setField(TCastLongDouble); }
   Operation& setCastRational()     { setArgumentsNumber(0); setConst(); return setField(TCastRational); }
   Operation& setPlusAssign()       { setArgumentsNumber(1); return setField(TPlusAssign); }
   Operation& setMinusAssign()      { setArgumentsNumber(1); return setField(TMinusAssign); }
   Operation& setPlus()  { setArgumentsNumber(1); setConstWithAssign(); return setField(TPlusAssign); }
   Operation& setMinus() { setArgumentsNumber(1); setConstWithAssign(); return setField(TMinusAssign); }
   Operation& setOppositeAssign()   { setArgumentsNumber(0); return setField(TOppositeAssign); }
   Operation& setAbsAssign()        { setArgumentsNumber(0); return setField(TAbsAssign); }
   Operation& setOpposite()         { setArgumentsNumber(0); setConstWithAssign(); return setField(TOppositeAssign); }
   Operation& setAbs()              { setArgumentsNumber(0); setConstWithAssign(); return setField(TAbsAssign); }
   Operation& setCompareLess()      { setArgumentsNumber(1); setConst(); return setField(TCompareLess); }
   Operation& setCompareLessOrEqual() { setArgumentsNumber(1); setConst(); return setField(TCompareLessOrEqual); }
   Operation& setCompareEqual()     { setArgumentsNumber(1); setConst(); return setField(TCompareEqual); }
   Operation& setCompareDifferent() { setArgumentsNumber(1); setConst(); return setField(TCompareDifferent); }
   Operation& setCompareGreater()   { setArgumentsNumber(1); setConst(); return setField(TCompareGreater); }
   Operation& setCompareGreaterOrEqual() { setArgumentsNumber(1); setConst(); return setField(TCompareGreaterOrEqual); }
   Operation& setIsInftyExponent()  { setArgumentsNumber(0); setConst(); return setField(TIsInftyExponent); }
   Operation& setIsNaN()            { setArgumentsNumber(0); setConst(); return setField(TIsNaN); }
   Operation& setIsQNaN()           { setArgumentsNumber(0); setConst(); return setField(TIsQNaN); }
   Operation& setIsSNaN()           { setArgumentsNumber(0); setConst(); return setField(TIsSNaN); }
   Operation& setIsPositive()       { setArgumentsNumber(0); setConst(); return setField(TIsPositive); }
   Operation& setIsZeroExponent()   { setArgumentsNumber(0); setConst(); return setField(TIsZeroExponent); }
   Operation& setIsNegative()    { setArgumentsNumber(0); setConst(); return setField(TIsNegative); }
   Operation& setCastMantissa()     { setArgumentsNumber(0); setConst(); return setField(TCastMantissa); }
   Operation& setMinAssign()        { setArgumentsNumber(1); return setField(TMinAssign); }
   Operation& setMaxAssign()        { setArgumentsNumber(1); return setField(TMaxAssign); }
   Operation& setMin()     { setArgumentsNumber(1); setConstWithAssign(); return setField(TMinAssign); }
   Operation& setMax()     { setArgumentsNumber(1); setConstWithAssign(); return setField(TMaxAssign); }
   Operation& setTimesAssign()      { setArgumentsNumber(1); return setField(TTimesAssign); }
   Operation& setDivideAssign()     { setArgumentsNumber(1); return setField(TDivideAssign); }
   Operation& setTimes()   { setArgumentsNumber(1); setConstWithAssign(); return setField(TTimesAssign); }
   Operation& setDivide()  { setArgumentsNumber(1); setConstWithAssign(); return setField(TDivideAssign); }
   Operation& setMultAddAssign()    { setArgumentsNumber(2); return setField(TMultAddAssign); }
   Operation& setMultSubAssign()    { setArgumentsNumber(2); return setField(TMultSubAssign); }
   Operation& setNegMultAddAssign() { setArgumentsNumber(2); return setField(TNegMultAddAssign); }
   Operation& setNegMultSubAssign() { setArgumentsNumber(2); return setField(TNegMultSubAssign); }
   Operation& setAcos()    { setArgumentsNumber(0); setConst(); return setField(TAcos); }
   Operation& setAsin()    { setArgumentsNumber(0); setConst(); return setField(TAsin); }
   Operation& setAtan()    { setArgumentsNumber(0); setConst(); return setField(TAtan); }
   Operation& setAtan2()   { setArgumentsNumber(0); setConst(); return setField(TAtan2); }
   Operation& setCeil()    { setArgumentsNumber(0); setConst(); return setField(TCeil); }
   Operation& setCos()     { setArgumentsNumber(0); setConst(); return setField(TCos); }
   Operation& setCosh()    { setArgumentsNumber(0); setConst(); return setField(TCosh); }
   Operation& setExp()     { setArgumentsNumber(0); setConst(); return setField(TExp); }
   Operation& setFabs()    { setArgumentsNumber(0); setConst(); return setField(TFabs); }
   Operation& setFloor()   { setArgumentsNumber(0); setConst(); return setField(TFloor); }
   Operation& setFmod()    { setArgumentsNumber(0); setConst(); return setField(TFmod); }
   Operation& setFrexp()   { setArgumentsNumber(0); setConst(); return setField(TFrexp); }
   Operation& setLdexp()   { setArgumentsNumber(0); setConst(); return setField(TLdexp); }
   Operation& setLog()     { setArgumentsNumber(0); setConst(); return setField(TLog); }
   Operation& setLog10()   { setArgumentsNumber(0); setConst(); return setField(TLog10); }
   Operation& setModf()    { setArgumentsNumber(0); setConst(); return setField(TModf); }
   Operation& setPow()     { setArgumentsNumber(0); setConst(); return setField(TPow); }
   Operation& setSin()     { setArgumentsNumber(0); setConst(); return setField(TSin); }
   Operation& setSinh()    { setArgumentsNumber(0); setConst(); return setField(TSinh); }
   Operation& setSqrt()    { setArgumentsNumber(0); setConst(); return setField(TSqrt); }
   Operation& setTan()     { setArgumentsNumber(0); setConst(); return setField(TTan); }
   Operation& setTanh()    { setArgumentsNumber(0); setConst(); return setField(TTanh); }

   void write(STG::IOObject::OSBase& out, const STG::IOObject::FormatParameters& params) const;
};

class CastMultiBitOperation : public DInt::TCastMultiBitOperation<Operation> {
  public:
   CastMultiBitOperation() { DInt::TCastMultiBitOperation<Operation>::setCastMultiBit(); }
};

class CastMultiBitOperationPointer : public DInt::TCastMultiBitOperation<Operation> {
  public:
   CastMultiBitOperationPointer() { DInt::TCastMultiBitOperation<Operation>::setCastMultiBitPointer(); }
};

DefineBasicOperation(CastChar)
DefineBasicOperation(CastSignedChar)
DefineBasicOperation(CastUnsignedChar)
DefineBasicOperation(CastShort)
DefineBasicOperation(CastUnsignedShort)
DefineBasicOperation(CastInt)
DefineBasicOperation(CastUnsignedInt)
DefineBasicOperation(CastLong)
DefineBasicOperation(CastUnsignedLong)

class CastMultiFloatOperation : public Scalar::DReal::TCastMultiFloatOperation<Operation> {
  public:
   CastMultiFloatOperation() { setCastMultiFloat(); }
};

DefineBasicOperation(CastFloat)
DefineBasicOperation(CastDouble)
DefineBasicOperation(CastLongDouble)
DefineBasicOperation(PlusAssign)
DefineBasicOperation(Plus)
DefineBasicOperation(MinusAssign)
DefineBasicOperation(Minus)
DefineBasicOperation(CompareLess)
DefineBasicOperation(CompareLessOrEqual)
DefineBasicOperation(CompareEqual)
DefineBasicOperation(CompareDifferent)
DefineBasicOperation(CompareGreaterOrEqual)
DefineBasicOperation(CompareGreater)
DefineBasicOperation(IsInftyExponent)
DefineBasicOperation(IsNaN)
DefineBasicOperation(IsQNaN)
DefineBasicOperation(IsSNaN)
DefineBasicOperation(IsPositive)
DefineBasicOperation(IsZeroExponent)
DefineBasicOperation(IsNegative)
DefineBasicOperation(CastMantissa)
DefineBasicOperation(MinAssign)
DefineBasicOperation(Min)
DefineBasicOperation(MaxAssign)
DefineBasicOperation(Max)
DefineBasicOperation(TimesAssign)
DefineBasicOperation(Times)
DefineBasicOperation(DivideAssign)
DefineBasicOperation(Divide)
DefineBasicOperation(OppositeAssign)
DefineBasicOperation(Opposite)
DefineBasicOperation(AbsAssign)
DefineBasicOperation(Abs)
DefineBasicOperation(MultAddAssign)
DefineBasicOperation(MultSubAssign)
DefineBasicOperation(NegMultAddAssign)
DefineBasicOperation(NegMultSubAssign)
DefineBasicOperation(Acos)
DefineBasicOperation(Asin)
DefineBasicOperation(Atan)
DefineBasicOperation(Atan2)
DefineBasicOperation(Ceil)
DefineBasicOperation(Cos)
DefineBasicOperation(Cosh)
DefineBasicOperation(Exp)
DefineBasicOperation(Fabs)
DefineBasicOperation(Floor)
DefineBasicOperation(Fmod)
DefineBasicOperation(Frexp)
DefineBasicOperation(Ldexp)
DefineBasicOperation(Log)
DefineBasicOperation(Log10)
DefineBasicOperation(Modf)
DefineBasicOperation(Pow)
DefineBasicOperation(Sin)
DefineBasicOperation(Sinh)
DefineBasicOperation(Sqrt)
DefineBasicOperation(Tan)
DefineBasicOperation(Tanh)

} // end of namespace Floating

namespace Real {

class Operation : public VirtualOperation {
  private:
   typedef VirtualOperation inherited;
   
  public:
   enum Type {
      TUndefined, TCastChar, TCastSignedChar, TCastUnsignedChar, TCastShort, TCastUnsignedShort,
         TCastInt, TCastUnsignedInt, TCastLong, TCastUnsignedLong, TCastFloat, TCastDouble,
         TCastLongDouble, TCastRational, TCastMultiBit, EndOfCast,
      TPlusAssign = EndOfCast, TMinusAssign, EndOfBinary,
      StartOfCompare = EndOfBinary, TCompareLess = StartOfCompare, TCompareLessOrEqual, TCompareEqual,
         TCompareDifferent, TCompareGreaterOrEqual, TCompareGreater, EndOfCompare,
      TMinAssign = EndOfCompare, TMaxAssign, EndOfAtomic,
      TTimesAssign = EndOfAtomic, TDivideAssign, TOppositeAssign, TAbsAssign, EndOfType
   };

  protected:
   DefineExtendedParameters(5, VirtualOperation)

  private:
   Operation& setField(Type type)
      {  AssumeCondition(!hasOwnField()) mergeOwnField(type); return *this; }

  protected:
   Operation& setCastMultiBit()  { setConst(); setArgumentsNumber(0); return setField(TCastMultiBit); }
   
  public:
   Operation() {}
   Operation(const Operation& source) = default;
   DefineCopy(Operation)
   virtual bool isValid() const override { return hasOwnField(); }

   Type getType() const { return (Type) queryOwnField(); }
   Operation& setType(Type type) { setOwnField(type); return *this; }

   Operation& setCastChar()         { setConst(); setArgumentsNumber(0); return setField(TCastChar); }
   Operation& setCastSignedChar()   { setConst(); setArgumentsNumber(0); return setField(TCastSignedChar); }
   Operation& setCastUnsignedChar() { setConst(); setArgumentsNumber(0); return setField(TCastUnsignedChar); }
   Operation& setCastShort()        { setConst(); setArgumentsNumber(0); return setField(TCastShort); }
   Operation& setCastUnsignedShort(){ setConst(); setArgumentsNumber(0); return setField(TCastUnsignedShort); }
   Operation& setCastInt()          { setConst(); setArgumentsNumber(0); return setField(TCastInt); }
   Operation& setCastUnsignedInt()  { setConst(); setArgumentsNumber(0); return setField(TCastUnsignedInt); }
   Operation& setCastLong()         { setConst(); setArgumentsNumber(0); return setField(TCastLong); }
   Operation& setCastUnsignedLong() { setConst(); setArgumentsNumber(0); return setField(TCastUnsignedLong); }
   Operation& setCastFloat()        { setConst(); setArgumentsNumber(0); return setField(TCastFloat); }
   Operation& setCastDouble()       { setConst(); setArgumentsNumber(0); return setField(TCastDouble); }
   Operation& setCastLongDouble()   { setConst(); setArgumentsNumber(0); return setField(TCastLongDouble); }
   Operation& setCastRational()     { setConst(); setArgumentsNumber(0); return setField(TCastRational); }

   Operation& setPlusAssign()       { setArgumentsNumber(1); return setField(TPlusAssign); }
   Operation& setMinusAssign()      { setArgumentsNumber(1); return setField(TMinusAssign); }
   Operation& setMinAssign()        { setArgumentsNumber(1); return setField(TMinAssign); }
   Operation& setMaxAssign()        { setArgumentsNumber(1); return setField(TMaxAssign); }
   Operation& setTimesAssign()      { setArgumentsNumber(1); return setField(TTimesAssign); }
   Operation& setDivideAssign()     { setArgumentsNumber(1); return setField(TDivideAssign); }
   Operation& setOppositeAssign()   { setArgumentsNumber(0); return setField(TOppositeAssign); }
   Operation& setAbsAssign()        { setArgumentsNumber(0); return setField(TAbsAssign); }

   Operation& setPlus()    { setConstWithAssign(); setArgumentsNumber(1); return setField(TPlusAssign); }
   Operation& setMinus()   { setConstWithAssign(); setArgumentsNumber(1); return setField(TMinusAssign); }
   Operation& setMin()     { setConstWithAssign(); setArgumentsNumber(1); return setField(TMinAssign); }
   Operation& setMax()     { setConstWithAssign(); setArgumentsNumber(1); return setField(TMaxAssign); }
   Operation& setTimes()   { setConstWithAssign(); setArgumentsNumber(1); return setField(TTimesAssign); }
   Operation& setDivide()  { setConstWithAssign(); setArgumentsNumber(1); return setField(TDivideAssign); }
   Operation& setOpposite(){ setConstWithAssign(); setArgumentsNumber(0); return setField(TOppositeAssign); }
   Operation& setAbs()     { setConstWithAssign(); setArgumentsNumber(0); return setField(TAbsAssign); }

   Operation& setCompareLess()      { setConst(); setArgumentsNumber(1); return setField(TCompareLess); }
   Operation& setCompareLessOrEqual() { setConst(); setArgumentsNumber(1); return setField(TCompareLessOrEqual); }
   Operation& setCompareEqual()     { setConst(); setArgumentsNumber(1); return setField(TCompareEqual); }
   Operation& setCompareDifferent() { setConst(); setArgumentsNumber(1); return setField(TCompareDifferent); }
   Operation& setCompareGreater()   { setConst(); setArgumentsNumber(1); return setField(TCompareGreater); }
   Operation& setCompareGreaterOrEqual() { setConst(); setArgumentsNumber(1); return setField(TCompareGreaterOrEqual); }
};

class CastMultiBitOperation : public DInt::TCastMultiBitOperation<Operation> {
  public:
   CastMultiBitOperation() { DInt::TCastMultiBitOperation<Operation>::setCastMultiBit(); setConst(); }
};

DefineBasicOperation(CastChar)
DefineBasicOperation(CastSignedChar)
DefineBasicOperation(CastUnsignedChar)
DefineBasicOperation(CastShort)
DefineBasicOperation(CastUnsignedShort)
DefineBasicOperation(CastInt)
DefineBasicOperation(CastUnsignedInt)
DefineBasicOperation(CastLong)
DefineBasicOperation(CastUnsignedLong)
DefineBasicOperation(CastFloat)
DefineBasicOperation(CastDouble)
DefineBasicOperation(CastLongDouble)
DefineBasicOperation(CastRational)
DefineBasicOperation(PlusAssign)
DefineBasicOperation(MinusAssign)
DefineBasicOperation(CompareLess)
DefineBasicOperation(CompareLessOrEqual)
DefineBasicOperation(CompareEqual)
DefineBasicOperation(CompareDifferent)
DefineBasicOperation(CompareGreater)
DefineBasicOperation(CompareGreaterOrEqual)
DefineBasicOperation(MinAssign)
DefineBasicOperation(MaxAssign)
DefineBasicOperation(TimesAssign)
DefineBasicOperation(DivideAssign)
DefineBasicOperation(OppositeAssign)
DefineBasicOperation(AbsAssign)

} // end of namespace Real

#undef DefineBasicOperation

}} // end of namespace Analyzer::Scalar

#endif // Analyzer_Scalar_ConcreteOperationH
