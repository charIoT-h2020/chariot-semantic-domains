/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : IntervalFloat.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a class of polymorph floating-point intervals.
//

#ifndef Analyzer_Scalar_Approximate_IntervalFloatH
#define Analyzer_Scalar_Approximate_IntervalFloatH

#include "Analyzer/Scalar/Approximate/IntervalInteger.h"
#include "Analyzer/Scalar/Constant/ConstantElement.h"

namespace Analyzer { namespace Scalar { namespace Floating { namespace Approximate {

namespace Details {

using VirtualRealInterval = Scalar::Approximate::Details::VirtualRealInterval;

}

class EpsilonInterval : public VirtualElement {
  private:
   typedef VirtualElement inherited;
   PPVirtualElement ppseValue;
   Scalar::Constant::PPVirtualElement ppcseEpsilon; // exact value

   class MethodApplyTable;
   class MethodConstraintTable;

  public:
   class Init : public VirtualElement::Init {
     private:
      typedef VirtualElement::Init inherited;
      PPVirtualElement ppveValue;
      Scalar::Constant::PPVirtualElement ppcseEpsilon;

     public:
      Init() {}
      Init(const Init& source) = default;
      Init& operator=(const Init& source) = default;
      DefineCopy(Init)
      DDefineAssign(Init)

      Init& setInterval(PPVirtualElement value, Scalar::Constant::PPVirtualElement epsilon)
         {  inherited::setBitSize(value->getSizeInBits());
            ppveValue = value;
            ppcseEpsilon = epsilon;
            return *this;
         }
      Init& setBitSize(int bitSize) { return (Init&) inherited::setBitSize(bitSize); }
     
      friend class EpsilonInterval;
   };
   EpsilonInterval(const Init& init)
      :  inherited(init), ppseValue(init.ppveValue), ppcseEpsilon(init.ppcseEpsilon) {}
   EpsilonInterval(const EpsilonInterval& source) = default;
   EpsilonInterval& operator=(const EpsilonInterval& source) = default;
   DefineCopy(EpsilonInterval)
   DDefineAssign(EpsilonInterval)

   virtual bool isValid() const override
      {  return VirtualElement::isValid() && ppseValue.isValid() && ppseValue->isValid(); }
   virtual int countAtomic() const override { return ppseValue->countAtomic(); }

   virtual bool applyTo(const VirtualOperation& operation, Approximate::VirtualElement&, EvaluationEnvironment& env) const override
      {  AssumeUnimplemented }
   virtual bool mergeWithTo(Approximate::VirtualElement& source, EvaluationEnvironment& env) const override
      {  AssumeUnimplemented }
   virtual bool containTo(const Approximate::VirtualElement& source, EvaluationEnvironment& env) const override
      {  AssumeUnimplemented }
   virtual bool intersectWithTo(Approximate::VirtualElement& source, EvaluationEnvironment& env) const override
      {  AssumeUnimplemented }
   virtual bool constraintTo(const VirtualOperation&, Approximate::VirtualElement& source, const Approximate::VirtualElement&, ConstraintEnvironment&) override
      {  AssumeUnimplemented }
   virtual ZeroResult queryZeroResult() const override { AssumeUncalled return ZeroResult(); }

   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override
      {  AssumeUnimplemented }
   virtual bool mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) override
      {  AssumeUnimplemented }
   virtual bool contain(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) const override
      {  AssumeUnimplemented }
   virtual bool intersectWith(const Approximate::VirtualElement& source, EvaluationEnvironment& env) override
      {  AssumeUnimplemented }
   virtual bool constraint(const VirtualOperation&, const Approximate::VirtualElement&, ConstraintEnvironment&) override
      {  AssumeUnimplemented }
   virtual bool query(const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) const override
      {  AssumeUnimplemented }
};


class Interval : public Details::VirtualRealInterval {
  private:
   typedef Details::VirtualRealInterval inherited;
   
