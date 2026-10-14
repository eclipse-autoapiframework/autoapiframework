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
 *   Thomas Pfleiderer - initial function designer
 ********************************************************************************/
#ifndef AFD_YAML_PARSER_H
#define AFD_YAML_PARSER_H

#include <string>

#include "yaml_node.h"

namespace afd 
{
    /// Parser for the block-style YAML subset used by the autoapiframework meta
    /// model and function specification (.afs) files.
    ///
    /// Supported: nested block mappings, block sequences, plain/quoted scalars,
    /// folded (`>`/`>-`) and literal (`|`/`|-`) block scalars, comments and blank
    /// lines. Flow collections (`{}`/`[]`), anchors and multiple documents are not
    /// supported and are reported as an error.
    class YamlParser 
    {
    public:
        /// Parses @p text. Returns nullptr and fills @p error on failure.
        YamlNodePtr ParseText(const std::string& text, std::string& error);
        /// Reads and parses @p path. Returns nullptr and fills @p error on failure.
        YamlNodePtr ParseFile(const std::string& path, std::string& error);
        /// Reads @p path. Returns file conent.
        std::string GetFileContent(const std::string& path);        
    };

} // namespace afd

#endif // AFD_YAML_PARSER_H
