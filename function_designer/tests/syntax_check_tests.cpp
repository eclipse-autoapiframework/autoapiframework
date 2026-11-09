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

#include "meta_model.h"
#include "validate_function.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

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
        std::ifstream file(path);
        Require(static_cast<bool>(file), "could not open file: " + path);
        std::ostringstream content;
        content << file.rdbuf();
        return content.str();
    }

    // Run ValidateFunction::SyntaxCheckIsOK on the given YAML text.
    bool Validate(const afd::MetaModel& metaModel, const std::string& yaml, std::string& error)
    {
        afd::ValidateFunction validator;
        return validator.SyntaxCheckIsOK(metaModel, yaml, error);
    }

    // Run ValidateFunction::SyntaxCheckIsOK on a file on disk.
    bool ValidateFile(const afd::MetaModel& metaModel, const std::string& path, std::string& error)
    {
        return Validate(metaModel, ReadFile(path), error);
    }

    // Load the meta model once; throws on failure.
    afd::MetaModel LoadMetaModel()
    {
        afd::MetaModel model;
        std::string error;
        const std::string path =
            std::string(AFD_EXAMPLES_DIR) + "/autoapiframework_metadata_V04.yaml";
        Require(model.Load(path, error), "meta model load failed: " + error);
        return model;
    }

    // -------------------------------------------------------------------------
    // Example file tests
    // -------------------------------------------------------------------------

    // All three well-formed example files must pass validation without error.
    void TestAllExamplesPassValidation(const afd::MetaModel& mm)
    {
        const std::vector<std::string> examples = {
            "speed_hazard_detection.afs",
            "vehicle_speed_fusion_multirate.afs",
            "wheel_speed_plausibility_check.afs"
        };

        for (const std::string& example : examples)
        {
            const std::string path = std::string(AFD_EXAMPLES_DIR) + "/" + example;
            std::string error;
            Require(ValidateFile(mm, path, error),
                    example + " should pass validation, but failed: " + error);
        }
    }

    // mandatory_property_is_missing_syntax_error.afs has a null description for
    // one data interface.  The validator must reject it because 'description' is
    // mandatory and its value is empty.
    void TestMandatoryPropertyMissingFails(const afd::MetaModel& mm)
    {
        const std::string path =
            std::string(AFD_EXAMPLES_DIR) + "/mandatory_property_is_missing_syntax_error.afs";
        std::string error;
        Require(!ValidateFile(mm, path, error),
                "mandatory_property_is_missing_syntax_error.afs was accepted but should fail");
        Require(error.find("mandatory") != std::string::npos,
                "error message should mention 'mandatory': " + error);
    }

    // -------------------------------------------------------------------------
    // Header validation tests
    // -------------------------------------------------------------------------

    // A document whose root key is not 'functionSpecification' must be rejected.
    void TestNoRootKeyFails(const afd::MetaModel& mm)
    {
        const std::string yaml =
            "notFunctionSpecification:\n"
            "  name: TestFunc\n"
            "  version: 1.0.0\n";
        std::string error;
        Require(!Validate(mm, yaml, error),
                "document without functionSpecification root was accepted");
        Require(!error.empty(),
                "missing root key produced no error message");
    }

    // 'name' is mandatory in the function header.
    void TestMissingHeaderNameFails(const afd::MetaModel& mm)
    {
        const std::string yaml =
            "functionSpecification:\n"
            "  version: 1.0.0\n"
            "  dataInterfaces: []\n"
            "  parameters: []\n"
            "  scheduling: []\n";
        std::string error;
        Require(!Validate(mm, yaml, error),
                "document without 'name' was accepted");
        Require(!error.empty(),
                "missing name produced no error message");
    }

    // 'version' is mandatory in the function header.
    void TestMissingHeaderVersionFails(const afd::MetaModel& mm)
    {
        const std::string yaml =
            "functionSpecification:\n"
            "  name: TestFunc\n"
            "  dataInterfaces: []\n"
            "  parameters: []\n"
            "  scheduling: []\n";
        std::string error;
        Require(!Validate(mm, yaml, error),
                "document without 'version' was accepted");
        Require(!error.empty(),
                "missing version produced no error message");
    }

    // -------------------------------------------------------------------------
    // Required section tests
    // -------------------------------------------------------------------------

    // All three of 'dataInterfaces', 'parameters', and 'scheduling' must be present.
    // Each absence is tested independently.
    void TestMissingRequiredSectionFails(const afd::MetaModel& mm)
    {
        struct Case { std::string missing; std::string yaml; };
        const Case cases[] = {
            {
                "dataInterfaces",
                "functionSpecification:\n"
                "  name: TestFunc\n"
                "  version: 1.0.0\n"
                "  parameters: []\n"
                "  scheduling: []\n"
            },
            {
                "parameters",
                "functionSpecification:\n"
                "  name: TestFunc\n"
                "  version: 1.0.0\n"
                "  dataInterfaces: []\n"
                "  scheduling: []\n"
            },
            {
                "scheduling",
                "functionSpecification:\n"
                "  name: TestFunc\n"
                "  version: 1.0.0\n"
                "  dataInterfaces: []\n"
                "  parameters: []\n"
            },
        };

        for (const Case& c : cases)
        {
            std::string error;
            Require(!Validate(mm, c.yaml, error),
                    "document missing '" + c.missing + "' was accepted");
            Require(!error.empty(),
                    "missing '" + c.missing + "' produced no error message");
        }
    }

    // -------------------------------------------------------------------------
    // Property name validation tests
    // -------------------------------------------------------------------------

    // A property key that is not defined in the meta model must be rejected.
    void TestUnknownPropertyFails(const afd::MetaModel& mm)
    {
        const std::string yaml =
            "functionSpecification:\n"
            "  name: TestFunc\n"
            "  version: 1.0.0\n"
            "  dataInterfaces:\n"
            "    - name: Vehicle.Speed\n"
            "      unknownCustomProperty: hello\n"
            "  parameters: []\n"
            "  scheduling: []\n";
        std::string error;
        Require(!Validate(mm, yaml, error),
                "data interface with unknown property was accepted");
        Require(error.find("Invalid") != std::string::npos ||
                error.find("invalid") != std::string::npos,
                "error message should mention invalid property: " + error);
    }

    // -------------------------------------------------------------------------
    // Enum validation tests
    // -------------------------------------------------------------------------

    // The 'direction' property accepts only 'input' and 'output'.
    void TestInvalidDirectionEnumFails(const afd::MetaModel& mm)
    {
        const std::string yaml =
            "functionSpecification:\n"
            "  name: TestFunc\n"
            "  version: 1.0.0\n"
            "  dataInterfaces:\n"
            "    - name: Vehicle.Speed\n"
            "      direction: sideways\n"
            "  parameters: []\n"
            "  scheduling: []\n";
        std::string error;
        Require(!Validate(mm, yaml, error),
                "data interface with invalid direction enum was accepted");
        Require(!error.empty(),
                "invalid direction produced no error message");
    }

    // The 'ASIL' property accepts only the values defined in the ASIL enum.
    void TestInvalidASILEnumFails(const afd::MetaModel& mm)
    {
        const std::string yaml =
            "functionSpecification:\n"
            "  name: TestFunc\n"
            "  version: 1.0.0\n"
            "  dataInterfaces:\n"
            "    - name: Vehicle.Speed\n"
            "      ASIL: X\n"
            "  parameters: []\n"
            "  scheduling: []\n";
        std::string error;
        Require(!Validate(mm, yaml, error),
                "data interface with invalid ASIL enum was accepted");
        Require(!error.empty(),
                "invalid ASIL produced no error message");
    }

    // The 'runType' scheduling property accepts only values defined in RunType.
    void TestInvalidRunTypeEnumFails(const afd::MetaModel& mm)
    {
        const std::string yaml =
            "functionSpecification:\n"
            "  name: TestFunc\n"
            "  version: 1.0.0\n"
            "  dataInterfaces: []\n"
            "  parameters: []\n"
            "  scheduling:\n"
            "    - functionName: TestFunc.Step\n"
            "      runType: always\n"
            "      description: Step function.\n"
            "      implementedASIL: B\n";
        std::string error;
        Require(!Validate(mm, yaml, error),
                "scheduling entry with invalid runType was accepted");
        Require(!error.empty(),
                "invalid runType produced no error message");
    }

    // When FuSa is false the ASIL field must be left empty.
    void TestFuSaFalseWithASILFails(const afd::MetaModel& mm)
    {
        const std::string yaml =
            "functionSpecification:\n"
            "  name: TestFunc\n"
            "  version: 1.0.0\n"
            "  dataInterfaces:\n"
            "    - name: Vehicle.Speed\n"
            "      FuSa: false\n"
            "      ASIL: B\n"
            "  parameters: []\n"
            "  scheduling: []\n";
        std::string error;
        Require(!Validate(mm, yaml, error),
                "FuSa=false with non-empty ASIL was accepted");
        Require(error.find("false") != std::string::npos ||
                error.find("ASIL") != std::string::npos,
                "error message should mention FuSa/ASIL conflict: " + error);
    }

    // -------------------------------------------------------------------------
    // Boolean validation tests
    // -------------------------------------------------------------------------

    // 'FuSa' accepts only 'true' or 'false'.
    void TestInvalidBooleanFuSaFails(const afd::MetaModel& mm)
    {
        const std::string yaml =
            "functionSpecification:\n"
            "  name: TestFunc\n"
            "  version: 1.0.0\n"
            "  dataInterfaces:\n"
            "    - name: Vehicle.Speed\n"
            "      FuSa: maybe\n"
            "  parameters: []\n"
            "  scheduling: []\n";
        std::string error;
        Require(!Validate(mm, yaml, error),
                "FuSa with non-boolean value was accepted");
        Require(error.find("FuSa") != std::string::npos ||
                error.find("true") != std::string::npos,
                "error message should mention FuSa boolean constraint: " + error);
    }

    // 'tunable' in a parameter accepts only 'true' or 'false'.
    void TestInvalidBooleanTunableFails(const afd::MetaModel& mm)
    {
        const std::string yaml =
            "functionSpecification:\n"
            "  name: TestFunc\n"
            "  version: 1.0.0\n"
            "  dataInterfaces: []\n"
            "  parameters:\n"
            "    - name: Vehicle.Param\n"
            "      tunable: perhaps\n"
            "  scheduling: []\n";
        std::string error;
        Require(!Validate(mm, yaml, error),
                "parameter with non-boolean tunable was accepted");
        Require(!error.empty(),
                "invalid tunable produced no error message");
    }

    // -------------------------------------------------------------------------
    // Type validation tests
    // -------------------------------------------------------------------------

    // A float data interface where min > max must be rejected.
    void TestFloatMinGreaterThanMaxFails(const afd::MetaModel& mm)
    {
        const std::string yaml =
            "functionSpecification:\n"
            "  name: TestFunc\n"
            "  version: 1.0.0\n"
            "  dataInterfaces:\n"
            "    - name: Vehicle.Speed\n"
            "      dataType: float\n"
            "      description: Vehicle speed.\n"
            "      unit: km/h\n"
            "      min: 300.0\n"
            "      max: 0.0\n"
            "      defaultValue: 0.0\n"
            "      resolution: 0.1\n"
            "      precision: 1\n"
            "      FuSa: true\n"
            "      ASIL: B\n"
            "      minUpdatePeriodMs: 10\n"
            "      accuracy: \"+/-0.2 km/h\"\n"
            "      direction: input\n"
            "      protectionRequired: true\n"
            "  parameters: []\n"
            "  scheduling: []\n";
        std::string error;
        Require(!Validate(mm, yaml, error),
                "float data interface with min > max was accepted");
        Require(error.find("max") != std::string::npos || error.find("min") != std::string::npos,
                "error message should mention min/max: " + error);
    }

    // An integer parameter where min > max must be rejected.
    void TestIntegerMinGreaterThanMaxFails(const afd::MetaModel& mm)
    {
        const std::string yaml =
            "functionSpecification:\n"
            "  name: TestFunc\n"
            "  version: 1.0.0\n"
            "  dataInterfaces: []\n"
            "  parameters:\n"
            "    - name: Vehicle.Param\n"
            "      dataType: uint32\n"
            "      defaultValue: 100\n"
            "      description: Test parameter.\n"
            "      unit: ms\n"
            "      tunable: false\n"
            "      min: 100\n"
            "      max: 10\n"
            "  scheduling: []\n";
        std::string error;
        Require(!Validate(mm, yaml, error),
                "integer parameter with min > max was accepted");
        Require(error.find("max") != std::string::npos || error.find("min") != std::string::npos,
                "error message should mention min/max: " + error);
    }

    // A non-integer cycleTimeMs in a scheduling entry must be rejected.
    void TestNonIntegerCycleTimeFails(const afd::MetaModel& mm)
    {
        const std::string yaml =
            "functionSpecification:\n"
            "  name: TestFunc\n"
            "  version: 1.0.0\n"
            "  dataInterfaces: []\n"
            "  parameters: []\n"
            "  scheduling:\n"
            "    - functionName: TestFunc.Step\n"
            "      runType: cyclic\n"
            "      description: Cyclic step function.\n"
            "      implementedASIL: B\n"
            "      cycleTimeMs: 20.5ms\n";
        std::string error;
        Require(!Validate(mm, yaml, error),
                "scheduling with non-integer cycleTimeMs was accepted");
        Require(error.find("cycleTimeMs") != std::string::npos ||
                error.find("integer") != std::string::npos,
                "error message should mention cycleTimeMs: " + error);
    }

    // A non-integer maturationTimeMs in an error interface must be rejected.
    void TestNonIntegerMaturationTimeFails(const afd::MetaModel& mm)
    {
        const std::string yaml =
            "functionSpecification:\n"
            "  name: TestFunc\n"
            "  version: 1.0.0\n"
            "  dataInterfaces: []\n"
            "  parameters: []\n"
            "  scheduling: []\n"
            "  errorInterfaces:\n"
            "    - name: TestFunc.SomeError.ErrorStatus\n"
            "      dataType: boolean\n"
            "      description: An error.\n"
            "      maturationTimeMs: not_a_number\n";
        std::string error;
        Require(!Validate(mm, yaml, error),
                "error interface with non-integer maturationTimeMs was accepted");
        Require(error.find("maturationTimeMs") != std::string::npos ||
                error.find("integer") != std::string::npos,
                "error message should mention maturationTimeMs: " + error);
    }

    // -------------------------------------------------------------------------
    // Error interface tests
    // -------------------------------------------------------------------------

    // A well-formed errorInterfaces section must pass validation.
    void TestValidErrorInterfacesPasses(const afd::MetaModel& mm)
    {
        const std::string yaml =
            "functionSpecification:\n"
            "  name: TestFunc\n"
            "  version: 1.0.0\n"
            "  dataInterfaces: []\n"
            "  parameters: []\n"
            "  scheduling: []\n"
            "  errorInterfaces:\n"
            "    - name: TestFunc.OverheatError.ErrorStatus\n"
            "      dataType: boolean\n"
            "      description: Temperature exceeded safe threshold.\n"
            "      severity: critical\n"
            "      maturationTimeMs: 200\n"
            "      resetTimeMs: 500\n"
            "      resetCondition: Temperature is below limit for resetTimeMs.\n"
            "      fallbackBehavior: Switch to safe mode.\n";

        // 'critical' is not in the ErrorSeverity enum (valid: information, warning,
        // degraded, shutdown) — this test verifies the enum check fires for errors too.
        std::string error;
        Require(!Validate(mm, yaml, error),
                "error interface with invalid severity enum was accepted");
        Require(!error.empty(),
                "invalid severity produced no error message");
    }

    // A valid error interface using only the ErrorSeverity enum values must pass.
    void TestValidErrorInterfaceEnumPasses(const afd::MetaModel& mm)
    {
        const std::string yaml =
            "functionSpecification:\n"
            "  name: TestFunc\n"
            "  version: 1.0.0\n"
            "  dataInterfaces: []\n"
            "  parameters: []\n"
            "  scheduling: []\n"
            "  errorInterfaces:\n"
            "    - name: TestFunc.OverheatError.ErrorStatus\n"
            "      dataType: boolean\n"
            "      description: Temperature exceeded safe threshold.\n"
            "      severity: degraded\n"
            "      maturationTimeMs: 200\n"
            "      resetTimeMs: 500\n"
            "      resetCondition: Temperature is below limit.\n"
            "      fallbackBehavior: Switch to safe mode.\n";
        std::string error;
        Require(Validate(mm, yaml, error),
                "valid error interface with 'degraded' severity was rejected: " + error);
    }

} // namespace

