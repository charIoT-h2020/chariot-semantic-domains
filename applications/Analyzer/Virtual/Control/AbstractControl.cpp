/////////////////////////////////
//
// Library   : Analyzer/Virtual
// Unit      : Control
// File      : AbstractControl.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of a base class for the hierarchy of control points and instructions.
//

#include "Analyzer/Virtual/Control/AbstractControl.h"

namespace Analyzer {}

#include "Collection/Collection.template"

namespace Analyzer { namespace Control {

void
MultiExecutionState::removeEmpty() {
   Cursor cursor(*this);
   if (cursor.setToFirst()) {
      bool doesRemovePrevious = !cursor.elementAt().pmemory().isValid();
      bool doesContinue;
      do {
         doesContinue = cursor.setToNext();
         if (doesRemovePrevious)
            freePrevious(cursor);
         if (doesContinue)
            doesRemovePrevious = !cursor.elementAt().pmemory().isValid();
      } while (doesContinue);
   };
}

void
MultiExecutionState::mergeAndAbsorb(MultiExecutionState& source, MergeParameters& params) {
   while (!source.isEmpty()) {
      PNT::AutoPointer<ExecutionState> currentSource(&source.extractFirst(), PNT::Pointer::Init());
      if (foreachSDo([&currentSource, &params](ExecutionState& currentThis)
         {  bool result = (&currentThis.controlPoint() != &currentSource->controlPoint())
                  && !currentThis.controlPoint().isEqual(currentSource->controlPoint());
            if (!result) {
               if (currentSource->pmemory().isValid()) {
                  if (currentThis.pmemory().isValid())
                     result = !currentThis.memory().mergeWith(currentSource->memory(), params);
                  else
                     currentThis.pmemory() = currentSource->pmemory();
               };
               if (((Iteration::AbstractState::MergeParameters&) params).doesFreeControlPoint()
                     && (&currentThis.controlPoint() != &currentSource->controlPoint())
                     && currentSource->isNewDestination())
                  currentSource->freeControlPoint();
            };
            return result;
         }))
         inherited::insertNewAtEnd(currentSource.extractElement());
   };
}

void
MultiExecutionState::mergeAndAbsorbOrigin(MultiExecutionState& source, MergeParameters& params) {
   while (!source.isEmpty()) {
      PNT::AutoPointer<ExecutionState> currentSource = source.extractFirst();
      if (foreachSDo([&currentSource, &params](ExecutionState& currentThis)
         {  bool result = (&currentThis.controlPoint() != &currentSource->controlPoint())
                  && !currentThis.controlPoint().isEqual(currentSource->controlPoint());
            if (!result && ((Iteration::AbstractState::MergeParameters&) params).doesSupportLabel()) {
               if (currentSource->pmemory().isValid()) {
                  if (currentThis.pmemory().isValid())
                     result = !currentThis.memory().mergeWith(currentSource->memory(), params);
                  else
                     currentThis.pmemory() = currentSource->pmemory();
               };
               if (((Iteration::AbstractState::MergeParameters&) params).doesFreeControlPoint()
                     && (&currentThis.controlPoint() != &currentSource->controlPoint()))
                  currentSource->freeControlPoint();
            };
            return result || !((Iteration::AbstractState::MergeParameters&) params).doesSupportLabel();
         }))
         inherited::insertNewAtEnd(currentSource.extractElement());
   };
}

/***************************************************/
/* Implementation of the class AbstractInstruction */
/***************************************************/

void
AbstractInstruction::interpret(MultiExecutionState& origins, MultiExecutionState& destinations,
      InterpretParameters& params) const {
   if (destinations.count() == 0) {
      origins.clear();
      return;
   };

   COL::TCopyCollection<COL::TArray<Memory::AbstractState> > memoryStates;
   MergeParameters& mergeParameters = params.getMergeParameters();
   origins.foreachSDo([&memoryStates, &mergeParameters](ExecutionState& state)
      {  PNT::PassPointer<Memory::AbstractState> newState(state.memory());
         if (memoryStates.foreachSDo([&newState, &mergeParameters](Memory::AbstractState& memoryState)
            {  return !memoryState.mergeWith(*newState, mergeParameters); }))
            memoryStates.insertNewAtEnd(newState.extractElement());
         return true;
      });

   MultiExecutionState::Cursor destinationCursor(destinations);
   ExecutionState* first = nullptr;
   if (destinationCursor.setToFirst()) {
      while (destinationCursor.setToNext()) {
         first = nullptr;
         memoryStates.foreachDo([&first, &destinations, &destinationCursor]
            (const Memory::AbstractState& memoryState)
            {  if (!first) {
                  first = &destinations.getSPreviousElement(destinationCursor);
                  first->pmemory().setElement(memoryState);
               }
               else {
                  ExecutionState* newState = new ExecutionState(*first,
                        ExecutionState::DuplicateParameters().setOnlyPoint());
                  destinations.add(newState, COL::VirtualCollection::ExtendedInsertionParameters()
                        .setBefore().setRemote().setFreeOnError(), &destinationCursor);
                  newState->pmemory().setElement(memoryState);
               };
               return true;
            });
      }
      first = nullptr;

      while (!memoryStates.isEmpty()) {
         PNT::PassPointer<Memory::AbstractState> memoryState(&memoryStates.extractFirst(), PNT::Pointer::Init());
         AssumeCondition(!destinationCursor.isValid())
         if (!first) {
            first = &destinations.getSLast();
            first->pmemory() = memoryState;
         }
         else {
            ExecutionState* newState = new ExecutionState(*first,
                  ExecutionState::DuplicateParameters().setOnlyPoint());
            destinations.insertNewAtEnd(newState);
            newState->pmemory() = memoryState;
         };
      }
   };
}

} // end of namespace Control

namespace Iteration {

/**********************************************/
/* Implementation of the class AbstractState  */
/**********************************************/

bool
AbstractState::merge(AbstractState& source, MergeParameters& params) {
   if (!isCompatible(source))
      return false;
   if (params.isForward()) {
      if (params.doesSupportLabel())
         source.cmesOrigin.moveAllTo(cmesOrigin);
      else
         cmesOrigin.mergeAndAbsorbOrigin(source.cmesOrigin, params);
      cmesDestination.mergeAndAbsorb(source.cmesDestination, params);
   }
   else {
      cmesOrigin.mergeAndAbsorbOrigin(source.cmesOrigin, params);
      if (params.doesSupportLabel())
         source.cmesDestination.moveAllTo(cmesDestination);
      else
         cmesDestination.mergeAndAbsorb(source.cmesDestination, params);
   };
   return true;
}

const Control::AbstractPoint*
AbstractState::advance(PNT::PassPointer<AbstractState>& copy) {
   AssumeCondition(pciInstruction)
   if (cmesDestination.isEmpty())
      return nullptr;
   while (!cmesDestination.getFirst().hasMemory()) {
      cmesDestination.freeFirst();
      if (cmesDestination.isEmpty())
         return nullptr;
   };

   if (cmesDestination.hasUnique()) {
      if (!cmesOrigin.isEmpty())
         cmesOrigin.freeAll();
      Control::ExecutionState* state = nullptr;
      cmesOrigin.addNew(state = &cmesDestination.extractFirst());
      const Control::AbstractPoint& destination = state->controlPoint();
      state->setControlPoint(*pciInstruction);
      pciInstruction = nullptr;
      return &destination;
   }
   else {
      Control::MultiExecutionState tempOrigin, tempDestination;
      const Control::AbstractInstruction* tempInstruction = pciInstruction;
      cmesOrigin.swap(tempOrigin);
      cmesDestination.swap(tempDestination);
      pciInstruction = nullptr;
      copy.absorbElement(createSCopy());
      Control::ExecutionState* state = nullptr;
      copy->cmesOrigin.addNew(state = &tempDestination.extractFirst());
      const Control::AbstractPoint& destination = state->controlPoint();
      state->setControlPoint(*tempInstruction);
      pciInstruction = tempInstruction;
      cmesOrigin.swap(tempOrigin);
      cmesDestination.swap(tempDestination);
      return &destination;
   };
}

}} // end of namespace Analyzer::Iteration

