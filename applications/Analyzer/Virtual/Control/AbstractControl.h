/////////////////////////////////
//
// Library   : Analyzer/Virtual
// Unit      : Control
// File      : AbstractControl.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a base class for the hierarchy of control points and instructions.
//

#ifndef Analyzer_Control_AbstractControlH
#define Analyzer_Control_AbstractControlH

#include "Analyzer/Virtual/Control/AbstractStateControl.h"

namespace Analyzer {

namespace Iteration { class AbstractState; }

namespace Control {

/*****************************************/
/* Definition of the class AbstractPoint */
/*****************************************/

class AbstractPoint : public STG::IOObject {
  public:
   typedef COL::TVirtualCollection<AbstractPoint> PointsCollection;
   typedef Analyzer::Direction Direction;

   class Registration {
     private:
      int uLocalIndex;

     public:
      Registration() : uLocalIndex(-1) {}
      class Record {};
      Registration(Record) : uLocalIndex(0) {}
      Registration(const Registration& source) = default;
      Registration& operator=(const Registration& source) = default;

      const int& getLocalIndex() const { AssumeCondition(uLocalIndex > 0) return uLocalIndex; }
      void setLocalIndex(int index)
         {  AssumeCondition((index > 0) && (uLocalIndex == 0))
            uLocalIndex = index;
         }
      void incIndex() { AssumeCondition(uLocalIndex > 0) uLocalIndex++; }
      void decIndex() { AssumeCondition(uLocalIndex > 1) uLocalIndex--; }

      bool isValid() const { return uLocalIndex > 0; }
      bool canRecord() const { AssumeCondition(uLocalIndex <= 0) return uLocalIndex == 0; }
      ComparisonResult compare(const Registration& source) const
         {  AssumeCondition(isValid() && source.isValid())
            return fcompare(uLocalIndex, source.uLocalIndex);
         }
   };

  private:
   typedef STG::IOObject inherited;

  protected:
   virtual ComparisonResult _compare(const EnhancedObject& asource) const override
      {  return inherited::_compare(asource); }
   virtual bool _isEqual(const AbstractPoint& source) const
      {  AssumeUncalled return true; }

   AbstractPoint() {}
   AbstractPoint(const AbstractPoint& source) = default;

  public:
   DefineCopy(AbstractPoint)

   virtual bool retrieveFormalNexts(const Direction& direction,
         Iteration::AbstractState& iterationState) const { return false; }
   virtual const Registration* getRegistration() const { return nullptr; }

   bool isEqual(const AbstractPoint& source) const { return _isEqual(source); }
};

typedef COL::TVirtualCollection<AbstractPoint> PointsCollection;

/******************************************/
/* Definition of the class ExecutionState */
/******************************************/

class ExecutionState : public COL::List::Node {
  private:
   typedef COL::List::Node inherited;
   const AbstractPoint* pacpPoint;
   typedef PNT::PassPointer<Memory::AbstractState> PPMemoryState;
   PPMemoryState ppmsMemory;

  public:
   class DuplicateParameters {
     private:
      enum Mode { MCptOnly, MAll };
      Mode mMode;
      
     public:
      DuplicateParameters() : mMode(MAll) {}
      DuplicateParameters(const DuplicateParameters& source) = default;
      DuplicateParameters& operator=(const DuplicateParameters& source) = default;
      DuplicateParameters& setOnlyPoint() { mMode = MCptOnly; return *this; }
      DuplicateParameters& setAll() { mMode = MAll; return *this; }
      bool isAll() const { return mMode == MAll; }
   };

   ExecutionState() : inherited(), pacpPoint(nullptr), ppmsMemory() {}
   ExecutionState(const AbstractPoint& point) : pacpPoint(&point) {}
   ExecutionState(const ExecutionState& source, const DuplicateParameters& params)
      :  inherited(source), pacpPoint(source.pacpPoint)
      {  if (params.isAll())
            ppmsMemory.fullAssign(source.ppmsMemory);
      }
   ExecutionState(const ExecutionState& source)
      :  inherited(source), pacpPoint(nullptr), ppmsMemory() {}
   ExecutionState& operator=(const ExecutionState& source) { return (ExecutionState&) inherited::operator=(source); }

   DefineCopy(ExecutionState)
   DDefineAssign(ExecutionState)

   void setFrom(const ExecutionState& source)
      {  AssumeCondition(!pacpPoint && !ppmsMemory.isValid())
         pacpPoint = source.pacpPoint;
         ppmsMemory.fullAssign(source.ppmsMemory);
      }
   PPMemoryState& pmemory() { return ppmsMemory; }
   const PPMemoryState& pmemory() const { return ppmsMemory; }
   Memory::AbstractState& memory() const { return *ppmsMemory; }
   bool hasMemory() const { return ppmsMemory.isValid(); }
   void absorbState(Memory::AbstractState* state) { ppmsMemory.absorbElement(state); }

