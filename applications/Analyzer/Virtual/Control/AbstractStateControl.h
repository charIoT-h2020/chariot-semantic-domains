/////////////////////////////////
//
// Library   : Analyzer/Virtual
// Unit      : Control
// File      : AbstractStateControl.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a base class for the hierarchy of memory states
//

#ifndef Analyzer_Control_AbstractStateControlH
#define Analyzer_Control_AbstractStateControlH

#include "Collection/Collection.hpp"
#include "StandardClasses/Persistence.h"

namespace Analyzer {

class Direction : protected ExtendedParameters {
  public:
   enum Type { DForward=0, DBackward=1, DEnd };

  protected:
   DefineExtendedParameters(1, ExtendedParameters)
      
  public:
   Direction() : ExtendedParameters() {}
   Direction(Type direction)
      {  AssumeCondition(direction < DEnd)
         mergeOwnField(direction);
      }
   Direction(const Direction& source) = default;
   Direction& operator=(const Direction& source) = default;

   Direction& setForward() { clearOwnField(); return *this; }
   Direction& setBackward() { mergeOwnField(1); return *this; }
   Direction& setInverse() { setOwnField(1-queryOwnField()); return *this; }
   Direction getInverse() const
      {  Direction result;
         result.setOwnField(1-queryOwnField());
         return result;
      }
   unsigned int implementation() const { return (unsigned int) queryOwnField(); }
   bool isForward() const { return !hasOwnField(); }
   bool isBackward() const { return hasOwnField(); }
};

namespace Control {

class AbstractInstruction;

/****************************************/
/* Definition of the class AbstractType */
/****************************************/

class AbstractType : public STG::IOObject, protected ExtendedParameters {
  private:
   typedef STG::IOObject inherited;

  protected:
   DefineExtendedParameters(1, ExtendedParameters)
   void setGeneric() { mergeOwnField(1); }
   virtual int _getSize() const { return 0; }
   
  public:
   AbstractType() {}
   AbstractType(const AbstractType& source) = default;
   DefineCopy(AbstractType)
   DDefineAssign(AbstractType)
   
   int getSize() const { return _getSize(); }
   bool isGeneric() const { return hasOwnField(); }
};

class GenericTypeSize : public AbstractType {
  private:
   typedef AbstractType inherited;
   int uBitSize;
   
  protected:
   virtual int _getSize() const override { return uBitSize; }
      
  public:
   GenericTypeSize(int bitSize) : uBitSize(bitSize) { setGeneric(); }
   GenericTypeSize(const GenericTypeSize& source) = default;
   DefineCopy(GenericTypeSize)
   DDefineAssign(GenericTypeSize)
   
   const int& getSize() const { return uBitSize; }
};

} // end of namespace Control

namespace Memory {

class AbstractState : public STG::IOObject {
  public:
   class MergeParameters : protected ExtendedParameters {
     private:
      int uHits;

     protected:
      enum Verdict { VConst=0, VApply=1, VApplyWithLoss=2, VNoApply=3 };
      DefineExtendedParameters(6, ExtendedParameters)
      DefineSubExtendedParameters(ConstOperation, 1, INHERITED)
      DefineSubExtendedParameters(Intersection, 1, ConstOperation)
      DefineSubExtendedParameters(ForceStable, 1, Intersection)
      DefineSubExtendedParameters(Verdict, 2, ForceStable)
      DefineSubExtendedParameters(SwapArgResult, 1, Verdict)
         
     public:
      MergeParameters() : uHits(0) {}
      MergeParameters(const MergeParameters& source) : ExtendedParameters(source), uHits(source.uHits) {}
      
      MergeParameters& setConstOperation() { mergeConstOperationField(1); return *this; }
      MergeParameters& clearConstOperation() { clearConstOperationField(); return *this; }
      bool isConstOperation() const { return hasConstOperationField(); }

      MergeParameters& setIntersection() { mergeIntersectionField(1); return *this; }
      MergeParameters& clearIntersection() { clearIntersectionField(); return *this; }
      bool isMergeOperation() const { return !hasIntersectionField(); }
      bool isIntersectionOperation() const { return hasIntersectionField(); }
      MergeParameters& setWidening(int hits)
         {  uHits = hits; clearIntersectionField(); mergeForceStableField(1); return *this; }
      MergeParameters& setNarrowing(int hits)
         {  uHits = hits; mergeIntersectionField(1); mergeForceStableField(1); return *this; }
      bool isWideningOperation() const { return !hasIntersectionField() && hasForceStableField(); }
      bool isNarrowingOperation() const { return hasIntersectionField() && hasForceStableField(); }
      const int& getHits() const { return uHits; }
      
