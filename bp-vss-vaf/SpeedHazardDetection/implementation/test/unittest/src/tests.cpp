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
#include "demo/hazard_detection_if_provider_mock.h"
#include "demo/hazard_if_consumer_mock.h"
#include "demo/speed_calculation_if_consumer_mock.h"
#include "demo/speed_hazard_detection.h"
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

class SpeedHazardDetectionUnitTest : public ::testing::Test {
 protected:
  SpeedHazardDetectionUnitTest() {}

  virtual ~SpeedHazardDetectionUnitTest() {}

  virtual void SetUp() { InitializeSignalHandling(); }

  virtual void TearDown() {}
};

TEST_F(SpeedHazardDetectionUnitTest, Test_1) {
  auto SpeedCalculationIfConsumerMock = std::make_shared<demo::SpeedCalculationIfConsumerMock>();
  auto HazardIfConsumerMock = std::make_shared<demo::HazardIfConsumerMock>();
  auto HazardDetectionIfProviderMock = std::make_shared<demo::HazardDetectionIfProviderMock>();

  EXPECT_CALL(*HazardIfConsumerMock, RegisterDataElementHandler_SpeedHazardForward(_, _)).Times(1);
  EXPECT_CALL(*HazardIfConsumerMock, RegisterDataElementHandler_SpeedHazardReverse(_, _)).Times(1);

  auto SpeedHazardDetection =
      std::make_shared<demo::SpeedHazardDetection>(demo::SpeedHazardDetection ::ConstructorToken{
          SpeedCalculationIfConsumerMock, HazardIfConsumerMock, HazardDetectionIfProviderMock});
}
}  // namespace vaf
