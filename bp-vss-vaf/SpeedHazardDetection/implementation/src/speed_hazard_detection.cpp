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
/*!        \file  speed_hazard_detection.cpp
 *         \brief
 *
 *********************************************************************************************************************/

#include "demo/speed_hazard_detection.h"
#include "vss/vehicle/chassis/impl_type_speedhazardreverse.h"
#include "vss/vehicle/impl_type_drivedirection.h"

namespace demo {

/*
  Generated based on configuration in ../../model/speed_hazard_detection.py

  Consumer interfaces
  ===================
    Data element API example for DriveDirection of type vss::vehicle::DriveDirection
      - ::vaf::Result<::vaf::ConstDataPtr<const vss::vehicle::DriveDirection>> GetAllocated_DriveDirection()
      - vss::vehicle::DriveDirection Get_DriveDirection()
      - void RegisterDataElementHandler_DriveDirection(vaf::String owner, std::function<void(const ::vaf::ConstDataPtr<const vss::vehicle::DriveDirection>)>&& f)

    - SpeedCalculationIfConsumer_ : demo::SpeedCalculationIfConsumer
      - Data elements
        - DriveDirection : vss::vehicle::DriveDirection
        - Speed : vss::vehicle::Speed
    - HazardIfConsumer_ : demo::HazardIfConsumer
      - Data elements
        - IsSignaling : vss::vehicle::body::lights::hazard::IsSignaling
        - SpeedHazardForward : vss::vehicle::chassis::SpeedHazardForward
        - SpeedHazardReverse : vss::vehicle::chassis::SpeedHazardReverse

  Provider interfaces
  ===================
    Data element API example for Request of type vss::vehicle::body::lights::hazard::Request
     - ::vaf::Result<::vaf::DataPtr<vss::vehicle::body::lights::hazard::Request>> Allocate_Request()
     - ::vaf::Result<void> SetAllocated_Request(::vaf::DataPtr<vss::vehicle::body::lights::hazard::Request>&& data)
     - ::vaf::Result<void> Set_Request(const vss::vehicle::body::lights::hazard::Request& data)

    - HazardDetectionIfProvider_ : demo::HazardDetectionIfProvider
      - Data elements
        - Request : vss::vehicle::body::lights::hazard::Request
*/

/**********************************************************************************************************************
  Constructor
**********************************************************************************************************************/
SpeedHazardDetection::SpeedHazardDetection(ConstructorToken&& token)
    : SpeedHazardDetectionBase(std::move(token)), hazard_forward_(-1.0), hazard_reverse_(1.0) {
  HazardIfConsumer_->RegisterDataElementHandler_SpeedHazardForward(GetName(), [this](const ::vaf::ConstDataPtr<
                                                                                     const vss::vehicle::chassis::
                                                                                         SpeedHazardForward>&
                                                                                         hazard_sample) {
    if (hazard_sample) {
      hazard_forward_ =
          *hazard_sample;  // Set actual forward hazard limit. Assumption, this info is broadcasted at least once (at startup).
      vaf::OutputSyncStream{} << "[SpeedHazardDetection] " << "SpeedHazardForward set to: " << *hazard_sample
                              << std::endl;
    }
  });
  HazardIfConsumer_->RegisterDataElementHandler_SpeedHazardReverse(GetName(), [this](const ::vaf::ConstDataPtr<
                                                                                     const vss::vehicle::chassis::
                                                                                         SpeedHazardReverse>&
                                                                                         hazard_sample) {
    if (hazard_sample) {
      hazard_reverse_ =
          *hazard_sample;  // Set actual reverse hazard limit. Assumption, this info is broadcasted at least once (at startup).
      vaf::OutputSyncStream{} << "[SpeedHazardDetection] " << "SpeedHazardReverse set to: " << *hazard_sample
                              << std::endl;
    }
  });
}

/**********************************************************************************************************************
  1 periodic task(s)
**********************************************************************************************************************/
// Task with name HazardCalculation10 and a period of 10ms.
void SpeedHazardDetection::HazardCalculation10() {
  vss::vehicle::DriveDirection direction = SpeedCalculationIfConsumer_->Get_DriveDirection();
  float speed = SpeedCalculationIfConsumer_->Get_Speed();
  vss::vehicle::body::lights::hazard::IsSignaling signaling = HazardIfConsumer_->Get_IsSignaling();

  if (!signaling) {  // Only react if hazard lights are not signaling yet...
    if ((direction == vss::vehicle::FORWARD) && (speed > hazard_forward_)) {
      HazardDetectionIfProvider_->Set_Request(true);
      vaf::OutputSyncStream{} << "[SpeedHazardDetection] " << "SpeedHazardForward detected (" << speed << " km/h > "
                              << hazard_forward_ << " km/h)." << std::endl;
    }
    if ((direction == vss::vehicle::REVERSE) && (speed < hazard_reverse_)) {
      HazardDetectionIfProvider_->Set_Request(true);
      vaf::OutputSyncStream{} << "[SpeedHazardDetection] " << "SpeedHazardReverse detected (" << speed << " km/h > "
                              << hazard_reverse_ << " km/h)." << std::endl;
    }
  }
}

//vaf::Result<void> SpeedHazardDetection::Init() {
//  return vaf::Result<void>{};
//}
//void SpeedHazardDetection::Start() {
//  ReportOperational();
//}
//void SpeedHazardDetection::Stop() {}
//void SpeedHazardDetection::DeInit() {}
//void SpeedHazardDetection::OnError(const vaf::Error& error) {
//  static_cast<void>(error);
//}

}  // namespace demo
