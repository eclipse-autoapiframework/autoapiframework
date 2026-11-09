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

#include "afs_file_parser.h"

#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>

#include "../yaml_parser.h"

namespace afp 
{
    namespace 
    {
        std::string Scalar(const afd::YamlNodePtr& node, const std::string& key)
        {
            const afd::YamlNodePtr value = node ? node->Find(key) : nullptr;
            return value && value->IsScalar() && value->GetScalar() != "null"
                ? value->GetScalar()
                : std::string();
        }

        bool ParseUnsigned(const std::string& value, uint32_t& result,
                                        const std::string& field, std::string& error)
        {
            if (value.empty())
            {
                result = 0;
                return true;
            }
            try
            {
                std::size_t parsed = 0;
                const unsigned long number = std::stoul(value, &parsed);
                if (parsed != value.size() || number > std::numeric_limits<uint32_t>::max())
                {
                    throw std::out_of_range("value");
                }
                result = static_cast<uint32_t>(number);
                return true;
            }
            catch (const std::exception&)
            {
                error = "invalid unsigned integer for " + field + ": " + value;
                return false;
            }
        }

        bool ParseDouble(const std::string& value, double& result,
                                        const std::string& field, std::string& error)
        {
            if (value.empty())
            {
                result = 0.0;
                return true;
            }
            try
            {
                std::size_t parsed = 0;
                result = std::stod(value, &parsed);
                if (parsed != value.size() || !std::isfinite(result))
                {
                    throw std::out_of_range("value");
                }
                return true;
            }
            catch (const std::exception&)
            {
                error = "invalid number for " + field + ": " + value;
                return false;
            }
        }

        bool ParseBool(const std::string& value, bool& result,
                                    const std::string& field, std::string& error)
        {
            if (value.empty())
            {
                result = false;
                return true;
            }
            if (value == "true")
            {
                result = true;
                return true;
            }
            if (value == "false")
            {
                result = false;
                return true;
            }
            error = "invalid boolean for " + field + ": " + value;
            return false;
        }

        bool ParseStringSequence(const afd::YamlNodePtr& node, const std::string& key,
                                                std::vector<std::string>& result, std::string& error)
        {
            const afd::YamlNodePtr values = node ? node->Find(key) : nullptr;
            if (!values)
            {
                return true;
            }
            if (!values->IsSequence())
            {
                error = key + " must be a sequence";
                return false;
            }
            for (const afd::YamlNodePtr& value : values->GetSequence())
            {
                if (!value || !value->IsScalar())
                {
                    error = key + " entries must be scalar values";
                    return false;
                }
                result.push_back(value->GetScalar());
            }
            return true;
        }

        bool ParseSupervision(const afd::YamlNodePtr& node, Schedule& schedule, std::string& error)
        {
            const afd::YamlNodePtr supervision = node ? node->Find("supervision") : nullptr;
            if (!supervision || !supervision->IsMap())
            {
                return true;
            }
            const afd::YamlNodePtr alive = supervision->Find("alive");
            if (alive && alive->IsMap())
            {
                if (!ParseUnsigned(Scalar(alive, "minIndications"), schedule.supervisionAlive.minIndications,
                                "supervision.alive.minIndications", error)
                    || !ParseUnsigned(Scalar(alive, "maxIndications"), schedule.supervisionAlive.maxIndications,
                                    "supervision.alive.maxIndications", error)
                    || !ParseUnsigned(Scalar(alive, "referenceCycleMs"), schedule.supervisionAlive.referenceCycleMs,
                                    "supervision.alive.referenceCycleMs", error))
                {
                    return false;
                }
            }
            const afd::YamlNodePtr deadline = supervision->Find("deadline");
            schedule.supervisionDeadline.description = Scalar(deadline, "description");
            if (!ParseUnsigned(Scalar(deadline, "maxExecutionTimeMs"),
                            schedule.supervisionDeadline.timeout,
                            "supervision.deadline.maxExecutionTimeMs", error))
            {
                return false;
            }
            return ParseStringSequence(supervision->Find("logical"), "predecessorRefs",
                                    schedule.supervisionLogical.predecessorRefs, error)
                && ParseStringSequence(supervision->Find("logical"), "successorRefs",
                                    schedule.supervisionLogical.successorRefs, error);
        }

