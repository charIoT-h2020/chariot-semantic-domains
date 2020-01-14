/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : VirtualElement.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a hierarchy for approximate scalar elements.
//   The approximate elements can be used in abstract interpretation
//     based analyses, in pure formal analyses or in model checking.
//

#ifndef Analyzer_Scalar_Approximate_VirtualElementH
#define Analyzer_Scalar_Approximate_VirtualElementH

#include "Analyzer/Virtual/Scalar/AtomicElement.h"
#include "Analyzer/Scalar/ConcreteOperation.h"
#include "Analyzer/Scalar/Implementation/General/MultiBitElement.h"
#include "Pointer/TreeMethods.h"

namespace Analyzer { namespace Scalar { namespace Approximate {

class PPVirtualElement;

}} // end of namespace Scalar::Approximate

namespace Memory {

class VirtualModificationDate : public EnhancedObject {
  private:
   typedef EnhancedObject inherited;

  public:
   VirtualModificationDate() {}
   VirtualModificationDate(const VirtualModificationDate& source) = default;
   DefineCopy(VirtualModificationDate)
};

class VirtualMemoryModifier : public EnhancedObject {
  private:
   typedef EnhancedObject inherited;

  public:
   VirtualMemoryModifier() {}
   VirtualMemoryModifier(const VirtualMemoryModifier& source) = default;
   DefineCopy(VirtualMemoryModifier)

   virtual bool getContentUpdated(PNT::SharedPointer*& caller, VirtualModificationDate& date) { return false; }
};

} // end of namespace Memory

namespace Scalar { namespace Approximate {

/******************************************/
/* Definition of the class QueryOperation */
/******************************************/

class VirtualQueryOperation : public ExtendedParameters {
  private:
   typedef ExtendedParameters inherited;
   typedef VirtualQueryOperation thisType;

  public:
   enum Type
      {  TUndefined, TOperation, TCompareSpecial, TSimplification, TDomain, TExtern, TExternMethod, TDuplication, EndOfType };
   enum TypeDomain
      {  TDUndefined, TDDisjunction, TDMultiBitConstant };
   enum TypeOperation
      {  TOUndefined, TOInverseCast };
   enum TypeExtern
      {  TEUndefined, TENativeOperation, EndOfTypeExtern };
   enum TypeCompareSpecial
      {  TCSUndefined, TCSGuard };
   enum TypeDuplication
      {  TCDUndefined, TCDClone, TCDSpecialize };

  protected:
   DefineExtendedParameters(3, ExtendedParameters)
   DefineSubExtendedParameters(Type, 3, INHERITED)
   DefineSubExtendedParameters(TypeDomain, 2, Type)
   DefineSubExtendedParameters(TypeOperation, 1, Type)
   DefineSubExtendedParameters(TypeExtern, 1, Type)
   DefineSubExtendedParameters(TypeCompareSpecial, 1, Type)
   DefineSubExtendedParameters(TypeDuplication, 2, Type)

   void setOperation() { AssumeCondition(!hasTypeField()) mergeTypeField(TOperation); }
   void setDomain() { AssumeCondition(!hasTypeField()) mergeTypeField(TDomain); }
   void setExtern() { AssumeCondition(!hasTypeField()) mergeTypeField(TExtern); }
   void setExternMethod() { AssumeCondition(!hasTypeField()) mergeTypeField(TExternMethod); }
   void setDuplication() { AssumeCondition(!hasTypeDuplicationField()) mergeTypeField(TDuplication); }

  public:
   VirtualQueryOperation& setCompareSpecial() { AssumeCondition(!hasTypeField()) mergeTypeField(TCompareSpecial); return *this; }
   VirtualQueryOperation& setSimplification() { AssumeCondition(!hasTypeField()) mergeTypeField(TSimplification); return *this; }
   VirtualQueryOperation& setNewDisjunction() { setDomain(); mergeTypeDomainField(TDDisjunction); return *this; }
   VirtualQueryOperation& setNewMultiBitConstant() { setDomain(); mergeTypeDomainField(TDMultiBitConstant); return *this; }
   VirtualQueryOperation& setInverseCast() { setOperation(); mergeTypeOperationField(TOInverseCast); return *this; }
   VirtualQueryOperation& setNativeOperation() { setExtern(); mergeTypeExternField(TENativeOperation); return *this; }
   VirtualQueryOperation& setGuard() { setCompareSpecial(); mergeTypeCompareSpecialField(TCSGuard); return *this; }
   VirtualQueryOperation& setCopy() { setDuplication(); mergeTypeDuplicationField(TCDClone); return *this; }
   VirtualQueryOperation& setSpecialize() { setDuplication(); mergeTypeDuplicationField(TCDSpecialize); return *this; }

  public:
   VirtualQueryOperation() {}
   VirtualQueryOperation(const VirtualQueryOperation& source) = default;

   bool isOperation() const { return queryTypeField() == TOperation; }
   bool isCompareSpecial() const { return queryTypeField() == TCompareSpecial; }
   bool isSimplification() const { return queryTypeField() == TSimplification; }
   bool isDomain() const { return queryTypeField() == TDomain; }
   bool isDuplication() const { return queryTypeField() == TDuplication; }
   bool isExtern() const { return queryTypeField() == TExtern; }
   bool isExternMethod() const { return queryTypeField() == TExternMethod; }
   Type getType() const { return (Type) queryTypeField(); }
   int getExtensions() const { return queryParams() >> END_Type_OF_MASK; }

   class Environment : protected ExtendedParameters {
     public:
      Environment() {}
      Environment(const Environment& source) = default;
#if DefineDebugLevel != 1
      virtual ~Environment() {}
#endif
      Environment& operator=(const Environment& source) = default;
   };

   class OperationEnvironment;
   class CompareSpecialEnvironment;
   class SimplificationEnvironment;
   class DomainEnvironment;
   class DisjunctionEnvironment;
   class ExtendedOperationEnvironment;
   class ExternNativeOperationEnvironment;
   class GuardEnvironment;
   class DuplicationEnvironment;
   typedef ExtendedOperationEnvironment InverseCastOperationEnvironment;
};

class VirtualQueryOperation::ExternNativeOperationEnvironment : public Environment {
  public:
   enum TypeOperation
      {  TOUndefined, TOBit, TOInteger, TOMultiBit, TOFloating, TORational, TOReal };

  protected:
   DefineExtendedParameters(3, Environment)
   void setTypeOperation(TypeOperation operation) { AssumeCondition(!hasOwnField()) mergeOwnField(operation); }

  public:
   ExternNativeOperationEnvironment() {}
   ExternNativeOperationEnvironment(const ExternNativeOperationEnvironment& source) = default;

   void setBit() { setTypeOperation(TOBit); }
   void setInteger() { setTypeOperation(TOInteger); }
   void setMultiBit() { setTypeOperation(TOMultiBit); }
   void setFloating() { setTypeOperation(TOFloating); }
   void setRational() { setTypeOperation(TORational); }
   void setReal() { setTypeOperation(TOReal); }

   TypeOperation getTypeOperation() const { return (TypeOperation) queryOwnField(); }
};

/*************************************************/
/* Definition of the class EvaluationEnvironment */
/*************************************************/

class VirtualElement;
class PPVirtualElement;
class ConstraintEnvironment;
class FloatingEnvironment;

namespace DEnvironment {

class ElementResult : public EnhancedObject {
  private:
   typedef EnhancedObject inherited;
   int uIndex;
   PPAbstractElement ppaeResult;

  public:
   ElementResult(int index, PPVirtualElement result);
   ElementResult(const ElementResult& source)
      :  inherited(source), uIndex(source.uIndex),
         ppaeResult(source.ppaeResult, PNT::Pointer::Duplicate()) {}
   ElementResult& operator=(const ElementResult& source)
      {  inherited::operator=(source);
         uIndex = source.uIndex;
         ppaeResult.fullAssign(source.ppaeResult);
         return *this;
      }
   DefineCopy(ElementResult)
   DDefineAssign(ElementResult)

   const int& getIndex() const { return uIndex; }
   VirtualElement& result() const;
   PPVirtualElement extractResult() const;

   class Key {
     public:
      Key(const COL::VirtualSortedCollection& support) {}
      typedef const int& KeyType;
      typedef int ControlKeyType;
      static const int& key(const ElementResult& source) { return source.getIndex(); }
      static ComparisonResult compare(const int& fstKey, const int& sndKey)
         {  return fcompare(fstKey, sndKey); }
   };
};

class FlagsValues : public COL::TSortedArray<ElementResult, ElementResult::Key> {
  private:
   typedef COL::TSortedArray<ElementResult, ElementResult::Key> inherited;

  public:
   class Transmit {};
   FlagsValues() {}
   FlagsValues(const FlagsValues& source) : inherited(source, AMDuplicate) {}
   FlagsValues(const FlagsValues& source, Transmit) : inherited(source)
      {  const_cast<FlagsValues&>(source).deleteAll(); }
   FlagsValues& operator=(const FlagsValues& source)
      {  inherited::_fullAssign(source, ExtendedReplaceParameters().setDuplicate()); return *this; }
   ~FlagsValues() { inherited::freeAll(); }

   PPVirtualElement extractFlag(int index);
   const VirtualElement* getFlag(int index) const;
   void absorbFlag(int index, PPVirtualElement flag);
};

} // end of namespace DEnvironment

class FloatingEnvironment : public Scalar::FloatingEnvironment {
  private:
   typedef Scalar::FloatingEnvironment inherited;
   DEnvironment::FlagsValues fvFlagsValues;

  public:
   FloatingEnvironment() {}
   FloatingEnvironment(const FloatingEnvironment& source) : inherited(source) {}
   FloatingEnvironment& operator=(const FloatingEnvironment& source)
      {  return (FloatingEnvironment&) inherited::operator=(source); }
   DefineCopy(FloatingEnvironment)
   DDefineAssign(FloatingEnvironment)

   DEnvironment::FlagsValues& flagsValues() { return fvFlagsValues; }
   const DEnvironment::FlagsValues& flagsValues() const { return fvFlagsValues; }
};

/*
class VirtualCastRingTraits {
  public:
   virtual VirtualCastRingTraits* clone() = 0;
   virtual VirtualElement* createCharElement(const Scalar::VirtualElement::Init& iInit) = 0;
   virtual VirtualElement* createSignedCharElement(const Scalar::VirtualElement::Init& iInit) = 0;
   virtual VirtualElement* createUnsignedCharElement(const Scalar::VirtualElement::Init& iInit) = 0;
   virtual VirtualElement* createShortElement(const Scalar::VirtualElement::Init& iInit) = 0;
   virtual VirtualElement* createUnsignedShortElement(const Scalar::VirtualElement::Init& iInit) = 0;
   virtual VirtualElement* createIntElement(const Scalar::VirtualElement::Init& iInit) = 0;
   virtual VirtualElement* createUnsignedIntElement(const Scalar::VirtualElement::Init& iInit) = 0;
   virtual VirtualElement* createLongElement(const Scalar::VirtualElement::Init& iInit) = 0;
   virtual VirtualElement* createUnsignedLongElement(const Scalar::VirtualElement::Init& iInit) = 0;
   virtual VirtualElement* createFloatElement(const Scalar::VirtualElement::InitFloat& iInit) = 0;
   virtual VirtualElement* createDoubleElement(const Scalar::VirtualElement::InitFloat& iInit) = 0;
   virtual VirtualElement* createLongDoubleElement(const Scalar::VirtualElement::InitFloat& iInit) = 0;
   virtual VirtualElement* createBitElement(const Scalar::VirtualElement::Init& iInit) = 0;
   virtual VirtualElement* createMultiBitElement(const Scalar::VirtualElement::Init& iInit) = 0;
   virtual VirtualElement* createMultiFloatElement(const Scalar::VirtualElement::Init& iInit) = 0;
   virtual Scalar::Constant::VirtualElement* createConstantMultiFloatElement(const Scalar::Constant::VirtualElement::Init& iInit) = 0;
};
*/

class EvaluationEnvironment : public Scalar::EvaluationEnvironment {
  public:
   class Init;
   class Verdict {
     public:
      enum TypeVerdict { EVNotPerformed=0, EVExactApplied=1, EVDegradateApplied=3 };

     private:
      TypeVerdict vVerdict;

     public:
      Verdict() : vVerdict(EVNotPerformed) {}
      Verdict(TypeVerdict verdict) : vVerdict(verdict) {}
      Verdict(const Verdict& source) = default;

      bool isValid() const { return vVerdict != EVNotPerformed; }
      bool isExact() const { return vVerdict == EVExactApplied; }
      bool isDegradate() const { return vVerdict == EVDegradateApplied; }
      const TypeVerdict& verdict() const { return vVerdict; }

      Verdict& setPrecise() { vVerdict = EVExactApplied; return *this; }
      Verdict& setDegradate() { vVerdict = EVDegradateApplied; return *this; }
   };

   class LatticeCreation {
     public:
      enum Creation { CUndefined, CFormal, CDisjunction, CInterval, CShareTop, CTop };

     private:
      Creation cSureCreation;
      Creation cMayCreation;

     public:
      LatticeCreation() : cSureCreation(CUndefined), cMayCreation(CUndefined) {}
      LatticeCreation(Creation creation) : cSureCreation(creation), cMayCreation(creation) {}
      LatticeCreation(Creation sureCreation, Creation mayCreation)
         : cSureCreation(sureCreation), cMayCreation(mayCreation) {}
      LatticeCreation(const LatticeCreation& source) = default;

      bool isFormal() const { return cSureCreation == CFormal; }
      bool isDisjunction() const { return cSureCreation == CDisjunction; }
      bool isInterval() const { return cSureCreation == CInterval; }
      bool isShareTop() const { return cSureCreation == CShareTop; }
      bool isTop() const { return cSureCreation == CTop; }
      const Creation& creation() const { return cSureCreation; }
      const Creation& sureCreation() const { return cSureCreation; }
      const Creation& mayCreation() const { return cMayCreation; }

      bool mayBeFormal() const { return cMayCreation <= CFormal; }
      bool mayBeDisjunction() const { return cMayCreation <= CDisjunction; }
      bool mayBeInterval() const { return cMayCreation <= CInterval; }
      bool mayBeShareTop() const { return cMayCreation <= CShareTop; }
      bool mayBeTop() const { return cMayCreation <= CTop; }

      void setFormal() { cSureCreation = cMayCreation = CFormal; }
      void setDisjunction() { cSureCreation = cMayCreation = CDisjunction; }
      void setInterval() { cSureCreation = cMayCreation = CInterval; }
      void setShareTop() { cSureCreation = cMayCreation = CShareTop; }
      void setTop() { cSureCreation = cMayCreation = CTop; }

      void authorizeFormal() { cMayCreation = CFormal; }
      void authorizeDisjunction() { cMayCreation = CDisjunction; }
      void authorizeInterval() { cMayCreation = CInterval; }
      void authorizeShareTop() { cMayCreation = CShareTop; }
      void authorizeTop() { cMayCreation = CTop; }

      bool isValid() const { return cSureCreation >= cMayCreation; }
   };

   class InformationKind {
     public:
      enum Kind { IKExact=0, IKMay=1, IKSure=2, IKUndefined=3 };

     private:
      Kind kKind;

     public:
      InformationKind() : kKind(IKExact) {}
      InformationKind(Kind kind) : kKind(kind) {}
      InformationKind(const InformationKind& source) = default;

      bool isExact() const { return kKind == IKExact; }
      bool isMay() const   { return kKind == IKMay; }
      bool isSure() const  { return kKind == IKSure; }
      bool isValid() const { return kKind != IKUndefined; }
      const Kind& kind() const { return kKind; }

      InformationKind& setExact()   { kKind = IKExact; return *this; }
      InformationKind& setMay()     { kKind = IKMay; return *this; }
      InformationKind& setSure()    { kKind = IKSure; return *this; }
   };
   class GuardTopLevel;

  private:
   typedef Scalar::EvaluationEnvironment inherited;
   typedef EvaluationEnvironment thisType;
   DEnvironment::FlagsValues fvErrorsValues;
   // Memory::VirtualMemoryModifier* spmmMemoryModifier;
   Memory::VirtualModificationDate* pmdModificationDate;

   unsigned int uMaxDepth;
   // VirtualCastRingTraits* pcrtCastRingTraits;

  protected:
   DefineExtendedParameters(20, inherited) // -> 32

   typedef inherited::DuplicateParameters InheritedDuplicateParameters;
   class DuplicateParameters : public InheritedDuplicateParameters {
     private:
      typedef InheritedDuplicateParameters inherited;
      enum Transmission { TNoTransmission, TDuplicate, TTransfert };
      Transmission tErrorsValuesTransmission;
      Transmission tFloatingFlagsValuesTransmission;

     public:
      DuplicateParameters() : tErrorsValuesTransmission(TNoTransmission), tFloatingFlagsValuesTransmission(TNoTransmission) {}
      DuplicateParameters(const DuplicateParameters& source) = default;

      DuplicateParameters& transmitField() { return (DuplicateParameters&) inherited::transmitField(); }
      DuplicateParameters& transmitFloatPart() { return (DuplicateParameters&) inherited::transmitFloatPart(); }
      DuplicateParameters& transmitFloatPart(bool hasFloating) { return (DuplicateParameters&) inherited::transmitFloatPart(hasFloating); }
      DuplicateParameters& duplicateErrorsValues() { tErrorsValuesTransmission = TDuplicate; return *this; }
      DuplicateParameters& transfertErrorsValues() { tErrorsValuesTransmission = TTransfert; return *this; }
      DuplicateParameters& duplicateFloatingFlagsValues()
         {  inherited::transmitFloatPart(); tFloatingFlagsValuesTransmission = TDuplicate; return *this; }
      DuplicateParameters& transfertFloatingFlagsValues()
         {  inherited::transmitFloatPart(); tFloatingFlagsValuesTransmission = TTransfert; return *this; }

      friend class Scalar::Approximate::EvaluationEnvironment;
   };

   EvaluationEnvironment(const EvaluationEnvironment& source, const DuplicateParameters& params)
      :  inherited(source, params), /* spmmMemoryModifier(source.spmmMemoryModifier), */ pmdModificationDate(source.pmdModificationDate), uMaxDepth(0) // , pcrtCastRingTraits(source.pcrtCastRingTraits)
      {  // clearControlRingField();
         if (params.tErrorsValuesTransmission == DuplicateParameters::TDuplicate)
            fvErrorsValues.addCopyAll(source.fvErrorsValues);
         else if (params.tErrorsValuesTransmission == DuplicateParameters::TTransfert)
            const_cast<EvaluationEnvironment&>(source).fvErrorsValues.moveAllTo(fvErrorsValues);
         if (params.tFloatingFlagsValuesTransmission == DuplicateParameters::TDuplicate)
            floatingPart().flagsValues().addCopyAll(source.floatingPart().flagsValues());
         else if (params.tFloatingFlagsValuesTransmission == DuplicateParameters::TTransfert)
            const_cast<EvaluationEnvironment&>(source).floatingPart().flagsValues().moveAllTo(floatingPart().flagsValues());
      }

   DefineSubExtendedParameters(SureError,7,INHERITED)
   DefineSubExtendedParameters(Verdict,2,SureError)
   DefineSubExtendedParameters(Unstable,1,Verdict)
   DefineSubExtendedParameters(InformationKind,2,Unstable)
   DefineSubExtendedParameters(LatticeCreation, 6, InformationKind)
   DefineSubExtendedParameters(RequireErrorsAsLattice, 1, LatticeCreation)
   DefineSubExtendedParameters(LowLevel, 1, RequireErrorsAsLattice)
   // DefineSubExtendedParameters(ControlRing, 1, LowLevel)

   friend class ConstraintEnvironment;

  public:
   Verdict getVerdict() const { return Verdict((Verdict::TypeVerdict) queryVerdictField()); }
   thisType& setVerdict(const Verdict& evVerdict) { setVerdictField(evVerdict.verdict()); return *this; }
   thisType& mergeVerdict(const Verdict& evVerdict) { mergeVerdictField(evVerdict.verdict()); return *this; }
   thisType& mergeVerdictExact() { mergeVerdictField(Verdict::EVExactApplied); return *this; }
   thisType& mergeVerdictDegradate() { mergeVerdictField(Verdict::EVDegradateApplied); return *this; }

   InformationKind getInformationKind() const { return InformationKind((InformationKind::Kind) queryInformationKindField()); }
   thisType& setInformationKind(const InformationKind& kind) { setInformationKindField(kind.kind()); return *this; }
   thisType& mergeInformationKind(const InformationKind& kind) { mergeInformationKindField(kind.kind()); return *this; }

   thisType& mergeUnstable(thisType& source) {  mergeUnstableField(source.queryUnstableField()); return *this; }
   void clearErrors()
      {  clearErrorField();
         clearSureErrorField();
      }
   void approximateErrors()
      {  mergeErrorField(querySureErrorField());
         clearSureErrorField();
      }

   EvaluationEnvironment(const Init& init);
   EvaluationEnvironment(const thisType& source)
      :  inherited(source), fvErrorsValues(source.fvErrorsValues),
         /* spmmMemoryModifier(source.spmmMemoryModifier), */
         pmdModificationDate(source.pmdModificationDate),
         uMaxDepth(source.uMaxDepth) //, pcrtCastRingTraits(source.pcrtCastRingTraits)
      {  // if (source.hasControlRingField() && pcrtCastRingTraits)
         //   pcrtCastRingTraits = pcrtCastRingTraits->clone();
         if (hasFloatingPart())
            floatingPart().flagsValues() = source.floatingPart().flagsValues();
      }
   thisType& operator=(const thisType& source)
      {  inherited::operator=(source);
         fvErrorsValues = source.fvErrorsValues;
         uMaxDepth = source.uMaxDepth;
         return *this;
      }

/* virtual ~EvaluationEnvironment()
      {  if (pcrtCastRingTraits && hasControlRingField())
            delete pcrtCastRingTraits;
      }
*/

   DefineCopy(EvaluationEnvironment)
   DDefineAssign(EvaluationEnvironment)
   virtual bool isApproximate() const override { return true; }
   virtual EvaluationEnvironment& getOriginal() { return *this; }

   class GuardLatticeCreation {
     private:
      EvaluationEnvironment* peeEnv;
      unsigned int uLatticeCreation;

     public:
      GuardLatticeCreation() : peeEnv(nullptr), uLatticeCreation(0) {}
      GuardLatticeCreation(EvaluationEnvironment& env) : peeEnv(&env), uLatticeCreation(env.queryLatticeCreationField()) {}
      ~GuardLatticeCreation() { if (peeEnv) peeEnv->setLatticeCreationField(uLatticeCreation); }

