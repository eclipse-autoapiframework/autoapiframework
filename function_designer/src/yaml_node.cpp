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
 
#include "yaml_node.h"

namespace afd 
{
    YamlNodePtr YamlNode::MakeScalar(std::string value, ScalarStyle style) 
    {
        auto node = std::make_shared<YamlNode>();
        node->m_type = Type::Scalar;
        node->m_scalar = std::move(value);
        node->m_style = style;
        return node;
    }

    YamlNodePtr YamlNode::MakeMap() 
    {
        auto node = std::make_shared<YamlNode>();
        node->m_type = Type::Map;
        return node;
    }

    YamlNodePtr YamlNode::MakeSequence() 
    {
        auto node = std::make_shared<YamlNode>();
        node->m_type = Type::Sequence;
        return node;
    }

    void YamlNode::Set(const std::string& key, YamlNodePtr value) 
    {
        for (auto& entry : m_map) 
        {
            if (entry.first == key) 
            {
                entry.second = std::move(value);
                return;
            }
        }
        m_map.emplace_back(key, std::move(value));
    }

    void YamlNode::Append(YamlNodePtr value) { m_sequence.push_back(std::move(value)); }

    bool YamlNode::RemoveAt(std::vector<YamlNodePtr>::size_type index)
    {
        if (!IsSequence() || index >= m_sequence.size())
        {
            return false;
        }
        m_sequence.erase(m_sequence.begin() + static_cast<std::vector<YamlNodePtr>::difference_type>(index));
        return true;
    }

    YamlNodePtr YamlNode::Find(const std::string& key) const 
    {
        for (const auto& entry : m_map) 
        {
            if (entry.first == key) 
            {
                return entry.second;
            }
        }
        return nullptr;
    }

    std::string YamlNode::ScalarOf(const std::string& key) const 
    {
        const YamlNodePtr child = Find(key);
        if (child && child->IsScalar()) 
        {
            return child->GetScalar();
        }
        return std::string();
    }

} // namespace afd
