/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate native scalar elements
// File      : BitFieldMask.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a class of elements that rely on native host implementation.
//   This defines template classes that enable to keep constraints on bits.
//   All this stuff is rather defined for source code analysis.
//

#ifndef Analyzer_Scalar_Approximate_BitFieldMaskH
#define Analyzer_Scalar_Approximate_BitFieldMaskH

#include "Analyzer/Scalar/Approximate/MethodTable.h"

namespace Analyzer { namespace Scalar { namespace Approximate {

template <class TypeAConstant>
class BitFieldMask : public Approximate::Details::IntOperationElement {
  public:
   typedef typename TypeAConstant::Operation Operation;
   class Constants : public Approximate::Details::BaseConstants {
     public:
      static const int NBIntersectToMethods = VirtualElement::ApproxKind::TUnknown+1;
   };

  private:
   typedef BitFieldMask<TypeAConstant> thisType;
   typedef Approximate::Details::IntOperationElement inherited;

   typedef Approximate::Details::TDomainTraits<BitFieldMask<TypeAConstant> > DomainTraits;
   class MethodMergeIntersectToTable : public Approximate::Details::MethodMergeIntersectToTable<DomainTraits> {
     public:
      MethodMergeIntersectToTable();
   };

   static MethodMergeIntersectToTable mmittMergeIntersectMethodTable;

   const TypeAConstant* pcReference;
   typename TypeAConstant::inheritedImplementation::SimulatedType uOneMask;  // the fields with sure 1 have 1
   typename TypeAConstant::inheritedImplementation::SimulatedType uZeroMask; // the fields with sure 0 have 0

  public:
   BitFieldMask(const TypeAConstant& reference,
         typename TypeAConstant::inheritedImplementation::SimulatedType oneMask,
         typename TypeAConstant::inheritedImplementation::SimulatedType zeroMask)
      :  inherited(Init().setBitSize(sizeof(oneMask)*8)),
         pcReference(&reference), uOneMask(oneMask), uZeroMask(zeroMask)
      {  setUnknown(); }
   BitFieldMask(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   TemplateDefineCopy(BitFieldMask, TypeAConstant)
   DTemplateDefineAssign(BitFieldMask, TypeAConstant)

   bool intersectWithToConstant(VirtualElement& source, EvaluationEnvironment& env) const;
   bool intersectWithToInterval(VirtualElement& source, EvaluationEnvironment& env) const;
   bool intersectWithToDisjunction(VirtualElement& source, EvaluationEnvironment& env) const;
   bool intersectWithToConjunction(VirtualElement& source, EvaluationEnvironment& env) const;
   bool intersectWithToUnprecise(VirtualElement& source, EvaluationEnvironment& env) const;

   virtual bool intersectWithTo(VirtualElement& source, EvaluationEnvironment& env) const override
      {  inherited::assume(mmittMergeIntersectMethodTable[source.getApproxKind().type()]
            .execute(*this, source, env));
         return true;
      }

   virtual bool isValid() const override { return !(uOneMask & ~uZeroMask); }
   virtual bool mergeWithTo(Approximate::VirtualElement& source, EvaluationEnvironment& env) const override { AssumeUncalled return false; }
   virtual bool containTo(const Approximate::VirtualElement&, EvaluationEnvironment&) const override {  AssumeUncalled return false; }
   virtual bool applyTo(const VirtualOperation&, Approximate::VirtualElement&, EvaluationEnvironment&) const override {  AssumeUncalled return false; }
   virtual bool constraintTo(const VirtualOperation&, Approximate::VirtualElement& source, const Approximate::VirtualElement&, ConstraintEnvironment&) override { AssumeUncalled return false; }

   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override { AssumeUncalled return false; }
   virtual bool mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) override { AssumeUncalled return false; }
   virtual bool contain(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) const override { AssumeUncalled return false; }
   virtual bool intersectWith(const Approximate::VirtualElement& source, EvaluationEnvironment& env) override { AssumeUncalled return false; }
   virtual bool constraint(const VirtualOperation&, const Approximate::VirtualElement&, ConstraintEnvironment& env) override { AssumeUncalled return false; }
};

template <class TypeAConstant>
class MBitFieldMask : public Approximate::Details::IntOperationElement {
  public:
   typedef typename TypeAConstant::Operation Operation;
   class Constants : public Approximate::Details::BaseConstants {
     public:
      static const int NBIntersectToMethods = VirtualElement::ApproxKind::TUnknown+1;
   };

