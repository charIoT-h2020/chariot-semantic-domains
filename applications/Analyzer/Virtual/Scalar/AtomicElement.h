/////////////////////////////////
//
// Library   : Analyzer/Virtual
// Unit      : Scalar
// File      : AtomicElement.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of the base hierarchy of scalar elements.
//

#ifndef Analyzer_Scalar_AtomicElementH
#define Analyzer_Scalar_AtomicElementH

#include "Analyzer/Virtual/Scalar/OperationElement.h"
#include "Collection/Collection.hpp"

namespace Analyzer { namespace Control {

class AbstractType;

} // end of namespace Control

namespace Scalar {

class AbstractElement;

} // end of namespace Scalar

namespace Scalar {

namespace Implementation {

class VirtualElement : public EnhancedObject, protected ExtendedParameters {
  public:
   VirtualElement() {}
   VirtualElement(const VirtualElement& source) = default;
   VirtualElement& operator=(const VirtualElement& source)
      {  EnhancedObject::operator=(source);
         AssumeCondition(queryParams() == source.queryParams())
         return *this;
      }
   DefineCopy(VirtualElement)
   DDefineAssign(VirtualElement)
};

} // end of namespace Implementation

/*******************************************/
/* Definition of the class AbstractElement */
/*******************************************/

class EvaluationEnvironment;
class AbstractElement : public STG::IOObject, public ExtendedParameters {
  protected:
   AbstractElement() {}

  public:
   AbstractElement(const AbstractElement& source) = default;
   DefineCopy(AbstractElement)
   DDefineAssign(AbstractElement)

   static void assume(bool result) { AssumeCondition(result) }
   class ZeroResult {
     private:
      enum Result { ZRUndefined, ZRZero, ZRDifferentZero };
      Result rResult;

     public:
      ZeroResult() : rResult(ZRUndefined) {}
      ZeroResult(const ZeroResult& source) = default;

      ZeroResult& setZero()   { rResult = ZRZero; return *this; }
      ZeroResult& setDifferentZero() { rResult = ZRDifferentZero; return *this; }
      ZeroResult& setFalse()  { rResult = ZRZero; return *this; }
      ZeroResult& setTrue()   { rResult = ZRDifferentZero; return *this; }
      ZeroResult& setBool(bool value) { rResult = value ? ZRDifferentZero : ZRZero; return *this; }

      void mergeWith(const ZeroResult& source)
         {  if (rResult != source.rResult)
               rResult = ZRUndefined;
         }
      void intersectWithOnZero(const ZeroResult& source)
         {  if (rResult != source.rResult) {
               if (rResult == ZRDifferentZero || source.rResult == ZRDifferentZero)
                  rResult = ZRDifferentZero;
               else
                  rResult = ZRUndefined;
            };
         }
      bool isValid() const    { return rResult != ZRUndefined; }
      bool isExact() const    { return rResult != ZRUndefined; }
      bool isZero() const     { return rResult == ZRZero; }
      bool isDifferentZero() const { return rResult == ZRDifferentZero; }
      bool isFalse() const    { return rResult == ZRZero; }
      bool isTrue() const     { return rResult == ZRDifferentZero; }
      bool mayBeFalse() const { return rResult != ZRDifferentZero; }
      bool mayBeTrue() const  { return rResult != ZRZero; }
      bool mayBeZero() const  { return rResult != ZRDifferentZero; }
      bool mayBeDifferentZero() const { return rResult != ZRZero; }
      operator bool() const   { return (rResult == ZRDifferentZero); }
      bool operator!() const  { return (rResult == ZRZero); }
   };

   virtual bool isBoolean() const { return false; }
   virtual bool isMultiBit() const { return false; }
   virtual bool isInt() const { return false; }
   virtual bool isFloat() const { return false; }
   virtual bool apply(const VirtualOperation&, EvaluationEnvironment&) { return false; }
   virtual ZeroResult queryZeroResult() const { AssumeUncalled return ZeroResult(); }

   class FormatParameters : public STG::IOObject::FormatParameters {
     protected:
      DefineExtendedParameters(2, STG::IOObject::FormatParameters)
      DefineSubExtendedParameters(OneLine, 1, INHERITED)
      DefineSubExtendedParameters(Deterministic, 1, OneLine)

     public:
      FormatParameters() {}
      FormatParameters(const FormatParameters& source) : STG::IOObject::FormatParameters(source) {}

      FormatParameters& setOneLine() { mergeOneLineField(1); return *this; }
      bool isOneLine() const { return hasOneLineField(); }
      FormatParameters& setDeterministic() { mergeDeterministicField(1); return *this; }
      bool isDeterministic() const { return hasDeterministicField(); }
   };
   class Methods;
};

typedef PNT::PassPointer<AbstractElement> PPAbstractElement;

class VirtualElement : public AbstractElement {
  private:
   typedef AbstractElement inherited;

  protected:
   enum BitOccupied {
      BOUndefined, BOOne, BOTwo, BOFour, BOEight, BOSixteen, BOThirtyTwo, BOSixtyFour,
      BOAHundredTwentyEight, BOExtern, BOEnd
   };
   DefineExtendedParameters(4, AbstractElement)

