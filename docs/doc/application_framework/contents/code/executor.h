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
#ifndef INCLUDE_VAF_EXECUTOR_H
#define INCLUDE_VAF_EXECUTOR_H

#include "vaf/logging.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace vaf {

    class RunnableHandle {
    public:
        RunnableHandle(std::string name, uint64_t period, std::function<void(void)> runnable, const std::string &owner,
                       const std::vector<std::string> &run_after, uint64_t offset, std::chrono::nanoseconds budget);

        const std::string &Name() const;

        bool IsActive() const;

        void Execute() const;

        uint64_t Period() const;

        void Start();

        void Stop();

        const std::string &Owner();

        const std::vector<std::string> &RunAfter();

        uint64_t Offset() const;

        std::chrono::nanoseconds Budget() const;

    private:
        std::string name_;
        bool is_active_{false};
        uint64_t period_;
        std::function<void(void)> runnable_;
        std::string owner_;
        std::vector<std::string> run_after_;
        uint64_t offset_;
        std::chrono::nanoseconds budget_;
    };

    class Executor {
    public:
        explicit Executor(std::chrono::milliseconds running_period);

        ~Executor();

        Executor(const Executor &) = delete;

        Executor(Executor &&) = delete;

        Executor &operator=(const Executor &) = delete;

        Executor &operator=(Executor &&) = delete;

        template<typename T>
        std::shared_ptr<RunnableHandle> RunPeriodic(std::chrono::milliseconds period,
                                                    T &&runnable,
                                                    const std::string &owner,
                                                    const std::vector<std::string> &run_after,
                                                    uint64_t offset = 0,
                                                    std::chrono::nanoseconds budget = std::chrono::nanoseconds{0}) {

            return RunPeriodic("", period, std::move(runnable), owner, run_after, {}, offset, budget);
        }

        template<typename T>
        std::shared_ptr<RunnableHandle> RunPeriodic(const std::string &name,
                                                    std::chrono::milliseconds period,
                                                    T &&runnable,
                                                    const std::string &owner,
                                                    const std::vector<std::string> &run_after,
                                                    const std::vector<std::string> &run_after_runnables = {},
                                                    uint64_t offset = 0,
                                                    std::chrono::nanoseconds budget = std::chrono::nanoseconds{0}) {
            auto check_can_run = [this, &run_after, &run_after_runnables](
                    std::vector<std::shared_ptr<RunnableHandle>>::iterator current_position) {
                auto pos{std::next(current_position)};
                bool can_run{true};
                for (; pos != runnables_.end(); ++pos) {
                    if (std::any_of(run_after.begin(), run_after.end(), [&pos](const std::string &run_after_element) {
                        return pos->get()->Owner() == run_after_element;
                    })) {
                        can_run = false;
                        break;
                    }
                    if (current_position->get()->Owner() == pos->get()->Owner()) {
                        if (std::any_of(run_after_runnables.begin(), run_after_runnables.end(),
                                        [&pos](const std::string &run_after_element) {
                                            return pos->get()->Name() == run_after_element;
                                        })) {
                            can_run = false;
                            break;
                        }
                    }
                }

                return can_run;
            };

            auto search_pos{runnables_.begin()};
            for (; search_pos != runnables_.end(); ++search_pos) {
                if (check_can_run(search_pos)) {
                    break;
                }
            }

            // TODO(virmlj) implement run_after_runnables

            auto insert_pos = runnables_.insert(search_pos,
                                                std::make_unique<RunnableHandle>(name, period / running_period_,
                                                                                 std::forward<T>(runnable), owner,
                                                                                 run_after, offset, budget));
            return *insert_pos;
        }

    private:
        void ExecutorThread();

        void ExecuteRunnable(RunnableHandle &runnable);

        std::chrono::milliseconds running_period_;
        std::vector<std::shared_ptr<RunnableHandle>> runnables_{};
        std::atomic<bool> exit_requested_{false};
        std::thread thread_;
        vaf::Logger &logger_;
    };

    class ModuleExecutor {
    public:
        ModuleExecutor(Executor &executor, std::string name, std::vector<std::string> dependencies);

        template<typename T>
        void RunPeriodic(std::chrono::milliseconds period, T &&runnable, uint64_t offset = 0,
                         std::chrono::nanoseconds budget = std::chrono::nanoseconds{0}) {
            handles_.emplace_back(
                    executor_.RunPeriodic(period, std::move(runnable), name_, dependencies_, offset, budget));

            if (started_) {
                handles_.back()->Start();
            }
        }

        template<typename T>
        void RunPeriodic(const std::string &name, std::chrono::milliseconds period, T &&runnable,
                         std::vector<std::string> runnable_dependencies = {}, uint64_t offset = 0,
                         std::chrono::nanoseconds budget = std::chrono::nanoseconds{0}) {
            handles_.emplace_back(executor_.RunPeriodic(name, period, std::move(runnable), name_, dependencies_,
                                                        std::move(runnable_dependencies), offset, budget));

            if (started_) {
                handles_.back()->Start();
            }
        }

        void Start();

        void Stop();

    private:
        Executor &executor_;
        std::vector<std::shared_ptr<RunnableHandle>> handles_;
        bool started_;
        std::string name_;
        std::vector<std::string> dependencies_;
    };

} // namespace vaf

#endif // INCLUDE_VAF_EXECUTOR_H