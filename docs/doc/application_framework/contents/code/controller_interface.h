/*!********************************************************************************************************************
 *  COPYRIGHT
 *  -------------------------------------------------------------------------------------------------------------------
 *  
 *  Copyright (c) 2025 by Vector Informatik GmbH. All rights reserved.
 *
 *                This software is copyright protected and proprietary to Vector Informatik GmbH.
 *                Vector Informatik GmbH grants to you only those rights as set out in the license conditions.
 *                All other rights remain with Vector Informatik GmbH.
 *  
 *  -------------------------------------------------------------------------------------------------------------------
 *  FILE DESCRIPTION
 *  -----------------------------------------------------------------------------------------------------------------*/
/*!        file  controller_interface.h
 *         brief
 *
 *********************************************************************************************************************/
#ifndef INCLUDE_VAF_CONTROLLER_INTERFACE_H
#define INCLUDE_VAF_CONTROLLER_INTERFACE_H

#include "vaf/error_domain.h"
#include "vaf/executable_controller_interface.h"
#include "vaf/executor.h"
#include "vaf/result.h"

namespace vaf {

class ControlInterface {
 public:
  ControlInterface(std::string name, std::vector<std::string> dependencies, ExecutableControllerInterface& executable_controller_interface, vaf::Executor& executor);
  ControlInterface(const ControlInterface&) = delete;
  ControlInterface(ControlInterface&&) = delete;
  ControlInterface& operator=(const ControlInterface&) = delete;
  ControlInterface& operator=(ControlInterface&&) = delete;

  virtual ~ControlInterface() = default;

  virtual vaf::Result<void> Init() noexcept = 0;
  virtual void Start() noexcept = 0;
  virtual void Stop() noexcept = 0;
  virtual void DeInit() noexcept = 0;

  void ReportOperational();
  void ReportError(ErrorCode error_code, std::string msg, bool critical = false);

  virtual void OnError(const vaf::Error& error);

  std::string GetName();
  std::vector<std::string> GetDependencies();

  void StartExecutor();
  void StopExecutor();

  virtual void StartEventHandlerForModule(const std::string& module_name);
  virtual void StopEventHandlerForModule(const std::string& module_name);

protected:
  std::string name_;
  std::vector<std::string> dependencies_;
  ExecutableControllerInterface& executable_controller_interface_;
  vaf::ModuleExecutor executor_;
};

} // namespace vaf

#endif  // INCLUDE_VAF_CONTROLLER_INTERFACE_H
