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
 
#include "meta_model.h"

#include <algorithm>
#include <sstream>

#include "yaml_parser.h"

namespace afd 
{
    bool MetaModel::Load(const std::string& path, std::string& error) 
    {
        Reset();
        YamlParser parser;
        const YamlNodePtr root = parser.ParseFile(path, error);
        if (!root) 
        {
            Reset();            
            return false;
        }
        const YamlNodePtr model = root->Find(kMetaModelKey);
        if (!model || !model->IsMap()) 
        {
            Reset();            
            error = std::string("'") + std::string(kMetaModelKey) + "' root key not found in " + path;
            return false;
        }
        
        if (!FindRequiredAttributesAndInterfaces(model, path, error))
        {
            Reset();
            return false;
        }

        if (!IsGreaterOrEqual(m_version, m_minimumVersion))
        {
            Reset();
            error = "Metamodel version " + m_version + " is lower than the minimum required version " + m_minimumVersion;        
            return false;
        }

        m_sourcePath = path;

        m_fileContent = parser.GetFileContent(path); // Initialize the file content to an empty string
        m_loaded = true;
        return true;
    }

    bool MetaModel::FindRequiredAttributesAndInterfaces(const YamlNodePtr model, 
                    const std::string& path,std::string& error)
    {
        m_name = model->ScalarOf(kNameKey);
        m_version = model->ScalarOf(kVersionKey);
        if (m_name.compare(kExpectedName) != 0 ||
            m_version.compare("") == 0)
        {
            error = "Incompatible metamodel file " + path;
            return false;
        }

        bool interfacesNotFound = true;        
        const YamlNodePtr interfaceTypes = model->Find(kInterfaceTypesKey);
        if (interfaceTypes && interfaceTypes->IsMap()) 
        {
            const YamlNodePtr dataInterfaces = interfaceTypes->Find(kDataInterfaceTypeKey);
            const YamlNodePtr parameters = interfaceTypes->Find(kParameterInterfaceTypeKey);
            const YamlNodePtr scheduling = interfaceTypes->Find(kSchedulingInterfaceTypeKey);
            const YamlNodePtr errors = interfaceTypes->Find(kErrorInterfaceTypeKey);
            if (dataInterfaces && parameters && scheduling && errors &&
                dataInterfaces->IsMap() && parameters->IsMap() && scheduling->IsMap() &&
                errors->IsMap())
            {
                const YamlNodePtr dataProperties = dataInterfaces->Find(kPropertiesKey);
                const YamlNodePtr parametersProperties = parameters->Find(kPropertiesKey);
                const YamlNodePtr schedulingParameters = scheduling->Find(kPropertiesKey);  
                const YamlNodePtr errorProperties = errors->Find(kPropertiesKey);
                if (dataProperties && parametersProperties && schedulingParameters && errorProperties &&
                    dataProperties->IsMap() && parametersProperties->IsMap() &&
                    schedulingParameters->IsMap() && errorProperties->IsMap())
                {
                    interfacesNotFound = false;                
                }
            }
            m_interfaceTypes.push_back(std::move(dataInterfaces));
            m_interfaceTypes.push_back(std::move(parameters));
            m_interfaceTypes.push_back(std::move(scheduling));
            m_interfaceTypes.push_back(std::move(errors));
        }

        if (interfacesNotFound)
        {
            error = "The required interface types are missing in the metamodel file " + path;
            return false;
        }
        CollectEnums(model);    

        return true;
    }    

    const YamlNodePtr* MetaModel::FindInterfaceType(const std::string& name) const 
    {
        static constexpr const char* interfaceTypeNames[] = {
            kDataInterfaceTypeKey,
            kParameterInterfaceTypeKey,
            kSchedulingInterfaceTypeKey,
            kErrorInterfaceTypeKey
        };

        for (std::size_t index = 0; index < m_interfaceTypes.size() &&
             index < std::size(interfaceTypeNames); ++index)
        {
            if (name == interfaceTypeNames[index])
            {
                return &m_interfaceTypes[index];
            }
        }

        return nullptr;
    }