  public:
   class Init : public EnhancedObject {
     private:
      BitOccupied boBits;
      PNT::PassPointer<Implementation::VirtualElement> ppiveConstant;
      int uExternSize;

     protected:
      void setSize(int size) { AssumeCondition(size > 0) boBits = BOExtern; uExternSize = size; }

     public:
      Init() : boBits(BOUndefined), uExternSize(0) {}
      Init(const Init& source) = default;
      Init& operator=(const Init& source) = default;
      DefineCopy(Init)
      DDefineAssign(Init)

      Init& set1()   { boBits = BOOne; return *this; }
      Init& set2()   { boBits = BOTwo; return *this; }
      Init& set4()   { boBits = BOFour; return *this; }
      Init& set8()   { boBits = BOEight; return *this; }
      Init& set16()  { boBits = BOSixteen; return *this; }
      Init& set32()  { boBits = BOThirtyTwo; return *this; }
      Init& set64()  { boBits = BOSixtyFour; return *this; }
      Init& set128() { boBits = BOAHundredTwentyEight; return *this; }
      Init& setBitSize(int bitSize)
         {  int codeBit = 6;
            while ((bitSize != (1 << codeBit)) && codeBit >= 0)
               --codeBit;
            if (codeBit < 0) {
               boBits = BOExtern;
               uExternSize = bitSize;
            }
            else
               boBits = (BitOccupied) (1+codeBit);
            return *this;
         }
      Init& absorbInitialValue(Implementation::VirtualElement* constant)
         {  ppiveConstant.absorbElement(constant); return *this; }
      Init& setInitialValue(const Implementation::VirtualElement& constant)
         {  ppiveConstant.setElement(constant); return *this; }
      bool hasInitialValue() const { return ppiveConstant.isValid(); }
      const Implementation::VirtualElement& getInitialValue() const { return *ppiveConstant; }
      friend class VirtualElement;
      int getBitSize() const
         { return (boBits < BOExtern) ? (1 << (boBits-1)) : uExternSize; }
   };
   friend class Init;

  protected:
   VirtualElement(const Init& init) { mergeOwnField(init.boBits); }
   virtual int _getSizeInBits() const
      {  BitOccupied bitsOccupied = (BitOccupied) queryOwnField();
         return (bitsOccupied < BOExtern) ? (1 << (bitsOccupied-1)) : 0;
      }
   
  public:
   VirtualElement(const VirtualElement& source) = default;
   DefineCopy(VirtualElement)
   virtual bool mergeWith(const VirtualElement&, EvaluationEnvironment&) { return false; }
   virtual bool contain(const VirtualElement&, EvaluationEnvironment&) const { return false; }
   virtual bool isAtomic() const { return true; }
   virtual bool isApproximate() const { return false; }
   virtual Control::AbstractType* createType() const;

   int getSizeInBits() const { return _getSizeInBits(); }
};

typedef PNT::TPassPointer<VirtualElement, PPAbstractElement> PPVirtualElement;

/*************************************************/
/* Definition of the class EvaluationEnvironment */
/*************************************************/

namespace DFloatingEnvironment {

class InputParameters : public ExtendedParameters {
  public:
   InputParameters() {}
   DefineExtendedParameters(24, ExtendedParameters)
   DefineSubExtendedParameters(Machine, 10, INHERITED)
   DefineSubExtendedParameters(Mode, 3, Machine)
   DefineSubExtendedParameters(Required, 11, Mode)
   
   void setRequireAll() { mergeRequiredField(0x1FF); }
   void setRequireNone() { clearRequiredField(); }
   void setRequireFlag(int flagIndex) { mergeRequiredField(1U << flagIndex); }
   bool requireFlag(int flagIndex) const { return queryRequiredField() & (1U << flagIndex); }
};

class OutputParameters : public ExtendedParameters {
  public:
   OutputParameters() {}
   DefineExtendedParameters(22, ExtendedParameters)
   DefineSubExtendedParameters(Fixed, 11, INHERITED)
   DefineSubExtendedParameters(Values, 11, Fixed)