  private:
   typedef MBitFieldMask<TypeAConstant> thisType;
   typedef Approximate::Details::IntOperationElement inherited;
   typedef typename TypeAConstant::inheritedImplementation MultiBitImplementation;

   typedef Approximate::Details::TDomainTraits<MBitFieldMask<TypeAConstant> > DomainTraits;
   class MethodMergeIntersectToTable : public Approximate::Details::MethodMergeIntersectToTable<DomainTraits> {
     public:
      MethodMergeIntersectToTable();
   };

   static MethodMergeIntersectToTable mmittMergeIntersectMethodTable;

   const TypeAConstant* pcReference;
   MultiBitImplementation uOneMask;  // the fields with 1 have 1
   MultiBitImplementation uZeroMask; // the fields with 0 have 0

  public:
   MBitFieldMask(const TypeAConstant& reference, const
         typename TypeAConstant::inheritedImplementation& oneMask,
         const typename TypeAConstant::inheritedImplementation& zeroMask)
      :  inherited(Init().setBitSize(oneMask.getSize())),
         pcReference(&reference), uOneMask(oneMask), uZeroMask(zeroMask)
      {  setUnknown(); }
   MBitFieldMask(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   TemplateDefineCopy(MBitFieldMask, TypeAConstant)
   DTemplateDefineAssign(MBitFieldMask, TypeAConstant)

   bool intersectWithToConstant(VirtualElement& source, EvaluationEnvironment& env) const;
   bool intersectWithToInterval(VirtualElement& source, EvaluationEnvironment& env) const;
   bool intersectWithToDisjunction(VirtualElement& source, EvaluationEnvironment& env) const;
   bool intersectWithToConjunction(VirtualElement& source, EvaluationEnvironment& env) const;
   bool intersectWithToUnprecise(VirtualElement& source, EvaluationEnvironment& env) const;

   virtual bool intersectWithTo(VirtualElement& source, EvaluationEnvironment& env) const override
      {  inherited::assume(mmittMergeIntersectMethodTable[source.getApproxKind().type()]
            .execute(*this, source, env));
         return true;
      }

   virtual bool isValid() const override
      {  MultiBitImplementation zero = uZeroMask;
         zero.neg();
         zero &= uOneMask;
         return zero.isZero();
      }
   virtual bool mergeWithTo(Approximate::VirtualElement& source, EvaluationEnvironment& env) const override { AssumeUncalled return false; }
   virtual bool containTo(const Approximate::VirtualElement&, EvaluationEnvironment&) const override {  AssumeUncalled return false; }
   virtual bool applyTo(const VirtualOperation&, Approximate::VirtualElement&, EvaluationEnvironment&) const override {  AssumeUncalled return false; }
   virtual bool constraintTo(const VirtualOperation&, Approximate::VirtualElement& source, const Approximate::VirtualElement&, ConstraintEnvironment&) override { AssumeUncalled return false; }

   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override { AssumeUncalled return false; }
   virtual bool mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) override { AssumeUncalled return false; }
   virtual bool contain(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) const override { AssumeUncalled return false; }
   virtual bool intersectWith(const Approximate::VirtualElement& source, EvaluationEnvironment& env) override { AssumeUncalled return false; }
   virtual bool constraint(const VirtualOperation&, const Approximate::VirtualElement&, ConstraintEnvironment& env) override { AssumeUncalled return false; }
};

}}} // end of namespace Analyzer::Scalar::Approximate

#endif // Analyzer_Scalar_Approximate_BitFieldMaskH