      void setFrom(EvaluationEnvironment& env)
         {  peeEnv = &env;
            uLatticeCreation = env.queryLatticeCreationField();
         }
   };
   friend class GuardLatticeCreation;

   class GuardTopLevel {
     private:
      EvaluationEnvironment& eeReference;
      bool fLowLevel;

     public:
      GuardTopLevel(EvaluationEnvironment& reference) : eeReference(reference), fLowLevel(reference.hasLowLevelField())
         {  reference.mergeLowLevelField(1); }
      ~GuardTopLevel()
         {  eeReference.setLowLevelField(fLowLevel); }
   };
   class GuardLowLevel {
     private:
      EvaluationEnvironment& eeReference;
      bool fLowLevel;

     public:
      GuardLowLevel(EvaluationEnvironment& reference) : eeReference(reference), fLowLevel(reference.hasLowLevelField())
         {  reference.clearLowLevelField(); }
      ~GuardLowLevel()
         {  eeReference.setLowLevelField(fLowLevel); }
   };
   class SetTopLevel {
     private:
      EvaluationEnvironment& eeReference;
      bool fTopLevel;

     public:
      SetTopLevel(EvaluationEnvironment& reference, Memory::VirtualModificationDate& modificationDate)
         :  eeReference(reference), fTopLevel(reference.hasLowLevelField())
         {  reference.clearLowLevelField();
            reference.setModificationDate(modificationDate);
         }
      ~SetTopLevel()
         {  eeReference.setLowLevelField(fTopLevel);
            eeReference.clearModificationDate();
         }
   };
   friend class GuardTopLevel;
   friend class SetTopLevel;

   void setLatticeCreation(const LatticeCreation& creation)
      {  setLatticeCreationField((creation.sureCreation() << 3) | creation.mayCreation()); }
   void setFormalLatticeCreation()
      {  setLatticeCreationField((LatticeCreation::CFormal << 3) | LatticeCreation::CFormal); }
   void setDisjunctionLatticeCreation()
      {  setLatticeCreationField((LatticeCreation::CDisjunction << 3) | LatticeCreation::CDisjunction); }
   void setIntervalLatticeCreation()
      {  setLatticeCreationField((LatticeCreation::CInterval << 3) | LatticeCreation::CInterval); }
   void setTopLatticeCreation()
      {  setLatticeCreationField((LatticeCreation::CTop << 3) | LatticeCreation::CTop); }
   void setTopLatticeCreationAuthorizeFormal()
      {  setLatticeCreationField((LatticeCreation::CTop << 3) | LatticeCreation::CFormal); }
   void setLatticeCreationAuthorizeFormal()
      {  unsigned int value = queryLatticeCreationField();
         setLatticeCreationField((value & 0x38) | LatticeCreation::CFormal);
      }
   void intersectLatticeCreation(const LatticeCreation& source)
      {  unsigned int creation = queryLatticeCreationField();
         LatticeCreation::Creation sureCreation = (LatticeCreation::Creation) (creation >> 3);
         LatticeCreation::Creation mayCreation = (LatticeCreation::Creation) (creation & 0x7);
         LatticeCreation::Creation sourceSureCreation = source.sureCreation();
         LatticeCreation::Creation sourceMayCreation = source.mayCreation();
         if (sourceSureCreation < sureCreation || sourceMayCreation < mayCreation) {
            sureCreation = (sourceSureCreation < sureCreation) ? sourceSureCreation : sureCreation;
            mayCreation = (sourceMayCreation < mayCreation) ? sourceMayCreation : mayCreation;
            setLatticeCreationField((sureCreation << 3) | mayCreation);
         };
      }
   void intersectFormalLatticeCreation()
      {  unsigned int creation = queryLatticeCreationField();
         LatticeCreation::Creation sureCreation = (LatticeCreation::Creation) (creation >> 3);
         LatticeCreation::Creation mayCreation = (LatticeCreation::Creation) (creation & 0x7);
         if (LatticeCreation::CFormal < sureCreation || LatticeCreation::CFormal < mayCreation) {
            sureCreation = (LatticeCreation::CFormal < sureCreation) ? LatticeCreation::CFormal : sureCreation;
            mayCreation = (LatticeCreation::CFormal < mayCreation) ? LatticeCreation::CFormal : mayCreation;
            setLatticeCreationField((sureCreation << 3) | mayCreation);
         };
      }
   void intersectDisjunctionLatticeCreation()
      {  unsigned int creation = queryLatticeCreationField();
         LatticeCreation::Creation sureCreation = (LatticeCreation::Creation) (creation >> 3);
         LatticeCreation::Creation mayCreation = (LatticeCreation::Creation) (creation & 0x7);
         if (LatticeCreation::CDisjunction < sureCreation || LatticeCreation::CDisjunction < mayCreation) {
            sureCreation = (LatticeCreation::CDisjunction < sureCreation) ? LatticeCreation::CDisjunction : sureCreation;
            mayCreation = (LatticeCreation::CDisjunction < mayCreation) ? LatticeCreation::CDisjunction : mayCreation;
            setLatticeCreationField((sureCreation << 3) | mayCreation);
         };
      }
   void intersectIntervalLatticeCreation()
      {  unsigned int creation = queryLatticeCreationField();
         LatticeCreation::Creation sureCreation = (LatticeCreation::Creation) (creation >> 3);
         LatticeCreation::Creation mayCreation = (LatticeCreation::Creation) (creation & 0x7);
         if (LatticeCreation::CInterval < sureCreation || LatticeCreation::CInterval < mayCreation) {
            sureCreation = (LatticeCreation::CInterval < sureCreation) ? LatticeCreation::CInterval : sureCreation;
            mayCreation = (LatticeCreation::CInterval < mayCreation) ? LatticeCreation::CInterval : mayCreation;
            setLatticeCreationField((sureCreation << 3) | mayCreation);
         };
      }
   void intersectTopLatticeCreation()
      {  unsigned int creation = queryLatticeCreationField();
         LatticeCreation::Creation sureCreation = (LatticeCreation::Creation) (creation >> 3);
         LatticeCreation::Creation mayCreation = (LatticeCreation::Creation) (creation & 0x7);
         if (LatticeCreation::CTop < sureCreation || LatticeCreation::CTop < mayCreation) {
            sureCreation = (LatticeCreation::CTop < sureCreation) ? LatticeCreation::CTop : sureCreation;
            mayCreation = (LatticeCreation::CTop < mayCreation) ? LatticeCreation::CTop : mayCreation;
            setLatticeCreationField((sureCreation << 3) | mayCreation);
         };
      }
   void intersectTopLatticeCreationAuthorizeFormal()
      {  unsigned int creation = queryLatticeCreationField();
         LatticeCreation::Creation sureCreation = (LatticeCreation::Creation) (creation >> 3);
         LatticeCreation::Creation mayCreation = (LatticeCreation::Creation) (creation & 0x7);
         if (LatticeCreation::CTop < sureCreation || LatticeCreation::CFormal < mayCreation) {
            sureCreation = (LatticeCreation::CTop < sureCreation) ? LatticeCreation::CTop : sureCreation;
            mayCreation = (LatticeCreation::CFormal < mayCreation) ? LatticeCreation::CFormal : mayCreation;
            setLatticeCreationField((sureCreation << 3) | mayCreation);
         };
      }
   void intersectLatticeCreationAuthorizeFormal()
      {  unsigned int creation = queryLatticeCreationField() & 0x38;
         LatticeCreation::Creation mayCreation = (LatticeCreation::Creation) (creation & 0x7);
         if (LatticeCreation::CFormal < mayCreation) {
            LatticeCreation::Creation sureCreation = (LatticeCreation::Creation) (creation >> 3);
            mayCreation = LatticeCreation::CFormal;
            setLatticeCreationField((sureCreation << 3) | mayCreation);
         };
      }

   unsigned int& maxDepth() { return uMaxDepth; }
   const unsigned int& maxDepth() const { return uMaxDepth; }

   LatticeCreation getLatticeCreation() const
      {  unsigned int creation = queryLatticeCreationField();
         return LatticeCreation((LatticeCreation::Creation) (creation >> 3), (LatticeCreation::Creation) (creation & 0x7));
      }
   void clearLatticeCreation() { clearLatticeCreationField(); }
   bool hasLatticeCreation() { return hasLatticeCreationField(); }
   bool isTopLevel() const { return !hasLowLevelField(); }
   bool isLowLevel() const { return hasLowLevelField(); }
   void setLowLevel() { mergeLowLevelField(1); }
   void setControlTopLevel(Memory::VirtualModificationDate& modificationDate)
      {  clearLowLevelField();
         pmdModificationDate = &modificationDate;
      } 
   void clearControlTopLevel()
      {  AssumeCondition(!hasLowLevelField() && pmdModificationDate)
         mergeLowLevelField(1);
         pmdModificationDate = nullptr;
      } 

   void setRequireErrorsAsLattice() { mergeRequireErrorsAsLatticeField(1); }
   void clearRequireErrorsAsLattice() { clearRequireErrorsAsLatticeField(); }
   bool doesRequireErrorsAsLattice() const { return hasRequireErrorsAsLatticeField(); }

   bool hasPerformedOperation() const { return getVerdict().isValid(); }
   bool isExact() const { return getVerdict().isExact(); }

   void setUnstable() { mergeUnstableField(1); /* true */ }
   bool isUnstable() const { return hasUnstableField(); }
   bool isStable() const { return !hasUnstableField(); }

   void clearMask()
      {  clearVerdictField(); clearUnstableField(); clearSureErrorField();
         clearErrorField(); clearApplicabilityField(); clearEmptyField();
         if (hasFloatingPart())
            floatingPart().outputs().clear();
      }
   void clearSecondaryResults()
      {  fvErrorsValues.freeAll();
         if (hasFloatingPart())
            floatingPart().flagsValues().freeAll();
      }
   void clearEmpty() { inherited::clearEmpty(); clearVerdictField(); clearUnstableField(); }
   virtual void clear() override { inherited::clear(); clearSecondaryResults(); clearMask(); clearRequireErrorsAsLattice(); }
   thisType& assignField(const EvaluationEnvironment& env)
      {  InformationKind::Kind kind = (InformationKind::Kind) queryInformationKindField();
         unsigned int creation = queryLatticeCreationField();
         bool requireErrorsAsLattice = doesRequireErrorsAsLattice();
         inherited::assignField(env);
         setInformationKindField(kind);
         setLatticeCreationField(creation);
         setRequireErrorsAsLatticeField(requireErrorsAsLattice);
         return *this;
      }
   thisType& mergeField(const EvaluationEnvironment& env)
      {  InformationKind::Kind kind = (InformationKind::Kind) queryInformationKindField();
         unsigned int creation = queryLatticeCreationField();
         bool requireErrorsAsLattice = doesRequireErrorsAsLattice();
         inherited::mergeField(env);
         setInformationKindField(kind);
         setLatticeCreationField(creation);
         setRequireErrorsAsLatticeField(requireErrorsAsLattice);
         return *this;
      }

   FloatingEnvironment& floatingPart()
      {  return (FloatingEnvironment&) inherited::floatingPart(); }
   const FloatingEnvironment& floatingPart() const
      {  return (const FloatingEnvironment&) inherited::floatingPart(); }
   DEnvironment::FlagsValues& errorsValues() { return fvErrorsValues; }
   const DEnvironment::FlagsValues& errorsValues() const { return fvErrorsValues; }

   // void setMemoryModifier(Memory::VirtualMemoryModifier& memoryModifier) { spmmMemoryModifier = &memoryModifier; }
   // void clearMemoryModifier() { spmmMemoryModifier = nullptr; }
   void setModificationDate(Memory::VirtualModificationDate& modificationDate) { pmdModificationDate = &modificationDate; }
   void clearModificationDate() { pmdModificationDate = nullptr; }
   // Memory::VirtualMemoryModifier* getSMemoryModifier() const { return spmmMemoryModifier; }
   Memory::VirtualModificationDate* getSModificationDate() const { return pmdModificationDate; }
   Memory::VirtualModificationDate& getModificationDate() const { AssumeCondition(pmdModificationDate) return *pmdModificationDate; }

   // bool hasRingTraits() const { return pcrtCastRingTraits != nullptr; }
   // VirtualCastRingTraits& getRingTraits() const { AssumeCondition(pcrtCastRingTraits) return *pcrtCastRingTraits; }

   VirtualElement& getFirstArgument() const;
   VirtualElement& getSecondArgument() const;
   VirtualElement& getResult() const;
   thisType& assignArguments(thisType& source) { inherited::assignArguments(source); return *this; }
   thisType& clearArguments() { inherited::clearArguments(); return *this; }
   thisType& setFirstArgument(const VirtualElement& argument);
   thisType& setSecondArgument(const VirtualElement& argument);
   thisType& setArgument(const VirtualElement& argument);

   // set the errors
   thisType& mergeSureError(const thisType& source) { mergeSureErrorField(source.queryErrorField()); return *this; }
   thisType& setSurePositiveOverflow()  { mergeSureErrorField(EEPositiveOverflow); return *this; }
   thisType& setSureNegativeOverflow()  { mergeSureErrorField(EENegativeOverflow); return *this; }
   thisType& setSurePositiveUnderflow() { mergeSureErrorField(EEPositiveUnderflow); return *this; }
   thisType& setSureNegativeUnderflow() { mergeSureErrorField(EENegativeUnderflow); return *this; }
   thisType& setSureDivisionByZero()    { mergeSureErrorField(EEDivisionByZero); return *this; }
   thisType& setSureNaN()               { mergeSureErrorField(EENaN); return *this; }

   thisType& setPositiveOverflow() { return (thisType&) inherited::setPositiveOverflow(); }
   thisType& setPositiveOverflow(const InformationKind& information)
      {  inherited::setPositiveOverflow();
         if (!information.isMay())
            mergeSureErrorField(EEPositiveOverflow);
         return *this;
      }
   thisType& setNegativeOverflow() {  return (thisType&) inherited::setNegativeOverflow(); }
   thisType& setNegativeOverflow(const InformationKind& information)
      {  inherited::setNegativeOverflow();
         if (!information.isMay())
            mergeSureErrorField(EENegativeOverflow);
         return *this;
      }
   thisType& setPositiveUnderflow() {  return (thisType&) inherited::setPositiveUnderflow(); }
   thisType& setPositiveUnderflow(const InformationKind& information)
      {  inherited::setPositiveUnderflow();
         if (!information.isMay())
            mergeSureErrorField(EEPositiveUnderflow);
         return *this;
      }
   thisType& setNegativeUnderflow() {  return (thisType&) inherited::setNegativeUnderflow(); }
   thisType& setNegativeUnderflow(const InformationKind& information)
      {  inherited::setNegativeUnderflow();
         if (!information.isMay())
            mergeSureErrorField(EENegativeUnderflow);
         return *this;
      }
   thisType& setDivisionByZero() {  return (thisType&) inherited::setDivisionByZero(); }
   thisType& setDivisionByZero(const InformationKind& information)
      {  inherited::setDivisionByZero();
         if (!information.isMay())
            mergeSureErrorField(EEDivisionByZero);
         return *this;
      }
   thisType& setNaN() {  return (thisType&) inherited::setNaN(); }
   thisType& setNaN(const InformationKind& information)
      {  inherited::setNaN();
         if (!information.isMay())
            mergeSureErrorField(EENaN);
         return *this;
      }

   unsigned getSureErrorCode() const  { return queryErrorField(); }
   bool isOnlyPositiveOverflow() const  { return queryErrorField() == EEPositiveOverflow; }
   bool isOnlyNegativeOverflow() const  { return queryErrorField() == EENegativeOverflow; }
   bool isSurePositiveOverflow() const  { return querySureErrorField() & EEPositiveOverflow; }
   bool isSureNegativeOverflow() const  { return querySureErrorField() & EENegativeOverflow; }
   bool isSureOverflow() const          { return querySureErrorField() & (EEPositiveOverflow | EENegativeOverflow); }
   bool isSurePositiveUnderflow() const { return querySureErrorField() & EEPositiveUnderflow; }
   bool isSureNegativeUnderflow() const { return querySureErrorField() & EENegativeUnderflow; }
   bool isSureDivisionByZero() const    { return querySureErrorField() & EEDivisionByZero; }
   bool isSureNaN() const               { return querySureErrorField() & EENaN; }
};

/***********************************************/
/* Definition of the class TransmitEnvironment */
/***********************************************/

class TransmitEnvironment : public EvaluationEnvironment {
  private:
   typedef EvaluationEnvironment inherited;
   typedef TransmitEnvironment thisType;
   EvaluationEnvironment* pseeUpdate;
   bool fStopEmptyness;

  public:
   TransmitEnvironment(EvaluationEnvironment& update)
      :  inherited(update, DuplicateParameters().transmitField().transmitFloatPart()),
         pseeUpdate(&update), fStopEmptyness(false) {}
   TransmitEnvironment(EvaluationEnvironment& update, const Init& params);
   TransmitEnvironment(const thisType& source) = default;
   virtual ~TransmitEnvironment()
      {  if (pseeUpdate) {
            pseeUpdate->mergeField(*this);
            if (isEmpty() && !fStopEmptyness)
              pseeUpdate->setEmpty();
         };
      }
   virtual EvaluationEnvironment& getOriginal() override
      {  AssumeCondition(pseeUpdate) return pseeUpdate->getOriginal(); }

   thisType& merge()
      {  if (pseeUpdate) {
            pseeUpdate->mergeField(*this);
            if (isEmpty() && !fStopEmptyness)
               pseeUpdate->setEmpty();
         };
         pseeUpdate = nullptr;
         return *this;
      }
   DefineCopy(TransmitEnvironment)
};

/*************************************************/
/* Definition of the class ConstraintEnvironment */
/*************************************************/

class ConstraintEnvironment : public EvaluationEnvironment {
  private:
   typedef EvaluationEnvironment inherited;
   typedef ConstraintEnvironment thisType;

  public:
   class Init;
   ConstraintEnvironment(const EvaluationEnvironment::Init& params);
   ConstraintEnvironment(const thisType& source) = default;
   ConstraintEnvironment(const thisType& source, const DuplicateParameters& params) : inherited(source, params) {}
   DefineCopy(ConstraintEnvironment)

   class Argument {
     private:
      enum Type { TResult, TFstArg, TSndArg };
      Type tType;

     public:
      Argument() : tType(TResult) {}
      Argument(const Argument& source) = default;

      bool isResult() const { return tType == TResult; }
      bool isFst() const { return tType == TFstArg; }
      bool isSnd() const { return tType == TSndArg; }
      Argument& setResult() { tType = TResult; return *this; }
      Argument& setFst() { tType = TFstArg; return *this; }
      Argument& setSnd() { tType = TSndArg; return *this; }
      Argument queryInverse() const
         {  Argument result;
            if (tType == TResult)
               result.tType = TFstArg;
            else
               result.tType = TResult;
            return result;
         }
   };
   thisType& absorbFirstArgument(VirtualElement* argument);
   thisType& absorbSecondArgument(VirtualElement* argument);
   thisType& absorbArgument(VirtualElement* argument);
   thisType& absorb(VirtualElement* argument, const Argument& arg);

   bool mergeWithIntersection(EvaluationEnvironment& source, const Argument& argumentConcerned);
   bool mergeWithIntersection(EvaluationEnvironment& source)
      {  return mergeWithIntersection(source, Argument()); }
};

inline bool
ConstraintEnvironment::mergeWithIntersection(EvaluationEnvironment& source,
      const Argument& argumentConcerned) {
   bool result = true;
   if (!source.hasPerformedOperation())
      result = false;
   else {
      inherited::mergeField(source);
      if (source.hasResult()) {
         if (argumentConcerned.isResult())
            presult() = source.presult();
         else if (argumentConcerned.isFst())
            inherited::absorbFirstArgument(source.presult().extractElement());
         else // argumentConcerned.isSnd()
            inherited::absorbSecondArgument(source.presult().extractElement());
      };
   };
   return result;
}

/************************************************/
/* Definition of the class IntersectEnvironment */
/************************************************/

class IntersectEnvironment : public EvaluationEnvironment {
  private:
   typedef EvaluationEnvironment inherited;
   typedef IntersectEnvironment thisType;
   typedef ConstraintEnvironment::Argument Argument;

   ConstraintEnvironment* pceeUpdate;
   Argument acArg;

  public:
   IntersectEnvironment(ConstraintEnvironment& update)
      :  inherited(update, DuplicateParameters().transmitField()), pceeUpdate(&update)
      {  setInformationKind(update.getInformationKind())
            .setErrorReporting(update.getErrorReporting());
      }
   IntersectEnvironment(ConstraintEnvironment& update, Argument arg)
      :  inherited(update, DuplicateParameters().transmitField()),
         pceeUpdate(&update), acArg(arg)
      {  setInformationKind(update.getInformationKind())
            .setErrorReporting(update.getErrorReporting());
      }
   IntersectEnvironment(ConstraintEnvironment& update, const ConstraintEnvironment::Init& params);
   IntersectEnvironment(const thisType& source) = default;
   virtual ~IntersectEnvironment()
      {  if (pceeUpdate) pceeUpdate->mergeWithIntersection(*this, acArg); }

   virtual EvaluationEnvironment& getOriginal() override
      {  AssumeCondition(pceeUpdate) return pceeUpdate->getOriginal(); }
   IntersectEnvironment& merge()
      {  if (pceeUpdate) pceeUpdate->mergeWithIntersection(*this, acArg); pceeUpdate = nullptr; return *this; }
   DefineCopy(IntersectEnvironment)
};

/*****************************************************************************************/
/* Definition of the classes EvaluationEnvironment::Init and ConstraintEnvironment::Init */
/*****************************************************************************************/

class EvaluationEnvironment::Init : public Scalar::EvaluationEnvironment::Init {
  protected:
   typedef EvaluationEnvironment::InformationKind InformationKind;
   typedef EvaluationEnvironment::ErrorReporting ErrorReporting;

  private:
   typedef Scalar::EvaluationEnvironment::Init inherited;
   InformationKind ikKind;
   LatticeCreation lcLatticeCreation;
   unsigned int uMaxDepth;
   // VirtualCastRingTraits* pcrtCastRingTraits;
   // bool fControlCastRing;
   bool fRequireErrorsAsLattice;
   bool fLowLevel;
   // Memory::VirtualMemoryModifier* spmmMemoryModifier;
   Memory::VirtualModificationDate* pmdModificationDate;