   bool isFixed(int flagIndex) const { return queryFixedField() & (1U << flagIndex); }
   void setValue(int flagIndex, bool value)
      {  AssumeCondition(!(queryFixedField() & (1U << flagIndex)))
         mergeFixedField(1U << flagIndex);
         if (value)
            mergeValuesField(1U << flagIndex);
      }
   bool mergeValue(int flagIndex, bool value)
      {  bool result = false;
         if (queryFixedField() & (1U << flagIndex)) {
            if ((queryValuesField() & (1U << flagIndex)) ? !value : value) {
               intersectFixedField(~(1U << flagIndex));
               intersectValuesField(~(1U << flagIndex));
            }
            else
               result = true;
         };
         return result;
      }
   void setValue(int flagIndex, int valueSize, unsigned int value)
      {  AssumeCondition(!(queryFixedField() & (1U << flagIndex)) && (value < ((1U << valueSize) - 1)))
         mergeFixedField((~(~0U << valueSize)) << flagIndex);
         mergeValuesField(value << flagIndex);
      }
   bool mergeValue(int flagIndex, int valueSize, unsigned int value)
      {  AssumeCondition(value < ((1U << valueSize) - 1))
         bool result = false;
         if (queryFixedField() & (1U << flagIndex)) {
            if (((queryValuesField() >> flagIndex) & ~(~0U << valueSize)) != value) {
               intersectFixedField(~(~(~0U << valueSize) << flagIndex));
               intersectValuesField(~(~(~0U << valueSize) << flagIndex));
            }
            else
               result = true;
         };
         return result;
      }
   void clear() { clearOwnField(); }
   bool queryBooleanValue(int flagIndex) const
      {  AssumeCondition(queryFixedField() & (1U << flagIndex))
         return queryValuesField() & (1U << flagIndex);
      }
   unsigned int queryIntValue(int flagIndex, int valueSize) const
      {  AssumeCondition(queryFixedField() & (1U << flagIndex))
         return (queryValuesField() >> flagIndex) & ~(~0U << valueSize);
      }
   void mergeField(const OutputParameters& source)
      {  unsigned int fixed = queryFixedField(), sourceFixed = source.queryFixedField();
         unsigned int values = queryValuesField(), sourceValues = source.queryValuesField();
         for (int bitIndex = 0; bitIndex < 11; ++bitIndex) {
            if (sourceFixed & (1U << bitIndex)) {
               if (fixed & (1U << bitIndex)) {
                  if ((values & (1U << bitIndex)) != (sourceValues & (1U << bitIndex))) {
                     fixed &= ~(1U << bitIndex);
                     values &= ~(1U << bitIndex);
                  };
               };
            }
            else if (fixed & (1U << bitIndex)) {
               fixed &= ~(1U << bitIndex);
               values &= ~(1U << bitIndex);
            };
         };
      }
};

} // end of namespace DFloatingEnvironment

class FloatingEnvironment : public EnhancedObject, public DFloatingEnvironment::OutputParameters {
  public:
   typedef DFloatingEnvironment::OutputParameters OutputParameters;
   typedef DFloatingEnvironment::InputParameters InputParameters;
   
  private:
   typedef EnhancedObject inherited;
   InputParameters ipInputs;
   
  public:
   FloatingEnvironment() {}
   FloatingEnvironment(const FloatingEnvironment& source) = default;
   FloatingEnvironment& operator=(const FloatingEnvironment& source)
      {  inherited::operator=(source);
         DFloatingEnvironment::OutputParameters::operator=(source);
         return *this;
      }
   
   DefineCopy(FloatingEnvironment)
   DDefineAssign(FloatingEnvironment)
   InputParameters& inputs() { return ipInputs; }
   const InputParameters& inputs() const { return ipInputs; }
   OutputParameters& outputs() { return *this; }
   const OutputParameters& outputs() const { return *this; }
};

class EvaluationEnvironment : public EnhancedObject, public EExtendedParameters {
  public:
   enum ControlArguments { CARefer, CAControl };
   
   class ErrorReporting {
     public:
      enum Reporting { ERPropagateError, ERStopError };
      
     private:
      Reporting rReport;
      
     public:
      ErrorReporting() : rReport(ERPropagateError) {}
      ErrorReporting(Reporting report) : rReport(report) {}
      ErrorReporting(const ErrorReporting& source) = default;
      
      bool doesPropagateError() const { return rReport == ERPropagateError; }
      ErrorReporting& setPropagateError() { rReport = ERPropagateError; return *this; }
      ErrorReporting& setStopError() { rReport = ERStopError; return *this; }
      const Reporting& getReport() const { return rReport; }
   };

  private:
   typedef EnhancedObject inherited;
   PPAbstractElement ppaeResult;
   AbstractElement* paeFirstArgument;
   AbstractElement* paeSecondArgument;
   PNT::AutoPointer<FloatingEnvironment> apfeFloatingEnvironment;

  protected:
   enum EvaluationError
      {  EENoError=0, EEPositiveOverflow=1<<1, EENegativeOverflow=1<<2,
         EEPositiveUnderflow=1<<3, EENegativeUnderflow=1<<4, EEDivisionByZero=1<<5,
         EENegativeModulo=1<<6, EENaN=1<<7
      };
   enum Applicability { ATotal, APartial=2, ANone=3}; 

   DefineExtendedParameters(12, EExtendedParameters)
   DefineSubExtendedParameters(Error, 7, INHERITED)
   DefineSubExtendedParameters(ErrorReporting, 1, Error)
   DefineSubExtendedParameters(Applicability, 2, ErrorReporting)
   DefineSubExtendedParameters(Empty, 1, Applicability)

