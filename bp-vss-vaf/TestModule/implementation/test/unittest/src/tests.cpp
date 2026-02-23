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
/*!        \file  tests.cpp
 *         \brief
 *
 *********************************************************************************************************************/

#include <csignal>
#include <cstddef>
#include <cstdint>
#include "demo/hazard_detection_if_consumer_mock.h"
#include "demo/hazard_if_provider_mock.h"
#include "demo/speed_if_provider_mock.h"
#include "demo/test_module.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// Put inside test to skip
//GTEST_SKIP() << "Skipping single test";
using ::testing::_;
using ::testing::Return;

namespace vaf {
/*!
 * \brief Initializes the signal handling.
 * \return void.
 */
void InitializeSignalHandling() noexcept {
  bool success{true};
  sigset_t signals;

  /* Block all signals except the SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV signals because blocking them will lead to
   * undefined behavior. Their default handling shall not be changed (dependent on underlying POSIX environment, usually
   * process is killed and a dump file is written). Signal mask will be inherited by subsequent threads. */

  success = success && (0 == sigfillset(&signals));
  success = success && (0 == sigdelset(&signals, SIGABRT));
  success = success && (0 == sigdelset(&signals, SIGBUS));
  success = success && (0 == sigdelset(&signals, SIGFPE));
  success = success && (0 == sigdelset(&signals, SIGILL));
  success = success && (0 == sigdelset(&signals, SIGSEGV));
  success = success && (0 == pthread_sigmask(SIG_SETMASK, &signals, nullptr));

  if (!success) {
    // Exit
  }
}

class TestModuleUnitTest : public ::testing::Test {
 protected:
  TestModuleUnitTest() {}

  virtual ~TestModuleUnitTest() {}

  virtual void SetUp() { InitializeSignalHandling(); }

  virtual void TearDown() {}
};

TEST_F(TestModuleUnitTest, Test_1) {
  auto HazardDetectionIfConsumerMock = std::make_shared<demo::HazardDetectionIfConsumerMock>();
  auto SpeedIfProviderMock = std::make_shared<demo::SpeedIfProviderMock>();
  auto HazardIfProviderMock = std::make_shared<demo::HazardIfProviderMock>();

  auto TestModule = std::make_shared<demo::TestModule>(
      demo::TestModule ::ConstructorToken{HazardDetectionIfConsumerMock, SpeedIfProviderMock, HazardIfProviderMock});
}
}  // namespace vaf
