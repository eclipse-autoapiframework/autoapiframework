/********************************************************************************
 * Copyright (c) 2026 ZF Friedrichshafen AG
 *
 * See the NOTICE file(s) distributed with this work for additional
 * information regarding copyright ownership.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Contributors:
 *   Thomas Pfleiderer - afs file parser added
 ********************************************************************************/

#include "afs_file_expectation.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace afp
{
    namespace
    {
        std::string Trim(const std::string& text)
        {
            const char* whitespace = " \t\r\n";
            const std::size_t first = text.find_first_not_of(whitespace);
            if (first == std::string::npos)
            {
                return std::string();
            }
            return text.substr(first, text.find_last_not_of(whitespace) - first + 1);
        }

        struct Expectations
        {
            std::map<std::string, std::size_t> counts;
            std::map<std::string, std::set<std::string>> supervision;
        };

        // Lines: `enums = 2`, `signals|parameters|schedules|errors = N`,
        // `supervision <functionName> = none|alive,deadline,logical`; '#' starts a comment.
        bool LoadExpectations(const std::string& path, Expectations& expectations, std::string& error)
        {
            std::ifstream input(path);
            if (!input)
            {
                error = "Cannot open expectations file: " + path;
                return false;
            }
            const std::set<std::string> countKeys = {"enums", "signals", "parameters", "schedules", "errors"};
            const std::set<std::string> kinds = {"none", "alive", "deadline", "logical"};
            std::string line;
            int lineNumber = 0;
            while (std::getline(input, line))
            {
                ++lineNumber;
                line = Trim(line.substr(0, line.find('#')));
                if (line.empty())
                {
                    continue;
                }
                const std::size_t equals = line.find('=');
                if (equals == std::string::npos)
                {
                    error = path + ":" + std::to_string(lineNumber) + ": expected 'key = value'";
                    return false;
                }
                const std::string key = Trim(line.substr(0, equals));
                const std::string value = Trim(line.substr(equals + 1));
                if (key.rfind("supervision ", 0) == 0)
                {
                    const std::string name = Trim(key.substr(std::string("supervision ").size()));
                    std::set<std::string> expected;
                    std::stringstream parts(value);
                    std::string part;
                    while (std::getline(parts, part, ','))
                    {
                        part = Trim(part);
                        if (kinds.find(part) == kinds.end())
                        {
                            error = path + ":" + std::to_string(lineNumber)
                                  + ": supervision must be none, alive, deadline or logical";
                            return false;
                        }
                        if (part != "none")
                        {
                            expected.insert(part);
                        }
                    }
                    expectations.supervision[name] = expected;
                }
                else if (countKeys.find(key) != countKeys.end())
                {
                    try
                    {
                        std::size_t parsed = 0;
                        const unsigned long number = std::stoul(value, &parsed);
                        if (parsed != value.size())
                        {
                            throw std::invalid_argument("value");
                        }
                        expectations.counts[key] = number;
                    }
                    catch (const std::exception&)
                    {
                        error = path + ":" + std::to_string(lineNumber) + ": invalid number '" + value + "'";
                        return false;
                    }
                }
                else
                {
                    error = path + ":" + std::to_string(lineNumber) + ": unknown key '" + key + "'";
                    return false;
                }
            }
            return true;
        }

        std::set<std::string> ActualSupervision(const Schedule& schedule)
        {
            std::set<std::string> kinds;
            const SupervisionAlive& alive = schedule.supervisionAlive;
            if (alive.minIndications != 0 || alive.maxIndications != 0 || alive.referenceCycleMs != 0)
            {
                kinds.insert("alive");
            }
            if (schedule.supervisionDeadline.timeout != 0 || !schedule.supervisionDeadline.description.empty())
            {
                kinds.insert("deadline");
            }
            if (!schedule.supervisionLogical.predecessorRefs.empty()
                || !schedule.supervisionLogical.successorRefs.empty())
            {
                kinds.insert("logical");
            }
            return kinds;
        }

        std::string Join(const std::set<std::string>& kinds)
        {
            if (kinds.empty())
            {
                return "none";
            }
            std::string text;
            for (const std::string& kind : kinds)
            {
                text += (text.empty() ? "" : ",") + kind;
            }
            return text;
        }

        std::vector<std::string> Compare(const Expectations& expectations, const FunctionSpecification& spec)
        {
            std::vector<std::string> mismatches;
            const std::map<std::string, std::size_t> actual = {
                {"enums", spec.enums.size()},
                {"signals", spec.signals.size()},
                {"parameters", spec.parameters.size()},
                {"schedules", spec.scheduling.size()},
                {"errors", spec.errors.size()}};
            for (const auto& expected : expectations.counts)
            {
                const std::size_t found = actual.at(expected.first);
                if (found != expected.second)
                {
                    mismatches.push_back(expected.first + ": expected " + std::to_string(expected.second)
                                         + ", found " + std::to_string(found));
                }
            }
            for (const auto& expected : expectations.supervision)
            {
                const auto schedule = std::find_if(spec.scheduling.begin(), spec.scheduling.end(),
                    [&](const Schedule& entry) { return entry.functionName == expected.first; });
                if (schedule == spec.scheduling.end())
                {
                    mismatches.push_back("supervision " + expected.first + ": schedule not found");
                    continue;
                }
                const std::set<std::string> found = ActualSupervision(*schedule);
                if (found != expected.second)
                {
                    mismatches.push_back("supervision " + expected.first + ": expected " + Join(expected.second)
                                         + ", found " + Join(found));
                }
            }
            return mismatches;
        }
    } // namespace

    bool AfsFileExpectation::CheckExpectation(const std::string& expectationsFile,
                                              const FunctionSpecification& spec, std::string& error) const
    {
        Expectations expectations;
        if (!LoadExpectations(expectationsFile, expectations, error))
        {
            return false;
        }
        const std::vector<std::string> mismatches = Compare(expectations, spec);
        for (const std::string& mismatch : mismatches)
        {
            error += (error.empty() ? "" : "\n") + std::string("Mismatch - ") + mismatch;
        }
        return mismatches.empty();
    }
} // namespace afp