  public:
   EvaluationEnvironment& setErrorReporting(const ErrorReporting& report)
      {  setErrorReportingField(report.getReport()); return *this; }
   EvaluationEnvironment& setPropagateError()
      {  clearErrorReportingField(); return *this; }
   EvaluationEnvironment& setStopError()
      {  mergeErrorReportingField(ErrorReporting::ERStopError); return *this; }
   EvaluationEnvironment& operator=(const EvaluationEnvironment& source)
      {  inherited::operator=(source);
         EExtendedParameters::operator=(source);
         ppaeResult = source.ppaeResult;
         paeFirstArgument = (doesControlArgs() && source.paeFirstArgument) ? source.paeFirstArgument->createSCopy() : source.paeFirstArgument;
         paeSecondArgument = (doesControlArgs() && source.paeSecondArgument) ? source.paeSecondArgument->createSCopy() : source.paeSecondArgument;
         apfeFloatingEnvironment = source.apfeFloatingEnvironment;
         return *this;
      }

   ErrorReporting getErrorReporting() const
      {  return ErrorReporting((ErrorReporting::Reporting) queryErrorReportingField()); }
   bool doesStopErrorStates() const { return hasErrorReportingField(); }
   bool doesPropagateErrorStates() const { return !hasErrorReportingField(); }
   virtual bool isApproximate() const { return false; }

  protected:
   DefineSubExtendedParameters(ControlArgs, 1, Empty)

   void setControlArgs() { setControlArgsField(CAControl); }
   static void clearControlAndArgs(EvaluationEnvironment& env)
      {  env.paeFirstArgument = nullptr;
         env.paeSecondArgument = nullptr;
         env.clearControlArgsField();
      }
   static void transmitControlAndArgs(EvaluationEnvironment& source, EvaluationEnvironment& receiver)
      {  receiver.paeFirstArgument = source.paeFirstArgument;
         receiver.paeSecondArgument = source.paeSecondArgument;
         source.paeFirstArgument = nullptr;
         source.paeSecondArgument = nullptr;
         if (source.doesControlArgs())
            receiver.setControlArgs();
      }

  public:
   bool doesControlArgs() const { return hasControlArgsField(); }

   class Init {
     private:
      ControlArguments caControl;
      ErrorReporting erErrorReporting;
      bool fHasFloating;
      
     protected:
      DFloatingEnvironment::InputParameters ipStaticFloatingParameters;

     public:
      class InputFloatingParameters : public DFloatingEnvironment::InputParameters {
        private:
         typedef DFloatingEnvironment::InputParameters inherited;
         Init& iReference;
         
        public:
         InputFloatingParameters(Init& init) : iReference(init) {}
         InputFloatingParameters(const InputFloatingParameters& source) = default;
         InputFloatingParameters& setMachine(unsigned int machineParams)
            { AssumeCondition(!hasMachineField()) mergeMachineField(machineParams); return *this; }
         InputFloatingParameters& setMode(unsigned int modeParams)
            { AssumeCondition(!hasModeField()) mergeModeField(modeParams); return *this; }
         InputFloatingParameters& setRequired(unsigned int requiredParams)
            { AssumeCondition(!hasRequiredField()) mergeRequiredField(requiredParams); return *this; }
         Init& close() const { iReference.ipStaticFloatingParameters.params() = queryParams(); return iReference; }
      };
      friend class InputFloatingParameters;
      
     public:
      Init() : caControl(CARefer), erErrorReporting(), fHasFloating(false) {}
      Init(const Init& source) = default;
      Init(const EvaluationEnvironment& env)
         :  caControl((ControlArguments) env.queryControlArgsField()),
            erErrorReporting((ErrorReporting::Reporting) env.queryErrorReportingField()),
            fHasFloating(env.apfeFloatingEnvironment.isValid())
         {  if (fHasFloating)
               ipStaticFloatingParameters = env.apfeFloatingEnvironment->inputs();
         }
            
      virtual ~Init() {}
      Init& operator=(const Init& source) = default;
      Init& setFrom(const EvaluationEnvironment& env)
         {  caControl = (ControlArguments) env.queryControlArgsField();
            erErrorReporting = ErrorReporting((ErrorReporting::Reporting) env.queryErrorReportingField());
            if ((fHasFloating = env.apfeFloatingEnvironment.isValid()) != false)
               ipStaticFloatingParameters = env.apfeFloatingEnvironment->inputs();
            return *this;
         }

      Init& propagateErrorStates() { erErrorReporting.setPropagateError(); return *this; }
      Init& propagateErrorStates(ErrorReporting::Reporting reporting) { erErrorReporting = reporting; return *this; }
      Init& propagateErrorStates(const ErrorReporting& reporting) { erErrorReporting = reporting; return *this; }
      Init& stopErrorStates() { erErrorReporting.setStopError(); return *this; }
      Init& setReferArguments() { caControl = CARefer; return *this;  }
      Init& setControlArguments() { caControl = CAControl; return *this; }
      InputFloatingParameters setOpenFloating()
         {  AssumeCondition(!ipStaticFloatingParameters.queryParams()) fHasFloating = true; return InputFloatingParameters(*this); }
      virtual FloatingEnvironment* createFloatingEnvironment() const
         {  FloatingEnvironment* result = new FloatingEnvironment();
            result->inputs() = ipStaticFloatingParameters;
            return result;
         }
      friend class EvaluationEnvironment;
      
      bool hasFloating() const { return fHasFloating; }
      const ErrorReporting& getErrorReporting() const { return erErrorReporting; }
   };
   friend class Init;
   