  public:
   typedef Init::BoundType BoundType;

  private:
   PPVirtualElement ppseMin, ppseMax;
   BoundType btType;

   class MethodApplyTable;
   class MethodConstraintTable;

   // intern classes
   class ExtremityProperties;

  protected:
   void setFromLimit(PPVirtualElement limit, Init::Compare init)
      {  AssumeUnimplemented }
   void setFromBoundInterval(PPVirtualElement min, PPVirtualElement max, Init::BoundType type)
      {  AssumeUnimplemented }
   void setFromInterval(PPVirtualElement min, PPVirtualElement max)
      {  ppseMin = min; ppseMax = max; }
   
  public:
   Interval(const Init& init) : inherited(init)
      {  setInterval();
         if (init.tType == Init::TLimit)
            setFromLimit(init.ppveFst, (Init::Compare) init.uQualification);
         else if (init.tType == Init::TBoundInterval)
            setFromBoundInterval(init.ppveFst, init.ppveSnd, (Init::BoundType) init.uQualification);
         else if (init.tType == Init::TInterval)
            setFromInterval(init.ppveFst, init.ppveSnd);
      }
   Interval(const Interval& source) = default;
   Interval& operator=(const Interval& source) = default;
   DefineCopy(Interval)
   DDefineAssign(Interval)

   virtual int countAtomic() const override
      {  return ppseMin->countAtomic() + ppseMax->countAtomic(); }
   virtual bool isValid() const override
      {  return inherited::isValid() && ppseMin.isValid() && ppseMax.isValid()
            && ppseMin->isValid() && ppseMax->isValid();
      }

   virtual bool applyTo(const VirtualOperation& operation, Approximate::VirtualElement&, EvaluationEnvironment& env) const override
      {  AssumeUnimplemented }
   virtual bool mergeWithTo(Approximate::VirtualElement& source, EvaluationEnvironment& env) const override
      {  AssumeUnimplemented }
   virtual bool containTo(const Approximate::VirtualElement& source, EvaluationEnvironment& env) const override
      {  AssumeUnimplemented }
   virtual bool intersectWithTo(Approximate::VirtualElement& source, EvaluationEnvironment& env) const override
      {  AssumeUnimplemented }
   virtual bool constraintTo(const VirtualOperation&, Approximate::VirtualElement& source, const Approximate::VirtualElement&, ConstraintEnvironment&) override
      {  AssumeUnimplemented }
   virtual ZeroResult queryZeroResult() const override { AssumeUncalled return ZeroResult(); }

   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override
      {  AssumeUnimplemented }
   virtual bool mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) override
      {  AssumeUnimplemented }
   virtual bool contain(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) const override
      {  AssumeUnimplemented }
   virtual bool intersectWith(const Approximate::VirtualElement& source, EvaluationEnvironment& env) override
      {  AssumeUnimplemented }
   virtual bool constraint(const VirtualOperation&, const Approximate::VirtualElement&, ConstraintEnvironment&) override
      {  AssumeUnimplemented }
   virtual bool query(const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) const override
      {  AssumeUnimplemented }

   /* apply methods */
      // applyTo
   bool applyToPlusAssignConstant(const VirtualOperation&, VirtualElement&, EvaluationEnvironment&) const
      {  AssumeUnimplemented }
   bool applyToMinusAssignConstant(const VirtualOperation&, VirtualElement&, EvaluationEnvironment& env) const
      {  AssumeUnimplemented }
   bool applyToTimesAssignConstant(const VirtualOperation&, VirtualElement&, EvaluationEnvironment& env) const
      {  AssumeUnimplemented }
   bool applyToDivideAssign(const VirtualOperation&, VirtualElement&, EvaluationEnvironment& env) const
      {  AssumeUnimplemented }

