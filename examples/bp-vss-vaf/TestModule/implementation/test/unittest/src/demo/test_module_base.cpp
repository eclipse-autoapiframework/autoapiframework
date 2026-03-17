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
/*!        \file  test_module_base.cpp
 *         \brief
 *
 *********************************************************************************************************************/

#include "demo/test_module_base.h"

namespace demo {

TestModuleBase::TestModuleBase(ConstructorToken&& token)
    : HazardDetectionIfConsumer_{std::move(token.HazardDetectionIfConsumer_)},
      SpeedIfProvider_{std::move(token.SpeedIfProvider_)},
      HazardIfProvider_{std::move(token.HazardIfProvider_)} {}

}  // namespace demo
