/**
 @file    dynamic_constraint.cc
 @author  Alexander W. Winkler (winklera@ethz.ch)
 @date    Dec 4, 2016
 @brief   Defines the DynamicConstraint class
 */

#include <xpp/opt/support_area_constraint.h>
#include <xpp/opt/optimization_variables.h>

namespace xpp {
namespace opt {

using namespace xpp::utils;
using Vector2d = Eigen::Vector2d;
using Vector3d = Eigen::Vector3d;

SupportAreaConstraint::SupportAreaConstraint ()
{
  // TODO Auto-generated constructor stub
}

SupportAreaConstraint::~SupportAreaConstraint ()
{
  // TODO Auto-generated destructor stub
}

void
SupportAreaConstraint::Init (const MotionStructure& motion_structure)
{
  motion_structure_ = motion_structure;
}

void
SupportAreaConstraint::UpdateVariables (const OptimizationVariables* opt_var)
{
  lambdas_   = opt_var->GetVariables(VariableNames::kConvexity);
  cop_       = opt_var->GetVariables(VariableNames::kCenterOfPressure);
  Eigen::VectorXd footholds = opt_var->GetVariables(VariableNames::kFootholds);
  footholds_ = utils::ConvertEigToStd(footholds);
}

SupportAreaConstraint::VectorXd
SupportAreaConstraint::EvaluateConstraint () const
{
  int m = motion_structure_.GetPhaseStampedVec().size() * kDim2d;
  Eigen::VectorXd g(m);

  int idx_lambda = 0;
  int k = 0;
  for (const auto& node : motion_structure_.GetPhaseStampedVec()) {

    Vector2d convex_contacts;
    convex_contacts.setZero();
    for (auto contact : node.phase_.free_contacts_) {
      Vector2d p = footholds_.at(contact.id);
      double lamdba = lambdas_(idx_lambda++);
      convex_contacts += lamdba*p;
    }

    for (auto contact : node.phase_.fixed_contacts_) {
      Vector2d p = contact.p.topRows<kDim2d>();
      double lamdba = lambdas_(idx_lambda++);
      convex_contacts += lamdba*p;
    }

    g.middleRows<kDim2d>(kDim2d*k) = convex_contacts - cop_.middleRows<kDim2d>(kDim2d*k);
    k++;
  }

  return g;
}

SupportAreaConstraint::VecBound
SupportAreaConstraint::GetBounds () const
{
  std::vector<Bound> bounds;
  int m = EvaluateConstraint().rows();
  for (int i=0; i<m; ++i)
    bounds.push_back(kEqualityBound_);

  return bounds;
}

// zmp_ almost exactly the same as in convexity constraint->merge!
SupportAreaConstraint::Jacobian
SupportAreaConstraint::GetJacobianWithRespectToLambdas() const
{
  int col_idx = 0;
  int row_idx = 0;
  int m = GetNumberOfConstraints();
  int n = motion_structure_.GetTotalNumberOfNodeContacts();
  Jacobian jac_(m, n);

  for (const auto& node : motion_structure_.GetPhaseStampedVec()) {
    int contacts_free = node.phase_.free_contacts_.size();
    int contacts_fixed = node.phase_.fixed_contacts_.size();

    for (int i=0; i<contacts_free; ++i) {
      Vector2d p = footholds_.at(node.phase_.free_contacts_.at(i).id);
      for (auto dim : {X, Y})
        jac_.insert(row_idx+dim,col_idx + i) = p(dim);
    }

    col_idx += contacts_free;

    for (int i=0; i<contacts_fixed; ++i) {
      Vector3d p = node.phase_.fixed_contacts_.at(i).p;
      for (auto dim : {X, Y})
        jac_.insert(row_idx+dim,col_idx + i) = p(dim);
    }

    col_idx += contacts_fixed;
    row_idx += kDim2d;
  }

  return jac_;
}

SupportAreaConstraint::Jacobian
SupportAreaConstraint::GetJacobianWithRespectToContacts () const
{
  int row_idx = 0;
  int m = GetNumberOfConstraints();
  int n = footholds_.size() * kDim2d;
  Jacobian jac_(m, n);

  int idx_lambdas = 0;
  for (const auto& node : motion_structure_.GetPhaseStampedVec()) {


    for (auto contact : node.phase_.free_contacts_) {
      for (auto dim : {X, Y}) {
        int idx_contact = ContactVars::Index(contact.id, dim);
        jac_.insert(row_idx+dim, idx_contact) = lambdas_(idx_lambdas);
      }
      idx_lambdas++;
    }

    for (auto contact : node.phase_.fixed_contacts_) {
      idx_lambdas++; // zmp_ this is bad, depends on order free->fixed with function above...
    }

    row_idx    += kDim2d;
  }

  return jac_;
}


SupportAreaConstraint::Jacobian
SupportAreaConstraint::GetJacobianWithRespectTo (std::string var_set) const
{
  Jacobian jac; // empty matrix

  if (var_set == VariableNames::kCenterOfPressure) {
    jac = GetJacobianWithRespectToCop();
  }

  if (var_set == VariableNames::kFootholds) {
    jac = GetJacobianWithRespectToContacts();
  }

  if (var_set == VariableNames::kConvexity) {
    jac = GetJacobianWithRespectToLambdas();
  }

  return jac;
}

SupportAreaConstraint::Jacobian
SupportAreaConstraint::GetJacobianWithRespectToCop () const
{
    int m = GetNumberOfConstraints();
    int n   = cop_.rows();
    Jacobian jac_(m, n);
    jac_.setIdentity();
    jac_ = -1*jac_;

    return jac_;
}

} /* namespace opt */
} /* namespace xpp */