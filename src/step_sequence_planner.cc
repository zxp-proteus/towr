/**
 @file    step_sequence_planner.cc
 @author  Alexander W. Winkler (winklera@ethz.ch)
 @date    Jul 21, 2016
 @brief   Brief description
 */

#include <xpp/hyq/step_sequence_planner.h>
#include <xpp/hyq/support_polygon.h>
#include <xpp/zmp/zmp_constraint_builder.h>
#include <cassert>

namespace xpp {
namespace hyq {

StepSequencePlanner::StepSequencePlanner ()
{
  // must still initialize current and goal state
  prev_swing_leg_ = RF;
}

StepSequencePlanner::~StepSequencePlanner ()
{
  // TODO Auto-generated destructor stub
}

void
StepSequencePlanner::SetCurrAndGoal (const State& curr, const State& goal)
{
  curr_state_ = curr;
  goal_state_ = goal;
}

StepSequencePlanner::LegIDVec
StepSequencePlanner::DetermineStepSequence (int curr_swing_leg)
{
  if (!IsStepNecessary()) {
    return std::vector<xpp::hyq::LegID>(); // empty vector, take no steps
  } else {

    //    // based on distance to cover
    //  const double length_per_step = 0.25;
    //  const double width_per_step = 0.15;
    //  Eigen::Vector2d start_to_goal = goal_state_.p.segment<2>(0) - curr_state_.p.segment<2>(0);
    //    int req_steps_by_length = std::ceil(std::fabs(start_to_goal.x())/length_per_step);
    //    int req_steps_by_width  = std::ceil(std::fabs(start_to_goal.y())/width_per_step);
    //    int req_steps_per_leg = std::max(req_steps_by_length,req_steps_by_width);
    // hardcoded 4 steps
    int req_steps_per_leg = 1;

    LegID sl;
    if (curr_swing_leg == hyq::NO_SWING_LEG)
      sl = prev_swing_leg_;
    else {
      sl = static_cast<LegID>(curr_swing_leg);
      prev_swing_leg_ = sl;
    }


    LegIDVec step_sequence;

    for (int step=0; step<req_steps_per_leg*4; ++step) {
      step_sequence.push_back(NextSwingLeg(sl));
      sl = step_sequence.back();
    }

    return step_sequence;
  }
}

bool
StepSequencePlanner::StartWithStancePhase (const VecFoothold& start_stance,
                                           double robot_height,
                                           LegID first_swingleg) const
{
  // determine, whether initial stance phase must be inserted
  Eigen::Vector2d zmp = xpp::zmp::ZeroMomentPoint::CalcZmp(curr_state_.Make3D(), robot_height);

  // remove first swingleg from current stance
  VecFoothold first_stance = start_stance;
  int idx_swingleg = Foothold::GetLastIndex(first_swingleg, first_stance);
  first_stance.erase(first_stance.begin() + idx_swingleg);

  // fixme zero margins, since i actually allow violation of zmp constraint
  // don't want 4ls to be inserted there
  MarginValues margins = hyq::SupportPolygon::GetDefaultMargins();
//  margins.at(DIAG)/=2.;
  hyq::SupportPolygon supp(first_stance, margins);

  bool zmp_inside = zmp::ZmpConstraintBuilder::IsZmpInsideSuppPolygon(zmp,supp);

  bool start_with_com_shift = false;
  // so 4ls-phase not always inserted b/c of short time zmp constraints are ignore
  // when switching between disjoint support triangles.
  if (/* !zmp_inside  && */ curr_state_.v.norm() < 0.01)
    start_with_com_shift = true;

  return start_with_com_shift;
}

bool
StepSequencePlanner::IsStepNecessary () const
{
  static const double min_distance_to_step = 0.08;//m
  Eigen::Vector2d start_to_goal = goal_state_.p.segment<2>(0) - curr_state_.p.segment<2>(0);

  bool step_necessary = (start_to_goal.norm() > min_distance_to_step)
                     || (curr_state_.v.norm() > 0.1);

  return step_necessary;
}

LegID
StepSequencePlanner::NextSwingLeg (LegID curr) const
{
  switch (curr) {
    case LH: return LF;
    case LF: return RH;
    case RH: return RF;
    case RF: return LH;
    default: assert(false); // this should never happen
  };
}

} /* namespace hyq */
} /* namespace xpp */