        bool ParseEnums(const afd::YamlNodePtr& specification,
                                        FunctionSpecification& functionSpec, std::string& error)
        {
            const afd::YamlNodePtr enums = specification->Find("enums");
            if (!enums)
            {
                return true;
            }
            if (!enums->IsMap())
            {
                error = "enums must be a mapping";
                return false;
            }
            for (const auto& entry : enums->GetMap())
            {
                EnumDefinition definition{};
                definition.name = entry.first;
                afd::YamlNodePtr valueList = entry.second;
                if (valueList && valueList->IsMap())
                {
                    definition.description = Scalar(valueList, "description");
                    valueList = valueList->Find("values");
                }
                if (!valueList || !valueList->IsSequence())
                {
                    error = "enums." + entry.first + " must be a sequence or define 'values'";
                    return false;
                }
                for (const afd::YamlNodePtr& value : valueList->GetSequence())
                {
                    if (!value || !value->IsScalar())
                    {
                        error = "enums." + entry.first + " entries must be scalar values";
                        return false;
                    }
                    definition.values.push_back(value->GetScalar());
                }
                functionSpec.enums.push_back(std::move(definition));
            }
            return true;
        }

        bool ParseSignals(const afd::YamlNodePtr& specification,
                                        FunctionSpecification& functionSpec, std::string& error)
        {
            const afd::YamlNodePtr values = specification->Find("dataInterfaces");
            if (!values)
            {
                return true;
            }
            if (!values->IsSequence())
            {
                error = "dataInterfaces must be a sequence";
                return false;
            }
            for (const afd::YamlNodePtr& value : values->GetSequence())
            {
                if (!value || !value->IsMap())
                {
                    error = "dataInterfaces entries must be mappings";
                    return false;
                }
                Signal signal{};
                signal.name = Scalar(value, "name");
                signal.dataType = Scalar(value, "dataType");
                signal.description = Scalar(value, "description");
                signal.unit = Scalar(value, "unit");
                signal.defaultValue = Scalar(value, "defaultValue");
                signal.ASIL = Scalar(value, "ASIL");
                signal.accuracy = Scalar(value, "accuracy");
                signal.direction = Scalar(value, "direction");
                const afd::YamlNodePtr runtimeCompanion = value->Find("runtimeCompanion");
                signal.runtimeCompanion = Scalar(runtimeCompanion, "qualityCode");
                if (runtimeCompanion && runtimeCompanion->IsMap())
                {
                    signal.runtimeCompanion = Scalar(runtimeCompanion->Find("qualityCode"), "name");
                }
                uint32_t minUpdatePeriod = 0;
                if (!ParseDouble(Scalar(value, "min"), signal.min, "dataInterfaces.min", error)
                    || !ParseDouble(Scalar(value, "max"), signal.max, "dataInterfaces.max", error)
                    || !ParseDouble(Scalar(value, "resolution"), signal.resolution, "dataInterfaces.resolution", error)
                    || !ParseDouble(Scalar(value, "precision"), signal.precision, "dataInterfaces.precision", error)
                    || !ParseUnsigned(Scalar(value, "minUpdatePeriodMs"), minUpdatePeriod,
                                    "dataInterfaces.minUpdatePeriodMs", error)
                    || !ParseBool(Scalar(value, "FuSa"), signal.FuSa, "dataInterfaces.FuSa", error)
                    || !ParseBool(Scalar(value, "protectionRequired"), signal.protectionRequired,
                                "dataInterfaces.protectionRequired", error))
                {
                    return false;
                }
                if (minUpdatePeriod > std::numeric_limits<uint16_t>::max())
                {
                    error = "dataInterfaces.minUpdatePeriodMs exceeds uint16 range";
                    return false;
                }
                signal.minUpdatePeriodMs = static_cast<uint16_t>(minUpdatePeriod);
                functionSpec.signals.push_back(std::move(signal));
            }
            return true;
        }

        bool ParseErrors(const afd::YamlNodePtr& specification,
                                        FunctionSpecification& functionSpec, std::string& error)
        {
            afd::YamlNodePtr values = specification->Find("errorInterfaces");
            if (!values)
            {
                return true;
            }
            if (!values->IsSequence())
            {
                error = "errors must be a sequence";
                return false;
            }
            for (const afd::YamlNodePtr& value : values->GetSequence())
            {
                if (!value || !value->IsMap())
                {
                    error = "errors entries must be mappings";
                    return false;
                }
                Error parsedError{};
                parsedError.name = Scalar(value, "name");
                parsedError.dataType = Scalar(value, "dataType");
                parsedError.description = Scalar(value, "description");
                parsedError.severity = Scalar(value, "severity");
                parsedError.resetCondition = Scalar(value, "resetCondition");
                parsedError.fallbackBehavior = Scalar(value, "fallbackBehavior");
                if (!ParseUnsigned(Scalar(value, "maturationTimeMs"), parsedError.maturationTimeMs,
                                "errors.maturationTimeMs", error)
                    || !ParseUnsigned(Scalar(value, "resetTimeMs"), parsedError.resetTimeMs,
                                    "errors.resetTimeMs", error)
                    || !ParseStringSequence(value, "dependencyRefs", parsedError.dependencyRefs, error)
                    || !ParseStringSequence(value, "raisesSafetyReactionRefs",
                                            parsedError.raisesSafetyReactionRefs, error))
                {
                    return false;
                }
                functionSpec.errors.push_back(std::move(parsedError));
            }
            return true;
        }

