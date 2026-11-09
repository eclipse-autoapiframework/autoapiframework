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
#include "afs_file_parser.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char* argv[])
{
    if (argc == 2 || argc == 3)
    {
        const std::string afsFile = argv[1];
        std::ifstream input(afsFile);
        if (!input)
        {
            std::cerr << "Cannot open file: " << afsFile << '\n';
            return 2;
        }
        std::ostringstream content;
        content << input.rdbuf();

        afp::FunctionSpecification functionSpec;
        std::string error;
        afp::AfsFileParser parser;

        const bool result = parser.ParseAfsFile(content.str(), functionSpec, error);

        if(result)
        {
            std::cout << "Parsing successful: " << functionSpec.enums.size() << " enums, "
                      << functionSpec.signals.size() << " signals, "
                      << functionSpec.parameters.size() << " parameters, "
                      << functionSpec.scheduling.size() << " schedules, "
                      << functionSpec.errors.size() << " errors.\n";

            if (argc == 3)
            {
                afp::AfsFileExpectation expectation;
                if (!expectation.CheckExpectation(argv[2], functionSpec, error))
                {
                    std::cerr << error << "\n";
                    return 3;
                }
                std::cout << "All expectations met.\n";
            }
        }
        else
        {
            std::cerr << "Parsing failed: " << error << "\n";
            return 1;
        }

        return 0;
    }

    std::cout << "Usage: AfsFileParser [file.afs] [expectations.txt]\n";
    return 1;
}
