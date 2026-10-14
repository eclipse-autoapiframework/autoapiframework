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
#ifndef AFD_YAML_WRITER_H
#define AFD_YAML_WRITER_H

#include <string>

#include "yaml_node.h"

namespace afd 
{
    /// Serializes a YamlNode tree back into the block style layout used by the
    /// autoapiframework function specification files.
    class YamlWriter 
    {
    public:
        /// Column at which folded scalars are wrapped.
        void SetWrapColumn(int column) { m_wrapColumn = column; }

        std::string WriteText(const YamlNodePtr& root) const;
        /// Writes @p root to @p path. Returns false and fills @p error on failure.
        bool WriteFile(const std::string& path, const YamlNodePtr& root, std::string& error) const;

    private:
        void EmitNode(const YamlNodePtr& node, int indent, std::string& out, bool inSequenceItem = false) const;
        void EmitMap(const YamlNodePtr& node, int indent, std::string& out, bool inSequenceItem) const;
        void EmitSequence(const YamlNodePtr& node, int indent, std::string& out) const;
        void EmitBlockScalar(const YamlNodePtr& node, int indent, std::string& out) const;

        int m_wrapColumn = 72;
    };

} // namespace afd

#endif // AFD_YAML_WRITER_H
