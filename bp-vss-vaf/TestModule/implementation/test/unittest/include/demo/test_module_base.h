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
/*!        \file  test_module_base.h
 *         \brief
 *
 *********************************************************************************************************************/

#ifndef DEMO_TEST_MODULE_BASE_H
#define DEMO_TEST_MODULE_BASE_H

#include <memory>
#include "vaf/controller_interface.h"

#include "demo/hazard_detection_if_consumer.h"
#include "demo/hazard_if_provider.h"
#include "demo/speed_if_provider.h"

namespace demo {

class TestModuleBase {
 public:
  struct ConstructorToken {
    std::shared_ptr<demo::HazardDetectionIfConsumer> HazardDetectionIfConsumer_;
    std::shared_ptr<demo::SpeedIfProvider> SpeedIfProvider_;
    std::shared_ptr<demo::HazardIfProvider> HazardIfProvider_;
  };

  TestModuleBase(ConstructorToken&& token);
  virtual ~TestModuleBase() = default;

  void ReportError(const vaf::Error&, bool) {}
  virtual void OnError(const vaf::Error&) {}
  vaf::String GetName() { return ""; }

  virtual void PeriodicTask200() = 0;
  virtual void PeriodicTask2000() = 0;

 protected:
  std::shared_ptr<demo::HazardDetectionIfConsumer> HazardDetectionIfConsumer_;
  std::shared_ptr<demo::SpeedIfProvider> SpeedIfProvider_;
  std::shared_ptr<demo::HazardIfProvider> HazardIfProvider_;
};

}  // namespace demo

#endif  // DEMO_TEST_MODULE_BASE_H
