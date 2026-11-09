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
 *   Thomas Pfleiderer - CTest added
 ********************************************************************************/

#include "../src/afs_file_parser/afs_file_parser.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace
{
    void Require(bool condition, const std::string& message)
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    std::string ReadFile(const std::string& path)
    {
        std::ifstream input(path);
        Require(static_cast<bool>(input), "could not open example: " + path);
        std::ostringstream content;
        content << input.rdbuf();
        return content.str();
    }

    // -------------------------------------------------------------------------
    // Existing tests
    // -------------------------------------------------------------------------

    void TestSampleExtraction()
    {
        afp::AfsFileParser parser;
        afp::FunctionSpecification functionSpec{};
        std::string error;
        const std::string path = std::string(AFS_PARSER_EXAMPLES_DIR) + "/wheel_speed_plausibility_check.afs";
        Require(parser.ParseAfsFile(ReadFile(path), functionSpec, error), "sample parse failed: " + error);
        Require(!functionSpec.signals.empty(), "signals were not extracted");
        Require(!functionSpec.parameters.empty(), "parameters were not extracted");
        Require(!functionSpec.scheduling.empty(), "scheduling entries were not extracted");

        bool foundLogicalSchedule = false;
        for (const afp::Schedule& schedule : functionSpec.scheduling)
        {
            if (schedule.functionName == "WheelSpeedPlausibilityCheck.Step")
            {
                foundLogicalSchedule = true;
                Require(schedule.supervisionLogical.predecessorRefs.size() == 1,
                        "logical predecessor was not extracted");
                Require(schedule.supervisionLogical.predecessorRefs.front() == "WheelSpeedPreProcessor.Step",
                        "logical predecessor value changed");
                Require(schedule.supervisionDeadline.timeout == 15,
                        "deadline timeout was not extracted");
            }
        }
        Require(foundLogicalSchedule, "expected schedule was not extracted");
    }

    void TestInvalidDocument()
    {
        afp::AfsFileParser parser;
        afp::FunctionSpecification functionSpec{};
        std::string error;
        Require(!parser.ParseAfsFile("notAnAfsDocument: true\n", functionSpec, error),
                "document without functionSpecification was accepted");
        Require(!error.empty(), "invalid document did not report an error");
    }

    // -------------------------------------------------------------------------
    // Example file coverage
    // -------------------------------------------------------------------------

    // Verify detailed field values extracted from speed_hazard_detection.afs.
    void TestSpeedHazardDetectionExample()
    {
        afp::AfsFileParser parser;
        afp::FunctionSpecification spec{};
        std::string error;
        const std::string path = std::string(AFS_PARSER_EXAMPLES_DIR) + "/speed_hazard_detection.afs";
        Require(parser.ParseAfsFile(ReadFile(path), spec, error),
                "speed_hazard_detection parse failed: " + error);

        // Three data interfaces: two inputs and one output.
        Require(spec.signals.size() == 3, "speed_hazard_detection should have 3 signals");

        const afp::Signal* speedSignal = nullptr;
        for (const afp::Signal& s : spec.signals)
        {
            if (s.name == "Vehicle.Speed") { speedSignal = &s; break; }
        }
        Require(speedSignal != nullptr, "Vehicle.Speed signal not found");
        Require(speedSignal->direction         == "input", "Vehicle.Speed direction should be 'input'");
        Require(speedSignal->dataType          == "float", "Vehicle.Speed dataType should be 'float'");
        Require(speedSignal->ASIL              == "B",     "Vehicle.Speed ASIL should be 'B'");
        Require(speedSignal->FuSa              == true,    "Vehicle.Speed FuSa should be true");
        Require(speedSignal->min               == 0.0,     "Vehicle.Speed min should be 0.0");
        Require(speedSignal->max               == 300.0,   "Vehicle.Speed max should be 300.0");
        Require(speedSignal->minUpdatePeriodMs == 10,      "Vehicle.Speed minUpdatePeriodMs should be 10");
        Require(speedSignal->runtimeCompanion  == "Vehicle.Speed.Qualifier",
                "Vehicle.Speed runtimeCompanion name wrong");

        bool foundOutput = false;
        for (const afp::Signal& s : spec.signals)
        {
            if (s.direction == "output") { foundOutput = true; break; }
        }
        Require(foundOutput, "no output signal found in speed_hazard_detection");

        // Two parameters.
        Require(spec.parameters.size() == 2, "speed_hazard_detection should have 2 parameters");

        const afp::Parameter* durationParam = nullptr;
        for (const afp::Parameter& p : spec.parameters)
        {
            if (p.name == "Vehicle.Chassis.HazardRequestDurationMs") { durationParam = &p; break; }
        }
        Require(durationParam != nullptr, "HazardRequestDurationMs parameter not found");
        Require(durationParam->dataType     == "uint32", "HazardRequestDurationMs dataType wrong");
        Require(durationParam->defaultValue == "3000",   "HazardRequestDurationMs defaultValue wrong");
        Require(durationParam->tunable      == false,    "HazardRequestDurationMs should not be tunable");
        Require(durationParam->min          == 0.0,      "HazardRequestDurationMs min should be 0.0");
        Require(durationParam->max          == 600000.0, "HazardRequestDurationMs max should be 600000");

        // Three scheduling entries: init, cyclic, terminate.
        Require(spec.scheduling.size() == 3, "speed_hazard_detection should have 3 scheduling entries");

        const afp::Schedule* stepSchedule = nullptr;
        for (const afp::Schedule& s : spec.scheduling)
        {
            if (s.functionName == "SpeedHazardDetection.Step") { stepSchedule = &s; break; }
        }
        Require(stepSchedule != nullptr, "SpeedHazardDetection.Step schedule not found");
        Require(stepSchedule->runType             == "cyclic",
                "SpeedHazardDetection.Step runType should be 'cyclic'");
        Require(stepSchedule->cycleTimeMs         == "20",
                "SpeedHazardDetection.Step cycleTimeMs should be '20'");
        Require(stepSchedule->implementedASIL     == "B",
                "SpeedHazardDetection.Step implementedASIL should be 'B'");
        Require(stepSchedule->previousRunnableRef == "SpeedHazardDetection.Init",
                "SpeedHazardDetection.Step previousRunnableRef wrong");

        bool foundInit = false;
        bool foundTerminate = false;
        for (const afp::Schedule& s : spec.scheduling)
        {
            if (s.runType == "init")      foundInit      = true;
            if (s.runType == "terminate") foundTerminate = true;
        }
        Require(foundInit,      "init runnable not found in speed_hazard_detection");
        Require(foundTerminate, "terminate runnable not found in speed_hazard_detection");
    }

    // Null min/max/resolution/precision fields in vehicle_speed_fusion_multirate.afs
    // must parse without error and default to zero.
    void TestVehicleSpeedFusionNullFields()
    {
        afp::AfsFileParser parser;
        afp::FunctionSpecification spec{};
        std::string error;
        const std::string path = std::string(AFS_PARSER_EXAMPLES_DIR) + "/vehicle_speed_fusion_multirate.afs";
        Require(parser.ParseAfsFile(ReadFile(path), spec, error),
                "vehicle_speed_fusion parse failed: " + error);

        // Five data interfaces: four wheel-speed inputs and one vehicle-speed output.
        Require(spec.signals.size() == 5, "vehicle_speed_fusion should have 5 signals");

        for (const afp::Signal& s : spec.signals)
        {
            if (s.direction == "input")
            {
                Require(s.min        == 0.0, s.name + ": null min should default to 0.0");
                Require(s.max        == 0.0, s.name + ": null max should default to 0.0");
                Require(s.resolution == 0.0, s.name + ": null resolution should default to 0.0");
            }
        }

        // The fusion step must reference the preprocessor as its predecessor.
        const afp::Schedule* stepSchedule = nullptr;
        for (const afp::Schedule& s : spec.scheduling)
        {
            if (s.functionName == "VehicleSpeedFusion.Step") { stepSchedule = &s; break; }
        }
        Require(stepSchedule != nullptr, "VehicleSpeedFusion.Step schedule not found");
        Require(stepSchedule->previousRunnableRef == "VehicleSpeedPreProcessor.Step",
                "VehicleSpeedFusion.Step should reference VehicleSpeedPreProcessor.Step");
        Require(stepSchedule->cycleTimeMs == "20",
                "VehicleSpeedFusion.Step cycleTimeMs should be '20'");
    }

    // The file named mandatory_property_is_missing_syntax_error.afs is
    // syntactically valid YAML, so the AFS parser accepts it.  Semantic checks
    // for mandatory business-level properties are the responsibility of the
    // higher-level validation layer.
    void TestMissingMandatoryPropertyFile()
    {
        afp::AfsFileParser parser;
        afp::FunctionSpecification spec{};
        std::string error;
        const std::string path = std::string(AFS_PARSER_EXAMPLES_DIR)
                                 + "/mandatory_property_is_missing_syntax_error.afs";
        Require(parser.ParseAfsFile(ReadFile(path), spec, error),
                "mandatory_property_is_missing example should parse successfully: " + error);
        Require(spec.signals.size()    == 3, "expected 3 signals");
        Require(spec.parameters.size() == 2, "expected 2 parameters");
        Require(spec.scheduling.size() == 3, "expected 3 scheduling entries");

        // Vehicle.Acceleration.Longitudinal has an explicitly empty description
        // in this file; it should parse as an empty string.
        const afp::Signal* accelSignal = nullptr;
        for (const afp::Signal& s : spec.signals)
        {
            if (s.name == "Vehicle.Acceleration.Longitudinal") { accelSignal = &s; break; }
        }
        Require(accelSignal != nullptr, "Vehicle.Acceleration.Longitudinal not found");
        Require(accelSignal->description.empty(),
                "an explicitly null description should parse as an empty string");
    }

    // -------------------------------------------------------------------------
    // Inline-document tests for features not exercised by the example files
    // -------------------------------------------------------------------------

    // The parser stores error interfaces under the key "errors".
    void TestErrorInterfaces()
    {
        afp::AfsFileParser parser;
        afp::FunctionSpecification spec{};
        std::string error;

        const std::string yaml =
            "functionSpecification:\n"
            "  name: TestFunc\n"
            "  version: 1.0.0\n"
            "  errors:\n"
            "    - name: TestFunc.OverheatError.ErrorStatus\n"
            "      dataType: boolean\n"
            "      description: Temperature exceeded safe threshold.\n"
            "      severity: critical\n"
            "      maturationTimeMs: 200\n"
            "      resetTimeMs: 500\n"
            "      resetCondition: Temperature is below limit for resetTimeMs.\n"
            "      fallbackBehavior: Switch to safe mode.\n"
            "      dependencyRefs:\n"
            "        - Vehicle.Powertrain.Temperature\n"
            "      raisesSafetyReactionRefs:\n"
            "        - SafeMode.Activate\n";

        Require(parser.ParseAfsFile(yaml, spec, error),
                "error-interfaces document parse failed: " + error);
        Require(spec.errors.size() == 1, "expected exactly one error interface");

        const afp::Error& e = spec.errors.front();
        Require(e.name             == "TestFunc.OverheatError.ErrorStatus", "error name wrong");
        Require(e.dataType         == "boolean",                            "error dataType wrong");
        Require(e.severity         == "critical",                           "error severity wrong");
        Require(e.maturationTimeMs == 200,                                  "error maturationTimeMs wrong");
        Require(e.resetTimeMs      == 500,                                  "error resetTimeMs wrong");
        Require(e.fallbackBehavior == "Switch to safe mode.",               "error fallbackBehavior wrong");
        Require(e.dependencyRefs.size()            == 1,
                "error dependencyRefs should have 1 entry");
        Require(e.dependencyRefs.front()           == "Vehicle.Powertrain.Temperature",
                "error dependencyRef value wrong");
        Require(e.raisesSafetyReactionRefs.size()  == 1,
                "error raisesSafetyReactionRefs should have 1 entry");
        Require(e.raisesSafetyReactionRefs.front() == "SafeMode.Activate",
                "error raisesSafetyReactionRef value wrong");
    }

    // Alive supervision fields are extracted into SupervisionAlive.
    void TestAliveSupervision()
    {
        afp::AfsFileParser parser;
        afp::FunctionSpecification spec{};
        std::string error;

        const std::string yaml =
            "functionSpecification:\n"
            "  name: TestFunc\n"
            "  version: 1.0.0\n"
            "  scheduling:\n"
            "    - functionName: TestFunc.Step\n"
            "      runType: cyclic\n"
            "      cycleTimeMs: 10\n"
            "      implementedASIL: B\n"
            "      supervision:\n"
            "        required: true\n"
            "        alive:\n"
            "          minIndications: 5\n"
            "          maxIndications: 15\n"
            "          referenceCycleMs: 100\n";

        Require(parser.ParseAfsFile(yaml, spec, error),
                "alive supervision document parse failed: " + error);
        Require(spec.scheduling.size() == 1, "expected one schedule entry");

        const afp::Schedule& schedule = spec.scheduling.front();
        Require(schedule.functionName == "TestFunc.Step", "schedule functionName wrong");
        Require(schedule.supervisionAlive.minIndications   == 5,
                "alive minIndications should be 5");
        Require(schedule.supervisionAlive.maxIndications   == 15,
                "alive maxIndications should be 15");
        Require(schedule.supervisionAlive.referenceCycleMs == 100,
                "alive referenceCycleMs should be 100");
    }

    // Multi-dimensional parameter arrays are stored in the dimensions vector.
    void TestParameterDimensions()
    {
        afp::AfsFileParser parser;
        afp::FunctionSpecification spec{};
        std::string error;

        const std::string yaml =
            "functionSpecification:\n"
            "  name: TestFunc\n"
            "  version: 1.0.0\n"
            "  parameters:\n"
            "    - name: TestFunc.FilterMatrix\n"
            "      dataType: float\n"
            "      defaultValue: 0.0\n"
            "      description: Filter coefficient matrix.\n"
            "      unit: ratio\n"
            "      tunable: false\n"
            "      min: 0.0\n"
            "      max: 1.0\n"
            "      dimensions:\n"
            "        - 3\n"
            "        - 3\n";

        Require(parser.ParseAfsFile(yaml, spec, error),
                "parameter dimensions document parse failed: " + error);
        Require(spec.parameters.size() == 1, "expected one parameter");

        const afp::Parameter& p = spec.parameters.front();
        Require(p.name             == "TestFunc.FilterMatrix", "parameter name wrong");
        Require(p.dimensions.size() == 2,                      "expected 2 dimensions");
        Require(p.dimensions[0]     == 3,                      "first dimension should be 3");
        Require(p.dimensions[1]     == 3,                      "second dimension should be 3");
    }

    // A minimal document with only a name is valid; all section vectors stay empty.
    void TestOptionalSectionsAreOptional()
    {
        afp::AfsFileParser parser;
        afp::FunctionSpecification spec{};
        std::string error;

        const std::string yaml =
            "functionSpecification:\n"
            "  name: MinimalFunc\n"
            "  version: 1.0.0\n";

        Require(parser.ParseAfsFile(yaml, spec, error),
                "minimal document parse failed: " + error);
        Require(spec.signals.empty(),    "signals should be empty for minimal document");
        Require(spec.parameters.empty(), "parameters should be empty for minimal document");
        Require(spec.scheduling.empty(), "scheduling should be empty for minimal document");
        Require(spec.errors.empty(),     "errors should be empty for minimal document");
    }

    // -------------------------------------------------------------------------
    // Error-handling tests
    // -------------------------------------------------------------------------

    // An unrecognised boolean literal must cause ParseAfsFile to return false
    // and fill the error string.
    void TestInvalidBooleanValue()
    {
        afp::AfsFileParser parser;
        afp::FunctionSpecification spec{};
        std::string error;

        const std::string yaml =
            "functionSpecification:\n"
            "  name: TestFunc\n"
            "  dataInterfaces:\n"
            "    - name: Signal.A\n"
            "      direction: input\n"
            "      dataType: float\n"
            "      FuSa: notABool\n"
            "      protectionRequired: false\n"
            "      min: 0.0\n"
            "      max: 1.0\n";

        Require(!parser.ParseAfsFile(yaml, spec, error),
                "document with invalid FuSa boolean was accepted");
        Require(!error.empty(), "invalid boolean did not produce an error message");
    }

    // A non-numeric value where a float is expected must be rejected.
    void TestInvalidNumberValue()
    {
        afp::AfsFileParser parser;
        afp::FunctionSpecification spec{};
        std::string error;

        const std::string yaml =
            "functionSpecification:\n"
            "  name: TestFunc\n"
            "  dataInterfaces:\n"
            "    - name: Signal.A\n"
            "      direction: input\n"
            "      dataType: float\n"
            "      FuSa: false\n"
            "      protectionRequired: false\n"
            "      min: notANumber\n"
            "      max: 1.0\n";

        Require(!parser.ParseAfsFile(yaml, spec, error),
                "document with invalid min number was accepted");
        Require(!error.empty(), "invalid number did not produce an error message");
    }

    // Each list section (dataInterfaces, parameters, scheduling, errors) must be
    // a YAML sequence; a scalar value in its place is an error.
    void TestSectionsRequireSequence()
    {
        afp::AfsFileParser parser;

        struct Case { std::string key; std::string description; };
        const Case cases[] = {
            { "dataInterfaces", "dataInterfaces" },
            { "parameters",     "parameters"     },
            { "scheduling",     "scheduling"     },
            { "errors",         "errors"         },
        };

        for (const Case& c : cases)
        {
            afp::FunctionSpecification spec{};
            std::string error;
            const std::string yaml =
                "functionSpecification:\n"
                "  name: TestFunc\n"
                "  " + c.key + ": not_a_sequence\n";

            Require(!parser.ParseAfsFile(yaml, spec, error),
                    c.description + " as scalar was accepted");
            Require(!error.empty(),
                    c.description + " type error did not produce an error message");
        }
    }

} // namespace

int main()
{
    try
    {
        // Original tests
        TestSampleExtraction();
        TestInvalidDocument();

        // Example file coverage
        TestSpeedHazardDetectionExample();
        TestVehicleSpeedFusionNullFields();
        TestMissingMandatoryPropertyFile();

        // Inline-document tests
        TestErrorInterfaces();
        TestAliveSupervision();
        TestParameterDimensions();
        TestOptionalSectionsAreOptional();

        // Error-handling tests
        TestInvalidBooleanValue();
        TestInvalidNumberValue();
        TestSectionsRequireSequence();
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
    std::cout << "All AFS file parser tests passed.\n";
    return 0;
}
