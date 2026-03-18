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
/*!        file  executable_controller_base.h
 *         brief
 *
 *********************************************************************************************************************/
#ifndef VAF_EXECUTABLE_CONTROLLER_BASE_H_
#define VAF_EXECUTABLE_CONTROLLER_BASE_H_

/*!********************************************************************************************************************
 *  INCLUDES
 *********************************************************************************************************************/
#include <atomic>
#include <thread>
#include <vector>

#include "vaf/controller_interface.h"
#include "vaf/executable_controller_interface.h"
#include "vaf/logging.h"
#include "vaf/module_states.h"
#include "vaf/runtime.h"
#include "vaf/user_controller_interface.h"

namespace vaf {

class ExecutableControllerBase : public vaf::ExecutableControllerInterface {
 public:
  ExecutableControllerBase();
  virtual ~ExecutableControllerBase() = default;

  ExecutableControllerBase(const ExecutableControllerBase&) = delete;
  ExecutableControllerBase(ExecutableControllerBase&&) = delete;
  ExecutableControllerBase& operator=(const ExecutableControllerBase&) = delete;
  ExecutableControllerBase& operator=(ExecutableControllerBase&&) = delete;

  void Run(bool use_exec_mgr = false) noexcept;
  void Run(int argc, char* argv[], bool use_exec_mgr = false) noexcept;
  void InitiateShutdown() noexcept;

  void RegisterModule(std::shared_ptr<vaf::ControlInterface> module);

 void ReportOperationalOfModule(std::string name) override;
 void ReportErrorOfModule(vaf::Error error, std::string name, bool critical) override;

 protected:
  virtual void DoInitialize();
  virtual void DoStart();
  virtual void DoShutdown();
  void WaitForShutdown();
  bool IsShutdownRequested();

 private:
  void ChangeStateOfModule(std::string name, ModuleStates state);
  void StartModules();
  void StartEventHandlersForModule(const std::string& module_name, const std::vector<std::string>& dependencies);
  void StopEventHandlersForModule(const std::string& module_name, const std::vector<std::string>& dependencies);
  void CheckStartingModules();

  class ModuleContainer {
  public:
    ModuleContainer(std::string name, std::shared_ptr<vaf::ControlInterface> module, std::vector<std::string> dependencies) 
      : name_{std::move(name)}, module_{std::move(module)}, dependencies_{std::move(dependencies)} {
    }
    std::string name_;
    std::shared_ptr<vaf::ControlInterface> module_;
    std::vector<std::string> dependencies_;
    ModuleStates state_{ModuleStates::kNotInitialized};
    uint64_t starting_counter_{0};
  };

  void SetupExecutionManager();
  void ReportStateToExecutionManager(bool is_running);

  /*!
   * \brief Entry point of the thread receiving signals from the execution manager.
   */
  void InitializeSignalHandling() noexcept;
  void SignalHandlerThread();

  int signal_handling_init_;
  vaf::Runtime runtime_;
  std::atomic_bool shutdown_requested_;
  bool use_execution_mgr_{false};
  
  vaf::Logger& logger_;
  std::vector<ModuleContainer> modules_;
  std::unique_ptr<UserControllerInterface> user_controller_;

  std::thread signal_handler_thread_;
};

}  // namespace vaf

#endif  // VAF_EXECUTABLE_CONTROLLER_BASE_H_
