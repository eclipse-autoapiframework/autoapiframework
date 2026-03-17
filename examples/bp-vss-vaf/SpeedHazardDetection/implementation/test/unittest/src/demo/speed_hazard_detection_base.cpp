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
/*!        \file  speed_hazard_detection_base.cpp
 *         \brief
 *
 *********************************************************************************************************************/

#include "demo/speed_hazard_detection_base.h"

namespace demo {

SpeedHazardDetectionBase::SpeedHazardDetectionBase(ConstructorToken&& token)
    : SpeedCalculationIfConsumer_{std::move(token.SpeedCalculationIfConsumer_)},
      HazardIfConsumer_{std::move(token.HazardIfConsumer_)},
      HazardDetectionIfProvider_{std::move(token.HazardDetectionIfProvider_)} {}

}  // namespace demo