  public:
   Init()
      :  uMaxDepth(0)/*, pcrtCastRingTraits(nullptr), fControlCastRing(false) */,
         fRequireErrorsAsLattice(false), fLowLevel(false), /* spmmMemoryModifier(nullptr), */
         pmdModificationDate(nullptr) { /* ikKind.setMay(); */ }
   Init& operator=(const Init& source) = default;

   Init& stopErrorStates() { return (Init&) inherited::stopErrorStates(); }
   Init& propagateErrorStates(ErrorReporting::Reporting reporting) { return (Init&) inherited::propagateErrorStates(reporting); }
   Init& propagateErrorStates(const ErrorReporting& reporting) { return (Init&) inherited::propagateErrorStates(reporting); }
   Init& propagateErrorStates() { return (Init&) inherited::propagateErrorStates(); }
   virtual FloatingEnvironment* createFloatingEnvironment() const
      {  FloatingEnvironment* result = new FloatingEnvironment();
         result->inputs() = ipStaticFloatingParameters;
         return result;
      }
   Init(const EvaluationEnvironment& env)
      :  inherited(env), ikKind(env.getInformationKind()),
         lcLatticeCreation(env.getLatticeCreation()), uMaxDepth(env.maxDepth())/*,
         pcrtCastRingTraits(nullptr), fControlCastRing(false) */,
         fRequireErrorsAsLattice(env.doesRequireErrorsAsLattice()),
         fLowLevel(env.isLowLevel()),
         // spmmMemoryModifier(env.getSMemoryModifier()),
         pmdModificationDate(env.getSModificationDate())
      {  // if (env.hasRingTraits())
         //    pcrtCastRingTraits = &env.getRingTraits();
      }
   Init(const Init& source) = default;

   Init& setInformationKind(const InformationKind& kind) { ikKind = kind; return *this; }
   Init& setMay()   { ikKind.setMay(); return *this; }
   Init& setSure()  { ikKind.setSure(); return *this; }
   Init& setExact() { ikKind.setExact(); return *this; }
   Init& setFormal() { lcLatticeCreation.setFormal(); return *this; }
   Init& setDisjunction() { lcLatticeCreation.setDisjunction(); return *this; }
   Init& setInterval() { lcLatticeCreation.setInterval(); return *this; }
   Init& setShareTop() { lcLatticeCreation.setShareTop(); return *this; }
   Init& setTop() { lcLatticeCreation.setTop(); return *this; }
   Init& setLowLevel() { fLowLevel = true; return *this; }
   Init& setMaxDepth(unsigned int uMaxDepthSource) { uMaxDepth = uMaxDepthSource; return *this; }
   // Init& absorbRingTraits(VirtualCastRingTraits* castRingTraits)
   //   {  pcrtCastRingTraits = castRingTraits; fControlCastRing = true; return *this; }

   Init& setFrom(const EvaluationEnvironment& env)
      {  inherited::setFrom(env);
         ikKind = env.getInformationKind();
         lcLatticeCreation = env.getLatticeCreation();
         fLowLevel = env.isLowLevel();
         return *this;
      }
   Init& setApproximatePartFrom(const EvaluationEnvironment& env)
      {  lcLatticeCreation = env.getLatticeCreation();
         fLowLevel = env.isLowLevel();
         return *this;
      }
   bool isValid() const { return ikKind.isValid(); }
   friend class EvaluationEnvironment;

   const InformationKind& getInformationKind() const { return ikKind; }
   const LatticeCreation& getLatticeCreation() const { return lcLatticeCreation; }
   const unsigned int& maxDepth() const { return uMaxDepth; }
};

class ConstraintEnvironment::Init : public EvaluationEnvironment::Init {
  protected:
   typedef EvaluationEnvironment::Init inherited;
   typedef ConstraintEnvironment::Argument Argument;

  private:
   Argument acArg;

  public:
   Init() {}
   Init(const EvaluationEnvironment& env) : inherited(env) {}
   Init(const EvaluationEnvironment& env, const Argument& arg)
      :  inherited(env), acArg(arg) {}
   Init(const Init& source) = default;

   Init& setInformationKind(const InformationKind& kind)
      {  return (Init&) inherited::setInformationKind(kind); }
   Init& setMay()   { return (Init&) inherited::setMay(); }
   Init& setSure()  { return (Init&) inherited::setSure(); }
   Init& setExact() { return (Init&) inherited::setExact(); }

   Init& propagateErrorStates()   { return (Init&) inherited::propagateErrorStates(); }
   Init& propagateErrorStates(const ErrorReporting& errorReporting) { return (Init&) inherited::propagateErrorStates(errorReporting); }
   Init& stopErrorStates() { return (Init&) inherited::stopErrorStates(); }

   Init& setFrom(const EvaluationEnvironment& env) { return (Init&) inherited::setFrom(env); }
   Init& setArgConcerned(Argument arg) { acArg = arg; return *this; }

   const Argument& getConcernedArgument() const { return acArg; }
};

inline
EvaluationEnvironment::EvaluationEnvironment(const Init& init)
   : inherited(init), /* spmmMemoryModifier(init.spmmMemoryModifier), */ pmdModificationDate(init.pmdModificationDate), uMaxDepth(init.uMaxDepth)//, pcrtCastRingTraits(init.pcrtCastRingTraits)
   {  // if (init.fControlCastRing)
      //    mergeControlRingField(1);
      mergeInformationKind(init.ikKind);
      mergeLatticeCreationField((init.lcLatticeCreation.sureCreation() << 3) | init.lcLatticeCreation.mayCreation());
      mergeRequireErrorsAsLatticeField(init.fRequireErrorsAsLattice);
      mergeLowLevelField(init.fLowLevel);
   }

inline
TransmitEnvironment::TransmitEnvironment(EvaluationEnvironment& update, const Init& init)
   :  inherited(update, DuplicateParameters().transmitField().transmitFloatPart(init.hasFloating())), pseeUpdate(&update),
      fStopEmptyness(update.doesStopErrorStates() && !init.getErrorReporting().doesPropagateError())
   {  setInformationKind(init.getInformationKind());
      setErrorReporting(init.getErrorReporting());
      setLatticeCreationField((init.getLatticeCreation().sureCreation() << 3) | init.getLatticeCreation().mayCreation());
      maxDepth() = init.maxDepth();
   }

inline
ConstraintEnvironment::ConstraintEnvironment(const EvaluationEnvironment::Init& params)
   : inherited(params) { setControlArgs(); }

inline
IntersectEnvironment::IntersectEnvironment(ConstraintEnvironment& update, const ConstraintEnvironment::Init& params)
   :  inherited(update, DuplicateParameters().transmitField()), pceeUpdate(&update),
      acArg(params.getConcernedArgument())
   {  setInformationKind(params.getInformationKind());
      setErrorReporting(params.getErrorReporting());
   }

/*******************************************/
/* Definition of the class BaseRequirement */
/*******************************************/

class BaseRequirement {
  public:
   enum Required
      {  RUndefined, RConstant=1, RExact=3, RConstantDisjunction=33, RExactDisjunction=35, RIntervalWithConstantBounds=5,
         RIntervalWithExactBounds=7, RSupInterval=15, RInfInterval=23, RTop=63
      };
   static void setLatticeCreation(EvaluationEnvironment& env, Required requirement)
      {  if ((requirement | RConstant) == RConstant) {
            env.setFormalLatticeCreation();
            // AssumeUncalled
         }
         else if ((requirement | RExact) == RExact)
            env.setFormalLatticeCreation();
         else if ((requirement | (RSupInterval | RInfInterval)) == (RSupInterval | RInfInterval))
            env.setIntervalLatticeCreation();
         else if ((requirement | (RExact | 32)) == (RExact | 32))
            env.setDisjunctionLatticeCreation();
         else
            env.setTopLatticeCreation();
      }
   static void intersectLatticeCreation(EvaluationEnvironment& env, Required requirement)
      {  if ((requirement | RConstant) == RConstant) {
            env.intersectFormalLatticeCreation();
            // AssumeUncalled
         }
         else if ((requirement | RExact) == RExact)
            env.intersectFormalLatticeCreation();
         else if ((requirement | (RSupInterval | RInfInterval)) == (RSupInterval | RInfInterval))
            env.intersectIntervalLatticeCreation();
         else if ((requirement | (RExact | 32)) == (RExact | 32))
            env.intersectDisjunctionLatticeCreation();
         else
            env.intersectTopLatticeCreation();
      }
};

/******************************************/
/* Definition of the class VirtualElement */
/******************************************/

class FormalImplementation;
class VirtualElement : public Scalar::VirtualElement {
  private:
   typedef Scalar::VirtualElement inherited;

  public:
   static int NBApplyCount;
   static int NBConstraintCount;
   static int NBMergeCount;

   typedef EvaluationEnvironment::Init EvaluationParameters;
   class EvaluationApplyParameters : public EvaluationParameters {
     public:
      EvaluationApplyParameters() { setReferArguments(); }
      EvaluationApplyParameters(const EvaluationEnvironment& env) : EvaluationParameters(env) { setReferArguments(); }
      EvaluationApplyParameters(const EvaluationApplyParameters& source) : EvaluationParameters(source) {}
   };
   typedef ConstraintEnvironment::Init ConstraintParameters;

  public:
   typedef Approximate::PPVirtualElement PPVirtualElement;
   class ApproxKind {
     public:
      enum Type
         {  TUndefined, TConstant, TCompilationConstant, TVariable, TFormalOperation, TGuard,
            TPointer, TUnprecise, TInterval, TConjunction, TDisjunction, TTop, TUnknown
         }; // TUnknown (unidirectional approach)

     private:
      Type tType;

     public:
      ApproxKind() : tType(TUndefined) {}
      ApproxKind(Type type) : tType(type) {}
      ApproxKind(const ApproxKind& source) = default;

      ComparisonResult compareApply(const ApproxKind& source) const
         {  return tcompare(tType, source.tType); }
      ComparisonResult compareConstraint(const ApproxKind& source) const
         {  ComparisonResult result = tcompare(tType, source.tType);
            if ((tType >= TInterval) && (tType <= TDisjunction)
                  && (source.tType >= TInterval) && (source.tType <= TDisjunction)) {
               if (tType == TInterval) {
                  if (source.tType >= TConjunction)
                     result = CRGreater;
               }
               else if (source.tType == TInterval) {
                  if (tType >= TConjunction)
                     result = CRLess;
               };
            };
            return result;
         }

      bool operator<(const ApproxKind& source) const { return tType < source.tType; }
      bool operator<=(const ApproxKind& source) const { return tType <= source.tType; }
      bool operator==(const ApproxKind& source) const { return tType == source.tType; }
      bool operator!=(const ApproxKind& source) const { return tType != source.tType; }
      bool operator>=(const ApproxKind& source) const { return tType >= source.tType; }
      bool operator>(const ApproxKind& source) const { return tType > source.tType; }

      ApproxKind& setConstant()        { tType = TConstant; return *this; }
      ApproxKind& setCompilationConstant() { tType = TCompilationConstant; return *this; }
      ApproxKind& setVariable()        { tType = TVariable; return *this; }
      ApproxKind& setFormalOperation() { tType = TFormalOperation; return *this; }
      ApproxKind& setGuard()           { tType = TGuard; return *this; }
      ApproxKind& setPointer()         { tType = TPointer; return *this; }
      ApproxKind& setUnprecise()       { tType = TUnprecise; return *this; }
      ApproxKind& setInterval()        { tType = TInterval; return *this; }
      ApproxKind& setDisjunction()     { tType = TDisjunction; return *this; }
      ApproxKind& setConjunction()     { tType = TConjunction; return *this; }
      ApproxKind& setTop()             { tType = TTop; return *this; }
      ApproxKind& setUnknown()         { tType = TUnknown; return *this; }

      bool isExact() const             { return tType == TConstant || tType == TVariable; }
      bool isConstant() const          { return tType == TConstant || tType == TCompilationConstant; }
      bool isPureConstant() const      { return tType == TConstant; }
      bool isCompilationConstant() const { return tType == TCompilationConstant; }
      bool isVariable() const          { return tType == TVariable; }
      bool isFormalOperation() const   { return tType == TFormalOperation; }
      bool isFormalConstruction() const { return tType == TFormalOperation || tType == TGuard; }
      bool isGuard() const             { return tType == TGuard; }
      bool isPointer() const           { return tType == TPointer; }
      bool isUnprecise() const         { return tType == TUnprecise; }
      bool isInterval() const          { return tType == TInterval; }
      bool isDisjunction() const       { return tType == TDisjunction; }
      bool isConjunction() const       { return tType == TConjunction; }
      bool isTop() const               { return tType == TTop; }
      bool isUnknown() const           { return tType == TUnknown; }
      bool mayBeAtomic() const
         { return isValid() && tType <= TPointer; }
      bool isAtomic() const
         { return isValid() && (tType <= TPointer) && (tType != TFormalOperation); }

      const Type& type() const { return tType; }
      bool isValid() const { return tType != TUndefined && tType <= TUnknown; }
   };

   typedef EvaluationEnvironment::Verdict EvaluationVerdict;
   typedef EvaluationEnvironment::InformationKind InformationKind;
   typedef EvaluationEnvironment::ErrorReporting ErrorReporting;
   typedef ConstraintEnvironment::Argument Argument;

   static EvaluationParameters EPMayStopErrorStates;
   static EvaluationParameters EPMayPropagateErrorStates;
   static EvaluationParameters EPExactStopErrorStates;
   static EvaluationParameters EPExactPropagateErrorStates;
   static EvaluationParameters EPSureStopErrorStates;
   static EvaluationParameters EPSurePropagateErrorStates;

   class DomainEnvironment : public VirtualQueryOperation::Environment {
     private:
      typedef VirtualQueryOperation::Environment inherited;
      typedef EvaluationEnvironment::InformationKind InformationKind;
      typedef EvaluationEnvironment::Verdict Verdict;
      PNT::AutoPointer<VirtualElement::Init> apiInit;

     protected:
      DefineExtendedParameters(4, inherited)
      DefineSubExtendedParameters(InformationKind, 2, INHERITED)
      DefineSubExtendedParameters(Verdict, 2, InformationKind)

     public:
      DomainEnvironment() {}
      DomainEnvironment(const DomainEnvironment& source) = default;

      DomainEnvironment& setInformationKind(const InformationKind& kind) { AssumeCondition(!hasInformationKindField()) mergeInformationKindField(kind.kind()); return *this; }
      DomainEnvironment& setMay() { AssumeCondition(!hasInformationKindField()) mergeInformationKindField(InformationKind::IKMay); return *this; }
      DomainEnvironment& setSure() { AssumeCondition(!hasInformationKindField()) mergeInformationKindField(InformationKind::IKSure); return *this; }
      DomainEnvironment& setExact() { AssumeCondition(!hasInformationKindField()) mergeInformationKindField(InformationKind::IKExact); return *this; }

      bool isMay() const { return queryInformationKindField() == InformationKind::IKMay; }
      bool isSure() const { return queryInformationKindField() == InformationKind::IKSure; }
      bool isExact() const { return queryInformationKindField() == InformationKind::IKExact; }

      void setPrecise() { mergeVerdictField(Verdict::EVExactApplied); }
      void setDegradate() { mergeVerdictField(Verdict::EVDegradateApplied); }

      void setInit(const VirtualElement::Init& init) { apiInit.setElement(init); }
      VirtualElement::Init& init() { return *apiInit; }
      const VirtualElement::Init& init() const { return *apiInit; }
      bool hasInit() const { return apiInit.isValid(); }

      Verdict verdict() const { return Verdict((Verdict::TypeVerdict) queryVerdictField()); }
   };

   class SimplificationEnvironment : public DomainEnvironment {
     private:
      typedef DomainEnvironment inherited;
      PPAbstractElement ppaeResult;

     protected:
      DefineExtendedParameters(2, inherited)
      DefineSubExtendedParameters(Empty, 1, INHERITED)
      DefineSubExtendedParameters(Fail, 1, Empty)

     public:
      SimplificationEnvironment() {}
      SimplificationEnvironment(const SimplificationEnvironment& source) : inherited(source) {}

      void setEmpty() { mergeEmptyField(1); }
      bool isEmpty() const { return hasEmptyField(); }
      PPVirtualElement& presult();
      bool hasResult() const { return ppaeResult.isValid(); }
      void setFail() { mergeFailField(1); }
      bool hasFailed() const { return hasFailField(); }
   };
   typedef VirtualQueryOperation::ExternNativeOperationEnvironment ExternNativeOperationEnvironment;

  protected:
   DefineExtendedParameters(4, inherited)
   DefineSubExtendedParameters(Approx, 4, INHERITED)

   void mergeType(ApproxKind::Type type) { mergeApproxField(type); }
   void setType(ApproxKind::Type type) { setApproxField(type); }
   void setType(const ApproxKind& kind) { setType(kind.type()); }

  public:
   ApproxKind::Type type() const { return (ApproxKind::Type) queryApproxField(); }
   ApproxKind getType() const { return ApproxKind(type()); }
   virtual FormalImplementation& _asFormal() { AssumeUncalled FormalImplementation* result = nullptr; return *result; }
   const FormalImplementation& asFormal() const { return const_cast<VirtualElement&>(*this)._asFormal(); }
   FormalImplementation& asFormal() { return _asFormal(); }

   class InitFloat : public Init {
     private:
      typedef Init inherited;
      int uSizeMantissa, uSizeExponent;

     public:
      InitFloat() : uSizeMantissa(0), uSizeExponent(0) {}
      explicit InitFloat(const Init& source) : inherited(source), uSizeMantissa(0), uSizeExponent(0) {}
      InitFloat(const InitFloat& source) = default;
      InitFloat& operator=(const InitFloat& source) = default;
      DefineCopy(InitFloat)
      DDefineAssign(InitFloat)

      InitFloat& setSizeMantissa(int sizeMantissa) { uSizeMantissa = sizeMantissa; return *this; }
      InitFloat& setSizeExponent(int sizeExponent) { uSizeExponent = sizeExponent; return *this; }
      const int& sizeMantissa() const { return uSizeMantissa; }
      const int& sizeExponent() const { return uSizeExponent; }
   };

  protected:
   VirtualElement(const Init& init) : inherited(init) {}

   void setConstant() { AssumeCondition(!hasApproxField()) mergeApproxField(ApproxKind::TConstant); }
   void setCompilationConstant()
      {  AssumeCondition(!hasApproxField() || queryApproxField() == ApproxKind::TConstant) setApproxField(ApproxKind::TCompilationConstant); }
   void setVariable() { AssumeCondition(!hasApproxField()) mergeApproxField(ApproxKind::TVariable); }
   void setFormalOperation() { AssumeCondition(!hasApproxField()) mergeApproxField(ApproxKind::TFormalOperation); }
   void setGuard() { AssumeCondition(!hasApproxField()) mergeApproxField(ApproxKind::TGuard); }
   void setPointer() { AssumeCondition(!hasApproxField()) mergeApproxField(ApproxKind::TPointer); }
   void setUnprecise() { AssumeCondition(!hasApproxField()) mergeApproxField(ApproxKind::TUnprecise); }
   void setInterval() { AssumeCondition(!hasApproxField()) mergeApproxField(ApproxKind::TInterval); }
   void setDisjunction() { AssumeCondition(!hasApproxField()) mergeApproxField(ApproxKind::TDisjunction); }
   void setConjunction() { AssumeCondition(!hasApproxField()) mergeApproxField(ApproxKind::TConjunction); }
   void setTop() { AssumeCondition(!hasApproxField()) mergeApproxField(ApproxKind::TTop); }
   void setUnknown() { AssumeCondition(!hasApproxField()) mergeApproxField(ApproxKind::TUnknown); }

   void mergeApproxKind(const ApproxKind& kind) { mergeApproxField(kind.type()); }
   void setApproxKind(const ApproxKind& kind) { setApproxField(kind.type()); }

  public:
   bool isPointer() const { return queryApproxField() == ApproxKind::TPointer; }
   bool isTop() const { return queryApproxField() == ApproxKind::TTop; }

  public:
   VirtualElement(const VirtualElement& source) = default;
   DefineCopy(VirtualElement)

   // Méthodes query
   enum TypeOperation { TOUndefined, TOBit, TOInteger, TOMultiBit, TOFloating, TORational, TOReal };

   ApproxKind getApproxKind() const { return ApproxKind(type()); }
   virtual bool isAtomic() const override { return getApproxKind().isAtomic(); }
   virtual bool isApproximate() const override { return true; }
   bool isClosed() const { return (queryParams() >> END_OF_MASK) == 0; }
   static bool isStandard(const VirtualElement& source) { return source.isClosed(); }

