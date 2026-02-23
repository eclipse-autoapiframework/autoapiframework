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
/*!        \file  vehicle_speed_calculation.h
 *         \brief
 *
 *********************************************************************************************************************/

#ifndef DEMO_VEHICLE_SPEED_CALCULATION_H
#define DEMO_VEHICLE_SPEED_CALCULATION_H

#include <cstdint>
#include "demo/vehicle_speed_calculation_base.h"
#include "vss/vehicle/impl_type_speed.h"

namespace demo {

class VehicleSpeedCalculation : public VehicleSpeedCalculationBase {
 public:
  VehicleSpeedCalculation(ConstructorToken&& token);

  //vaf::Result<void> Init() override;
  //void Start() noexcept override;
  //void Stop() noexcept override;
  //void DeInit() noexcept override;
  //void OnError(const vaf::Error& error) override;

  void SpeedCalculation10() override;
  void DriveDirectionCalculation100() override;

 private:
  vss::vehicle::powertrain::transmission::DriveType drivetype_;
  float speed_;
  uint32_t forward_;
  uint32_t reverse_;
  uint32_t standstill_;
};

}  // namespace demo

#endif  // DEMO_VEHICLE_SPEED_CALCULATION_H