      // apply
   bool applyPlusAssignConstant(const VirtualOperation&, EvaluationEnvironment& env)
      {  AssumeUnimplemented }
   bool applyPlusAssignInterval(const VirtualOperation&, EvaluationEnvironment& env)
      {  AssumeUnimplemented }
   bool applyMinusAssignConstant(const VirtualOperation&, EvaluationEnvironment& env)
      {  AssumeUnimplemented }
   bool applyMinusAssignInterval(const VirtualOperation&, EvaluationEnvironment& env)
      {  AssumeUnimplemented }
   bool applyTimesAssignConstant(const VirtualOperation&, EvaluationEnvironment& env)
      {  AssumeUnimplemented }
   bool applyTimesAssignInterval(const VirtualOperation&, EvaluationEnvironment& env)
      {  AssumeUnimplemented }
   bool applyDivideAssignConstant(const VirtualOperation&, EvaluationEnvironment& env)
      {  AssumeUnimplemented }
   bool applyDivideAssignInterval(const VirtualOperation&, EvaluationEnvironment& env)
      {  AssumeUnimplemented }
   bool applyOppositeAssign(const VirtualOperation&, EvaluationEnvironment& env)
      {  AssumeUnimplemented }
   bool applyOpposite(const VirtualOperation&, EvaluationEnvironment& env)
      {  AssumeUnimplemented }

   /* merge and intersection methods */
      // intersection methods
   bool intersectWithToConstant(VirtualElement& source, EvaluationEnvironment& env) const
      {  AssumeUnimplemented }
   bool intersectWithConstant(const VirtualElement& source, EvaluationEnvironment& env)
      {  AssumeUnimplemented }
   bool intersectWithInterval(const VirtualElement& source, EvaluationEnvironment& env)
      {  AssumeUnimplemented }

      // merge methods
   bool containToConstant(const VirtualElement& source, EvaluationEnvironment& env) const
      {  AssumeUnimplemented }
   bool containConstant(const VirtualElement& source, EvaluationEnvironment& env) const
      {  AssumeUnimplemented }
   bool containInterval(const VirtualElement& source, EvaluationEnvironment& env) const
      {  AssumeUnimplemented }

   bool mergeWithToConstant(VirtualElement& source, EvaluationEnvironment& env) const
      {  AssumeUnimplemented }
   bool mergeWithConstant(const VirtualElement& source, EvaluationEnvironment& env)
      {  AssumeUnimplemented }
   bool mergeWithInterval(const VirtualElement& source, EvaluationEnvironment& env)
      {  AssumeUnimplemented }

   /* constraint methods */
   bool constraintPlusAssign(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintMinusAssign(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintTimesAssignInterval(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintTimesAssignConstant(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintDivideAssignInterval(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintDivideAssignConstant(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintOppositeAssign(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }

#ifdef DefineIncludeFloatOperation
   bool applyACos(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applyASin(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applyAtan(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applyATan2Constant(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applyATan2Interval(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applyCeil(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applyCos(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applyCosh(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applyExp(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applyFabs(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applyFloor(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applyFmodConstant(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applyFmodInterval(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applyFrexpConstantPointer(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applyLdexpConstantInt(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applyLdexpIntervalInt(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applyLog(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applyLog10(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applyModfConstantPointer(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applyPowConstant(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applyPowInterval(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applySin(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applySinh(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applySqrt(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applyTan(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }
   bool applyTanh(const VirtualOperation&, EvaluationEnvironment&)
      {  AssumeUnimplemented }

   bool constraintACos(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintASin(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintAtan(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintATan2(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintCeil(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintCos(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintCosh(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintExp(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintFabs(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintFloor(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintFmod(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintFrexp(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintLdexp(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintLog(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintLog10(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintModf(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintPow(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintSin(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintSinh(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintSqrt(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintTan(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
   bool constraintTanh(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&)
      {  AssumeUnimplemented }
#endif
};

}}}} // end of namespace Analyzer::Scalar::Approximate::Floating

#endif // Analyzer_Scalar_Approximate_IntervalFloatH