int main()
{
    afd::MetaModel metaModel;
    try
    {
        metaModel = LoadMetaModel();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Setup failed: " << e.what() << '\n';
        return 1;
    }

    try
    {
        // Example file tests
        TestAllExamplesPassValidation(metaModel);
        TestMandatoryPropertyMissingFails(metaModel);

        // Header validation
        TestNoRootKeyFails(metaModel);
        TestMissingHeaderNameFails(metaModel);
        TestMissingHeaderVersionFails(metaModel);

        // Required section tests
        TestMissingRequiredSectionFails(metaModel);

        // Property name validation
        TestUnknownPropertyFails(metaModel);

        // Enum validation
        TestInvalidDirectionEnumFails(metaModel);
        TestInvalidASILEnumFails(metaModel);
        TestInvalidRunTypeEnumFails(metaModel);
        TestFuSaFalseWithASILFails(metaModel);

        // Boolean validation
        TestInvalidBooleanFuSaFails(metaModel);
        TestInvalidBooleanTunableFails(metaModel);

        // Type validation
        TestFloatMinGreaterThanMaxFails(metaModel);
        TestIntegerMinGreaterThanMaxFails(metaModel);
        TestNonIntegerCycleTimeFails(metaModel);
        TestNonIntegerMaturationTimeFails(metaModel);

        // Error interface tests
        TestValidErrorInterfacesPasses(metaModel);
        TestValidErrorInterfaceEnumPasses(metaModel);
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }

    std::cout << "All syntax check tests passed.\n";
    return 0;
}
