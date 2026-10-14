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
 
#include "function_specification.h"

#include "yaml_parser.h"
#include "yaml_writer.h"

namespace afd 
{
    bool FunctionSpecification::Load(const std::string& expectedVersion, const std::string& path, std::string& error) 
    {
        YamlParser parser;
        const YamlNodePtr root = parser.ParseFile(path, error);
        if (!root) 
        {
            return false;
        }
        const YamlNodePtr spec = root->Find(kRootKey);
        if (!spec || !spec->IsMap()) 
        {
            error = "'" + std::string(kRootKey) + "' root key not found in " + path;
            return false;
        }
        const YamlNodePtr ref = spec->Find(kMetaModelRefKey);
        if (!CheckMetaModelVersion(ref, expectedVersion,path, error))
        {
            return false;
        }

        m_root = root;
        m_spec = spec;
        m_sourcePath = path;
        return true;
    }

    void FunctionSpecification::New(const std::string& metaModelName, const std::string& metaModelVersion)
    {
        YamlNodePtr metaModelRef = YamlNode::MakeMap();
        metaModelRef->Set(kNameKey , YamlNode::MakeScalar(metaModelName));
        metaModelRef->Set(kVersionKey, YamlNode::MakeScalar(metaModelVersion));

        YamlNodePtr spec = YamlNode::MakeMap();
        spec->Set(kMetaModelRefKey, metaModelRef);
        spec->Set(kNameKey , YamlNode::MakeScalar("NewExample"));
        spec->Set(kVersionKey, YamlNode::MakeScalar("0.0.0"));
        spec->Set(kDescriptionKey, YamlNode::MakeScalar(""));
        spec->Set(kDataInterfacesKey, YamlNode::MakeSequence());
        spec->Set(kParametersKey, YamlNode::MakeSequence());
        spec->Set(kSchedulingKey, YamlNode::MakeSequence());
        spec->Set(kErrorsKey, YamlNode::MakeSequence());

        YamlNodePtr root = YamlNode::MakeMap();
        root->Set(kRootKey, spec);

        m_root = root;
        m_spec = spec;
        m_sourcePath.clear();
    }

    bool FunctionSpecification::CheckMetaModelVersion(const YamlNodePtr ref, const std::string& expectedVersion, 
        const std::string& path, std::string& error) 
    {
        if (ref && ref->IsMap() )
        {
            auto version = ref->ScalarOf("version");
            if (version.compare(expectedVersion) != 0)
            {
                error = "Meta Model version '" + version + "' not supported, expected '" + expectedVersion + "'";
                return false;
            }
        }
        else
        {
            error = "'metaModelRef' not found or not a map in " + path;
            return false;
        }

        return true;
    }

    bool FunctionSpecification::Save(const std::string& path, std::string& error) const
    {
        if (!m_root) 
        {
            error = "no function specification loaded";
            return false;
        }
        YamlWriter writer;
        return writer.WriteFile(path, m_root, error);
    }

    std::string FunctionSpecification::ToText() const
    {
        return m_root ? YamlWriter().WriteText(m_root) : std::string();
    }

    std::string FunctionSpecification::GetName() const 
    {
        return m_spec ? m_spec->ScalarOf("name") : std::string();
    }

    std::string FunctionSpecification::GetVersion() const 
    {
        return m_spec ? m_spec->ScalarOf("version") : std::string();
    }

    std::string FunctionSpecification::GetDescription() const 
    {
        return m_spec ? m_spec->ScalarOf("description") : std::string();
    }

    void FunctionSpecification::SetName(const std::string& value)
    {
        SetEditableAttribute("name", value);
    }

    void FunctionSpecification::SetVersion(const std::string& value)
    {
        SetEditableAttribute("version", value);
    }

    void FunctionSpecification::SetDescription(const std::string& value)
    {
        SetEditableAttribute("description", value);
    }

    void FunctionSpecification::SetEditableAttribute(const std::string& key, const std::string& value)
    {
        if (!m_spec)
        {
            return;
        }

        const YamlNodePtr attribute = m_spec->Find(key);
        if (attribute && attribute->IsScalar())
        {
            attribute->SetScalar(value);
        }
        else
        {
            m_spec->Set(key, YamlNode::MakeScalar(value));
        }
    }

    std::string FunctionSpecification::GetMetaModelName() const 
    {
        if (!m_spec) 
        {
            return std::string();
        }
        const YamlNodePtr ref = m_spec->Find("metaModelRef");
        return ref && ref->IsMap() ? ref->ScalarOf("name") : std::string();
    }

    std::string FunctionSpecification::GetMetaModelVersion() const 
    {
        if (!m_spec) 
        {
            return std::string();
        }
        const YamlNodePtr ref = m_spec->Find("metaModelRef");
        return ref && ref->IsMap() ? ref->ScalarOf("version") : std::string();
    }

    std::vector<std::pair<std::string, std::string>> FunctionSpecification::GetAttributes() const 
    {
        std::vector<std::pair<std::string, std::string>> attributes;
        if (!m_spec) 
        {
            return attributes;
        }
        for (const auto& entry : m_spec->GetMap()) 
        {
            if (!entry.second) 
            {
                continue;
            }
            if (entry.second->IsScalar()) 
            {
                attributes.emplace_back(entry.first, entry.second->GetScalar());
            } 
            else if (entry.second->IsMap()) 
            {
                for (const auto& nested : entry.second->GetMap()) 
                {
                    if (nested.second && nested.second->IsScalar()) 
                    {
                        attributes.emplace_back(entry.first + "." + nested.first, nested.second->GetScalar());
                    }
                }
            } else if (entry.second->IsSequence()) 
            {
                attributes.emplace_back(entry.first,
                                        std::to_string(entry.second->GetSequence().size()) + " entries");
            }
        }
        return attributes;
    }

    std::vector<YamlNodePtr> FunctionSpecification::GetCollection(const std::string& key) const 
    {
        if (!m_spec) 
        {
            return {};
        }
        const YamlNodePtr node = m_spec->Find(key);
        if (!node || !node->IsSequence()) 
        {
            return {};
        }
        return node->GetSequence();
    }

    bool FunctionSpecification::AddCollectionItem(const std::string& key, YamlNodePtr item)
    {
        if (!m_spec || !item)
        {
            return false;
        }
        const YamlNodePtr collection = m_spec->Find(key);
        if (!collection || !collection->IsSequence())
        {
            return false;
        }
        collection->Append(std::move(item));
        return true;
    }

    bool FunctionSpecification::RemoveCollectionItem(const std::string& key, std::size_t index)
    {
        if (!m_spec)
        {
            return false;
        }
        const YamlNodePtr node = m_spec->Find(key);
        if (!node || !node->IsSequence())
        {
            return false;
        }
        return node->RemoveAt(index);
    }
} // namespace afd
