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
/*!        \file  test_module.h
 *         \brief
 *
 *********************************************************************************************************************/

#ifndef DEMO_TEST_MODULE_H
#define DEMO_TEST_MODULE_H

#include "demo/test_module_base.h"

namespace demo {

class TestModule : public TestModuleBase {
 public:
  TestModule(ConstructorToken&& token);

  //vaf::Result<void> Init() override;
  //void Start() noexcept override;
  //void Stop() noexcept override;
  //void DeInit() noexcept override;
  //void OnError(const vaf::Error& error) override;

  void PeriodicTask200() override;
  void PeriodicTask2000() override;

 private:
  uint32_t ctr_;
};

}  // namespace demo

#endif  // DEMO_TEST_MODULE_H