   static bool queryBitNativeOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment&);
   static bool queryIntegerNativeOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment&);
   static bool queryMultiBitNativeOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment&);
   static bool queryFloatingNativeOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment&);
   static bool queryRationalNativeOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment&);
   static bool queryRealNativeOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment&);
   static bool queryCopy(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment&);
   static bool querySpecialize(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment&);

   // Définition des méthodes virtuelles
   virtual int countAtomic() const { return 0; }
   virtual bool query(const VirtualQueryOperation&, VirtualQueryOperation::Environment& env) const { return false; }
   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override;
   bool applyArg(const VirtualOperation& operation, const VirtualElement& source, EvaluationEnvironment& env);
   virtual bool applyTo(const VirtualOperation&, VirtualElement&, EvaluationEnvironment& env) const { return false; }
   virtual bool mergeWith(const Scalar::VirtualElement& asource, Scalar::EvaluationEnvironment& aenv) override
      {  ++NBMergeCount;
         EvaluationEnvironment& env = (EvaluationEnvironment&) aenv;
         const VirtualElement& source = (const VirtualElement&) asource;
         // AssumeCondition(getSizeInBits() == source.getSizeInBits())
         bool result = true;
         AssumeCondition(getSizeInBits() == source.getSizeInBits())
         result = (source.getType().compareApply(getType()) == CRGreater) ? source.mergeWithTo(*this, env) : false;
         return result;
      }
   virtual bool contain(const Scalar::VirtualElement& asource, Scalar::EvaluationEnvironment& aenv) const override
      {  EvaluationEnvironment& env = (EvaluationEnvironment&) aenv;
         const VirtualElement& source = (const VirtualElement&) asource;
         AssumeCondition(getSizeInBits() == source.getSizeInBits())
         return (source.getType().compareApply(getType()) == CRGreater) ? source.containTo(*this, env) : false;
      }
   virtual bool mergeWithTo(VirtualElement&, EvaluationEnvironment&) const { return false; }
   virtual bool containTo(const VirtualElement&, EvaluationEnvironment&) const { return false; }
   virtual bool intersectWith(const VirtualElement& source, EvaluationEnvironment& env)
      {  AssumeCondition(getSizeInBits() == source.getSizeInBits())
         return (source.getType().isUnknown() || (source.getType().compareConstraint(getType()) == CRLess))
            ? source.intersectWithTo(*this, env) : false;
      }
   virtual bool intersectWithTo(VirtualElement&, EvaluationEnvironment&) const { return false; }
   virtual ZeroResult queryZeroResult() const override { AssumeUncalled return ZeroResult(); }
   virtual bool constraintTo(const VirtualOperation&, VirtualElement& source, const VirtualElement& result,
         ConstraintEnvironment&) { return false; }
   virtual bool constraint(const VirtualOperation&, const VirtualElement& result, ConstraintEnvironment&);
   virtual bool guard(PPVirtualElement thenSource, PPVirtualElement elseSource, EvaluationEnvironment& env);

   static bool guardConstant(bool condition, PPVirtualElement thenSource, PPVirtualElement elseSource, EvaluationEnvironment& env);
   bool guardApproximate(PPVirtualElement thenSource, PPVirtualElement elseSource, EvaluationEnvironment& env);
   bool guardFormal(PPVirtualElement thenSource, PPVirtualElement elseSource, EvaluationEnvironment& env);

   class Methods;

   typedef bool (*PointerQueryFunction)(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment&);

  protected:
   class FunctionQueryDecisionNode : public PNT::DecisionNode<PointerQueryFunction> {
     private:
      typedef PNT::DecisionNode<PointerQueryFunction> inherited;

     public:
      FunctionQueryDecisionNode() {}
      FunctionQueryDecisionNode(PointerQueryFunction function)
         :  PNT::DecisionNode<PointerQueryFunction>(function) {}

      void setMethod(PointerQueryFunction method) { inherited::setMethod(method); }
      void resetMethod(PointerQueryFunction method) { inherited::resetMethod(method); }

      bool execute(const VirtualElement& element, const VirtualQueryOperation& operation,
               VirtualQueryOperation::Environment& env) const
         {  return inherited::hasMethod() ? (*inherited::getMethod())(element, operation, env) : false; }
   };

  public:
   class FunctionQueryTable : public PNT::StaticMethodTable<FunctionQueryDecisionNode> {
     private:
      typedef FunctionQueryTable thisType;
      typedef PNT::StaticMethodTable<FunctionQueryDecisionNode> inherited;

     public:
      FunctionQueryTable() {}
      FunctionQueryTable(const FunctionQueryTable& source) = default;
      void clear() { inherited::clear(); }

      FunctionQueryTable& getFunctionTable() { return *this; }
      bool execute(const VirtualElement& element, const VirtualQueryOperation& operation,
            VirtualQueryOperation::Environment& env) const
         {  return elementAt(operation.getExtensions()).execute(element, operation, env); }
   };

   class QueryTable : public PNT::ConstMethodTable<FunctionQueryTable, VirtualQueryOperation::EndOfType> {
     private:
      typedef PNT::ConstMethodTable<FunctionQueryTable, VirtualQueryOperation::EndOfType> inherited;

     public:
      QueryTable() {}
      QueryTable(const QueryTable& source) = default;

      const FunctionQueryTable& getFunctionTable(int index) const { return inherited::elementAt(index); }
      bool execute(const VirtualElement& element, const VirtualQueryOperation& operation,
            VirtualQueryOperation::Environment& env) const
         {  return elementAt(operation.getType()).execute(element, operation, env); }
   };

  public:
   static bool queryIdentity(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment&);
   virtual const FunctionQueryTable& functionQueryTable(int queryIndex) const
      {  AssumeUncalled const FunctionQueryTable* result = nullptr; return *result; }
};

/* Inline methods of the class EvaluationEnvironment */

inline VirtualElement&
EvaluationEnvironment::getFirstArgument() const
   {  return (VirtualElement&) inherited::getFirstArgument(); }

inline VirtualElement&
EvaluationEnvironment::getSecondArgument() const
   {  return (VirtualElement&) inherited::getSecondArgument(); }

inline VirtualElement&
EvaluationEnvironment::getResult() const
   {  return (VirtualElement&) inherited::getResult(); }

inline EvaluationEnvironment&
EvaluationEnvironment::setFirstArgument(const VirtualElement& argument)
   {  return (EvaluationEnvironment&) inherited::setFirstArgument(argument); }
inline EvaluationEnvironment&
EvaluationEnvironment::setSecondArgument(const VirtualElement& argument)
   {  return (EvaluationEnvironment&) inherited::setSecondArgument(argument); }
inline EvaluationEnvironment&
EvaluationEnvironment::setArgument(const VirtualElement& argument)
   {  return (EvaluationEnvironment&) inherited::setArgument(argument); }

inline ConstraintEnvironment&
ConstraintEnvironment::absorbFirstArgument(VirtualElement* argument)
   {  return (ConstraintEnvironment&) inherited::absorbFirstArgument(argument); }
inline ConstraintEnvironment&
ConstraintEnvironment::absorbSecondArgument(VirtualElement* argument)
   {  return (ConstraintEnvironment&) inherited::absorbSecondArgument(argument); }
inline ConstraintEnvironment&
ConstraintEnvironment::absorbArgument(VirtualElement* argument)
   {  return (ConstraintEnvironment&) inherited::absorbArgument(argument); }

inline ConstraintEnvironment&
ConstraintEnvironment::absorb(VirtualElement* argument, const Argument& arg) {
   if (arg.isResult()) {
      presult().absorbElement(argument);
      return *this;
   }
   else if (arg.isFst())
      inherited::absorbFirstArgument(argument);
   else if (arg.isSnd())
      inherited::absorbSecondArgument(argument);
   else
      { AssumeUncalled };
   return *this;
}

/* Inline methods of the class VirtualElement */

inline bool
VirtualElement::applyArg(const VirtualOperation& operation, const VirtualElement& source, EvaluationEnvironment& env)
   {  env.setFirstArgument(source);
      return apply(operation, env);
   }

inline bool
VirtualElement::apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& aenv) {
   ++NBApplyCount;
   bool result = AbstractElement::apply(operation, aenv);
   if (!result) {
      EvaluationEnvironment& env = (EvaluationEnvironment&) aenv;
      int args = operation.getArgumentsNumber();
      if ((args > 0) && (env.getFirstArgument().getApproxKind().compareApply(getApproxKind()) == CRGreater)) {
         result = env.getFirstArgument().applyTo(operation, *this, env);
         AssumeCondition(result)
      };
   };
   return result;
}

class PPVirtualElement : public PNT::TPassPointer<Approximate::VirtualElement, PPAbstractElement> {
  private:
   typedef PNT::TPassPointer<Approximate::VirtualElement, PPAbstractElement> inherited;

  public:
   PPVirtualElement() {}
   PPVirtualElement(VirtualElement* element, PNT::Pointer::Init init) : inherited(element, init) {}
   PPVirtualElement(const VirtualElement& element) : inherited(element) {}
   PPVirtualElement(const PPAbstractElement& source) : inherited(source) {}
   PPVirtualElement(const PPAbstractElement& source, PNT::Pointer::Duplicate duplicate)
      : inherited(source, duplicate) {}
};

inline bool
VirtualElement::guard(PPVirtualElement thenSource, PPVirtualElement elseSource, EvaluationEnvironment& env)
   {  return false; }

class VirtualQueryOperation::DuplicationEnvironment : public Environment {
  private:
   typedef Environment inherited;
   Memory::VirtualMemoryModifier* pmmMemoryModifier;
   PPVirtualElement ppveResult;
   PNT::SharedPointer* pspOriginCaller;

  public:
   DuplicationEnvironment(Memory::VirtualMemoryModifier* memoryModifier, PNT::SharedPointer* originCaller=nullptr)
      :  pmmMemoryModifier(memoryModifier), pspOriginCaller(originCaller) {}
   DuplicationEnvironment(const DuplicationEnvironment& source)
      :  inherited(source), pmmMemoryModifier(source.pmmMemoryModifier),
         pspOriginCaller(source.pspOriginCaller) {}

   void absorbResult(VirtualElement* result) { ppveResult.absorbElement(result); }
   PPVirtualElement& presult() { return ppveResult; }
   bool hasMemoryModifier() const { return pmmMemoryModifier; }
   bool hasOriginCaller() const { return pspOriginCaller; }
   void setOriginCaller(PNT::SharedPointer& originCaller) { pspOriginCaller = &originCaller; }
   PNT::SharedPointer& originCaller() const { AssumeCondition(pspOriginCaller) return *pspOriginCaller; }
   Memory::VirtualMemoryModifier& getMemoryModifier() const { AssumeCondition(pmmMemoryModifier) return *pmmMemoryModifier; }
};

namespace DEnvironment {

inline
ElementResult::ElementResult(int index, PPVirtualElement result)
   :  uIndex(index), ppaeResult(result) {}

inline VirtualElement&
ElementResult::result() const { return (VirtualElement&) *ppaeResult; }

inline PPVirtualElement
ElementResult::extractResult() const { return PPVirtualElement(ppaeResult); }

inline PPVirtualElement
FlagsValues::extractFlag(int index) {
   PPVirtualElement result;
   Cursor cursor(*this);
   if (locateKey(index, cursor, RPExact)) {
      result = cursor.elementSAt().extractResult();
      AssumeCondition(result.isValid())
   };
   return result;
}

inline const VirtualElement*
FlagsValues::getFlag(int index) const {
   const VirtualElement* result = nullptr;
   Cursor cursor(*this);
   if (locateKey(index, cursor, RPExact))
      result = &cursor.elementAt().result();
   return result;
}

inline void
FlagsValues::absorbFlag(int index, PPVirtualElement flag) {
   Cursor cursor(*this);
   LocationResult result = locateKey(index, cursor);
   AssumeCondition(!result)
   insertNewAt(cursor, new ElementResult(index, flag), result.getInsertionPosition());
}

} // end of namespace DEnvironment

inline PPVirtualElement&
VirtualElement::SimplificationEnvironment::presult() { return (PPVirtualElement&) ppaeResult; }

/*****************************************************/
/* Definition of the arguments for the query methods */
/*****************************************************/

class VirtualQueryOperation::OperationEnvironment : public Environment {
  private:
   const VirtualOperation* pvoReference;
   VirtualOperation voResult;
   PNT::AutoPointer<VirtualOperation> apvoResult;

  public:
   OperationEnvironment() : pvoReference(nullptr) {}
   OperationEnvironment(const OperationEnvironment& source) = default;

   void setReference(const VirtualOperation& reference) { pvoReference = &reference; }
   void setReferenceResult(const VirtualOperation& reference)
      {  pvoReference = &reference;
         apvoResult.setElement(reference);
         apvoResult->clear();
      }
   const VirtualOperation& result() const { return apvoResult.isValid() ? *apvoResult : voResult; }
   VirtualOperation& result() { return apvoResult.isValid() ? *apvoResult : voResult; }
   bool hasSimpleResult() const { return !apvoResult.isValid(); }
   void absorbResult(VirtualOperation* result) { apvoResult.absorbElement(result); }
   VirtualOperation* extractResult() { return apvoResult.extractElement(); }

   bool hasReference() const { return pvoReference; }

#define DefineTypedResult(OperationFamily)                                                      \
   OperationFamily::Operation& referenceAs##OperationFamily()                                   \
      {  return *(Scalar::OperationFamily::Operation*) pvoReference; }                          \
                                                                                                \
   OperationFamily::Operation& resultAs##OperationFamily()                                      \
      {  Scalar::OperationFamily::Operation* result = (Scalar::OperationFamily::Operation*) &voResult;\
         if (apvoResult.isValid()) {                                                            \
            AssumeCondition(dynamic_cast<const Scalar::OperationFamily::Operation*>(apvoResult.key()))\
            result = (Scalar::OperationFamily::Operation*) apvoResult.key();                    \
         };                                                                                     \
         return *result;                                                                        \
      }

   DefineTypedResult(Bit)
   DefineTypedResult(Integer)
   DefineTypedResult(MultiBit)
   DefineTypedResult(Floating)
   DefineTypedResult(Rational)
   DefineTypedResult(Real)
#undef DefineTypedResult
};

class VirtualQueryOperation::CompareSpecialEnvironment : public Environment {
  private:
   typedef Environment inherited;

  public:
   CompareSpecialEnvironment() {}
   CompareSpecialEnvironment(const CompareSpecialEnvironment& source) = default;
   virtual ~CompareSpecialEnvironment() {}
   virtual void mergeWith(const CompareSpecialEnvironment& source) {}
   virtual void intersectWith(const CompareSpecialEnvironment& source) {}
   virtual void fill() {}
   virtual bool isEmpty() const { return true; }
};

class VirtualQueryOperation::SimplificationEnvironment : public VirtualElement::SimplificationEnvironment {
  private:
   typedef VirtualElement::SimplificationEnvironment inherited;

  public:
   SimplificationEnvironment() {}
   SimplificationEnvironment(const SimplificationEnvironment& source) = default;
};

class VirtualQueryOperation::DomainEnvironment : public VirtualElement::DomainEnvironment {
  private:
   typedef VirtualElement::DomainEnvironment inherited;
   PPVirtualElement ppveResult;

  public:
   DomainEnvironment() {}
   DomainEnvironment(const DomainEnvironment& source) = default;

   PPVirtualElement& presult() { return ppveResult; }
};

/*********************************************************/
/* Definition of the class ConstraintTransmitEnvironment */
/*********************************************************/

class ConstraintTransmitEnvironment : public ConstraintEnvironment {
  private:
   typedef ConstraintEnvironment inherited;
   ConstraintEnvironment* pceeUpdate;
   Argument acArg;
   bool fRetransmission;

  public:
   ConstraintTransmitEnvironment(inherited& update)
      :  inherited(update, DuplicateParameters().transmitField()), pceeUpdate(&update), fRetransmission(true)
      {  AssumeCondition(update.doesControlArgs() && !doesControlArgs())
         clearControlAndArgs(update);
         setControlArgs();
      }
   ConstraintTransmitEnvironment(inherited& update, PPVirtualElement argument)
      :  inherited(update, DuplicateParameters().transmitField()), pceeUpdate(&update), fRetransmission(true)
      {  AssumeCondition(update.doesControlArgs() && !doesControlArgs())
         clearControlAndArgs(update);
         setControlArgs();
         absorbFirstArgument(argument.extractElement());
      }
   ConstraintTransmitEnvironment(inherited& update, PPVirtualElement argument, const Argument& arg)
      :  inherited(update, DuplicateParameters().transmitField()),
         pceeUpdate(&update), acArg(arg), fRetransmission(true)
      {  AssumeCondition(update.doesControlArgs() && !doesControlArgs())
         clearControlAndArgs(update);
         setControlArgs();
         absorbFirstArgument(argument.extractElement());
      }
   ConstraintTransmitEnvironment(inherited& update, const EvaluationEnvironment::Init& params)
      :  inherited(params), pceeUpdate(&update), fRetransmission(false) {}
   ConstraintTransmitEnvironment(inherited& update, PPVirtualElement argument, const Init& params)
      :  inherited(params), pceeUpdate(&update), acArg(params.getConcernedArgument()), fRetransmission(false)
      {  absorbFirstArgument(argument.extractElement()); }
   ConstraintTransmitEnvironment(const ConstraintTransmitEnvironment& source)
      :  inherited(source), pceeUpdate(source.pceeUpdate),
         acArg(source.acArg), fRetransmission(false) {}
   virtual ~ConstraintTransmitEnvironment();
   virtual EvaluationEnvironment& getOriginal() override
      {  AssumeCondition(pceeUpdate) return pceeUpdate->getOriginal(); }

   ConstraintTransmitEnvironment& merge();
   DefineCopy(ConstraintTransmitEnvironment)
};

inline ConstraintTransmitEnvironment&
ConstraintTransmitEnvironment::merge() {
   pceeUpdate->mergeVerdict(getVerdict());
   pceeUpdate->mergeUnstable(*this);
   if (fRetransmission)
      transmitControlAndArgs(*this, *pceeUpdate);
   if (hasResult()) {
      if (acArg.isResult())
         pceeUpdate->presult() = presult();
      else if (acArg.isFst())
         pceeUpdate->absorbFirstArgument((VirtualElement*) presult().extractElement());
      else
         pceeUpdate->absorbSecondArgument((VirtualElement*) presult().extractElement());
   };
   pceeUpdate = nullptr;
   return *this;
}

inline
ConstraintTransmitEnvironment::~ConstraintTransmitEnvironment()
   { if (pceeUpdate) merge(); }

/**********************************************/
/* Definition of the class VirtualDisjunction */
/**********************************************/

class VirtualDisjunction : public VirtualElement {
  private:
   typedef VirtualElement inherited;

  protected:
   virtual void _add(PPVirtualElement element, const InformationKind& kind) { AssumeUncalled }

  public:
   VirtualDisjunction(const Init& init) : inherited(init) {}
   VirtualDisjunction(const VirtualDisjunction& source) = default;
   DefineCopy(VirtualDisjunction)

   // construction methods
   VirtualDisjunction& add(VirtualElement* element, const InformationKind& kind)
      {  _add(PPVirtualElement(element, PNT::Pointer::Init()), kind); return *this; }
   VirtualDisjunction& addExact(VirtualElement* element)
      {  _add(PPVirtualElement(element, PNT::Pointer::Init()), InformationKind().setExact()); return *this; }
   VirtualDisjunction& addSure(VirtualElement* element)
      {  _add(PPVirtualElement(element, PNT::Pointer::Init()), InformationKind().setSure()); return *this; }
   VirtualDisjunction& addMay(VirtualElement* element)
      {  _add(PPVirtualElement(element, PNT::Pointer::Init()), InformationKind().setMay()); return *this; }
   VirtualDisjunction& add(PPVirtualElement element, const InformationKind& kind)
      {  _add(element, kind); return *this; }

   VirtualDisjunction& addExact(PPVirtualElement element)
      {  _add(element, InformationKind().setExact()); return *this; }
   VirtualDisjunction& addSure(PPVirtualElement element)
      {  _add(element, InformationKind().setSure()); return *this; }
   VirtualDisjunction& addMay(PPVirtualElement element)
      {  _add(element, InformationKind().setMay()); return *this; }
};

/***************************************************/
/* Definition of the class VirtualElement::Methods */
/***************************************************/

class VirtualElement::Methods {
  public:
   typedef ZeroResult BooleanResult;
   typedef EvaluationEnvironment::Init EvaluationParameters;
   typedef VirtualElement::EvaluationApplyParameters EvaluationApplyParameters;

   static PPVirtualElement apply(const VirtualElement& source, const VirtualOperation& operation,
         EvaluationEnvironment& env);
   static PPVirtualElement apply(const VirtualElement& source, const VirtualOperation& operation,
         EvaluationEnvironment& env, const EvaluationParameters& params);
   static PPVirtualElement apply(const VirtualElement& source, const VirtualOperation& operation,
         const EvaluationParameters& params);
   static PPVirtualElement apply(const VirtualElement& source, const VirtualOperation& operation,
         const VirtualElement& arg, EvaluationEnvironment& env);
   static PPVirtualElement apply(const VirtualElement& source, const VirtualOperation& operation,
         const VirtualElement& arg, EvaluationEnvironment& env, const EvaluationParameters& params);
   static PPVirtualElement apply(const VirtualElement& source, const VirtualOperation& operation,
         const VirtualElement& arg, const EvaluationParameters& params);

   static BooleanResult applyBoolean(const VirtualElement& source, const VirtualOperation& operation,
         const VirtualElement& arg);
   static BooleanResult convertBoolean(const VirtualElement& source)
      {  return source.queryZeroResult(); }

   static PPVirtualElement applyAssign(PPVirtualElement source, const VirtualOperation& operation,
         const EvaluationParameters& params);
   static PPVirtualElement applyAssign(PPVirtualElement source, const VirtualOperation& operation,
         EvaluationEnvironment& env);
   static PPVirtualElement applyAssign(PPVirtualElement source, const VirtualOperation& operation,
         EvaluationEnvironment& env, const EvaluationParameters& params);
   static PPVirtualElement applyAssign(PPVirtualElement source, const VirtualOperation& operation,
         const VirtualElement& arg, const EvaluationParameters& params);
   static PPVirtualElement applyAssign(PPVirtualElement source, const VirtualOperation& operation,
         const VirtualElement& arg, EvaluationEnvironment& env);
   static PPVirtualElement applyAssign(PPVirtualElement source, const VirtualOperation& operation,
         const VirtualElement& arg, EvaluationEnvironment& env, const EvaluationParameters& params);

   static PPVirtualElement constraint(PPVirtualElement source, const VirtualOperation& operation,
         const VirtualElement& result, const EvaluationParameters& params);
   static PPVirtualElement constraint(PPVirtualElement source, const VirtualOperation& operation,
         PPVirtualElement& arg, const VirtualElement& result, const EvaluationParameters& params);
   static PPVirtualElement constraint(PPVirtualElement source, const VirtualOperation& operation,
         const VirtualElement& arg, const VirtualElement& result, const EvaluationParameters& params);

   static PPVirtualElement merge(PPVirtualElement source, const VirtualElement& arg,
         const EvaluationParameters& params);
   static PPVirtualElement merge(PPVirtualElement source, PPVirtualElement arg,
         const EvaluationParameters& params);
   static PPVirtualElement merge(PPVirtualElement source, PPVirtualElement arg,
         EvaluationEnvironment& env);
   static PPVirtualElement merge(PPVirtualElement source, const VirtualElement& arg,
         EvaluationEnvironment& env);
   static PPVirtualElement intersect(PPVirtualElement source, const VirtualElement& arg,
         const EvaluationParameters& params, bool* isStable=nullptr);
   static PPVirtualElement intersect(PPVirtualElement source, const VirtualElement& arg,
         EvaluationEnvironment& env);

   static bool contain(const VirtualElement& source, const VirtualElement& arg, const EvaluationParameters& params);

   static PPVirtualElement simplify(PPVirtualElement source);
   static PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> newDisjunction(const VirtualElement& source);
   static PPVirtualElement newMultiBitConstant(const VirtualElement& source, const VirtualElement::Init& init);
   static VirtualOperation queryInverseCastOperation(const VirtualElement& source, const VirtualOperation& reference);
   static PPVirtualElement newGuard(const VirtualElement& source);
   static PPVirtualElement queryCopy(const VirtualElement& source, Memory::VirtualMemoryModifier& memoryModifier);
   static void setSpecialize(VirtualElement& source, Memory::VirtualMemoryModifier& memoryModifier);

   static ExternNativeOperationEnvironment::TypeOperation queryTypeOperation(const VirtualElement& source);
};

/***********************************/
/* Definition of specific requests */
/***********************************/