   EvaluationEnvironment(const Init& init) :  paeFirstArgument(nullptr), paeSecondArgument(nullptr)
      {  mergeControlArgsField(init.caControl);
         mergeErrorReportingField(init.erErrorReporting.getReport());
         if (init.fHasFloating)
            apfeFloatingEnvironment.absorbElement(init.createFloatingEnvironment());
      }
   EvaluationEnvironment(const EvaluationEnvironment& source)
      :  inherited(source), EExtendedParameters(source),
         paeFirstArgument((source.doesControlArgs() && source.paeFirstArgument) ? source.paeFirstArgument->createSCopy() : source.paeFirstArgument),
         paeSecondArgument((source.doesControlArgs() && source.paeSecondArgument) ? source.paeSecondArgument->createSCopy() : source.paeSecondArgument),
         apfeFloatingEnvironment(source.apfeFloatingEnvironment) {}

   class DuplicateParameters {
     private:
      bool fTransmitField;
      bool fTransmitFloatPart;

     public:
      DuplicateParameters() : fTransmitField(false), fTransmitFloatPart(false) {}
      DuplicateParameters(const DuplicateParameters& source) = default;
      DuplicateParameters& operator=(const DuplicateParameters& source) = default;

      DuplicateParameters& transmitField() { fTransmitField = true; return *this; }
      DuplicateParameters& transmitFloatPart(bool transmitFloatPart) { fTransmitFloatPart = transmitFloatPart; return *this; }
      DuplicateParameters& transmitFloatPart() { fTransmitFloatPart = true; return *this; }
      friend class EvaluationEnvironment;
   };
   EvaluationEnvironment(const EvaluationEnvironment& source, const DuplicateParameters& params)
      :  inherited(source), EExtendedParameters(source), paeFirstArgument(source.paeFirstArgument),
         paeSecondArgument(source.paeSecondArgument)
      {  if (params.fTransmitField)
            clearControlArgsField();
         else {
            paeFirstArgument = (doesControlArgs() && source.paeFirstArgument) ? source.paeFirstArgument->createSCopy() : source.paeFirstArgument;
            paeSecondArgument = (doesControlArgs() && source.paeSecondArgument) ? source.paeSecondArgument->createSCopy() : source.paeSecondArgument;
         };
         if (params.fTransmitFloatPart)
            apfeFloatingEnvironment = source.apfeFloatingEnvironment;
      }
   virtual ~EvaluationEnvironment() { clearArguments(); }
   DefineCopy(EvaluationEnvironment)
   DDefineAssign(EvaluationEnvironment)

   bool hasFloatingPart() const { return apfeFloatingEnvironment.isValid(); }
   FloatingEnvironment& floatingPart() { return *apfeFloatingEnvironment; }
   const FloatingEnvironment& floatingPart() const { return *apfeFloatingEnvironment; }

   // handle the bits field.
   void assignField(const EvaluationEnvironment& env)
      {  ControlArguments control = (ControlArguments) queryControlArgsField();
         unsigned int errorReporting = queryErrorReportingField();
         params() = env.queryParams();
         setControlArgsField(control);
         setErrorReportingField(errorReporting);
         if (env.apfeFloatingEnvironment.isValid()) {
            if (apfeFloatingEnvironment.isValid())
               apfeFloatingEnvironment->outputs() = env.apfeFloatingEnvironment->outputs();
            else
               apfeFloatingEnvironment = env.apfeFloatingEnvironment;
         }
         else
            apfeFloatingEnvironment.release();
      }
   EvaluationEnvironment& mergeField(const EvaluationEnvironment& env)
      {  ControlArguments control = (ControlArguments) queryControlArgsField();
         unsigned int errorReporting = queryErrorReportingField();
         unsigned int applicability = queryApplicabilityField() | env.queryApplicabilityField();
         bool isEmpty = hasEmptyField();
         params() |= env.queryParams();
         setControlArgsField(control);
         setErrorReportingField(errorReporting);
         setApplicabilityField(applicability);
         setEmptyField(isEmpty);
         if (env.apfeFloatingEnvironment.isValid()) {
            if (apfeFloatingEnvironment.isValid())
               apfeFloatingEnvironment->outputs().mergeField(env.apfeFloatingEnvironment->outputs());
            else
               apfeFloatingEnvironment = env.apfeFloatingEnvironment;
         };
         return *this;
      }

   // set the errors
   EvaluationEnvironment& setPositiveOverflow()  { mergeErrorField(EEPositiveOverflow); return *this; }
   EvaluationEnvironment& setNegativeOverflow()  { mergeErrorField(EENegativeOverflow); return *this; }
   EvaluationEnvironment& setPositiveUnderflow() { mergeErrorField(EEPositiveUnderflow); return *this; }
   EvaluationEnvironment& setNegativeUnderflow() { mergeErrorField(EENegativeUnderflow); return *this; }
   EvaluationEnvironment& setDivisionByZero()    { mergeErrorField(EEDivisionByZero); return *this; }
   EvaluationEnvironment& setNegativeModulo()    { mergeErrorField(EENegativeModulo); return *this; }
   EvaluationEnvironment& setNaN()               { mergeErrorField(EENaN); return *this; }
   void setErrorEmpty() { if (doesStopErrorStates()) setEmpty(); }
   void clearResult()   { ppaeResult.release(); }
   virtual void clear()
      {  clearErrorField(); clearResult(); clearArguments(); clearEmptyField(); clearApplicabilityField();
         if (apfeFloatingEnvironment.isValid()) {
            apfeFloatingEnvironment->inputs().setRequireNone();
            apfeFloatingEnvironment->outputs().clear();
         };
      }

