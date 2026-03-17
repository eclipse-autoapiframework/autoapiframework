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
/*!        \file  test_module.cpp
 *         \brief
 *
 *********************************************************************************************************************/

#include "demo/test_module.h"
#include "vss/vehicle/powertrain/transmission/impl_type_drivetype.h"

namespace demo {

/*
  Generated based on configuration in ../../model/test_module.py

  Consumer interfaces
  ===================
    Data element API example for Request of type vss::vehicle::body::lights::hazard::Request
      - ::vaf::Result<::vaf::ConstDataPtr<const vss::vehicle::body::lights::hazard::Request>> GetAllocated_Request()
      - vss::vehicle::body::lights::hazard::Request Get_Request()
      - void RegisterDataElementHandler_Request(vaf::String owner, std::function<void(const ::vaf::ConstDataPtr<const vss::vehicle::body::lights::hazard::Request>)>&& f)

    - HazardDetectionIfConsumer_ : demo::HazardDetectionIfConsumer
      - Data elements
        - Request : vss::vehicle::body::lights::hazard::Request

  Provider interfaces
  ===================
    Data element API example for Longitudinal of type vss::vehicle::acceleration::Longitudinal
     - ::vaf::Result<::vaf::DataPtr<vss::vehicle::acceleration::Longitudinal>> Allocate_Longitudinal()
     - ::vaf::Result<void> SetAllocated_Longitudinal(::vaf::DataPtr<vss::vehicle::acceleration::Longitudinal>&& data)
     - ::vaf::Result<void> Set_Longitudinal(const vss::vehicle::acceleration::Longitudinal& data)

    - SpeedIfProvider_ : demo::SpeedIfProvider
      - Data elements
        - Longitudinal : vss::vehicle::acceleration::Longitudinal
        - SpeedRow1WheelLeft : vss::vehicle::chassis::axle::row1::wheel::left::Speed
        - SpeedRow1WheelRight : vss::vehicle::chassis::axle::row1::wheel::right::Speed
        - SpeedRow2WheelLeft : vss::vehicle::chassis::axle::row2::wheel::left::Speed
        - SpeedRow2WheelRight : vss::vehicle::chassis::axle::row2::wheel::right::Speed
        - DriveType : vss::vehicle::powertrain::transmission::DriveType
    - HazardIfProvider_ : demo::HazardIfProvider
      - Data elements
        - IsSignaling : vss::vehicle::body::lights::hazard::IsSignaling
        - SpeedHazardForward : vss::vehicle::chassis::SpeedHazardForward
        - SpeedHazardReverse : vss::vehicle::chassis::SpeedHazardReverse
*/

/**********************************************************************************************************************
  Constructor
**********************************************************************************************************************/
TestModule::TestModule(ConstructorToken&& token) : TestModuleBase(std::move(token)), ctr_(0) {}

/**********************************************************************************************************************
  2 periodic task(s)
**********************************************************************************************************************/
// Task with name PeriodicTask200 and a period of 200ms.
void TestModule::PeriodicTask200() {
  // Simulate a stop and go scenario with short standstill phase in between...
  if (ctr_ < 50) {
    SpeedIfProvider_->Set_SpeedRow1WheelLeft(static_cast<float>(ctr_));
    SpeedIfProvider_->Set_SpeedRow1WheelRight(static_cast<float>(ctr_));
    SpeedIfProvider_->Set_Longitudinal(static_cast<float>(1.39));
  }
  if (ctr_ >= 50) {
    SpeedIfProvider_->Set_Longitudinal(static_cast<float>(0.0));
    SpeedIfProvider_->Set_SpeedRow1WheelLeft(static_cast<float>(0));
    SpeedIfProvider_->Set_SpeedRow1WheelRight(static_cast<float>(0));
  }
  if (ctr_ == 100) {
    ctr_ = 0;  // Reset scenario.
  } else {
    ctr_++;
  }
}

// Task with name PeriodicTask2000 and a period of 2000ms.
void TestModule::PeriodicTask2000() {
  // Distribute configuration data to application parts.
  SpeedIfProvider_->Set_DriveType(vss::vehicle::powertrain::transmission::FORWARD_WHEEL_DRIVE);
  HazardIfProvider_->Set_SpeedHazardForward(40);
  // HazardIfProvider_->Set_SpeedHazardReverse(10);
}

//vaf::Result<void> TestModule::Init() {
//  return vaf::Result<void>{};
//}
//void TestModule::Start() {
//  ReportOperational();
//}
//void TestModule::Stop() {}
//void TestModule::DeInit() {}
//void TestModule::OnError(const vaf::Error& error) {
//  static_cast<void>(error);
//}

}  // namespace demo
