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
 
#ifndef AFD_META_MODEL_H
#define AFD_META_MODEL_H

#include <map>
#include <string>
#include <stdint.h>
#include <vector>

#include "yaml_node.h"

namespace afd 
{
    /// @brief Expected strings within the the autoapiframework_meta_model.yaml file.
    static constexpr const char* kExpectedName = "Eclipse-autoapiframework-Metamodel";
    static constexpr const char* kMetaModelKey = "metamodel";
    static constexpr const char* kNameKey = "name";
    static constexpr const char* kFunctionNameKey = "functionName";
    static constexpr const char* kDescriptionKey = "description";    
    static constexpr const char* kVersionKey = "version";    
    static constexpr const char* kEnumKey = "enum";
    static constexpr const char* kEnumsKey = "enums";    
    static constexpr const char* kEnumRefKey = "enumRef";
    static constexpr const char* kInterfaceTypesKey = "interfaceTypes";

    static constexpr const char* kDataInterfaceTypeKey = "Data";
    static constexpr const char* kParameterInterfaceTypeKey = "Parameter";
    static constexpr const char* kSchedulingInterfaceTypeKey = "Scheduling";
    static constexpr const char* kErrorInterfaceTypeKey = "Error";
    static constexpr const char* kPropertiesKey = "properties";

    static constexpr const char* kRuntimeCompanionKey = "runtimeCompanion";
    static constexpr const char* kQualityCodeKey = "qualityCode";
    static constexpr const char* kQualityNameKey = "name";
    static constexpr const char* kDatatypeKey = "datatype";
    static constexpr const char* kMandatoryKey = "mandatory";
    static constexpr const char* kBooleanType = "boolean";    
    static constexpr const char* kDataQualityKey = "DataQuality";
    static constexpr const char* kExecutionResultTypeKey = "type";
    static constexpr const char* kExecutionResultKey = "executionResult";

    static constexpr const char* kSupervisionRequiredPath = "supervision.required";
    static constexpr const char* kExecutionResultTypeFieldPath = "executionResult.type";
    static constexpr const char* kSupervisionKey = "supervision";
    static constexpr const char* kSupervisionRequiredKey = "required";
    static constexpr const char* kFuSaKey = "FuSa";
    static constexpr const char* kASILKey = "ASIL";    

    
    /// In-memory representation of autoapiframework_meta_model.yaml.
    class MetaModel 
    {
    public:
        /// Loads the meta model from @p path. Returns false and fills @p error on failure.
        bool Load(const std::string& path, std::string& error);
        std::vector<std::string> GetAllProperties();
        bool IsLoaded() const { return m_loaded; }
        bool FindRequiredAttributesAndInterfaces(const YamlNodePtr model, 
                                                 const std::string& path, std::string& error);

        const std::string& GetName() const { return m_name; }
        const std::string& GetVersion() const { return m_version; }
        const std::string& GetSourcePath() const { return m_sourcePath; }
        const std::string& GetFileContent() const { return m_fileContent; }

        const YamlNodePtr* FindInterfaceType(const std::string& name) const;

        /// Column order for a section: meta model properties first, followed by
        /// any additional keys that occur in @p entries but are not in the model.
        std::vector<std::string> ColumnsFor(const std::string& interfaceTypeName,
                                            const std::vector<YamlNodePtr>& entries) const;

        bool ValidateEnumValue(const std::string& propertyName, const std::string& enumName, 
                               const std::string& entry, std::string& error) const;

    private:                                            
        void CollectEnums(const YamlNodePtr& model);                                    

        bool m_loaded = false;
        std::string m_minimumVersion = "0.4.1";
        std::string m_fileContent;        
        std::string m_name;
        std::string m_version;
        std::string m_sourcePath;
        std::vector<YamlNodePtr> m_interfaceTypes;
        std::map<std::string, std::vector<std::string>> m_enums;
        std::vector<uint32_t> SplitVersion(const std::string& version) const;
        bool IsGreaterOrEqual(const std::string& version1, const std::string& version2) const;
        void Reset();
    };

} // namespace afd

#endif // AFD_META_MODEL_H
