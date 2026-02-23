/*!********************************************************************************************************************
 *  COPYRIGHT
 *  -------------------------------------------------------------------------------------------------------------------
 *  \verbatim
 *  Copyright (c) 2024-2026 by Vector Informatik GmbH. All rights reserved.
 *  SPDX-License-Identifier: Apache-2.0
 *  \endverbatim
 *  -------------------------------------------------------------------------------------------------------------------
 *  FILE DESCRIPTION
 *  -----------------------------------------------------------------------------------------------------------------*/
/*!        \file  speed_hazard_detection_base.h
 *         \brief
 *
 *********************************************************************************************************************/

#ifndef DEMO_SPEED_HAZARD_DETECTION_BASE_H
#define DEMO_SPEED_HAZARD_DETECTION_BASE_H

#include <memory>
#include "vaf/controller_interface.h"

#include "demo/hazard_detection_if_provider.h"
#include "demo/hazard_if_consumer.h"
#include "demo/speed_calculation_if_consumer.h"

namespace demo {

class SpeedHazardDetectionBase {
 public:
  struct ConstructorToken {
    std::shared_ptr<demo::SpeedCalculationIfConsumer> SpeedCalculationIfConsumer_;
    std::shared_ptr<demo::HazardIfConsumer> HazardIfConsumer_;
    std::shared_ptr<demo::HazardDetectionIfProvider> HazardDetectionIfProvider_;
  };

  SpeedHazardDetectionBase(ConstructorToken&& token);
  virtual ~SpeedHazardDetectionBase() = default;

  void ReportError(const vaf::Error&, bool) {}
  virtual void OnError(const vaf::Error&) {}
  vaf::String GetName() { return ""; }

  virtual void HazardCalculation10() = 0;

 protected:
  std::shared_ptr<demo::SpeedCalculationIfConsumer> SpeedCalculationIfConsumer_;
  std::shared_ptr<demo::HazardIfConsumer> HazardIfConsumer_;
  std::shared_ptr<demo::HazardDetectionIfProvider> HazardDetectionIfProvider_;
};

}  // namespace demo

#endif  // DEMO_SPEED_HAZARD_DETECTION_BASE_H