class VirtualQueryOperation::DisjunctionEnvironment : public VirtualQueryOperation::DomainEnvironment {
  private:
   typedef VirtualQueryOperation::DomainEnvironment inherited;

  public:
   DisjunctionEnvironment() {}
   DisjunctionEnvironment(const DisjunctionEnvironment& source) = default;

   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> newResult()
      {  return PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>(inherited::presult()); }
};

class VirtualQueryOperation::GuardEnvironment : public VirtualQueryOperation::CompareSpecialEnvironment {
  private:
   typedef VirtualQueryOperation::CompareSpecialEnvironment inherited;
   PPVirtualElement ppveResult;

  public:
   GuardEnvironment() {}
   GuardEnvironment(const GuardEnvironment& source) : inherited(source) {}

   PPVirtualElement& presult() { return ppveResult; }
};

class VirtualQueryOperation::ExtendedOperationEnvironment : public VirtualQueryOperation::OperationEnvironment {
  private:
   typedef VirtualQueryOperation::OperationEnvironment inherited;
   VirtualOperation voReferenceCastOperation;

  public:
   ExtendedOperationEnvironment(const VirtualOperation& reference) : voReferenceCastOperation(reference) {}
   ExtendedOperationEnvironment(const ExtendedOperationEnvironment& source) = default;
   ExtendedOperationEnvironment& operator=(const ExtendedOperationEnvironment& source) = default;

   const VirtualOperation& reference() const { return voReferenceCastOperation; }
};

inline PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
VirtualElement::Methods::newDisjunction(const VirtualElement& source) {
   VirtualQueryOperation::DisjunctionEnvironment env;
   source.query(VirtualQueryOperation().setNewDisjunction(), env);
   return env.newResult();
}

inline PPVirtualElement
VirtualElement::Methods::newMultiBitConstant(const VirtualElement& source, const VirtualElement::Init& init) {
   VirtualQueryOperation::DomainEnvironment env;
   env.setInit(init);
   source.query(VirtualQueryOperation().setNewMultiBitConstant(), env);
   return env.presult();
}

inline VirtualOperation
VirtualElement::Methods::queryInverseCastOperation(const VirtualElement& source, const VirtualOperation& reference) {
   VirtualQueryOperation::InverseCastOperationEnvironment env(reference);
   source.query(VirtualQueryOperation().setInverseCast(), env);
   AssumeCondition(env.hasSimpleResult())
   return env.result();
}

inline PPVirtualElement
VirtualElement::Methods::queryCopy(const VirtualElement& source, Memory::VirtualMemoryModifier& memoryModifier) {
   VirtualQueryOperation::DuplicationEnvironment env(&memoryModifier);
   source.query(VirtualQueryOperation().setCopy(), env);
   AssumeCondition(env.presult().isValid())
   return env.presult();
}

inline void
VirtualElement::Methods::setSpecialize(VirtualElement& source, Memory::VirtualMemoryModifier& memoryModifier) {
   VirtualQueryOperation::DuplicationEnvironment env(&memoryModifier);
   source.query(VirtualQueryOperation().setSpecialize(), env);
}

inline PPVirtualElement
VirtualElement::Methods::newGuard(const VirtualElement& source) {
   VirtualQueryOperation::GuardEnvironment env;
   source.query(VirtualQueryOperation().setGuard(), env);
   return env.presult();
}

class Top;

namespace Details {

class VirtualIntegerInterval;

/***********************************************/
/* Definition of the class IntOperationElement */
/***********************************************/

template <class TypeDerived>
class TIntAndRealElement : public VirtualElement {
  private:
   typedef VirtualElement inherited;
   typedef TIntAndRealElement<TypeDerived> thisType;

  protected:
   TIntAndRealElement(const Init& init) : inherited(init) {}
   TIntAndRealElement(const thisType& source) : inherited(source) {}

  public:
   static bool constraintMinAssign(VirtualElement&, const VirtualOperation&, VirtualElement& veSource, const VirtualElement& veResult, ConstraintEnvironment&);
   static bool constraintMaxAssign(VirtualElement&, const VirtualOperation&, VirtualElement& veSource, const VirtualElement& veResult, ConstraintEnvironment&);
};

class IntOperationElement : public TIntAndRealElement<IntOperationElement> {
  private:
   typedef TIntAndRealElement<IntOperationElement> inherited;
   typedef IntOperationElement thisType;

  protected:
   IntOperationElement(const Init& init) : inherited(init) {}

  public:
   IntOperationElement(const IntOperationElement& source) = default;

   virtual bool isInt() const { return true; }
   virtual bool isMultiBit() const { return true; }

   class BitDomainEnvironment : public VirtualQueryOperation::CompareSpecialEnvironment {
     private:
      typedef VirtualQueryOperation::CompareSpecialEnvironment inherited;
      typedef Scalar::MultiBit::Implementation::MultiBitElement MultiBitElement;
      const IntOperationElement& ioeReference;
      MultiBitElement mbeFixed;
      int uLastBitPos;

     public:
      BitDomainEnvironment(const IntOperationElement& reference)
         :  ioeReference(reference), mbeFixed(reference.getSizeInBits()), uLastBitPos(0) {}
      BitDomainEnvironment(const BitDomainEnvironment& source) = default;
      BitDomainEnvironment& operator=(const BitDomainEnvironment& source)
         {  inherited::operator=(source);
            mbeFixed = source.mbeFixed;
            uLastBitPos = source.uLastBitPos;
            return *this;
         }

      PPVirtualElement maxZero() const;
      PPVirtualElement maxOne() const;
      virtual void mergeWith(const VirtualQueryOperation::CompareSpecialEnvironment& source);
      virtual void intersectWith(const VirtualQueryOperation::CompareSpecialEnvironment& source);

      MultiBitElement& fixedPrefix() { return mbeFixed; }
      const MultiBitElement& fixedPrefix() const { return mbeFixed; }
      int& lastPos() { return uLastBitPos; }
      const int& lastPos() const { return uLastBitPos; }
      virtual void fill()
         {  uLastBitPos = mbeFixed.getSize();
            (mbeFixed = 0U).neg();
         }
      bool isValid() const { return uLastBitPos >= 0; }
      virtual bool isEmpty() const { return uLastBitPos < 0; }
   };

   class BoundDomainEnvironment : public VirtualQueryOperation::CompareSpecialEnvironment {
     private:
      typedef VirtualQueryOperation::CompareSpecialEnvironment inherited;
      const IntOperationElement& ioeReference;
      PPVirtualElement ppveLower, ppveUpper;

     protected:
      DefineExtendedParameters(2, inherited)
      enum Require { RNone, RLower, RUpper, RBoth };

     public:
      BoundDomainEnvironment(const IntOperationElement& reference) : ioeReference(reference) {}
      BoundDomainEnvironment(const BoundDomainEnvironment& source) = default;
      BoundDomainEnvironment& operator=(const BoundDomainEnvironment& source)
         {  inherited::operator=(source);
            ppveLower = source.ppveLower;
            ppveUpper = source.ppveUpper;
            return *this;
         }

      BoundDomainEnvironment& setRequireLower() { mergeOwnField(RLower); return *this; }
      BoundDomainEnvironment& setRequireUpper() { mergeOwnField(RUpper); return *this; }
      bool doesRequireLower() const { return queryOwnField() & RLower; }
      bool doesRequireUpper() const { return queryOwnField() & RUpper; }

      virtual void mergeWith(const VirtualQueryOperation::CompareSpecialEnvironment& source);
      virtual void intersectWith(const VirtualQueryOperation::CompareSpecialEnvironment& source);
      virtual void fill();

      void setLower(PPVirtualElement lower) { ppveLower = lower; }
      void setUpper(PPVirtualElement upper) { ppveUpper = upper; }
      PPVirtualElement newLower() { return ppveLower; }
      PPVirtualElement newUpper() { return ppveUpper; }
      bool isValid() const
         {  return (!doesRequireLower() || ppveLower.isValid()) && (!doesRequireUpper() || ppveUpper.isValid()); }
      virtual bool isEmpty() const { return !ppveLower.isValid() && !ppveUpper.isValid(); }
   };

   class CompareSpecialEnvironment : public VirtualQueryOperation::CompareSpecialEnvironment {
     private:
      typedef VirtualQueryOperation::CompareSpecialEnvironment inherited;

     public:
      typedef EvaluationEnvironment::InformationKind InformationKind;
      enum Behaviour
         {  BEqualMinint=1, BEqualMinintP1=2, BLessM1=4, BEqualM1=8, BEqual0=16,
            BEqual1=32, BGreater1=64, BEqualMaxint=128, BEND=256
         };

     protected:
      DefineExtendedParameters(10, inherited)
      DefineSubExtendedParameters(Domain, 8, INHERITED)
      DefineSubExtendedParameters(Information, 2, Domain)

     public:
      CompareSpecialEnvironment() { mergeInformationField(InformationKind::IKMay); }
      CompareSpecialEnvironment(const InformationKind& kind) { mergeInformationField(kind.kind()); }
      CompareSpecialEnvironment(const CompareSpecialEnvironment& source) = default;

      // query methods on the operations
      virtual void mergeWith(const VirtualQueryOperation::CompareSpecialEnvironment& asource)
         {  AssumeCondition(dynamic_cast<const CompareSpecialEnvironment*>(&asource))
            const CompareSpecialEnvironment& source = (const CompareSpecialEnvironment&) asource;
            mergeDomainField(source.queryDomainField());
         }
      virtual void intersectWith(const VirtualQueryOperation::CompareSpecialEnvironment& asource)
         {  AssumeCondition(dynamic_cast<const CompareSpecialEnvironment*>(&asource))
            const CompareSpecialEnvironment& source = (const CompareSpecialEnvironment&) asource;
            intersectDomainField(source.queryDomainField());
         }
      void mergeBehaviour(Behaviour behaviour) { mergeDomainField(behaviour); }
      InformationKind getInformationKind() const
         { return InformationKind((InformationKind::Kind) queryInformationField()); }

      CompareSpecialEnvironment& opposite();
      unsigned getCodeDomain() const { return queryDomainField(); }
      bool isNotZero() const                 { return !(queryDomainField() & BEqual0); }
      bool isZero() const                    { return queryDomainField() == BEqual0; }
      bool mayBeZero() const                 { return queryDomainField() & BEqual0; }
      bool mayBeDifferentZero() const        { return queryDomainField() & ~BEqual0; }

      bool isOne() const                     { return queryDomainField() == BEqual1; }
      bool mayBeOne() const                  { return queryDomainField() & BEqual1; }
      bool isMinusOne() const                { return queryDomainField() == BEqualM1; }
      bool mayBeMinusOne() const             { return queryDomainField() & BEqualM1; }
      bool isMinint() const                  { return queryDomainField() == BEqualMinint; }
      bool mayBeMinint() const               { return queryDomainField() & BEqualMinint; }
      bool isMinintPlusOne() const           { return queryDomainField() == BEqualMinintP1; }
      bool mayBeMinintPlusOne() const        { return queryDomainField() & BEqualMinintP1; }
      bool isMaxint() const                  { return queryDomainField() == BEqualMaxint; }
      bool mayBeMaxint() const               { return queryDomainField() & BEqualMaxint; }

      bool isGreaterZero() const             { return !(queryDomainField() & (BEqual1-1)); }
      bool mayBeGreaterZero() const          { return queryDomainField() & ~(BEqual1-1); }
      bool isGreaterOrEqualZero() const      { return !(queryDomainField() & (BEqual0-1)); }
      bool mayBeGreaterOrEqualZero() const   { return queryDomainField() & ~(BEqual0-1); }
      bool isLessZero() const                { return !(queryDomainField() & ~(BEqual0-1)); }
      bool mayBeLessZero() const             { return queryDomainField() & (BEqual0-1); }
      bool isLessOrEqualZero() const         { return !(queryDomainField() & ~(BEqual1-1)); }
      bool mayBeLessOrEqualZero() const      { return queryDomainField() & (BEqual1-1); }

      bool isLessMinusOne() const            { return !(queryDomainField() & ~(BEqualM1-1)); }
      bool mayBeLessMinusOne() const         { return queryDomainField() & (BEqualM1-1); }
      bool isGreaterOne() const              { return !(queryDomainField() & (BGreater1-1)); }
      bool mayBeGreaterOne() const           { return queryDomainField() & ~(BGreater1-1); }

      void clear() { clearDomainField(); }
      virtual void fill() { mergeDomainField(Domain_MASK); }
      void fillPositive() { mergeDomainField(BEqual0 | BEqual1 | BGreater1 | BEqualMaxint); }
      virtual bool isEmpty() const { return !hasDomainField(); }

      void unify();
   };

   typedef VirtualQueryOperation::InverseCastOperationEnvironment InverseCastOperationEnvironment;

   class QueryOperation : public Approximate::VirtualQueryOperation {
     private:
      typedef Approximate::VirtualQueryOperation inherited;

     public:
      enum TypeExtern
         {  TEUndefined, TENativeOperation, TESigned, EndOfTypeExtern };
      enum TypeOperation
         {  TOUndefined, TOInverseCast, TONextAssign, TONextSignedAssign, TONextUnsignedAssign,
            TOPrevAssign, TOPrevSignedAssign, TOPrevUnsignedAssign,
            TOMinus, TOMinusSigned, TOMinusUnsigned, TOMinusAssign, TOMinusSignedAssign, TOMinusUnsignedAssign,
            TOPlus, TOPlusSigned, TOPlusUnsigned, TOPlusAssign, TOPlusSignedAssign, TOPlusUnsignedAssign,
            TODivide, TODivideSigned, TODivideUnsigned, TODivideAssign, TODivideSignedAssign, TODivideUnsignedAssign,
            TOTimes, TOTimesSigned, TOTimesUnsigned, TOTimesAssign, TOTimesSignedAssign, TOTimesUnsignedAssign,
            TOModulo, TOModuloSigned, TOModuloUnsigned, TOModuloAssign, TOModuloSignedAssign, TOModuloUnsignedAssign,
            TORightShift, TORightShiftSigned, TORightShiftUnsigned, TORightShiftAssign, TORightShiftSignedAssign, TORightShiftUnsignedAssign,
            TOLeftShift, TOLeftShiftAssign, TOInverseRotateAssign, TOBitNegate, TOBitNegateAssign, TOBitOrAssign, TOBitOr,
            TOBitAndAssign, TOBitAnd, TOBitExclusiveOrAssign, TOBitExclusiveOr, TOOpposite, TOOppositeAssign,
            TOMin, TOMinSigned, TOMinUnsigned, TOMinAssign, TOMinSignedAssign, TOMinUnsignedAssign,
            TOMax, TOMaxSigned, TOMaxUnsigned, TOMaxAssign, TOMaxSignedAssign, TOMaxUnsignedAssign,
            TOCompareGreaterOrEqual, TOCompareGreaterOrEqualSigned, TOCompareGreaterOrEqualUnsigned,
            TOCompareGreater, TOCompareGreaterSigned, TOCompareGreaterUnsigned, TOCompareLessOrEqual,
            TOCompareLessOrEqualSigned, TOCompareLessOrEqualUnsigned, TOCompareLess,
            TOCompareLessSigned, TOCompareLessUnsigned, TOCompareEqual,  TOCompareDifferent, EndOfTypeOperation
         };
      enum TypeDomain
         {  TDUndefined, TDDisjunction, TDMultiBitConstant, TDMultiFloatConstant, TDInterval,
            TDBooleanDisjunction, TDTop, TDFalse, TDTrue, TDIntForShift, TDConstant, TDZero,
            TDMinusOne, TDOne, TDMin, TDMinSigned, TDMinUnsigned, TDMax, TDMaxSigned, TDMaxUnsigned,
            EndOfTypeDomain
         };
      enum TypeSimplification
         {  TSUndefined, TSConstant, TSInterval, TSConstantDisjunction, EndOfTypeSimplification
         };
      enum TypeCompareSpecial
         {  TCSUndefined, TCSGuard, TCSNatural, TCSSigned, TCSUnsigned, TCSBitDomain, TCSBound, EndOfTypeCompareSpecial };

     protected:
      DefineExtendedParameters(7, inherited)
      DefineSubExtendedParameters(TypeExtern, 2, INHERITED)
      DefineSubExtendedParameters(TypeOperation, 7, INHERITED)
      DefineSubExtendedParameters(TypeDomain, 5, INHERITED)
      DefineSubExtendedParameters(TypeSimplification, 2, INHERITED)
      DefineSubExtendedParameters(TypeCompareSpecial, 3, INHERITED)

     public:
      QueryOperation() {}
      QueryOperation(const QueryOperation& source) = default;

      QueryOperation& setIsSigned() { inherited::setExtern(); mergeTypeExternField(TESigned); return *this; }
      typedef IntOperationElement::CompareSpecialEnvironment CompareSpecialEnvironment;
      typedef IntOperationElement::BitDomainEnvironment BitDomainEnvironment;
      TypeDomain getTypeDomain() const
         {  AssumeCondition(isDomain())
            return (TypeDomain) queryTypeDomainField();
         }

      QueryOperation& setTypeOperation(TypeOperation type) { inherited::setOperation(); mergeTypeOperationField(type); return *this; }
#define DefineOperation(TypeOperation)                                                             \
      QueryOperation& set##TypeOperation() { inherited::setOperation(); mergeTypeOperationField(TO##TypeOperation); return *this; }

      DefineOperation(InverseCast)
      DefineOperation(NextAssign)
      DefineOperation(PrevAssign)
      DefineOperation(Minus)
      DefineOperation(MinusSigned)
      DefineOperation(MinusAssign)
      DefineOperation(Plus)
      DefineOperation(PlusAssign)
      DefineOperation(Divide)
      DefineOperation(DivideAssign)
      DefineOperation(Times)
      DefineOperation(TimesAssign)
      DefineOperation(ModuloAssign)
      DefineOperation(Modulo)
      DefineOperation(LeftShift)
      DefineOperation(LeftShiftAssign)
      DefineOperation(RightShift)
      DefineOperation(RightShiftAssign)
      DefineOperation(RightShiftSignedAssign)
      DefineOperation(InverseRotateAssign)
      DefineOperation(BitNegate)
      DefineOperation(BitNegateAssign)
      DefineOperation(BitOrAssign)
      DefineOperation(BitOr)
      DefineOperation(BitAndAssign)
      DefineOperation(BitAnd)
      DefineOperation(BitExclusiveOrAssign)
      DefineOperation(BitExclusiveOr)
      DefineOperation(Opposite)
      DefineOperation(OppositeAssign)
      DefineOperation(MinAssign)
      DefineOperation(Min)
      DefineOperation(MaxAssign)
      DefineOperation(Max)
      DefineOperation(CompareGreaterOrEqual)
      DefineOperation(CompareGreater)
      DefineOperation(CompareLessOrEqual)
      DefineOperation(CompareLess)
      DefineOperation(CompareEqual)
      DefineOperation(CompareDifferent)
#undef DefineOperation

#define DefineNewDomain(TypeOperation)                                                              \
      QueryOperation& setNew##TypeOperation() { inherited::setDomain(); mergeTypeDomainField(TD##TypeOperation); return *this; }

      DefineNewDomain(MultiBitConstant)
      DefineNewDomain(MultiFloatConstant)
      DefineNewDomain(Disjunction)
      DefineNewDomain(Interval)
      DefineNewDomain(BooleanDisjunction)
      DefineNewDomain(Top)
      DefineNewDomain(False)
      DefineNewDomain(True)
      DefineNewDomain(IntForShift)
      DefineNewDomain(Constant)
      DefineNewDomain(Zero)
      DefineNewDomain(MinusOne)
      DefineNewDomain(One)
      DefineNewDomain(Min)
      DefineNewDomain(MinSigned)
      DefineNewDomain(MinUnsigned)
      DefineNewDomain(Max)
      DefineNewDomain(MaxSigned)
      DefineNewDomain(MaxUnsigned)
#undef DefineNewDomain

#define DefineSimplification(TypeOperation)                                                       \
      QueryOperation& setSimplificationAs##TypeOperation() { inherited::setSimplification(); mergeTypeDomainField(TS##TypeOperation); return *this; }

      DefineSimplification(Constant)
      DefineSimplification(Interval)
      DefineSimplification(ConstantDisjunction)
#undef DefineSimplification

      QueryOperation& setCompareSpecial() { inherited::setCompareSpecial(); mergeTypeCompareSpecialField(TCSNatural); return *this; }
      QueryOperation& setGuard() { inherited::setCompareSpecial(); mergeTypeCompareSpecialField(TCSGuard); return *this; }
      QueryOperation& setCompareSpecialSigned() { inherited::setCompareSpecial(); mergeTypeCompareSpecialField(TCSSigned); return *this; }
      QueryOperation& setCompareSpecialUnsigned() { inherited::setCompareSpecial(); mergeTypeCompareSpecialField(TCSUnsigned); return *this; }
      QueryOperation& setBitDomain() { inherited::setCompareSpecial(); mergeTypeCompareSpecialField(TCSBitDomain); return *this; }
   };

   class Constants {
     public:
      static const int NBQueryOperationMethods = QueryOperation::EndOfTypeOperation;
      static const int NBQueryDomainMethods = QueryOperation::EndOfTypeDomain;
      static const int NBQueryExternMethods = QueryOperation::EndOfTypeExtern;
      static const int NBQueryExternTrueMethods = 1;
   };

   typedef QueryOperation::DomainEnvironment DomainEnvironment;
   class IntervalEnvironment;

   class IntForShiftEnvironment : public QueryOperation::DomainEnvironment {
     private:
      typedef QueryOperation::DomainEnvironment inherited;
      int uShift;

     public:
      IntForShiftEnvironment(int shift) : uShift(shift) {}
      const int& shift() const { return uShift; }
   };

   class BooleanEnvironment : public QueryOperation::Environment {
     private:
      typedef QueryOperation::Environment inherited;
      enum Result { RUndefined, RFalse, RTrue };
      Result rResult;

     public:
      BooleanEnvironment() : rResult(RUndefined) {}

      void setTrue() { rResult = RTrue; }
      void setFalse() { rResult = RFalse; }
      void setBool(bool result) { rResult = result ? RTrue : RFalse; }
      bool result() const { AssumeCondition(rResult) return (rResult == RFalse) ? false : true; }
   };

   virtual VirtualElement::ZeroResult queryZeroResult() const
      {  CompareSpecialEnvironment compareResult;
         query(QueryOperation().setCompareSpecial(), compareResult);
         ZeroResult result;
         if (compareResult.isZero())
            result.setZero();
         else if (compareResult.isNotZero())
            result.setDifferentZero();
         return result;
      }