   bool hasEvaluationError() const  { return hasErrorField(); }
   unsigned getErrorCode() const    { return queryErrorField(); }
   bool isPositiveOverflow() const  { return queryErrorField() & EEPositiveOverflow; }
   bool isNegativeOverflow() const  { return queryErrorField() & EENegativeOverflow; }
   bool isOverflow() const          { return queryErrorField() & (EEPositiveOverflow | EENegativeOverflow); }
   bool isPositiveUnderflow() const { return queryErrorField() & EEPositiveUnderflow; }
   bool isNegativeUnderflow() const { return queryErrorField() & EENegativeUnderflow; }
   bool isDivisionByZero() const    { return queryErrorField() & EEDivisionByZero; }
   bool isNegativeModulo() const    { return queryErrorField() & EENegativeModulo; }
   bool isNaN() const               { return queryErrorField() & EENaN; }

   // store the results
   PPAbstractElement& presult() { return ppaeResult; }
   void storeResult(AbstractElement* result) { ppaeResult.absorbElement(result); }
   void setResult(const AbstractElement& result) { ppaeResult.setElement(result); }
   bool hasResult() const { return ppaeResult.isValid(); }
   AbstractElement& getResult() const { return *ppaeResult; }
   AbstractElement* extractResult() { return ppaeResult.extractElement(); }

   // handle the arguments
   void assignArguments(EvaluationEnvironment& source);
   void clearArguments()
      {  if (doesControlArgs()) {
            if (paeFirstArgument) delete paeFirstArgument;
            if (paeSecondArgument) delete paeSecondArgument;
         };
         paeFirstArgument = paeSecondArgument = nullptr;
      }

   void setPartialApplied() { mergeApplicabilityField(APartial); }
   void setNoneApplied() { mergeApplicabilityField(ANone); }
   void clearTotalApplied() { setApplicabilityField(APartial); }
   void clearPartialApplied() { clearApplicabilityField(); }
   bool isTotalApplied() const { return queryApplicabilityField() == ATotal; }
   bool isPartialApplied() const { return queryApplicabilityField() == APartial; }
   bool isNoneApplied() const { return queryApplicabilityField() == ANone; }

