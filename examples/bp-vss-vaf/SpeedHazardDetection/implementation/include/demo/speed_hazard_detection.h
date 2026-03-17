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
/*!        \file  speed_hazard_detection.h
 *         \brief
 *
 *********************************************************************************************************************/

#ifndef DEMO_SPEED_HAZARD_DETECTION_H
#define DEMO_SPEED_HAZARD_DETECTION_H

#include "demo/speed_hazard_detection_base.h"

namespace demo {

class SpeedHazardDetection : public SpeedHazardDetectionBase {
 public:
  SpeedHazardDetection(ConstructorToken&& token);

  //vaf::Result<void> Init() override;
  //void Start() noexcept override;
  //void Stop() noexcept override;
  //void DeInit() noexcept override;
  //void OnError(const vaf::Error& error) override;

  void HazardCalculation10() override;

 private:
  float hazard_forward_;
  float hazard_reverse_;
};

}  // namespace demo

#endif  // DEMO_SPEED_HAZARD_DETECTION_H
