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
#include "demo/speed_calculation_if_provider_mock.h"
#include "demo/speed_if_consumer_mock.h"
#include "demo/vehicle_speed_calculation.h"
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

class VehicleSpeedCalculationUnitTest : public ::testing::Test {
 protected:
  VehicleSpeedCalculationUnitTest() {}

  virtual ~VehicleSpeedCalculationUnitTest() {}

  virtual void SetUp() { InitializeSignalHandling(); }

  virtual void TearDown() {}
};

TEST_F(VehicleSpeedCalculationUnitTest, Test_1) {
  auto SpeedIfConsumerMock = std::make_shared<demo::SpeedIfConsumerMock>();
  auto SpeedCalculationIfProviderMock = std::make_shared<demo::SpeedCalculationIfProviderMock>();

  EXPECT_CALL(*SpeedIfConsumerMock, RegisterDataElementHandler_DriveType(_, _)).Times(1);

  auto VehicleSpeedCalculation = std::make_shared<demo::VehicleSpeedCalculation>(
      demo::VehicleSpeedCalculation ::ConstructorToken{SpeedIfConsumerMock, SpeedCalculationIfProviderMock});
}

TEST_F(VehicleSpeedCalculationUnitTest, Test_2) {
  auto SpeedIfConsumerMock = std::make_shared<demo::SpeedIfConsumerMock>();
  auto SpeedCalculationIfProviderMock = std::make_shared<demo::SpeedCalculationIfProviderMock>();

  EXPECT_CALL(*SpeedIfConsumerMock, RegisterDataElementHandler_DriveType(_, _)).Times(1);

  auto VehicleSpeedCalculation = std::make_shared<demo::VehicleSpeedCalculation>(
      demo::VehicleSpeedCalculation ::ConstructorToken{SpeedIfConsumerMock, SpeedCalculationIfProviderMock});

  EXPECT_CALL(*SpeedIfConsumerMock, Get_SpeedRow1WheelLeft()).Times(1).WillOnce(Return(1.0));
  EXPECT_CALL(*SpeedIfConsumerMock, Get_SpeedRow1WheelRight()).Times(1).WillOnce(Return(1.0));
  EXPECT_CALL(*SpeedIfConsumerMock, Get_SpeedRow2WheelLeft()).Times(1).WillOnce(Return(1.1));
  EXPECT_CALL(*SpeedIfConsumerMock, Get_SpeedRow2WheelRight()).Times(1).WillOnce(Return(1.2));

  EXPECT_CALL(*SpeedCalculationIfProviderMock, Set_Speed(1.075)).Times(1);

  VehicleSpeedCalculation->SpeedCalculation10();
}

}  // namespace vaf