   void clear() { pacpPoint = nullptr; ppmsMemory.release(); }
   void freeControlPoint()
      {  if (pacpPoint != nullptr) delete pacpPoint;
         pacpPoint = nullptr;
      }
   bool hasControlPoint() const { return pacpPoint != nullptr; }
   const AbstractPoint& controlPoint() const {  AssumeCondition(pacpPoint) return *pacpPoint; }
   void setControlPoint(const AbstractPoint& point) { pacpPoint = &point; }
   virtual bool isNewDestination() const { return false; }
};

class MultiExecutionState : public COL::TCopyCollection<COL::TList<ExecutionState> > {
  private:
   typedef COL::TCopyCollection<COL::TList<ExecutionState> > inherited;

  public:
   class Duplicate {};
   MultiExecutionState() {}
   MultiExecutionState(const MultiExecutionState& source) = default;
   MultiExecutionState(const MultiExecutionState& source, Duplicate) : inherited(source)
      {  Cursor thisCursor(*this), sourceCursor(source);
         if (thisCursor.setToFirst()) {
            do {
               sourceCursor.setToNext();
               thisCursor.elementSAt().setFrom(sourceCursor.elementAt());
            } while (thisCursor.setToNext());
            sourceCursor.setToNext();
         };
         AssumeCondition(!sourceCursor.isValid())
      }
   DefineCopy(MultiExecutionState)

   void add(ExecutionState* state, const ExtendedInsertionParameters& parameters, Cursor* cursor=nullptr)
      {  return inherited::add(state, parameters, cursor); }

   bool hasUnique() const { return inherited::count() == 1; }
   void removeEmpty();
   typedef Memory::AbstractState::MergeParameters MergeParameters;
   void mergeAndAbsorbOrigin(MultiExecutionState& mesSource, MergeParameters& mpParams);
   void mergeAndAbsorb(MultiExecutionState& mesSource, MergeParameters& mpParams);
   void clear() { foreachSDo([](ExecutionState& state){ state.pmemory().release(); return true; }); }
};

/***************************************/
/* Definition of the class Instruction */
/***************************************/

class AbstractInstruction : public AbstractPoint {
  private:
   typedef AbstractPoint inherited;

  protected:
   AbstractInstruction() {}
   AbstractInstruction(const AbstractInstruction& source) = default;

  public:
   DefineCopy(AbstractInstruction)

   typedef Memory::AbstractState::MergeParameters MergeParameters;
   typedef Memory::AbstractState::InterpretParameters InterpretParameters;
   virtual bool retrieveFormalNexts(const Direction& direction, Iteration::AbstractState& iterState) const override;
   virtual void interpret(MultiExecutionState& origins, MultiExecutionState& destinations,
         InterpretParameters& params) const;
};

} // end of namespace Control

namespace Iteration {

/*****************************************/
/* Definition of the class AbstractState */
/*****************************************/
/* synchronized iteration states         */

class AbstractState : public STG::IOObject {
  public:
   class MergeParameters : public Memory::AbstractState::MergeParameters {
     private:
      typedef Memory::AbstractState::MergeParameters inherited;

     protected:
      DefineExtendedParameters(3, inherited)
      DefineSubExtendedParameters(Direction, 1, INHERITED)
      DefineSubExtendedParameters(SupportLabel, 1, Direction)
      DefineSubExtendedParameters(FreeControlPoint, 1, SupportLabel)

     public:
      MergeParameters(const Direction& direction) { mergeDirectionField(direction.implementation()); }
      bool isForward() const { return Direction((Direction::Type) queryDirectionField()).isForward(); }
      bool isBackward() const { return Direction((Direction::Type) queryDirectionField()).isBackward(); }
      
      MergeParameters& setForward() { clearDirectionField(); return *this; }
      MergeParameters& setBackward() { mergeDirectionField(1); return *this; }
      bool doesSupportLabel() const { return hasSupportLabelField(); }
      bool doesFreeControlPoint() const { return hasFreeControlPointField(); }
      MergeParameters& setSupportLabel() { mergeSupportLabelField(1); return *this; }
      MergeParameters& clearSupportLabel() { clearSupportLabelField(); return *this; }
      MergeParameters& setFreeControlPoint() { mergeFreeControlPointField(1); return *this; }
      MergeParameters& clearFreeControlPoint() { clearFreeControlPointField(); return *this; }
   };
   
   class InterpretParameters : public Memory::AbstractState::InterpretParameters {
     private:
      typedef Memory::AbstractState::InterpretParameters inherited;
      
     public:
      InterpretParameters() {}
      InterpretParameters(const InterpretParameters& source) = default;
      InterpretParameters& operator=(const InterpretParameters& source) = default;
      typedef AbstractState::MergeParameters MergeParameters;
      MergeParameters& mergeParameters() { return (MergeParameters&) getMergeParameters(); }
   };
   
