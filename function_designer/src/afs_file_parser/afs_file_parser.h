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

#ifndef AFP_AFS_FILE_PARSER_H
#define AFP_AFS_FILE_PARSER_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "afs_types.h"
#include "../yaml_node.h"

namespace afp 
{
    class AfsFileParser
    {
    public:
        bool ParseAfsFile(const std::string& content, FunctionSpecification& functionSpec, std::string& error);

    private:

    };
} // namespace afp

#endif // AFP_AFS_FILE_PARSER_H
