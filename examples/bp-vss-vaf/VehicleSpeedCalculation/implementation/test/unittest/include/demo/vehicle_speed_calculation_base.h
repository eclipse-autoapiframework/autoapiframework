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
/*!        \file  vehicle_speed_calculation_base.h
 *         \brief
 *
 *********************************************************************************************************************/

#ifndef DEMO_VEHICLE_SPEED_CALCULATION_BASE_H
#define DEMO_VEHICLE_SPEED_CALCULATION_BASE_H

#include <memory>
#include "vaf/controller_interface.h"

#include "demo/speed_calculation_if_provider.h"
#include "demo/speed_if_consumer.h"

namespace demo {

class VehicleSpeedCalculationBase {
 public:
  struct ConstructorToken {
    std::shared_ptr<demo::SpeedIfConsumer> SpeedIfConsumer_;
    std::shared_ptr<demo::SpeedCalculationIfProvider> SpeedCalculationIfProvider_;
  };

  VehicleSpeedCalculationBase(ConstructorToken&& token);
  virtual ~VehicleSpeedCalculationBase() = default;

  void ReportError(const vaf::Error&, bool) {}
  virtual void OnError(const vaf::Error&) {}
  vaf::String GetName() { return ""; }

  virtual void SpeedCalculation10() = 0;
  virtual void DriveDirectionCalculation100() = 0;

 protected:
  std::shared_ptr<demo::SpeedIfConsumer> SpeedIfConsumer_;
  std::shared_ptr<demo::SpeedCalculationIfProvider> SpeedCalculationIfProvider_;
};

}  // namespace demo

#endif  // DEMO_VEHICLE_SPEED_CALCULATION_BASE_H
