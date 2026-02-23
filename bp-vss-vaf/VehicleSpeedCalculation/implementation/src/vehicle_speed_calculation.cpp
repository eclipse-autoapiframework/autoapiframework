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
/*!        \file  vehicle_speed_calculation.cpp
 *         \brief
 *
 *********************************************************************************************************************/

#include "demo/vehicle_speed_calculation.h"
#include "vss/vehicle/impl_type_drivedirection.h"
#include "vss/vehicle/powertrain/transmission/impl_type_drivetype.h"

namespace demo {

/*
  Generated based on configuration in ../../model/vehicle_speed_calculation.py

  Consumer interfaces
  ===================
    Data element API example for Longitudinal of type vss::vehicle::acceleration::Longitudinal
      - ::vaf::Result<::vaf::ConstDataPtr<const vss::vehicle::acceleration::Longitudinal>> GetAllocated_Longitudinal()
      - vss::vehicle::acceleration::Longitudinal Get_Longitudinal()
      - void RegisterDataElementHandler_Longitudinal(vaf::String owner, std::function<void(const ::vaf::ConstDataPtr<const vss::vehicle::acceleration::Longitudinal>)>&& f)

    - SpeedIfConsumer_ : demo::SpeedIfConsumer
      - Data elements
        - Longitudinal : vss::vehicle::acceleration::Longitudinal
        - SpeedRow1WheelLeft : vss::vehicle::chassis::axle::row1::wheel::left::Speed
        - SpeedRow1WheelRight : vss::vehicle::chassis::axle::row1::wheel::right::Speed
        - SpeedRow2WheelLeft : vss::vehicle::chassis::axle::row2::wheel::left::Speed
        - SpeedRow2WheelRight : vss::vehicle::chassis::axle::row2::wheel::right::Speed
        - DriveType : vss::vehicle::powertrain::transmission::DriveType

  Provider interfaces
  ===================
    Data element API example for DriveDirection of type vss::vehicle::DriveDirection
     - ::vaf::Result<::vaf::DataPtr<vss::vehicle::DriveDirection>> Allocate_DriveDirection()
     - ::vaf::Result<void> SetAllocated_DriveDirection(::vaf::DataPtr<vss::vehicle::DriveDirection>&& data)
     - ::vaf::Result<void> Set_DriveDirection(const vss::vehicle::DriveDirection& data)

    - SpeedCalculationIfProvider_ : demo::SpeedCalculationIfProvider
      - Data elements
        - DriveDirection : vss::vehicle::DriveDirection
        - Speed : vss::vehicle::Speed
*/

/**********************************************************************************************************************
  Constructor
**********************************************************************************************************************/
VehicleSpeedCalculation::VehicleSpeedCalculation(ConstructorToken&& token)
    : VehicleSpeedCalculationBase(std::move(token)),
      drivetype_(vss::vehicle::powertrain::transmission::ALL_WHEEL_DRIVE),
      speed_(0.0),
      forward_(0),
      reverse_(0),
      standstill_(0) {
  SpeedIfConsumer_->RegisterDataElementHandler_DriveType(
      GetName(), [this](const ::vaf::ConstDataPtr<const vss::vehicle::powertrain::transmission::DriveType>& dt_sample) {
        if (dt_sample) {
          drivetype_ =
              *dt_sample;  // Set actual drivetype. Assumption, this info is broadcasted at least once (at startup).
          vaf::OutputSyncStream{} << "[VehicleSpeedCalculation] " << "DriveType set to: " << *dt_sample << std::endl;
        }
      });
}

/**********************************************************************************************************************
  2 periodic task(s)
**********************************************************************************************************************/
// Task with name SpeedCalculation10 and a period of 10ms.
void VehicleSpeedCalculation::SpeedCalculation10() {
  // Average the wheel speeds
  if (drivetype_ == vss::vehicle::powertrain::transmission::FORWARD_WHEEL_DRIVE) {
    speed_ = (SpeedIfConsumer_->Get_SpeedRow1WheelLeft() + SpeedIfConsumer_->Get_SpeedRow1WheelRight()) / 2;
  }
  if (drivetype_ == vss::vehicle::powertrain::transmission::REAR_WHEEL_DRIVE) {
    speed_ = (SpeedIfConsumer_->Get_SpeedRow2WheelLeft() + SpeedIfConsumer_->Get_SpeedRow2WheelRight()) / 2;
  }
  if (drivetype_ == vss::vehicle::powertrain::transmission::ALL_WHEEL_DRIVE) {
    speed_ = (SpeedIfConsumer_->Get_SpeedRow1WheelLeft() + SpeedIfConsumer_->Get_SpeedRow1WheelRight() +
              SpeedIfConsumer_->Get_SpeedRow2WheelLeft() + SpeedIfConsumer_->Get_SpeedRow2WheelRight()) /
             4;
  }
  SpeedCalculationIfProvider_->Set_Speed(speed_);
}

// Task with name DriveDirectionCalculation100 and a period of 100ms.
void VehicleSpeedCalculation::DriveDirectionCalculation100() {
  vss::vehicle::acceleration::Longitudinal acceleration = SpeedIfConsumer_->Get_Longitudinal();
  // Detect drive direction FORWARD
  if (acceleration > 0) {
    standstill_ = 0;
    if (speed_ > 5) {  // Wheel speed is greater than 5 km/h
      forward_++;
    } else {
      forward_ = 0;
    }
  } else {
    forward_ = 0;
  }
  if (forward_ > 5) {  // Holds for at least 500ms
    SpeedCalculationIfProvider_->Set_DriveDirection(vss::vehicle::FORWARD);
    vaf::OutputSyncStream{} << "[VehicleSpeedCalculation] " << "Vehicle is moving FORWARD (" << speed_ << " km/h)."
                            << std::endl;
  }

  // Detect drive direction REVERSE
  if (acceleration < 0) {
    standstill_ = 0;
    if (speed_ > 2) {  // Wheel speed is greater than 2 km/h
      reverse_++;
    } else {
      reverse_ = 0;
    }
  } else {
    reverse_ = 0;
  }
  if (reverse_ > 20) {  // Holds for at least 2000ms
    SpeedCalculationIfProvider_->Set_DriveDirection(vss::vehicle::REVERSE);
    vaf::OutputSyncStream{} << "[VehicleSpeedCalculation] " << "Vehicle is moving REVERSE (" << speed_ << " km/h)."
                            << std::endl;
  }

  // Detect STANDSTILL
  if (acceleration == 0) {
    if (speed_ == 0.0) {
      standstill_++;
    } else {
      standstill_ = 0;
    }
  }
  if (standstill_ > 50) {  // Holds for at least 5000ms
    SpeedCalculationIfProvider_->Set_DriveDirection(vss::vehicle::STANDSTILL);
    vaf::OutputSyncStream{} << "[VehicleSpeedCalculation] " << "Vehicle is in STANDSTILL." << std::endl;
  }
}

//vaf::Result<void> VehicleSpeedCalculation::Init() {
//  return vaf::Result<void>{};
//}
//void VehicleSpeedCalculation::Start() {
//  ReportOperational();
//}
//void VehicleSpeedCalculation::Stop() {}
//void VehicleSpeedCalculation::DeInit() {}
//void VehicleSpeedCalculation::OnError(const vaf::Error& error) {
//  static_cast<void>(error);
//}

}  // namespace demo
