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
 *   Thomas Pfleiderer - Command-line application (CLI) added
 ********************************************************************************/

#include "meta_model.h"
#include "validate_function.h"
#include "yaml_node.h"
#include "yaml_parser.h"
#include "yaml_writer.h"

 #include <filesystem>
 #include <string>
 #include <iostream>
 #include <fstream>
 #include <sstream>

namespace
{
    std::filesystem::path ResolveSpecificationPath(const std::string& inputPath, const char* executablePath)
    {
        std::error_code error;
        const std::filesystem::path requestedPath(inputPath);
        if (std::filesystem::exists(requestedPath, error) && !error)
        {
            return requestedPath;
        }

        error.clear();
        const std::filesystem::path executable = std::filesystem::absolute(executablePath, error);
        if (!error)
        {
            const std::filesystem::path besideExecutable = executable.parent_path() / requestedPath;
            if (std::filesystem::exists(besideExecutable, error) && !error)
            {
                return besideExecutable;
            }
        }

        error.clear();
        const std::filesystem::path examplePath = std::filesystem::path(AFD_EXAMPLES_DIR) / requestedPath;
        if (std::filesystem::exists(examplePath, error) && !error)
        {
            return examplePath;
        }
       
        return requestedPath;
    }

    int CheckSpecification(const std::string& inputPath, const char* executablePath)
    {
        const std::filesystem::path specificationPath = ResolveSpecificationPath(inputPath, executablePath);
        std::ifstream input(specificationPath, std::ios::binary);
        if (!input)
        {
            std::cout << "Could not read function specification: " << inputPath << '\n';
            return 2;
        }
        std::ostringstream content;
        content << input.rdbuf();

        afd::MetaModel metaModel;
        std::string error;
        const std::string metaModelPath = std::string(AFD_EXAMPLES_DIR) + "/autoapiframework_metadata_V04.yaml";
        if (!metaModel.Load(metaModelPath, error))
        {
            std::cout << "Could not load meta model: " << error << '\n';
            return 2;
        }      

        afd::ValidateFunction validator;
        if (!validator.SyntaxCheckIsOK(metaModel, content.str(), error))
        {
            std::cout << "Syntax error in " << inputPath << ":\n" << error << '\n';
            return 1;
        }

        std::cout << "Syntax OK: '" << inputPath << "' seems to be a valid afs file. \n";
        return 0;
    }
}

int main(int argc, char* argv[])
{
    if (argc == 2)
    {
        return CheckSpecification(argv[1], argv[0]);
    }

    std::cout << "Usage: AutoAPIFunctionDesigner_SyntaxCheck [file.afs]\n";
    return 3;
}