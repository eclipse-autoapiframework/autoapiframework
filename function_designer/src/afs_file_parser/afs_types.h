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

#ifndef AFP_AFS_TYPES_H
#define AFP_AFS_TYPES_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>


namespace afp 
{
    struct SupervisionAlive
    {
        uint32_t minIndications;
        uint32_t maxIndications;
        uint32_t referenceCycleMs;                
    };

    struct SupervisionDeadline
    {
        std::string description;
        uint32_t timeout;
    };
    struct SupervisionLogical
    {
        std::vector<std::string> predecessorRefs;
        std::vector<std::string> successorRefs;
    };
    struct Schedule
    {
        std::string functionName;
        std::string runType;
        std::string description;
        std::string cycleTimeMs;
        std::string implementedASIL;
        std::string previousRunnableRef;
        std::string schedulingPolicy;
        SupervisionAlive supervisionAlive;
        SupervisionDeadline supervisionDeadline;
        SupervisionLogical supervisionLogical;
        uint32_t stackSizeBytes;   
    };
    struct Error
    {
        std::string name;
        std::string dataType;
        std::string description;
        std::string severity;
        uint32_t maturationTimeMs;
        uint32_t resetTimeMs;
        std::string resetCondition;
        std::vector<std::string> dependencyRefs;
        std::string fallbackBehavior;
        std::vector<std::string> raisesSafetyReactionRefs;

    };
    struct Parameter
    {
        std::string name;
        std::string dataType;
        std::string defaultValue;
        std::string description;
        std::string unit;
        bool tunable;
        double min;
        double max;
        std::vector<uint32_t> dimensions;
    };     
    struct Signal
    {
        std::string name;
        std::string dataType;
        std::string description;
        std::string unit;
        double min;
        double max;
        std::string defaultValue;
        double resolution;
        double precision;
        bool FuSa;
        std::string ASIL;
        uint16_t minUpdatePeriodMs;
        std::string accuracy;
        std::string direction;
        bool protectionRequired;
        std::string runtimeCompanion;
    };    

    struct EnumDefinition
    {
        std::string name;
        std::string description;
        std::vector<std::string> values;
    };

    struct FunctionSpecification
    {
        std::vector<Signal> signals;
        std::vector<Schedule> scheduling;
        std::vector<Error> errors;      
        std::vector<Parameter> parameters;
        SupervisionAlive supervisionAlive;
        SupervisionDeadline supervisionDead;
        SupervisionLogical supervisionLogical;
        std::vector<EnumDefinition> enums;
    };
} // namespace afp

#endif // AFP_AFS_TYPES_H