   // generic constraint methods with respect to the comparison constraints
   static bool constraintCastBit(VirtualElement&, const VirtualOperation&, const VirtualElement& result, ConstraintEnvironment&);
   static bool constraintCastInteger(VirtualElement&, const VirtualOperation&, const VirtualElement& result, ConstraintEnvironment&);
   static bool constraintPrevAssign(VirtualElement&, const VirtualOperation&, const VirtualElement& result, ConstraintEnvironment&);
   static bool constraintNextAssign(VirtualElement&, const VirtualOperation&, const VirtualElement& result, ConstraintEnvironment&);

   static bool constraintPlusAssign(VirtualElement&, const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment&);
   static bool constraintMinusAssign(VirtualElement&, const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment&);
   static bool constraintTimesAssign(VirtualElement&, const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment&);
   static bool constraintDivideAssign(VirtualElement&, const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment&);

   static bool constraintOppositeAssign(VirtualElement&, const VirtualOperation&, const VirtualElement& result, ConstraintEnvironment&);

   static bool constraintModuloAssign(VirtualElement&, const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment&);
   static bool constraintBitExclusiveOrAssign(VirtualElement&, const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment&);
   static bool constraintBitOrAssign(VirtualElement&, const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment&);
   static bool constraintBitAndAssign(VirtualElement&, const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment&);
   static bool constraintBinaryNoPropagation(VirtualElement&, const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment&);
   static bool constraintBitNegateAssign(VirtualElement&, const VirtualOperation&, const VirtualElement& result, ConstraintEnvironment&);
   static bool constraintRotateAssign(VirtualElement&, const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment&);

   static bool constraintLogicalAndAssign(VirtualElement&, const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment&);
   static bool constraintLogicalOrAssign(VirtualElement&, const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment&);
   static bool constraintLogicalNegateAssign(VirtualElement&, const VirtualOperation&, const VirtualElement& result, ConstraintEnvironment&);

   static bool queryGuardAll(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment&);
   class Methods;
};

class VirtualIntegerInterval : public IntOperationElement {
  private:
   typedef IntOperationElement inherited;

  public:
   class Init : public IntOperationElement::Init {
     private:
      typedef IntOperationElement::Init inherited;

     public:
      enum Type { TUndefined, TLimit, TBoundInterval, TInterval };
      enum Compare { ILess, ILessOrEqual, IEqual, IGreaterOrEqual, IGreater };
      enum BoundType { BTClosed, BTExcludeMin, BTExcludeMax, BTExcludeMinMax };
      enum Unsigned { UUndefined, USigned, UUnsigned };

      Type tType;
      PPVirtualElement ppveFst, ppveSnd;
      unsigned int uQualification;
      Unsigned fUnsigned;

     public:
      Init() : tType(TUndefined), uQualification(0), fUnsigned(UUndefined) {}
      Init(const Init& source) = default;
      Init& operator=(const Init& source) = default;
      DefineCopy(Init)
      DDefineAssign(Init)

      Init& setBitSize(int bitSize) { return (Init&) inherited::setBitSize(bitSize); }
      Init& setLessThan(PPVirtualElement limit)
         {  inherited::setBitSize(limit->getSizeInBits());
            AssumeCondition(!tType)
            ppveFst = limit; tType = TLimit; uQualification = ILess;
            return *this;
         }
      Init& setLessOrEqualThan(PPVirtualElement limit)
         {  inherited::setBitSize(limit->getSizeInBits());
            AssumeCondition(!tType)
            ppveFst = limit; tType = TLimit; uQualification = ILessOrEqual;
            return *this;
         }
      Init& setGreaterThan(PPVirtualElement limit)
         {  inherited::setBitSize(limit->getSizeInBits());
            AssumeCondition(!tType)
            ppveFst = limit; tType = TLimit; uQualification = IGreater;
            return *this;
         }
      Init& setGreaterOrEqualThan(PPVirtualElement limit)
         {  inherited::setBitSize(limit->getSizeInBits());
            AssumeCondition(!tType)
            ppveFst = limit; tType = TLimit; uQualification = IGreaterOrEqual;
            return *this;
         }
      Init& setEqual(PPVirtualElement limit)
         {  inherited::setBitSize(limit->getSizeInBits());
            AssumeCondition(!tType)
            ppveFst = limit; tType = TLimit; uQualification = IEqual;
            return *this;
         }

      Init& setInterval(PPVirtualElement min, PPVirtualElement max)
         {  inherited::setBitSize(min->getSizeInBits());
            AssumeCondition(!tType)
            tType = TInterval; ppveFst = min; ppveSnd = max;
            return *this;
         }
      Init& excludeMin()
         {  AssumeCondition(tType >= TBoundInterval)
            tType = TBoundInterval; uQualification |= BTExcludeMin; return *this;
         }
      Init& excludeMax()
         {  AssumeCondition(tType >= TBoundInterval)
            tType = TBoundInterval; uQualification |= BTExcludeMax; return *this;
         }
      Init& setUnsigned() { fUnsigned = UUnsigned; return *this; }
      Init& setUnsigned(bool isUnsigned) { fUnsigned = isUnsigned ? UUnsigned : USigned; return *this; }
      Init& setSigned() { fUnsigned = USigned; return *this; }
      friend class IntegerInterval;
   };

   VirtualIntegerInterval(const Init& init) : inherited(init) {}
   VirtualIntegerInterval(const VirtualIntegerInterval& source) = default;
   virtual const VirtualElement& getMin() const { AssumeUncalled const VirtualElement* result = nullptr; return *result; }
   virtual const VirtualElement& getMax() const { AssumeUncalled const VirtualElement* result = nullptr; return *result; }
   virtual PPVirtualElement& getSMin() { AssumeUncalled PPVirtualElement* result = NULL; return *result; }
   virtual PPVirtualElement& getSMax() { AssumeUncalled PPVirtualElement* result = NULL; return *result; }
};

class IntOperationElement::IntervalEnvironment : public VirtualQueryOperation::DomainEnvironment {
  public:
   typedef VirtualIntegerInterval::Init Init;

  private:
   typedef VirtualQueryOperation::DomainEnvironment inherited;

  public:
   IntervalEnvironment(PPVirtualElement min, PPVirtualElement max)
      {  setInit(Init().setInterval(min, max)); }
   IntervalEnvironment(const Init& init) { setInit(init); }
   IntervalEnvironment(const IntervalEnvironment& source) = default;

   const Init& init() const { return (const Init&) inherited::init(); }
   Init& init() { return (Init&) inherited::init(); }
   IntervalEnvironment& setUnsigned() { init().setUnsigned(); return *this; }
   IntervalEnvironment& setUnsigned(bool isUnsigned) { init().setUnsigned(isUnsigned); return *this; }
   IntervalEnvironment& setSigned() { init().setSigned(); return *this; }

   PNT::TPassPointer<VirtualIntegerInterval, PPAbstractElement> newResult()
      { return PNT::TPassPointer<VirtualIntegerInterval, PPAbstractElement>(inherited::presult()); }
};

/********************************************************/
/* Definition of the class IntOperationElement::Methods */
/********************************************************/

class IntOperationElement::Methods : public VirtualElement::Methods {
  public:
   static PPVirtualElement constraintBoolean(PPVirtualElement source, bool result,
         const EvaluationParameters& params);
   static PPVirtualElement constraintBoolean(PPVirtualElement source, const VirtualOperation&
         compareOperation, PPVirtualElement& operand, bool result, const EvaluationParameters& params);

//   static PNT::TPassPointer<VirtualIntegerInterval, PPAbstractElement> newInterval(const VirtualElement& source,
//         PPVirtualElement min, PPVirtualElement max)
//      {  IntervalEnvironment env(min, max);
//         source.query(QueryOperation().setNewInterval(), env);
//         return env.newResult();
//      }
   static PNT::TPassPointer<VirtualIntegerInterval, PPAbstractElement> newInterval(
         const VirtualElement& source, const VirtualIntegerInterval::Init& init)
      {  IntervalEnvironment env(init);
         source.query(QueryOperation().setNewInterval(), env);
         return env.newResult();
      }
   static PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> newBooleanDisjunction(
         const VirtualElement& source)
      {  VirtualQueryOperation::DisjunctionEnvironment env;
         source.query(QueryOperation().setNewBooleanDisjunction(), env);
         return env.newResult();
      }