      void setSwapArgResult() { mergeSwapArgResultField(1); }
      bool doesSwapArgResult() const { return hasSwapArgResultField(); }
      void clearSwapArgResult() { clearSwapArgResultField(); }

      MergeParameters& clearVerdict() { clearVerdictField(); return *this; }
      void setApply() {  if (queryVerdictField() != VApplyWithLoss) mergeVerdictField(VApply); }
      void setApplyWithLoss()
         {  if (queryVerdictField() != VApply) mergeVerdictField(VApplyWithLoss); else setVerdictField(VApplyWithLoss); }
      void setNoApply() { mergeVerdictField(VNoApply); }
      
      bool doesContain() const { return !hasVerdictField(); }
      bool hasApplied() const { return queryVerdictField() <= VApplyWithLoss; }
      bool hasLostInformation() const { return queryVerdictField() == VApplyWithLoss; }
      bool hasRefusedApply() const { return queryVerdictField() == VNoApply; }
   };

   class InterpretParameters : public EnhancedObject, public Direction {
     public:
      InterpretParameters() {}
      InterpretParameters(const Direction& direction) : Direction(direction) {}
      InterpretParameters(const InterpretParameters& source) : Direction(source) {}
      DefineCopy(InterpretParameters)
      DDefineAssign(InterpretParameters)
      
      virtual MergeParameters& getMergeParameters()
         {  AssumeUncalled MergeParameters* result = nullptr; return *result; }
      // merge parameters that are used for the local interpretation
   };

  private:
   typedef STG::IOObject inherited;

  public:
   AbstractState() {}
   AbstractState(const AbstractState& source) : inherited(source) {}
   DefineCopy(AbstractState)

   virtual bool _mergeWith(AbstractState& source, MergeParameters& params) { return false; }
  
   bool canMerge(AbstractState& source, MergeParameters& params)
      {  bool result = _mergeWith(source, params.setConstOperation().clearVerdict());
         AssumeCondition(result)
         return params.hasApplied();
      }
   bool mergeWith(AbstractState& source, MergeParameters& params)
      {  bool result = _mergeWith(source, params.clearConstOperation().clearVerdict());
         AssumeCondition(result)
         return params.hasApplied();
      }
   bool contain(const AbstractState& source, MergeParameters& params) const
      {  bool result = const_cast<AbstractState&>(*this)._mergeWith(
            const_cast<AbstractState&>(source), (MergeParameters&) params.clearConstOperation().clearVerdict());
         AssumeCondition(result)
         return params.doesContain();
      }
   bool intersectWith(AbstractState& source, MergeParameters& params)
      {  bool result = _mergeWith(source, params.clearConstOperation().clearVerdict().setIntersection());
         AssumeCondition(result)
         return params.hasApplied();
      }
   bool widenWith(AbstractState& source, int hits, MergeParameters& params)
      {  bool result = _mergeWith(source, params.clearConstOperation().clearVerdict().setWidening(hits));
         AssumeCondition(result)
         return params.hasApplied();
      }
   bool narrowWith(AbstractState& source, int hits, MergeParameters& params)
      {  bool result = _mergeWith(source, params.clearConstOperation().clearVerdict().setNarrowing(hits));
         AssumeCondition(result)
         return params.hasApplied();
      }
   bool mergeWithTo(AbstractState& source, MergeParameters& params) { return source.mergeWith(*this, params); }
   bool intersectWithTo(AbstractState& source, MergeParameters& params) { return source.intersectWith(*this, params); }
   bool widenWithTo(AbstractState& source, int hits, MergeParameters& params) { return source.widenWith(*this, hits, params); }
   bool narrowWithTo(AbstractState& source, int hits, MergeParameters& params) { return source.narrowWith(*this, hits, params); }

   virtual bool interpret(const Control::AbstractInstruction& point, InterpretParameters& params)
      {  AssumeUncalled return true; } 
};

}} // end of namespace Analyzer::Memory

#endif // Analyzer_Control_AbstractStateControlH

