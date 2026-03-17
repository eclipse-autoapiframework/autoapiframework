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
/*!        \file  vehicle_speed_calculation_base.cpp
 *         \brief
 *
 *********************************************************************************************************************/

#include "demo/vehicle_speed_calculation_base.h"

namespace demo {

VehicleSpeedCalculationBase::VehicleSpeedCalculationBase(ConstructorToken&& token)
    : SpeedIfConsumer_{std::move(token.SpeedIfConsumer_)},
      SpeedCalculationIfProvider_{std::move(token.SpeedCalculationIfProvider_)} {}

}  // namespace demo