   void setEmpty() { mergeEmptyField(1); }
   void clearEmpty() { clearEmptyField(); setApplicabilityField(ATotal); }
   bool isEmpty() const { return hasEmptyField(); }
   virtual bool isValid() const override { return !hasEmptyField(); }
   AbstractElement& getFirstArgument() const;
   AbstractElement& getSecondArgument() const;
   AbstractElement* extractFirstArgument();
   AbstractElement* extractSecondArgument();
   bool hasFirstArgument() const { return paeFirstArgument; }
   bool hasSecondArgument() const { return paeSecondArgument; }
   EvaluationEnvironment& setFirstArgument(const AbstractElement& argument);
   EvaluationEnvironment& setSecondArgument(const AbstractElement& argument);
   EvaluationEnvironment& setArgument(const AbstractElement& argument);
   EvaluationEnvironment& setFirstArgument(PPAbstractElement argument);
   EvaluationEnvironment& setSecondArgument(PPAbstractElement argument);
   EvaluationEnvironment& setArgument(PPAbstractElement argument);
   EvaluationEnvironment& absorbFirstArgument(AbstractElement* argument);
   EvaluationEnvironment& absorbSecondArgument(AbstractElement* argument);
   EvaluationEnvironment& absorbArgument(AbstractElement* argument);
};

/* handle the arguments */

inline void
EvaluationEnvironment::assignArguments(EvaluationEnvironment& source)
   {  paeFirstArgument = ((doesControlArgs() && source.paeFirstArgument) ? source.paeFirstArgument->createSCopy() : source.paeFirstArgument);
      paeSecondArgument = ((doesControlArgs() && source.paeSecondArgument) ? source.paeSecondArgument->createSCopy() : source.paeSecondArgument);
   }

inline AbstractElement&
EvaluationEnvironment::getFirstArgument() const { AssumeCondition(paeFirstArgument) return *paeFirstArgument; }

inline AbstractElement&
EvaluationEnvironment::getSecondArgument() const { AssumeCondition(paeSecondArgument) return *paeSecondArgument; }

inline AbstractElement*
EvaluationEnvironment::extractFirstArgument()
   {  AssumeCondition(doesControlArgs())
      AbstractElement* result = paeFirstArgument;
      paeFirstArgument = nullptr;
      return result;
   }

inline AbstractElement*
EvaluationEnvironment::extractSecondArgument()
   {  AssumeCondition(doesControlArgs())
      AbstractElement* result = paeSecondArgument;
      paeSecondArgument = nullptr;
      return result;
   }

inline EvaluationEnvironment&
EvaluationEnvironment::setFirstArgument(const AbstractElement& argument)
   {  if (!doesControlArgs())
         paeFirstArgument = &const_cast<AbstractElement&>(argument);
      else {
         if (paeFirstArgument)
            delete paeFirstArgument;
         paeFirstArgument = nullptr;
         paeFirstArgument = argument.createSCopy();
      };
      return *this;
   }

inline EvaluationEnvironment&
EvaluationEnvironment::setSecondArgument(const AbstractElement& argument)
   {  if (!doesControlArgs())
         paeFirstArgument = &const_cast<AbstractElement&>(argument);
      else {
         if (paeSecondArgument)
            delete paeSecondArgument;
         paeSecondArgument = nullptr;
         paeSecondArgument = argument.createSCopy();
      };
      return *this;
   }

inline EvaluationEnvironment&
EvaluationEnvironment::setArgument(const AbstractElement& argument)
   {  AssumeCondition(!paeFirstArgument || !paeFirstArgument)
      (paeFirstArgument ? paeSecondArgument : paeFirstArgument)
         = (doesControlArgs() ? argument.createSCopy()
                              : &const_cast<AbstractElement&>(argument));
      return *this;
   }


inline EvaluationEnvironment&
EvaluationEnvironment::absorbFirstArgument(AbstractElement* pargument)
   {  AssumeCondition(doesControlArgs())
      if (paeFirstArgument)
         delete paeFirstArgument;
      paeFirstArgument = pargument;
      return *this;
   }

inline EvaluationEnvironment&
EvaluationEnvironment::absorbSecondArgument(AbstractElement* pargument)
   {  AssumeCondition(doesControlArgs())
      if (paeSecondArgument)
         delete paeSecondArgument;
      paeSecondArgument = pargument;
      return *this;
   }

inline EvaluationEnvironment&
EvaluationEnvironment::absorbArgument(AbstractElement* argument)
   {  AssumeCondition((doesControlArgs()) && (!paeFirstArgument || !paeFirstArgument))
      (paeFirstArgument ? paeSecondArgument : paeFirstArgument) = argument;
      return *this;
   }

inline EvaluationEnvironment&
EvaluationEnvironment::setFirstArgument(PPAbstractElement argument)
   {  AssumeCondition(doesControlArgs())
      if (paeFirstArgument)
         delete paeFirstArgument;
      paeFirstArgument = argument.extractElement();
      return *this;
   }

inline EvaluationEnvironment&
EvaluationEnvironment::setSecondArgument(PPAbstractElement argument)
   {  AssumeCondition(doesControlArgs())
      if (paeSecondArgument)
         delete paeSecondArgument;
      paeSecondArgument = argument.extractElement();
      return *this;
   }

inline EvaluationEnvironment&
EvaluationEnvironment::setArgument(PPAbstractElement argument)
   {  AssumeCondition((doesControlArgs()) && (!paeFirstArgument || !paeFirstArgument))
      (paeFirstArgument ? paeSecondArgument : paeFirstArgument) = argument.extractElement();
      return *this;
   }

class AbstractElement::Methods {
  public:
   typedef ZeroResult BooleanResult;

   static PPAbstractElement apply(const AbstractElement& source, const VirtualOperation& operation,
         EvaluationEnvironment& env)
      {  AssumeCondition(operation.isConst())
         assume(const_cast<AbstractElement&>(source).apply(operation, env));
         PPAbstractElement result = env.presult();
         env.clear();
         return result;
      }
   static PPAbstractElement apply(const AbstractElement& source, const VirtualOperation& operation,
         const AbstractElement& arg, EvaluationEnvironment& env)
      {  AssumeCondition(operation.isConst())
         env.setFirstArgument(arg);
         assume(const_cast<AbstractElement&>(source).apply(operation, env));
         PPAbstractElement result = env.presult();
         env.clear();
         return result;
      }

   static BooleanResult applyBoolean(const AbstractElement& source, const VirtualOperation& operation,
         const AbstractElement& arg)
      {  AssumeCondition(operation.isConst())
         EvaluationEnvironment env(EvaluationEnvironment::Init().stopErrorStates());
         env.setFirstArgument(arg);
         assume(const_cast<AbstractElement&>(source).apply(operation, env));
         return env.getResult().queryZeroResult();
      }
   static BooleanResult convertBoolean(const AbstractElement& source)
      {  return source.queryZeroResult(); }

   static PPAbstractElement applyAssign(PPAbstractElement source, const VirtualOperation& operation,
         EvaluationEnvironment& env)
      {  AssumeCondition(!operation.isConst())
         assume(source->apply(operation, env));
         return !env.isEmpty() ? (env.hasResult() ? env.presult() : source)
            : PPAbstractElement();
      }
   static PPAbstractElement applyAssign(PPAbstractElement source, const VirtualOperation& operation,
         const AbstractElement& arg, EvaluationEnvironment& env)
      {  AssumeCondition(!operation.isConst())
         env.setFirstArgument(arg);
         assume(source->apply(operation, env));
         return !env.isEmpty()
            ? (env.hasResult() ? PPAbstractElement(env.presult()) : source)
            : PPAbstractElement();
      }
};

template <class TypeElement, class TypeBaseEnvironment>
class TEvaluationEnvironment : public TypeBaseEnvironment {
  private:
   typedef TypeBaseEnvironment inherited;
   typedef TEvaluationEnvironment<TypeElement, TypeBaseEnvironment> thisType;

