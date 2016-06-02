/**
 @file    final_state_equation.h
 @author  Alexander W. Winkler (winklera@ethz.ch)
 @date    May 26, 2016
 @brief   Brief description
 */

#ifndef USER_TASK_DEPENDS_XPP_OPT_INCLUDE_XPP_ZMP_FINAL_STATE_EQUATION_H_
#define USER_TASK_DEPENDS_XPP_OPT_INCLUDE_XPP_ZMP_FINAL_STATE_EQUATION_H_

#include <xpp/zmp/i_linear_equation_builder.h>
#include <xpp/zmp/continuous_spline_container.h>

namespace xpp {
namespace zmp {

class FinalStateEquation : public ILinearEquationBuilder {
public:
  typedef xpp::utils::Point2d State2d;

  FinalStateEquation (const State2d& final_state_xy_,
                      const ContinuousSplineContainer&);
  virtual  ~FinalStateEquation () {}

  MatVec BuildLinearEquation() const override;

private:
  const State2d final_state_xy_;
  const ContinuousSplineContainer splines_;
};

} /* namespace zmp */
} /* namespace xpp */

#endif /* USER_TASK_DEPENDS_XPP_OPT_INCLUDE_XPP_ZMP_FINAL_STATE_EQUATION_H_ */