   static PPVirtualElement newTop(const VirtualElement& source)
      {  DomainEnvironment env;
         source.query(QueryOperation().setNewTop(), env);
         return env.presult();
      }
   static PPVirtualElement newMultiFloatConstant(const VirtualElement& source, const VirtualElement::Init& init)
      {  DomainEnvironment env;
         env.setInit(init);
         source.query(QueryOperation().setNewMultiFloatConstant(), env);
         return env.presult();
      }

#define DefineOperation(TypeOperation)                                                          \
   static VirtualOperation query##TypeOperation(const VirtualElement& source)                   \
      {  QueryOperation::OperationEnvironment env;                                              \
         source.query(QueryOperation().set##TypeOperation(), env);                              \
         AssumeCondition(env.hasSimpleResult())                                                 \
         return env.result();                                                                   \
      }

#define DefineBasicOperation(TypeOperation)                                                     \
   static VirtualOperation query##TypeOperation(const VirtualElement& source)                   \
      {  QueryOperation::OperationEnvironment env;                                              \
         source.query(QueryOperation().setTypeOperation(QueryOperation::TO##TypeOperation), env);\
         AssumeCondition(env.hasSimpleResult())                                                 \
         return env.result();                                                                   \
      }

#define DefineRefOperation(TypeOperation)                                                       \
   static PNT::PassPointer<VirtualOperation> query##TypeOperation(const VirtualElement& source, const VirtualOperation& reference)\
      {  QueryOperation::OperationEnvironment env;                                              \
         env.setReferenceResult(reference);                                                     \
         source.query(QueryOperation().set##TypeOperation(), env);                              \
         return PNT::PassPointer<VirtualOperation>(env.extractResult(), PNT::Pointer::Init());  \
      }

   DefineBasicOperation(NextSignedAssign)
   DefineBasicOperation(NextUnsignedAssign)
   DefineBasicOperation(PrevSignedAssign)
   DefineBasicOperation(PrevUnsignedAssign)
   DefineBasicOperation(MinusUnsigned)
   DefineBasicOperation(MinusSignedAssign)
   DefineBasicOperation(MinusUnsignedAssign)
   DefineBasicOperation(PlusSigned)
   DefineBasicOperation(PlusUnsigned)
   DefineBasicOperation(PlusSignedAssign)
   DefineBasicOperation(PlusUnsignedAssign)
   DefineBasicOperation(DivideSigned)
   DefineBasicOperation(DivideUnsigned)
   DefineBasicOperation(DivideSignedAssign)
   DefineBasicOperation(DivideUnsignedAssign)
   DefineBasicOperation(TimesSigned)
   DefineBasicOperation(TimesUnsigned)
   DefineBasicOperation(TimesSignedAssign)
   DefineBasicOperation(TimesUnsignedAssign)
   DefineBasicOperation(ModuloSigned)
   DefineBasicOperation(ModuloUnsigned)
   DefineBasicOperation(ModuloSignedAssign)
   DefineBasicOperation(ModuloUnsignedAssign)
   DefineBasicOperation(RightShiftSigned)
   DefineBasicOperation(RightShiftUnsigned)
   DefineBasicOperation(RightShiftSignedAssign)
   DefineBasicOperation(RightShiftUnsignedAssign)
   DefineBasicOperation(MinSigned)
   DefineBasicOperation(MinUnsigned)
   DefineBasicOperation(MinSignedAssign)
   DefineBasicOperation(MinUnsignedAssign)
   DefineBasicOperation(MaxSigned)
   DefineBasicOperation(MaxUnsigned)
   DefineBasicOperation(MaxSignedAssign)
   DefineBasicOperation(MaxUnsignedAssign)
   DefineBasicOperation(CompareGreaterOrEqualSigned)
   DefineBasicOperation(CompareGreaterOrEqualUnsigned)
   DefineBasicOperation(CompareGreaterSigned)
   DefineBasicOperation(CompareGreaterUnsigned)
   DefineBasicOperation(CompareLessOrEqualSigned)
   DefineBasicOperation(CompareLessOrEqualUnsigned)
   DefineBasicOperation(CompareLessSigned)
   DefineBasicOperation(CompareLessUnsigned)

   DefineOperation(NextAssign)
   DefineOperation(PrevAssign)
   DefineOperation(Minus)
   DefineRefOperation(Minus)
   DefineOperation(MinusSigned)
   DefineOperation(MinusAssign)
   DefineOperation(Plus)
   DefineRefOperation(Plus)
   DefineOperation(PlusAssign)
   DefineOperation(Divide)
   DefineRefOperation(Divide)
   DefineOperation(DivideAssign)
   DefineOperation(Times)
   DefineRefOperation(Times)
   DefineOperation(TimesAssign)
   DefineOperation(ModuloAssign)
   DefineOperation(Modulo)
   DefineRefOperation(Modulo)
   DefineOperation(LeftShift)
   DefineOperation(LeftShiftAssign)
   DefineOperation(RightShift)
   DefineOperation(RightShiftAssign)

   static VirtualOperation queryInverseRotateAssignOperation(const VirtualElement& source, const VirtualOperation& reference)
      {  VirtualQueryOperation::ExtendedOperationEnvironment env(reference);
         source.query(QueryOperation().setInverseRotateAssign(), env);
         AssumeCondition(env.hasSimpleResult())
         return env.result();
      }

   DefineOperation(BitNegate)
   DefineOperation(BitNegateAssign)
   DefineOperation(BitOrAssign)
   DefineOperation(BitOr)
   DefineRefOperation(BitOr)
   DefineOperation(BitAndAssign)
   DefineOperation(BitAnd)
   DefineRefOperation(BitAnd)
   DefineOperation(BitExclusiveOrAssign)
   DefineOperation(BitExclusiveOr)
   DefineRefOperation(BitExclusiveOr)
   DefineOperation(Opposite)
   DefineRefOperation(Opposite)
   DefineOperation(OppositeAssign)
   DefineOperation(MinAssign)
   DefineOperation(Min)
   DefineOperation(MaxAssign)
   DefineOperation(Max)
   DefineOperation(CompareGreaterOrEqual)
   DefineOperation(CompareGreater)
   DefineOperation(CompareLessOrEqual)
   DefineOperation(CompareLess)
   DefineOperation(CompareEqual)
   DefineOperation(CompareDifferent)
#undef DefineOperation
#undef DefineBasicOperation
#undef DefineRefOperation

   typedef IntOperationElement::CompareSpecialEnvironment CompareSpecialEnvironment;
   static CompareSpecialEnvironment compareSpecial(const VirtualElement& source)
      {  CompareSpecialEnvironment result;
         source.query(QueryOperation().setCompareSpecial(), result);
         return result;
      }
   static CompareSpecialEnvironment compareSpecialSigned(const VirtualElement& source)
      {  CompareSpecialEnvironment result;
         source.query(QueryOperation().setCompareSpecialSigned(), result);
         return result;
      }
   static CompareSpecialEnvironment compareSpecialUnsigned(const VirtualElement& source)
      {  CompareSpecialEnvironment result;
         source.query(QueryOperation().setCompareSpecialUnsigned(), result);
         return result;
      }

#define DefineNewDomain(TypeOperation)                                                          \
   static PPVirtualElement new##TypeOperation(const VirtualElement& source)                     \
      {  DomainEnvironment env;                                                                 \
         source.query(QueryOperation().setNew##TypeOperation(), env);                           \
         return env.presult();                                                                  \
      }

   DefineNewDomain(False)
   DefineNewDomain(True)

   static PPVirtualElement newIntForShift(const VirtualElement& source, int domain)
      {  IntForShiftEnvironment env(domain);
         source.query(QueryOperation().setNewIntForShift(), env);
         return env.presult();
      }
   static PPVirtualElement newConstant(const VirtualElement& source, const Scalar::MultiBit::Implementation::MultiBitElement& integer)
      {  DomainEnvironment env;
         env.setInit(VirtualElement::Init());
         env.init().setBitSize(integer.getSize()).setInitialValue(integer);
         source.query(QueryOperation().setNewConstant(), env);
         return env.presult();
      }
   static PPVirtualElement newConstant(const VirtualElement& source, const VirtualElement::Init& init)
      {  DomainEnvironment env;
         env.setInit(init);
         source.query(QueryOperation().setNewConstant(), env);
         return env.presult();
      }

   DefineNewDomain(Zero)
   DefineNewDomain(MinusOne)
   DefineNewDomain(One)
   DefineNewDomain(Min)
   DefineNewDomain(MinSigned)
   DefineNewDomain(MinUnsigned)
   DefineNewDomain(Max)
   DefineNewDomain(MaxSigned)
   DefineNewDomain(MaxUnsigned)
#undef DefineNewDomain

#define DefineSimplification(TypeOperation)                                                     \
   static PPVirtualElement simplifyAs##TypeOperation(PPVirtualElement source)                   \
      {  SimplificationEnvironment env;                                                         \
         source->query(QueryOperation().setSimplificationAs##TypeOperation(), env);             \
         if (env.hasFailed())                                                                   \
            source.release();                                                                   \
         else if (env.hasResult())                                                              \
            source = env.presult();                                                             \
         return source;                                                                         \
      }

      DefineSimplification(Constant)
      DefineSimplification(Interval)
      DefineSimplification(ConstantDisjunction)
#undef DefineSimplification

#define DefineSimplification(TypeOperation)                                                     \
   static PPVirtualElement ssimplifyAs##TypeOperation(PPVirtualElement& source)                 \
      {  SimplificationEnvironment env;                                                         \
         source->query(QueryOperation().setSimplificationAs##TypeOperation(), env);             \
         if (env.hasFailed())                                                                   \
            return PPVirtualElement();                                                          \
         else if (env.hasResult())                                                              \
            source = env.presult();                                                             \
         return source;                                                                         \
      }

      DefineSimplification(Constant)
      DefineSimplification(Interval)
      DefineSimplification(ConstantDisjunction)
#undef DefineSimplification

   static bool isSigned(const VirtualElement& source)
      {  BooleanEnvironment env;
         source.query(QueryOperation().setIsSigned(), env);
         return env.result();
      }
};

inline PPVirtualElement
IntOperationElement::BitDomainEnvironment::maxZero() const
   {  return Methods::newConstant(ioeReference, mbeFixed); }

inline PPVirtualElement
IntOperationElement::BitDomainEnvironment::maxOne() const
   {  return Methods::newConstant(ioeReference,
         (const MultiBitElement&) MultiBitElement(mbeFixed).neg(uLastBitPos));
   }

/************************************************/
/* Definition of the class RealOperationElement */
/************************************************/

class RealOperationElement : public TIntAndRealElement<RealOperationElement> {
  private:
   typedef TIntAndRealElement<RealOperationElement> inherited;
   typedef RealOperationElement thisType;

  public:
   RealOperationElement(const InitFloat& init) : inherited(init) {}
   RealOperationElement(const thisType& source) : inherited(source) {}
   virtual bool isFloat() const { return true; }

   class CompareSpecialEnvironment : public VirtualQueryOperation::CompareSpecialEnvironment {
     private:
      typedef VirtualQueryOperation::CompareSpecialEnvironment inherited;

     public:
      enum Behaviour
         {  BMInfty = 1, BMin=2, BLessM1=4, BEqualM1=8, BBetweenM1_0=16, BEqual0=32,
            BBetween0_1=64, BEqual1=128, BGreater1=256, BMax=512, BInfty=1024, BNaN=2048
         };

     protected:
      DefineExtendedParameters(12, ExtendedParameters)

     public:
      CompareSpecialEnvironment() {}

      virtual void mergeWith(const VirtualQueryOperation::CompareSpecialEnvironment& asource)
         {  AssumeCondition(dynamic_cast<const CompareSpecialEnvironment*>(&asource))
            const CompareSpecialEnvironment& source = (const CompareSpecialEnvironment&) asource;
            mergeOwnField(source.queryOwnField());
         }
      unsigned getCodeDomain() const { return queryOwnField(); }

      // query methods on the operations
      void mergeBehaviour(Behaviour behaviour) { mergeOwnField(behaviour); }
      bool isNotZero() const     { return !(queryOwnField() & BEqual0); }
      bool isZero() const        { return queryOwnField() == BEqual0; }
      bool mayBeZero() const  	{ return (queryOwnField() & BEqual0); }
      bool isOne() const         { return queryOwnField() == BEqual1; }
      bool isMinusOne() const    { return queryOwnField() == BEqualM1; }
      bool isGreaterZero() const
         {  return !(queryOwnField() & ~(BBetween0_1|BEqual1|BGreater1|BMax|BInfty)); }
      bool isContractant() const
         {  return !(queryOwnField() & ~(BEqualM1|BBetweenM1_0|BEqual0|BBetween0_1|BEqual1)); }
      bool isStrictContractant() const
         {  return !(queryOwnField() & ~(BBetweenM1_0|BEqual0|BBetween0_1)); }
      bool mayBeStrictContractant() const { return (queryOwnField() & (BBetweenM1_0|BEqual0|BBetween0_1)); }
      bool mayBeExpensive() const { return (queryOwnField() & (BMInfty|BMin|BLessM1|BGreater1|BMax|BInfty)); }
      bool mayBeGreaterZero() const { return (queryOwnField() & (BBetween0_1|BEqual1|BGreater1|BMax|BInfty)); }
      bool isGreaterOrEqualZero() const
         {  return !(queryOwnField() & ~(BEqual0|BBetween0_1|BEqual1|BGreater1|BMax|BInfty)); }
      bool isLessZero() const
         {  return !(queryOwnField() & ~(BBetweenM1_0|BEqualM1|BLessM1|BMin|BMInfty)); }
      bool mayBeLessZero() const { return (queryOwnField() & (BLessM1|BEqualM1|BBetweenM1_0|BMin|BMInfty)); }
      bool isLessOrEqualZero() const
         {  return !(queryOwnField() & ~(BEqual0|BBetweenM1_0|BEqualM1|BLessM1|BMin|BMInfty)); }
      bool isMinusInfty() const { return queryOwnField() == BMInfty; }
      bool mayBeMinusInfty() const { return queryOwnField() & BMInfty; }
      bool isInfty() const { return queryOwnField() == BInfty; }
      bool mayBeInfty() const { return queryOwnField() & BInfty; }
      bool isMin() const { return queryOwnField() == BMin; }
      bool mayBeMin() const { return queryOwnField() & BMin; }
      bool isMax() const { return queryOwnField() == BMax; }
      bool mayBeMax() const { return queryOwnField() & BMax; }

      void clear() { clearOwnField(); }
      void unify();
   };
   class QueryOperation : public VirtualQueryOperation {
     private:
      typedef Approximate::VirtualQueryOperation inherited;

     public:
      enum TypeExternMethods
         {  TEMUndefined, TEMSizes, EndOfTypeExternMethods };
      enum TypeOperation
         {  TOUndefined, TOInverseCast, TOMinus, TOMinusSigned, TOMinusAssign, TOCompareLess,
            TOCompareLessOrEqual, TOCompareEqual, TOCompareGreaterOrEqual, TOCompareGreater, EndOfTypeOperation
         };
      enum TypeDomain
         {  TDUndefined, TDDisjunction, TDMultiBitConstant, TDMultiFloatConstant, TDInterval,
            TDBooleanDisjunction, TDTop, TDZero, TDMin, TDMax, TDTrue, TDFalse, EndOfTypeDomain
         };
      enum TypeCompareSpecial
         {  TCSUndefined, TCSGuard, EndOfTypeCompareSpecial };

     protected:
      DefineExtendedParameters(4, inherited)
      DefineSubExtendedParameters(TypeExternMethods, 1, INHERITED)
      DefineSubExtendedParameters(TypeOperation, 4, INHERITED)
      DefineSubExtendedParameters(TypeDomain, 4, INHERITED)
      DefineSubExtendedParameters(TypeCompareSpecial, 1, INHERITED)

     public:
      QueryOperation() {}
      QueryOperation(const QueryOperation& source) : inherited(source) {}
      typedef RealOperationElement::CompareSpecialEnvironment CompareSpecialEnvironment;

      QueryOperation& setSizes() { inherited::setExternMethod(); mergeTypeExternMethodsField(TEMSizes); return *this; }

#define DefineNewOperation(TypeOperation)                                                        \
      QueryOperation& set##TypeOperation() { inherited::setOperation(); mergeTypeOperationField(TO##TypeOperation); return *this; }

      DefineNewOperation(InverseCast)
      DefineNewOperation(Minus)
      DefineNewOperation(MinusSigned)
      DefineNewOperation(MinusAssign)
      DefineNewOperation(CompareLess)
      DefineNewOperation(CompareLessOrEqual)
      DefineNewOperation(CompareEqual)
      DefineNewOperation(CompareGreaterOrEqual)
      DefineNewOperation(CompareGreater)

#undef DefineNewOperation

#define DefineNewDomain(TypeOperation)                                                           \
      QueryOperation& setNew##TypeOperation() { inherited::setDomain(); mergeTypeDomainField(TD##TypeOperation); return *this; }

      DefineNewDomain(MultiBitConstant)
      DefineNewDomain(MultiFloatConstant)
      DefineNewDomain(Disjunction)
      DefineNewDomain(Interval)
      DefineNewDomain(BooleanDisjunction)
      DefineNewDomain(Top)
      DefineNewDomain(Zero)
      DefineNewDomain(Min)
      DefineNewDomain(Max)
      DefineNewDomain(True)
      DefineNewDomain(False)

#undef DefineNewDomain

      class SizesEnvironment : public QueryOperation::Environment {
        private:
         typedef QueryOperation::Environment inherited;
         int uSizeMantissa;
         int uSizeExponent;
         int uSize;

        public:
         SizesEnvironment() : uSizeMantissa(0), uSizeExponent(0), uSize(0) {}

         SizesEnvironment& setSizeMantissa(int sizeMantissa) { uSizeMantissa = sizeMantissa; return *this; }
         SizesEnvironment& setSizeExponent(int sizeExponent) { uSizeExponent = sizeExponent; return *this; }
         SizesEnvironment& setSize(int size) { uSize = size; return *this; }

         int sizeMantissa() const { AssumeCondition(uSizeMantissa > 0) return uSizeMantissa; }
         int sizeExponent() const { AssumeCondition(uSizeExponent > 0) return uSizeExponent; }
         int size() const { AssumeCondition(uSize > 0) return uSize; }
      };
   };
   class IntervalEnvironment;
   typedef QueryOperation::SizesEnvironment SizesEnvironment;

   virtual VirtualElement::ZeroResult queryZeroResult() const
      {  CompareSpecialEnvironment compareResult;
         query(QueryOperation().setCompareSpecial(), compareResult);
         ZeroResult result;
         if (compareResult.isZero())
            result.setZero();
         else if (compareResult.isNotZero())
            result.setDifferentZero();
         return result;
      }

   class Constants {
     public:
      static const int NBQueryOperationMethods = QueryOperation::EndOfTypeOperation;
      static const int NBQueryCompareSpecialMethods = QueryOperation::EndOfTypeCompareSpecial;
      static const int NBQueryDomainMethods = QueryOperation::EndOfTypeDomain;
      static const int NBQueryValueMethods = 1;
      static const int NBQueryExternMethods = QueryOperation::EndOfTypeExtern;
      static const int NBQueryExternTrueMethods = QueryOperation::EndOfTypeExternMethods;
   };

   static bool queryGuardAll(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment&);
   class Methods;
};

class VirtualRealInterval : public RealOperationElement {
  private:
   typedef RealOperationElement inherited;

  public:
   class Init : public RealOperationElement::InitFloat {
     private:
      typedef RealOperationElement::InitFloat inherited;

     public:
      enum Type { TUndefined, TLimit, TBoundInterval, TInterval };
      enum Compare { ILess, ILessOrEqual, IEqual, IGreaterOrEqual, IGreater };
      enum BoundType { BTClosed, BTExcludeMin, BTExcludeMax, BTExcludeMinMax };

      Type tType;
      PPVirtualElement ppveFst, ppveSnd;
      unsigned int uQualification;

     public:
      Init() : tType(TUndefined), uQualification(0) {}
      Init(const Init& source) = default;
      Init& operator=(const Init& source) = default;
      DefineCopy(Init)
      DDefineAssign(Init)

      Init& setBitSize(int bitSize) { return (Init&) inherited::setBitSize(bitSize); }
      Init& setLessThan(PPVirtualElement limit)
         {  inherited::setBitSize(limit->getSizeInBits());
            AssumeCondition(!tType)
            ppveFst = limit; tType = TLimit; uQualification = ILess;
            return *this;
         }
      Init& setLessOrEqualThan(PPVirtualElement limit)
         {  inherited::setBitSize(limit->getSizeInBits());
            AssumeCondition(!tType)
            ppveFst = limit; tType = TLimit; uQualification = ILessOrEqual;
            return *this;
         }
      Init& setGreaterThan(PPVirtualElement limit)
         {  inherited::setBitSize(limit->getSizeInBits());
            AssumeCondition(!tType)
            ppveFst = limit; tType = TLimit; uQualification = IGreater;
            return *this;
         }
      Init& setGreaterOrEqualThan(PPVirtualElement limit)
         {  inherited::setBitSize(limit->getSizeInBits());
            AssumeCondition(!tType)
            ppveFst = limit; tType = TLimit; uQualification = IGreaterOrEqual;
            return *this;
         }
      Init& setEqual(PPVirtualElement limit)
         {  inherited::setBitSize(limit->getSizeInBits());
            AssumeCondition(!tType)
            ppveFst = limit; tType = TLimit; uQualification = IEqual;
            return *this;
         }

      Init& setInterval(PPVirtualElement min, PPVirtualElement max)
         {  inherited::setBitSize(min->getSizeInBits());
            AssumeCondition(!tType)
            tType = TInterval; ppveFst = min; ppveSnd = max;
            return *this;
         }
      Init& excludeMin()
         {  AssumeCondition(tType >= TBoundInterval)
            tType = TBoundInterval; uQualification |= BTExcludeMin; return *this;
         }
      Init& excludeMax()
         {  AssumeCondition(tType >= TBoundInterval)
            tType = TBoundInterval; uQualification |= BTExcludeMax; return *this;
         }
   };
   typedef Init InitFloat;

   VirtualRealInterval(const Init& init) : inherited(init) {}
   VirtualRealInterval(const VirtualRealInterval& source) : inherited(source) {}
};

class RealOperationElement::IntervalEnvironment : public VirtualQueryOperation::DomainEnvironment {
  public:
   typedef VirtualRealInterval::Init Init;

  private:
   typedef VirtualQueryOperation::DomainEnvironment inherited;

  public:
   IntervalEnvironment(PPVirtualElement min, PPVirtualElement max)
      {  setInit(Init().setInterval(min, max)); }
   IntervalEnvironment(const Init& init) { setInit(init); }
   IntervalEnvironment(const IntervalEnvironment& source) = default;

   const Init& init() const { return (const Init&) inherited::init(); }
   Init& init() { return (Init&) inherited::init(); }

   PNT::TPassPointer<VirtualRealInterval, PPAbstractElement> newResult()
      {  return PNT::TPassPointer<VirtualRealInterval, PPAbstractElement>(inherited::presult()); }
};

class RealOperationElement::Methods : public VirtualElement::Methods {
  public:
   static PNT::TPassPointer<VirtualRealInterval, PPAbstractElement> newInterval(
         const VirtualElement& source, PPVirtualElement min, PPVirtualElement max)
      {  IntervalEnvironment env(min, max);
         source.query(QueryOperation().setNewInterval(), env);
         return env.newResult();
      }
   static PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> newBooleanDisjunction(const VirtualElement& source)
      {  VirtualQueryOperation::DisjunctionEnvironment env;
         source.query(QueryOperation().setNewBooleanDisjunction(), env);
         return env.newResult();
      }
   static PNT::TPassPointer<VirtualRealInterval, PPAbstractElement> newInterval(
         const VirtualElement& source, const VirtualRealInterval::Init& init)
      {  IntervalEnvironment env(init);
         source.query(QueryOperation().setNewInterval(), env);
         return env.newResult();
      }

#define DefineOperation(TypeOperation)                                                         \
   static VirtualOperation query##TypeOperation(const VirtualElement& source)                  \
      {  QueryOperation::OperationEnvironment env;                                             \
         source.query(QueryOperation().set##TypeOperation(), env);                             \
         return env.result();                                                                  \
      }

   DefineOperation(InverseCast)
   DefineOperation(Minus)
   DefineOperation(MinusSigned)
   DefineOperation(MinusAssign)
   DefineOperation(CompareLess)
   DefineOperation(CompareLessOrEqual)
   DefineOperation(CompareEqual)
   DefineOperation(CompareGreaterOrEqual)
   DefineOperation(CompareGreater)
#undef DefineOperation

   static PPVirtualElement newMultiFloatConstant(const VirtualElement& source, const VirtualElement::Init& init)
      {  QueryOperation::DomainEnvironment env;
         env.setInit(init);
         source.query(QueryOperation().setNewMultiFloatConstant(), env);
         return env.presult();
      }

#define DefineNewDomain(TypeOperation)                                                          \
   static PPVirtualElement new##TypeOperation(const VirtualElement& source)                     \
      {  QueryOperation::DomainEnvironment env;                                                 \
         source.query(QueryOperation().setNew##TypeOperation(), env);                           \
         return env.presult();                                                                  \
      }

   DefineNewDomain(Top)
   DefineNewDomain(Zero)
   DefineNewDomain(Min)
   DefineNewDomain(Max)
   DefineNewDomain(True)
   DefineNewDomain(False)
#undef DefineNewDomain

   static CompareSpecialEnvironment compareSpecial(const VirtualElement& source)
      {  CompareSpecialEnvironment result;
         source.query(QueryOperation().setCompareSpecial(), result);
         return result;
      }
   static int querySizeMantissa(const VirtualElement& source)
      {  SizesEnvironment result;
         source.query(QueryOperation().setSizes(), result);
         return result.sizeMantissa();
      }
   static int querySizeExponent(const VirtualElement& source)
      {  SizesEnvironment result;
         source.query(QueryOperation().setSizes(), result);
         return result.sizeExponent();
      }
};

} // end of namespace Details

typedef Details::IntOperationElement::Methods IntMethods;

/***************************************************/
/* Definition of the class VirtualElement::Methods */
/***************************************************/

/* Definition of the methods VirtualElement::Methods::apply */

inline PPVirtualElement
VirtualElement::Methods::apply(const VirtualElement& source, const VirtualOperation& operation,
      const EvaluationParameters& params) {
   AssumeCondition(operation.isConst())
   EvaluationEnvironment env(params);
   bool result = const_cast<VirtualElement&>(source).apply(operation, env);
   AssumeCondition(result)
   return env.presult();
}

inline PPVirtualElement
VirtualElement::Methods::apply(const VirtualElement& source, const VirtualOperation& operation,
      EvaluationEnvironment& env, const EvaluationParameters& params) {
   AssumeCondition(operation.isConst())
   TransmitEnvironment copyEnv(env, params);
   bool result = const_cast<VirtualElement&>(source).apply(operation, copyEnv);
   AssumeCondition(result)
   return copyEnv.presult();
}

inline PPVirtualElement
VirtualElement::Methods::apply(const VirtualElement& source, const VirtualOperation& operation,
      EvaluationEnvironment& env) {
   AssumeCondition(operation.isConst())
   TransmitEnvironment copyEnv(env);
   bool result = const_cast<VirtualElement&>(source).apply(operation, copyEnv);
   AssumeCondition(result)
   return copyEnv.presult();
}

inline PPVirtualElement
VirtualElement::Methods::apply(const VirtualElement& source, const VirtualOperation& operation,
      const VirtualElement& arg, const EvaluationParameters& params) {
   AssumeCondition(operation.isConst())
   EvaluationEnvironment env(params);
   env.setFirstArgument(arg);
   bool result = const_cast<VirtualElement&>(source).apply(operation, env);
   AssumeCondition(result)
   return env.presult();
}

inline PPVirtualElement
VirtualElement::Methods::apply(const VirtualElement& source, const VirtualOperation& operation,
      const VirtualElement& arg, EvaluationEnvironment& env) {
   AssumeCondition(operation.isConst())
   TransmitEnvironment copyEnv(env);
   copyEnv.setFirstArgument(arg);
   bool result = const_cast<VirtualElement&>(source).apply(operation, copyEnv);
   AssumeCondition(result)
   return copyEnv.presult();
}

inline PPVirtualElement
VirtualElement::Methods::apply(const VirtualElement& source, const VirtualOperation& operation,
      const VirtualElement& arg, EvaluationEnvironment& env, const EvaluationParameters& params) {
   AssumeCondition(operation.isConst())
   TransmitEnvironment copyEnv(env, params);
   copyEnv.setFirstArgument(arg);
   bool result = const_cast<VirtualElement&>(source).apply(operation, copyEnv);
   AssumeCondition(result)
   return copyEnv.presult();
}

/* Definition of the methods VirtualElement::Methods::applyAssign */

inline PPVirtualElement
VirtualElement::Methods::applyAssign(PPVirtualElement source, const VirtualOperation& operation,
      const EvaluationParameters& params) {
   AssumeCondition(!operation.isConst())
   EvaluationEnvironment env(params);
   bool result = source->apply(operation, env);
   AssumeCondition(result)
   return !env.isEmpty() ? (env.hasResult() ? PPVirtualElement(env.presult()) : source)
      : PPVirtualElement();
}

inline PPVirtualElement
VirtualElement::Methods::applyAssign(PPVirtualElement source, const VirtualOperation& operation,
      EvaluationEnvironment& env) {
   AssumeCondition(!operation.isConst())
   TransmitEnvironment copyEnv(env);
   bool result = source->apply(operation, copyEnv);
   AssumeCondition(result)
   return !copyEnv.isEmpty() ? (copyEnv.hasResult()
         ? PPVirtualElement(copyEnv.presult()) : source)
      : PPVirtualElement();
}

inline PPVirtualElement
VirtualElement::Methods::applyAssign(PPVirtualElement source, const VirtualOperation& operation,
      EvaluationEnvironment& env, const EvaluationParameters& params) {
   AssumeCondition(!operation.isConst())
   TransmitEnvironment copyEnv(env, params);
   bool result = source->apply(operation, copyEnv);
   AssumeCondition(result)
   return !copyEnv.isEmpty() ? (copyEnv.hasResult()
         ? PPVirtualElement(copyEnv.presult()) : source)
      : PPVirtualElement();
}

inline PPVirtualElement
VirtualElement::Methods::applyAssign(PPVirtualElement source, const VirtualOperation& operation,
      const VirtualElement& arg, const EvaluationParameters& params) {
   AssumeCondition(!operation.isConst())
   EvaluationEnvironment env(params);
   env.setFirstArgument(arg);
   bool result = source->apply(operation, env);
   AssumeCondition(result)
   return !env.isEmpty() ? (env.hasResult() ? PPVirtualElement(env.presult()) : source)
      : PPVirtualElement();
}

inline PPVirtualElement
VirtualElement::Methods::applyAssign(PPVirtualElement source, const VirtualOperation& operation,
      const VirtualElement& arg, EvaluationEnvironment& env) {
   AssumeCondition(!operation.isConst())
   TransmitEnvironment copyEnv(env);
   copyEnv.setFirstArgument(arg);
   bool result = source->apply(operation, copyEnv);
   AssumeCondition(result)
   return !copyEnv.isEmpty() ? (copyEnv.hasResult()
         ? PPVirtualElement(copyEnv.presult()) : source)
      : PPVirtualElement();
}

inline PPVirtualElement
VirtualElement::Methods::applyAssign(PPVirtualElement source, const VirtualOperation& operation,
      const VirtualElement& arg, EvaluationEnvironment& env, const EvaluationParameters& params) {
   AssumeCondition(!operation.isConst())
   TransmitEnvironment copyEnv(env, params);
   copyEnv.setFirstArgument(arg);
   bool result = source->apply(operation, copyEnv);
   AssumeCondition(result)
   return !copyEnv.isEmpty() ? (copyEnv.hasResult()
         ? PPVirtualElement(copyEnv.presult()) : source)
      : PPVirtualElement();
}

/* Definition of the methods VirtualElement::Methods::constraint */

inline PPVirtualElement
VirtualElement::Methods::constraint(PPVirtualElement sourceElement, const VirtualOperation& operation,
      const VirtualElement& resultElement, const EvaluationParameters& params) {
   ConstraintEnvironment env(params);
   bool result = sourceElement->constraint(operation, resultElement, env);
   AssumeCondition(result)
   return !env.isEmpty() ? ((env.hasResult())
         ? PPVirtualElement(env.presult()) : sourceElement)
      : PPVirtualElement();
}

inline PPVirtualElement
VirtualElement::Methods::constraint(PPVirtualElement source, const VirtualOperation& operation,
      PPVirtualElement& arg, const VirtualElement& result, const EvaluationParameters& params) {
   ConstraintEnvironment env(params);
   env.absorbFirstArgument(arg.extractElement());
   bool constraintResult = source->constraint(operation, result, env);
   AssumeCondition(constraintResult)
   arg.absorbElement((VirtualElement*) env.extractFirstArgument());
   return !env.isEmpty() ? ((env.hasResult())
         ? PPVirtualElement(env.presult()) : source)
      : PPVirtualElement();
}

inline PPVirtualElement
VirtualElement::Methods::constraint(PPVirtualElement source, const VirtualOperation& operation,
      const VirtualElement& arg, const VirtualElement& result, const EvaluationParameters& params) {
   ConstraintEnvironment env(params);
   env.setFirstArgument(arg);
   bool constraintResult = source->constraint(operation, result, env);
   AssumeCondition(constraintResult)
   return !env.isEmpty() ? ((env.hasResult())
         ? PPVirtualElement(env.presult()) : source)
      : PPVirtualElement();
}

/* Definition of the methods VirtualElement::Methods::merge and VirtualElement::Methods::intersect */

inline PPVirtualElement
VirtualElement::Methods::merge(PPVirtualElement source, const VirtualElement& arg,
      const EvaluationParameters& params) {
   if (!source.isValid())
      source.setElement(arg);
   else {
      EvaluationEnvironment env(params);
      bool result = source->mergeWith(arg, env);
      AssumeCondition(result && !env.isEmpty())
      if (env.hasResult())
         source = env.presult();
   };
   return source;
}

inline PPVirtualElement
VirtualElement::Methods::merge(PPVirtualElement source, PPVirtualElement arg,
      const EvaluationParameters& params) {
   if (!source.isValid() || !arg.isValid()) {
      if (arg.isValid())
         source = arg;
   }
   else {
      EvaluationEnvironment env(params);
      bool result = source->mergeWith(*arg, env);
      AssumeCondition(result && !env.isEmpty())
      if (env.hasResult())
         source = env.presult();
   };
   return source;
}

inline PPVirtualElement
VirtualElement::Methods::merge(PPVirtualElement source, PPVirtualElement arg,
      EvaluationEnvironment& aenv) {
   if (!source.isValid() || !arg.isValid()) {
      if (arg.isValid())
         source = arg;
   }
   else {
      TransmitEnvironment env(aenv);
      bool result = source->mergeWith(*arg, env);
      AssumeCondition(result && !env.isEmpty())
      if (env.hasResult())
         source = env.presult();
   };
   return source;
}

inline PPVirtualElement
VirtualElement::Methods::merge(PPVirtualElement source, const VirtualElement& arg,
      EvaluationEnvironment& aenv) {
   if (!source.isValid())
      source.setElement(arg);
   else {
      TransmitEnvironment env(aenv);
      bool result = source->mergeWith(arg, env);
      AssumeCondition(result && !env.isEmpty())
      if (env.hasResult())
         source = env.presult();
   };
   return source;
}

inline PPVirtualElement
VirtualElement::Methods::intersect(PPVirtualElement source, const VirtualElement& arg,
      const EvaluationParameters& params, bool* isStable) {
   EvaluationEnvironment env(params);
   bool result = source->intersectWith(arg, env);
   AssumeCondition(result)
   if (isStable)
      *isStable = env.isStable();
   return !env.isEmpty() ? (env.hasResult()
         ? PPVirtualElement(env.presult()) : source)
      : PPVirtualElement();
}

inline bool
VirtualElement::Methods::contain(const VirtualElement& source, const VirtualElement& arg,
      const EvaluationParameters& params) {
   EvaluationEnvironment env(params);
   assume(source.contain(arg, env));
   return env.isTotalApplied();
}

inline PPVirtualElement
VirtualElement::Methods::intersect(PPVirtualElement source, const VirtualElement& arg,
      EvaluationEnvironment& aenv) {
   PPVirtualElement result;
   {  TransmitEnvironment env(aenv);
      bool intersectionResult = source->intersectWith(arg, env);
      AssumeCondition(intersectionResult)
      result = !env.isEmpty() ? (env.hasResult()
            ? PPVirtualElement(env.presult()) : source)
         : PPVirtualElement();
   };
   aenv.clearEmpty();
   return result;
}


inline PPVirtualElement
VirtualElement::Methods::simplify(PPVirtualElement source) {
   SimplificationEnvironment result;
   bool queryResult = source->query(VirtualQueryOperation().setSimplification(), result);
   AssumeCondition(queryResult)
   return !result.isEmpty() ? (!result.presult().isValid() ? source : result.presult()) : PPVirtualElement();
}

inline VirtualElement::ExternNativeOperationEnvironment::TypeOperation
VirtualElement::Methods::queryTypeOperation(const VirtualElement& source) {
   ExternNativeOperationEnvironment result;
   source.query(VirtualQueryOperation().setNativeOperation(), result);
   return result.getTypeOperation();
}

} // end of class Approximate

/***************************************/
/* Declaration of the concrete classes */
/***************************************/

namespace Bit { namespace Approximate {

using namespace Scalar::Approximate;
using VirtualElement = Scalar::Approximate::VirtualElement;
using PPVirtualElement = Scalar::Approximate::PPVirtualElement;
using EvaluationEnvironment = Scalar::Approximate::EvaluationEnvironment;

namespace Details {

class BitElement : public VirtualElement {
  public:
   typedef VirtualElement VirtualElement;

   BitElement(const Init& init) : VirtualElement(init) {}
   BitElement(const BitElement& source) = default;

   virtual bool isBoolean() const { return true; }

   class QueryOperation : public VirtualQueryOperation {
     protected:
      DefineExtendedParameters(3, VirtualQueryOperation)
      DefineSubExtendedParameters(TypeDomain, 3, INHERITED)

     public:
      enum TypeDomain
         {  TDUndefined, TDDisjunction, TDMultiBitConstant, TDIntConstant, TDTop,
            TDTrue, TDFalse, EndOfTypeDomain
         };

      class CompareSpecialEnvironment : public VirtualQueryOperation::CompareSpecialEnvironment {
        private:
         typedef VirtualQueryOperation::CompareSpecialEnvironment inherited;
         enum Result { RUndefined, RFalse, RTrue, RTop };
         int rResult;

        public:
         CompareSpecialEnvironment() : rResult(RUndefined) {}
         CompareSpecialEnvironment(const CompareSpecialEnvironment& source) = default;

         CompareSpecialEnvironment& setZero()   { rResult |= RFalse; return *this; }
         CompareSpecialEnvironment& setOne()   { rResult |= RTrue; return *this; }
         CompareSpecialEnvironment& setDifferentZero() { rResult |= RTrue; return *this; }
         CompareSpecialEnvironment& setFalse()  { rResult |= RFalse; return *this; }
         CompareSpecialEnvironment& setTrue()   { rResult |= RTrue; return *this; }
         CompareSpecialEnvironment& setBool(bool fValue)   { rResult |= fValue ? RTrue : RFalse; return *this; }

         bool isValid() const    { return rResult != RUndefined; }
         bool isExact() const    { return (rResult != RUndefined) && (rResult != RTop); }
         bool isZero() const     { return rResult == RFalse; }
         bool isDifferentZero() const { return rResult == RTrue; }
         bool isFalse() const    { return rResult == RFalse; }
         bool isTrue() const     { return rResult == RTrue; }
         bool mayBeFalse() const { return rResult & RFalse; }
         bool mayBeTrue() const  { return rResult & RTrue; }
         bool mayBeZero() const  { return rResult & RFalse; }
         bool mayBeDifferentZero() const { return rResult & RTrue; }
         operator bool() const { return (rResult == RTrue); }
         bool operator!() const { return (rResult == RFalse); }

         virtual void mergeWith(const VirtualQueryOperation::CompareSpecialEnvironment& asource)
            {  AssumeCondition(dynamic_cast<const CompareSpecialEnvironment*>(&asource))
               const CompareSpecialEnvironment& source = (const CompareSpecialEnvironment&) asource;
               rResult |= source.rResult;
            }
      };

#define DefineNewValue(TypeOperation)                                                              \
      QueryOperation& setNew##TypeOperation() { VirtualQueryOperation::setDomain(); mergeTypeDomainField(TD##TypeOperation); return *this; }

      DefineNewValue(MultiBitConstant)
      DefineNewValue(Top)
      DefineNewValue(True)
      DefineNewValue(False)

#undef DefineNewValue
   };
   static bool queryGuardAll(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment&);

   class Methods : public VirtualElement::Methods {
     public:
      static PPVirtualElement newMultiBitConstant(const VirtualElement& element, const VirtualElement::Init& init)
         {  VirtualQueryOperation::DomainEnvironment env;
            env.setInit(init);
            element.query(QueryOperation().setNewMultiBitConstant(), env);
            return env.presult();
         }

#define DefineNewValue(TypeOperation)                                                              \
      static PPVirtualElement new##TypeOperation(const VirtualElement& element)                    \
         {  VirtualQueryOperation::DomainEnvironment env;                                          \
            element.query(QueryOperation().setNew##TypeOperation(), env);                          \
            return env.presult();                                                                  \
         }

      DefineNewValue(Top)
      DefineNewValue(True)
      DefineNewValue(False)

#undef DefineNewValue

      typedef QueryOperation::CompareSpecialEnvironment CompareSpecialEnvironment;
      static CompareSpecialEnvironment compareSpecial(const VirtualElement& element)
         {  CompareSpecialEnvironment result;
            element.query(QueryOperation().setCompareSpecial(), result);
            return result;
         }
   };
};

class BaseAlgorithms {
  public:
   typedef BitElement::QueryOperation QueryOperation;
   static bool queryInverseCastOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
};

} // end of namespace Details

template <class TypeBase>
class TVirtualElement : public TypeBase, public Details::BaseAlgorithms {
  private:
   typedef TypeBase inherited;
   typedef TVirtualElement<TypeBase> thisType;

  protected:
   TVirtualElement(const typename TypeBase::Init& init) : inherited(init) {}

  public:
   TVirtualElement(const thisType& source) = default;
   typedef Details::BitElement::QueryOperation QueryOperation;
};

class ConstantElement;

}} // end of namespace Bit::Approximate

namespace MultiBit { namespace Approximate {

using namespace Scalar::Approximate;
using VirtualElement = Scalar::Approximate::VirtualElement;
using PPVirtualElement = Scalar::Approximate::PPVirtualElement;
using EvaluationEnvironment = Scalar::Approximate::EvaluationEnvironment;

namespace Details {

using IntOperationElement = Scalar::Approximate::Details::IntOperationElement;

class BaseAlgorithms {
  public:
   typedef MultiBit::Operation Operation;
   typedef Operation::Type TypeOperation;
   typedef MultiBit::ReduceOperation ReduceOperation;
   typedef MultiBit::ConcatOperation ConcatOperation;
   typedef MultiBit::BitSetOperation BitSetOperation;
   typedef Scalar::Approximate::Details::IntOperationElement IntOperationElement;
   typedef IntOperationElement::Methods Methods;
   typedef IntOperationElement::QueryOperation QueryOperation;
   typedef EvaluationEnvironment::Init EvaluationParameters;
   typedef VirtualElement::Methods::EvaluationApplyParameters EvaluationApplyParameters;
   typedef ConstraintEnvironment::Argument Argument;

   class ConstraintConstants;
   class ConstraintDomainTraits;
   class IGenericConstraintTable;
   class GenericConstraintTable {
     private:
      IGenericConstraintTable* pTable;

     public:
      GenericConstraintTable();
      ~GenericConstraintTable();
      bool execute(VirtualElement& top, const Operation& operation, const VirtualElement& result, ConstraintEnvironment& env);
   };
   static GenericConstraintTable gctGenericContraintTable;

   static bool constraintConcat(VirtualElement&, const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment&);
   static bool constraintExtendWithZero(VirtualElement&, const VirtualOperation&, const VirtualElement& result, ConstraintEnvironment&);
   static bool constraintExtendWithSign(VirtualElement&, const VirtualOperation&, const VirtualElement& result, ConstraintEnvironment&);
   static bool constraintReduce(VirtualElement&, const VirtualOperation&, const VirtualElement& result, ConstraintEnvironment&);
   static bool constraintBitSet(VirtualElement&, const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment&);

   template <class OperationTraits>
   static bool constraintMinOperationAssign(OperationTraits traits, VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment& env);
   template <class OperationTraits>
   static bool constraintMaxOperationAssign(OperationTraits traits, VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment& env);
   template <class OperationTraits>
   static bool constraintPlusOperationAssign(OperationTraits traits, VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment& env);
   template <class OperationTraits>
   static bool constraintPlusConstantOperationAssign(OperationTraits traits, VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment::Argument acArg, ConstraintEnvironment& env);
   template <class OperationTraits>
   static bool constraintMinusOperationAssign(OperationTraits traits, VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment& env);
   template <class OperationTraits>
   static bool constraintMinusConstantOperationAssign(OperationTraits traits, VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment::Argument acArg, ConstraintEnvironment& env);
   template <class OperationTraits>
   static bool constraintTimesOperationAssign(OperationTraits traits, VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment& env);
   template <class OperationTraits>
   static bool constraintDivideOperationAssign(OperationTraits traits, VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment& env);
   template <class OperationTraits>
   static bool constraintModuloOperationAssign(OperationTraits traits, VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment& env);

   class SignedOperationTraits {
     public:
      typedef CompareLessSignedOperation LessOperation;
      typedef CompareLessOrEqualSignedOperation LessOrEqualOperation;
      typedef CompareEqualOperation EqualOperation;
      typedef CompareGreaterSignedOperation GreaterOperation;
      typedef CompareGreaterOrEqualSignedOperation GreaterOrEqualOperation;
      typedef MinusSignedAssignOperation MinusOperation;
      typedef PlusSignedAssignOperation PlusOperation;
      typedef TimesSignedAssignOperation TimesOperation;
      typedef DivideSignedAssignOperation DivideOperation;
      typedef NextSignedAssignOperation NextOperation;
      typedef PrevSignedAssignOperation PrevOperation;
      static bool isSigned() { return true; }
      static IntOperationElement::CompareSpecialEnvironment
         compareSpecial(const VirtualElement& element)
         {  return Methods::compareSpecial(element); }
   };
   class UnsignedOperationTraits {
     public:
      typedef CompareLessUnsignedOperation LessOperation;
      typedef CompareLessOrEqualUnsignedOperation LessOrEqualOperation;
      typedef CompareEqualOperation EqualOperation;
      typedef CompareGreaterUnsignedOperation GreaterOperation;
      typedef CompareGreaterOrEqualUnsignedOperation GreaterOrEqualOperation;
      typedef MinusUnsignedAssignOperation MinusOperation;
      typedef PlusUnsignedAssignOperation PlusOperation;
      typedef TimesUnsignedAssignOperation TimesOperation;
      typedef DivideUnsignedAssignOperation DivideOperation;
      typedef NextUnsignedAssignOperation NextOperation;
      typedef PrevUnsignedAssignOperation PrevOperation;
      static bool isSigned() { return false; }
      static IntOperationElement::CompareSpecialEnvironment
         compareSpecial(const VirtualElement& element)
         {  return Methods::compareSpecial(element); }
   };
   class FloatOperationTraits {
     public:
      typedef CompareLessFloatOperation LessOperation;
      typedef CompareLessOrEqualFloatOperation LessOrEqualOperation;
      typedef CompareEqualOperation EqualOperation;
      typedef CompareGreaterFloatOperation GreaterOperation;
      typedef CompareGreaterOrEqualFloatOperation GreaterOrEqualOperation;
      typedef MinusFloatAssignOperation MinusOperation;
      typedef PlusFloatAssignOperation PlusOperation;
   };
   static bool constraintMinUnsignedAssign(VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment& env);
   static bool constraintMinSignedAssign(VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment& env);
   static bool constraintMinFloatAssign(VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment& env);
   static bool constraintMaxUnsignedAssign(VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment& env);
   static bool constraintMaxSignedAssign(VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment& env);
   static bool constraintMaxFloatAssign(VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment& env);
   static bool constraintPlusUnsignedAssign(VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment& env);
   static bool constraintPlusUnsignedWithSignedAssign(VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment& env);
   static bool constraintPlusSignedAssign(VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment& env);
   static bool constraintPlusFloatAssign(VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment& env);
   static bool constraintTimesUnsignedAssign(VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment& env);
   static bool constraintTimesSignedAssign(VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment& env);
   static bool constraintDivideUnsignedAssign(VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment& env);
   static bool constraintDivideSignedAssign(VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment& env);
   static bool constraintModuloUnsignedAssign(VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment& env);
   static bool constraintModuloSignedAssign(VirtualElement& thisElement, const VirtualOperation&,
         VirtualElement& sourceElement, const VirtualElement& result, ConstraintEnvironment& env);

   template <class TypeQueryTable, class TypeElement> static void fillTable(TypeQueryTable& table, TypeElement*);

   static bool queryNextSignedAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNextUnsignedAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryPrevSignedAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryPrevUnsignedAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinusUnsignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinusSignedAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinusUnsignedAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryPlusSignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryPlusUnsignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryPlusSignedAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryPlusUnsignedAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryDivideSignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryDivideUnsignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryDivideSignedAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryDivideUnsignedAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryTimesSignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryTimesUnsignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryTimesSignedAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryTimesUnsignedAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryModuloSignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryModuloUnsignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryModuloSignedAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryModuloUnsignedAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryRightShiftSignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryRightShiftUnsignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryRightShiftSignedAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryRightShiftUnsignedAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinSignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinUnsignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinSignedAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinUnsignedAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMaxSignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMaxUnsignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMaxSignedAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMaxUnsignedAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareGreaterOrEqualSignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareGreaterOrEqualUnsignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareGreaterSignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareGreaterUnsignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareLessOrEqualSignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareLessOrEqualUnsignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareLessSignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareLessUnsignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);

   static bool queryInverseCastOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNextAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryPrevAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinusOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinusSignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinusAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryPlusOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryPlusAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryDivideOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryDivideAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryTimesOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryTimesAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryModuloAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryModuloOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryLeftShiftOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryLeftShiftAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryRightShiftOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryRightShiftAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryInverseRotateAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryBitNegateOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryBitNegateAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryBitOrAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryBitOrOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryBitAndAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryBitAndOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryBitExclusiveOrAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryBitExclusiveOrOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryOppositeOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryOppositeAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMaxAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMaxOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareGreaterOrEqualOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareGreaterOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareLessOrEqualOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareLessOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareEqualOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareDifferentOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
};

} // end of namespace Details

template <class TypeBase>
class TVirtualElement : public TypeBase,  public Details::BaseAlgorithms {
  private:
   typedef TypeBase inherited;
   typedef TVirtualElement<TypeBase> thisType;

  protected:
   TVirtualElement(const typename TypeBase::Init& init) : inherited(init) {}

  public:
   TVirtualElement(const thisType& source) = default;

   typedef MultiBit::Operation Operation;
   typedef MultiBit::Operation::Type TypeOperation;
   typedef MultiBit::ReduceOperation ReduceOperation;
   typedef MultiBit::ConcatOperation ConcatOperation;
   typedef MultiBit::BitSetOperation BitSetOperation;
   typedef Scalar::Approximate::Details::IntOperationElement IntOperationElement;
   typedef IntOperationElement::Methods Methods;
   typedef IntOperationElement::QueryOperation QueryOperation;
   typedef EvaluationEnvironment::Init EvaluationParameters;
   typedef ConstraintEnvironment::Argument Argument;

   class QueryTable : public TypeBase::QueryTable {
     private:
      typedef typename TypeBase::QueryTable inherited;

     public:
      QueryTable();
      QueryTable(const QueryTable& source) = default;
   };
   friend class QueryTable;
};

template <class TypeMultiTraits>
class TConstantElement;

class Interval;

}} // end of namespace MultiBit::Approximate

namespace Integer { namespace Approximate {

using namespace Scalar::Approximate;
using VirtualElement = Scalar::Approximate::VirtualElement;
using PPVirtualElement = Scalar::Approximate::PPVirtualElement;
using EvaluationEnvironment = Scalar::Approximate::EvaluationEnvironment;

namespace Details {

using IntOperationElement = Scalar::Approximate::Details::IntOperationElement;

class BaseAlgorithms {
  public:
   template <class TypeQueryTable, class TypeElement> static void fillTable(TypeQueryTable& table, TypeElement*);

   typedef Integer::Operation Operation;
   typedef Operation::Type TypeOperation;
   typedef Scalar::Approximate::Details::IntOperationElement IntOperationElement;
   typedef IntOperationElement::QueryOperation QueryOperation;

   static bool queryNextAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryPrevAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinusOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinusSignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinusAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryPlusOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryPlusAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryDivideOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryDivideAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryTimesOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryTimesAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryModuloAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryModuloOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryLeftShiftOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryLeftShiftAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryRightShiftOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryRightShiftAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryInverseRotateAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryBitNegateOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryBitNegateAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryBitOrAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryBitOrOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryBitAndAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryBitAndOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryBitExclusiveOrAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryBitExclusiveOrOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryOppositeOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryOppositeAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMaxAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMaxOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareGreaterOrEqualOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareGreaterOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareLessOrEqualOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareLessOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareEqualOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareDifferentOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
};

} // end of namespace Details

template <class TypeBase>
class TVirtualElement : public TypeBase, public Details::BaseAlgorithms {
  private:
   typedef TypeBase inherited;
   typedef TVirtualElement<TypeBase> thisType;

  protected:
   TVirtualElement(const typename TypeBase::Init& init) : inherited(init) {}

  public:
   TVirtualElement(const thisType& source) = default;

   typedef Integer::Operation Operation;
   typedef Operation::Type TypeOperation;
   typedef typename TypeBase::QueryOperation QueryOperation;

   class QueryTable : public TypeBase::QueryTable {
     private:
      typedef typename TypeBase::QueryTable inherited;

     public:
      QueryTable();
      QueryTable(const QueryTable& source) = default;
   };
   friend class QueryTable;
};

class CharElement;
class SignedCharElement;
class UnsignedCharElement;
class ShortElement;
class UnsignedShortElement;
class IntElement;
class UnsignedIntElement;
class LongElement;
class UnsignedLongElement;

class Interval;

}} // end of namespace Integer::Approximate

namespace Floating { namespace Approximate {

using namespace Scalar::Approximate;
using VirtualElement = Scalar::Approximate::VirtualElement;
using PPVirtualElement = Scalar::Approximate::PPVirtualElement;
using EvaluationEnvironment = Scalar::Approximate::EvaluationEnvironment;

namespace Details {

using RealOperationElement = Scalar::Approximate::Details::RealOperationElement;

class BaseAlgorithms {
  public:
   template <class TypeQueryTable, class TypeElement> static void fillTable(TypeQueryTable& table, TypeElement*);

   typedef Floating::Operation Operation;
   typedef Scalar::Approximate::Details::RealOperationElement RealOperationElement;
   typedef RealOperationElement::QueryOperation QueryOperation;

   static bool queryMinusOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinusSignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinusAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareLessOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareLessOrEqualOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareEqualOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareGreaterOrEqualOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareGreaterOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
};

} // end of namespace Details

template <class TypeBase>
class TVirtualElement : public TypeBase, public Details::BaseAlgorithms {
  private:
   typedef TypeBase inherited;
   typedef TVirtualElement<TypeBase> thisType;

  protected:
   TVirtualElement(const typename TypeBase::InitFloat& init) : inherited(init) {}

  public:
   TVirtualElement(const thisType& source) = default;

   typedef Floating::Operation Operation;
   typedef typename TypeBase::QueryOperation QueryOperation;

   static bool constraintPlusAssign(VirtualElement&, const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment&);
   static bool constraintMinusAssign(VirtualElement&, const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment&);
   static bool constraintTimesAssign(VirtualElement&, const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment&);
   static bool constraintDivideAssign(VirtualElement&, const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment&);

   static bool constraintOppositeAssign(VirtualElement&, const VirtualOperation&, const VirtualElement& result, ConstraintEnvironment&);
   class QueryTable : public TypeBase::QueryTable {
     private:
      typedef typename TypeBase::QueryTable inherited;

     public:
      QueryTable();
      QueryTable(const QueryTable& source) : inherited(source) {}
   };
   friend class QueryTable;
};

class FloatElement;
class DoubleElement;
class LongDoubleElement;

class Interval;

template <class TypeMultiTraits>
class TMultiConstantElement;

}} // end of namespace Floating::Approximate

namespace Rational { namespace Approximate {

using namespace Scalar::Approximate;
using VirtualElement = Scalar::Approximate::VirtualElement;
using PPVirtualElement = Scalar::Approximate::PPVirtualElement;
using EvaluationEnvironment = Scalar::Approximate::EvaluationEnvironment;

template <class TypeBase>
class TVirtualElement : public TypeBase {
  private:
   typedef TypeBase inherited;
   typedef TVirtualElement<TypeBase> thisType;

  protected:
   TVirtualElement(const typename TypeBase::Init& init) : inherited(init) {}

  public:
   TVirtualElement(const thisType& source) = default;

   typedef Rational::Operation Operation;
   static bool queryInverseCastOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinusOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinusSignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinusAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareLessOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareLessOrEqualOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareEqualOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareGreaterOrEqualOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareGreaterOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
};

class Interval;
class ContantElement;

}} // end of namespace Rational::Approximate

namespace Real { namespace Approximate {

using namespace Scalar::Approximate;
using VirtualElement = Scalar::Approximate::VirtualElement;
using PPVirtualElement = Scalar::Approximate::PPVirtualElement;
using EvaluationEnvironment = Scalar::Approximate::EvaluationEnvironment;

template <class TypeBase>
class TVirtualElement : public TypeBase {
  private:
   typedef TypeBase inherited;
   typedef TVirtualElement<TypeBase> thisType;

  protected:
   TVirtualElement(const typename TypeBase::Init& init) : inherited(init) {}

  public:
   TVirtualElement(const thisType& source) = default;

   typedef Real::Operation Operation;
   static bool queryInverseCastOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinusOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinusSignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryMinusAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareLessOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareLessOrEqualOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareEqualOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareGreaterOrEqualOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryCompareGreaterOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
};

class Interval;

}} // end of namespace Real::Approximate

namespace Approximate {

class Disjunction;
class Top;
class OperationElement;
class Conjunction;

}}} // end of namespace Analyzer::Scalar::Approximate

#endif // Analyzer_Scalar_Approximate_VirtualElementH