    std::vector<std::string> MetaModel::ColumnsFor(const std::string& interfaceTypeName,
                                                const std::vector<YamlNodePtr>& entries) const 
    {
        std::vector<std::string> columns;
        if (const YamlNodePtr* type = FindInterfaceType(interfaceTypeName)) 
        {
            const YamlNodePtr properties = (*type)->Find(kPropertiesKey);
            if (properties && properties->IsMap())
            {
                for (const auto& property : properties->GetMap())
                {
                    columns.push_back(property.first);
                }
            }
            if (interfaceTypeName == kDataInterfaceTypeKey)
            {
                const YamlNodePtr companion = (*type)->Find(kRuntimeCompanionKey);
                if (companion && companion->IsMap())
                {
                    for (const auto& property : companion->GetMap())
                    {
                        columns.push_back(property.first);
                    }
                }
            }
        }
        for (const YamlNodePtr& entry : entries) 
        {
            if (!entry || !entry->IsMap())
            {
                continue;
            }
            for (const auto& field : entry->GetMap()) 
            {
                if (interfaceTypeName == kDataInterfaceTypeKey && field.first == kRuntimeCompanionKey)
                {
                    continue;
                }
                if (std::find(columns.begin(), columns.end(), field.first) == columns.end()) 
                {
                    columns.push_back(field.first);
                }
            }
        }

        return columns;
    }

   void MetaModel::CollectEnums(const YamlNodePtr& model) 
    {
        if (const YamlNodePtr enums = model->Find("enums")) 
        {
            m_enums.clear(); 
            for (const auto& entry : enums->GetMap()) 
            {
                std::vector<std::string> values;
                if (entry.second && entry.second->IsSequence()) 
                {
                    for (const YamlNodePtr& value : entry.second->GetSequence()) 
                    {
                        if (value && value->IsScalar()) 
                        {
                            values.push_back(value->GetScalar());
                        }
                    }
                    m_enums.emplace(entry.first, std::move(values));                    
                }
                else
                {
                    const YamlNodePtr declaredValues = entry.second
                        ? entry.second->Find("values")
                        : nullptr;
                    if (declaredValues && declaredValues->IsSequence())
                    {
                        for (const YamlNodePtr& value : declaredValues->GetSequence())
                        {
                            if (value && value->IsScalar())
                            {
                                values.push_back(value->GetScalar());
                            }
                        }
                    }

                    m_enums.emplace(entry.first, std::move(values));
                }
            }
        }
    }

    bool MetaModel::ValidateEnumValue( const std::string& propertyName, const std::string& enumName, 
                    const std::string& entry, std::string& error) const
    {
        auto it = m_enums.find(enumName);
        if (it == m_enums.end())
        {
            error = "(" + propertyName + ") Enum '" + enumName + "' not found.";
            error += "\n\nValid would be:\n";
            for (const auto& value : m_enums)
            {
                error += "  - " + value.first + "\n";
            }            
            return false;
        }
        const auto& values = it->second;
        if (std::find(values.begin(), values.end(), entry) == values.end())
        {
            error = "Invalid enum value '" + entry +"' for property '" + enumName + "' in item '" + propertyName + "'.";

            error += "\n\nValid would be:\n";
            for (const auto& value : values)
            {
                error += "  - " + value + "\n";
            }
            return false;
        }

        return true;
    }    

    std::vector<uint32_t> MetaModel::SplitVersion(const std::string& version) const
    {
        std::vector<uint32_t> parts;
        std::stringstream ss(version);
        std::string item;

        while (std::getline(ss, item, '.'))
        {
            parts.push_back(std::stoi(item));
        }

        return parts;
    }

    bool MetaModel::IsGreaterOrEqual(const std::string& version1, const std::string& version2) const
    {
        auto v1 = SplitVersion(version1);
        auto v2 = SplitVersion(version2);

        const size_t maxSize = std::max(v1.size(), v2.size());

        v1.resize(maxSize, 0);
        v2.resize(maxSize, 0);

        for (size_t i = 0; i < maxSize; ++i)
        {
            if (v1[i] > v2[i])
            {
                return true;
            }
            if (v1[i] < v2[i])
            {
                return false;
            }
        }

        return true; // equal
    }    

    void MetaModel::Reset()
    {
        m_loaded = false;
        m_minimumVersion = "";
        m_fileContent = "";
        m_name = "";
        m_version = "";
        m_sourcePath = "";
        m_interfaceTypes.clear();
        m_enums.clear();
    }
    
} // namespace afd