        bool ParseParameters(const afd::YamlNodePtr& specification,
                                            FunctionSpecification& functionSpec, std::string& error)
        {
            const afd::YamlNodePtr values = specification->Find("parameters");
            if (!values)
            {
                return true;
            }
            if (!values->IsSequence())
            {
                error = "parameters must be a sequence";
                return false;
            }
            for (const afd::YamlNodePtr& value : values->GetSequence())
            {
                if (!value || !value->IsMap())
                {
                    error = "parameters entries must be mappings";
                    return false;
                }
                Parameter parameter{};
                parameter.name = Scalar(value, "name");
                parameter.dataType = Scalar(value, "dataType");
                parameter.defaultValue = Scalar(value, "defaultValue");
                parameter.description = Scalar(value, "description");
                parameter.unit = Scalar(value, "unit");
                if (!ParseBool(Scalar(value, "tunable"), parameter.tunable, "parameters.tunable", error)
                    || !ParseDouble(Scalar(value, "min"), parameter.min, "parameters.min", error)
                    || !ParseDouble(Scalar(value, "max"), parameter.max, "parameters.max", error))
                {
                    return false;
                }
                const afd::YamlNodePtr dimensions = value->Find("dimensions");
                if (dimensions)
                {
                    if (!dimensions->IsSequence())
                    {
                        error = "parameters.dimensions must be a sequence";
                        return false;
                    }
                    for (const afd::YamlNodePtr& dimension : dimensions->GetSequence())
                    {
                        uint32_t parsed = 0;
                        if (!dimension || !dimension->IsScalar()
                            || !ParseUnsigned(dimension->GetScalar(), parsed, "parameters.dimensions", error))
                        {
                            if (error.empty())
                            {
                                error = "parameters.dimensions entries must be unsigned integers";
                            }
                            return false;
                        }
                        parameter.dimensions.push_back(parsed);
                    }
                }
                functionSpec.parameters.push_back(std::move(parameter));
            }
            return true;
        }

        bool ParseScheduling(const afd::YamlNodePtr& specification,
                                            FunctionSpecification& functionSpec, std::string& error)
        {
            const afd::YamlNodePtr values = specification->Find("scheduling");
            if (!values)
            {
                return true;
            }
            if (!values->IsSequence())
            {
                error = "scheduling must be a sequence";
                return false;
            }
            for (const afd::YamlNodePtr& value : values->GetSequence())
            {
                if (!value || !value->IsMap())
                {
                    error = "scheduling entries must be mappings";
                    return false;
                }
                Schedule schedule{};
                schedule.functionName = Scalar(value, "functionName");
                schedule.runType = Scalar(value, "runType");
                schedule.description = Scalar(value, "description");
                schedule.cycleTimeMs = Scalar(value, "cycleTimeMs");
                schedule.implementedASIL = Scalar(value, "implementedASIL");
                schedule.previousRunnableRef = Scalar(value, "previousRunnableRef");
                schedule.schedulingPolicy = Scalar(value, "schedulingPolicy");
                if (!ParseUnsigned(Scalar(value, "stackSizeBytes"), schedule.stackSizeBytes,
                                "scheduling.stackSizeBytes", error)
                    || !ParseSupervision(value, schedule, error))
                {
                    return false;
                }
                functionSpec.scheduling.push_back(std::move(schedule));
            }
            return true;
        }
    }

    bool AfsFileParser::ParseAfsFile(const std::string& content, FunctionSpecification& afsStructure, std::string& error)
    {
        error.clear();
        afd::YamlParser parser;
        const afd::YamlNodePtr root = parser.ParseText(content, error);
        if (!root)
        {
            return false;
        }
        if (!root->IsMap())
        {
            error = "AFS document root must be a mapping";
            return false;
        }
        const afd::YamlNodePtr specification = root->Find("functionSpecification");
        if (!specification || !specification->IsMap())
        {
            error = "AFS document must contain a functionSpecification mapping";
            return false;
        }

        FunctionSpecification parsedStructure{};
        if (!ParseEnums(specification, parsedStructure, error)
            || !ParseSignals(specification, parsedStructure, error)
            || !ParseParameters(specification, parsedStructure, error)
            || !ParseScheduling(specification, parsedStructure, error)
            || !ParseErrors(specification, parsedStructure, error))
        {
            return false;
        }
        afsStructure = std::move(parsedStructure);
        return true;
    }
} // namespace afp