  public:
   TEvaluationEnvironment(const typename inherited::Init& init) : inherited(init) {}
   TEvaluationEnvironment(const thisType& source) = default;
   
   void storeResult(TypeElement* result) { inherited::storeResult(result); }
   TypeElement& getResult() const { return (TypeElement&) inherited::getResult(); }
   TypeElement* extractResult() { return (TypeElement*) inherited::extractResult(); }
   TypeElement& getFirstArgument() const { return (TypeElement&) inherited::getFirstArgument(); }
   TypeElement& getSecondArgument() const { return (TypeElement&) inherited::getFirstArgument(); }

   thisType& setFirstArgument(const TypeElement& argument)
      {  return (thisType&) inherited::setFirstArgument(argument); }
   thisType& setSecondArgument(const TypeElement& argument)
      {  return (thisType&) inherited::setSecondArgument(argument); }
   thisType& setArgument(const TypeElement& argument)
      {  return (thisType&) inherited::setArgument(argument); }
};

class VariableDeclaration : public PNT::SharedElement, public STG::IOObject, protected ExtendedParameters {
  private:
   typedef PNT::SharedElement inherited;
   PPAbstractElement ppveDomain;

  protected:
   virtual ComparisonResult _compare(const EnhancedObject& asource) const override
      {  return inherited::_compare(asource); }

  public:
   VariableDeclaration() {}
   VariableDeclaration(const VariableDeclaration& source)
      :  inherited(source), STG::IOObject(source),
         ExtendedParameters(source), ppveDomain(source.ppveDomain, PNT::Pointer::Duplicate()) {}
   VariableDeclaration& operator=(const VariableDeclaration& source)
      {  inherited::operator=(source);
         ExtendedParameters::operator=(source);
         ppveDomain.fullAssign(source.ppveDomain);
         return *this;
      }
   DefineCopy(VariableDeclaration)
   DDefineAssign(VariableDeclaration)
   StaticInheritConversions(VariableDeclaration, inherited)
   typedef HandlerCast<VariableDeclaration, PNT::SharedElement> VariablesSupportCast;
      
   int getType() const { return queryParams(); }
   virtual bool isValid() const override { return inherited::isValid() && (getType() != 0); }
   virtual STG::SubString name() const { return STG::SString(); }
   void setDomain(const AbstractElement& domain) { ppveDomain.setElement(domain); }
   const AbstractElement& domain() const { return *ppveDomain; }
   AbstractElement& domain() { return *ppveDomain; }
   PPAbstractElement& sdomain() { return ppveDomain; }
};

namespace DVariable {

class BaseDeclaration : public COL::List::Node {
  public:
   BaseDeclaration() {}
   BaseDeclaration(const BaseDeclaration& source) = default;
   DefineCopy(BaseDeclaration)
};

typedef COL::TList<BaseDeclaration> BaseDeclarationBrothers;

} // end of namespace DVariable

class ScopedVariableDeclaration
   :  public COL::TGenericParentTreeElement
         <DVariable::BaseDeclarationBrothers, DVariable::BaseDeclaration>,
      public VariableDeclaration {
  private:
   typedef COL::TGenericParentTreeElement
      <DVariable::BaseDeclarationBrothers, DVariable::BaseDeclaration> inherited;
   PPAbstractElement ppveDomain;

  protected:
   virtual ComparisonResult _compare(const EnhancedObject& asource) const override
      {  return inherited::_compare(asource); }

  public:
   ScopedVariableDeclaration() {}
   ScopedVariableDeclaration(const ScopedVariableDeclaration& source) = default;
   ScopedVariableDeclaration& operator=(const ScopedVariableDeclaration& source)
      {  inherited::operator=(source);
         VariableDeclaration::operator=(source);
         return *this;
      }
   DefineCopy(ScopedVariableDeclaration)
   DDefineAssign(ScopedVariableDeclaration)
   StaticInheritConversions(ScopedVariableDeclaration, inherited)
      
   typedef HandlerCast<ScopedVariableDeclaration, inherited> RegistrationCast;
};

class VariableScope
   :  public COL::TCopyCollection<
         COL::TreeParent<ScopedVariableDeclaration, ScopedVariableDeclaration::RegistrationCast> > {
  private:
   typedef COL::TCopyCollection<COL::TreeParent<ScopedVariableDeclaration, ScopedVariableDeclaration::RegistrationCast> > inherited;

  public:
   VariableScope() {}
   VariableScope(const VariableScope& source) = default;
   DefineCopy(VariableScope)
};

}} // end of namespace Analyzer::Scalar

#endif // Analyzer_Interpretation_Scalar_AtomicElementH