  private:
   typedef STG::IOObject inherited;
   InterpretParameters* pipInterpretParameters;
   Control::MultiExecutionState        cmesOrigin;       // origin execution state
   const Control::AbstractInstruction* pciInstruction;   // instruction to execute
   Control::MultiExecutionState        cmesDestination;  // destination execution state

  protected:
   virtual ComparisonResult _compare(const EnhancedObject& asource) const override
      {  ComparisonResult result = inherited::_compare(asource);
         const AbstractState& source = (const AbstractState&) asource;
         AssumeCondition(pciInstruction && source.pciInstruction)
         return (result == CREqual) ? pciInstruction->compare(*source.pciInstruction) : result;
      }
   virtual bool isCompatible(const AbstractState& state) const
      {  AssumeCondition(pciInstruction && state.pciInstruction)
         return pciInstruction->isEqual(*state.pciInstruction);
      }

  public:
   AbstractState(InterpretParameters& interpretParameters, Memory::AbstractState* memoryState)
      :  pipInterpretParameters(&interpretParameters), pciInstruction(nullptr)
      {  Control::ExecutionState* firstState = new Control::ExecutionState();
         origins().insertNewAtEnd(firstState);
         firstState->pmemory().absorbElement(memoryState);
      }
   AbstractState(InterpretParameters& interpretParameters, const Control::AbstractPoint& origin,
         Control::ExecutionState& state)
      :  pipInterpretParameters(&interpretParameters), pciInstruction(nullptr)
      { Control::ExecutionState* newState = state.createSCopy();
        newState->setControlPoint(origin);
        cmesOrigin.addNew(newState);
        newState->pmemory() = state.pmemory();
      }
   AbstractState(const AbstractState& source) = default;
   AbstractState& operator=(const AbstractState& source)
      {  inherited::operator=(source);
         cmesOrigin = source.cmesOrigin;
         cmesDestination = source.cmesDestination;
         return *this;
      }

   DefineCopy(AbstractState)
   DDefineAssign(AbstractState)
   InterpretParameters& interpretParameters() const { return *pipInterpretParameters; }
   void changeInterpretParameters(InterpretParameters& parameters) { pipInterpretParameters = &parameters; }
   virtual bool merge(AbstractState& source, MergeParameters& params);

   void shiftOrigin(const Control::AbstractPoint& newOrigin)
      {  AssumeCondition(cmesOrigin.count() == 1)
         cmesOrigin.getSFirst().setControlPoint(newOrigin);
      }
   const Control::AbstractPoint& getOriginPoint() const
      {  return cmesOrigin.getFirst().controlPoint(); }
   Memory::AbstractState& getOriginMemory() const
      {  return cmesOrigin.getFirst().memory(); }
   Memory::AbstractState& getDestinationMemory() const
      {  return cmesDestination.getFirst().memory(); }

   void addDestination(const Control::AbstractPoint& point)
      { Control::ExecutionState* newState = origins().getFirst().createSCopy();
        newState->setControlPoint(point);
        cmesDestination.addNew(newState);
      }
   void setInstruction(const Control::AbstractInstruction& instruction) { pciInstruction = &instruction; }
   bool hasInstruction() const { return pciInstruction; }
   void swapDestinations(AbstractState& source)
      {  const Control::AbstractInstruction* temp = pciInstruction;
         pciInstruction = source.pciInstruction;
         source.pciInstruction = temp;
         cmesDestination.swap(source.cmesDestination);
      } 
   void freeInstruction()
      {  if (pciInstruction)
            delete pciInstruction;
         pciInstruction = nullptr;
      }
   const Control::MultiExecutionState& destinations() const { return cmesDestination; }
   const Control::MultiExecutionState& origins() const { return cmesOrigin; }
   Control::MultiExecutionState& destinations() { return cmesDestination; }
   Control::MultiExecutionState& origins() { return cmesOrigin; }

   // interpretation methods
   void interpret()
      {  AssumeCondition(pciInstruction)
         pciInstruction->interpret(cmesOrigin, cmesDestination, *pipInterpretParameters);
      }
   const Control::AbstractInstruction& getInstruction() const
      {  AssumeCondition(pciInstruction) return *pciInstruction; }

   // advance methods
   bool isTerminated() const { return cmesDestination.isEmpty(); }

   const Control::AbstractPoint* advance(PNT::PassPointer<AbstractState>& copy);
   void retrieveFormalNexts(const Control::AbstractPoint& destination, const Direction& direction)
      {  AssumeCondition(!pciInstruction && cmesDestination.isEmpty())
         bool result = destination.retrieveFormalNexts(direction, *this);
         AssumeCondition(result)
      }
}; 

} // end of namespace Iteration

namespace Control {

inline bool
AbstractInstruction::retrieveFormalNexts(const Direction& direction, Iteration::AbstractState& iterState) const {
   inherited::retrieveFormalNexts(direction, iterState);
   iterState.setInstruction(*this);
   return false;
}
   
}} // end of namespace Analyzer::Control

#endif // Analyzer_Control_AbstractControlH

