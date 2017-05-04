/**
 @file    time_discretization_constraint.cc
 @author  Alexander W. Winkler (winklera@ethz.ch)
 @date    Mar 30, 2017
 @brief   Brief description
 */

#include <xpp/opt/constraints/time_discretization_constraint.h>
#include <cmath>

namespace xpp {
namespace opt {

TimeDiscretizationConstraint::TimeDiscretizationConstraint (double T, double dt,
                                                            const OptVarsPtr& opt_vars)
{
  dts_.clear();
  double t = 0.0;
  for (int i=0; i<floor(T/dt); ++i) {
    dts_.push_back(t);
    t += dt;
  }

  AddComposite(opt_vars);
}

TimeDiscretizationConstraint::~TimeDiscretizationConstraint ()
{
}

int
TimeDiscretizationConstraint::GetNumberOfNodes () const
{
  return dts_.size();
}

VectorXd
TimeDiscretizationConstraint::GetValues () const
{
  VectorXd g = VectorXd::Zero(GetRows());

  int k = 0;
  for (double t : dts_)
    UpdateConstraintAtInstance(t, k++, g);

  return g;
}

VecBound
TimeDiscretizationConstraint::GetBounds () const
{
  VecBound bounds(GetRows());

  int k = 0;
  for (double t : dts_)
    UpdateBoundsAtInstance(t, k++, bounds);

  return bounds;
}

void
TimeDiscretizationConstraint::FillJacobianWithRespectTo (std::string var_set,
                                                        Jacobian& jac) const
{
  int k = 0;
  for (double t : dts_)
    UpdateJacobianAtInstance(t, k++, jac, var_set);
}

} /* namespace opt */
} /* namespace xpp */

