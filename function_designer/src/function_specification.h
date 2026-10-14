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
 
#ifndef AFD_FUNCTION_SPECIFICATION_H
#define AFD_FUNCTION_SPECIFICATION_H

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "yaml_node.h"

namespace afd 
{
    /// In-memory representation of a function specification (*.afs, YAML content).
    ///
    /// The parsed YAML tree is kept as-is so that unknown keys survive a
    /// load/save round trip and the file can be written in the original layout.
    class FunctionSpecification 
    {
    public:
        /// @brief Expected strings within the autoapiframework function specification file
        static constexpr const char* kRootKey = "functionSpecification";
        static constexpr const char* kNameKey = "name";
        static constexpr const char* kVersionKey = "version";
        static constexpr const char* kDescriptionKey = "description";        
        static constexpr const char* kMetaModelRefKey = "metaModelRef";
        static constexpr const char* kDataInterfacesKey = "dataInterfaces";
        static constexpr const char* kParametersKey = "parameters";
        static constexpr const char* kSchedulingKey = "scheduling";
        static constexpr const char* kErrorsKey = "errorInterfaces";
        static constexpr const char* kPropertiesKey = "properties";   
        static constexpr const char* kNamePathKey = "name";
        static constexpr const char* kFunctionNameKey = "functionName";

        static constexpr const char* kMinKey = "min";
        static constexpr const char* kMaxKey = "max";
        static constexpr const char* kDefaultValueKey = "defaultValue";
        static constexpr const char* kDataTypeKey = "dataType";
        static constexpr const char* kFloatDataType = "float";
        static constexpr const char* kDoubleDataType = "double";
        static constexpr const char* kBoolDataType = "bool";        
        static constexpr const char* kInt8DataType = "int8";        
        static constexpr const char* kInt16DataType = "int16";        
        static constexpr const char* kInt32DataType = "int32";        
        static constexpr const char* kInt64DataType = "int64";        
        static constexpr const char* kUInt8DataType = "uint8";        
        static constexpr const char* kUInt16DataType = "uint16";        
        static constexpr const char* kUInt32DataType = "uint32";
        static constexpr const char* kUInt64DataType = "uint64";              
        static constexpr const char* kPrecisionKey = "precision";
        static constexpr const char* kMinUpdatePeriodMsKey = "minUpdatePeriodMs";
        static constexpr const char* kCycleTimeMsKey = "cycleTimeMs";
        static constexpr const char* kMaturationTimeMsKey = "maturationTimeMs";
        static constexpr const char* kResetTimeMsKey = "resetTimeMs";
        static constexpr const char* kStackSizeBytesKey = "stackSizeBytes";

        bool Load(const std::string& expectedVersion, const std::string& path, std::string& error);
        /// Creates a fresh, empty function specification referencing the given meta model.
        void New(const std::string& metaModelName, const std::string& metaModelVersion);
        bool Save(const std::string& path, std::string& error) const;
        std::string ToText() const;
        bool CheckMetaModelVersion(const YamlNodePtr ref, const std::string& expectedVersion, 
            const std::string& path, std::string& error);

        bool IsLoaded() const { return m_root != nullptr; }
        const std::string& GetSourcePath() const { return m_sourcePath; }

        std::string GetName() const;
        std::string GetVersion() const;
        std::string GetDescription() const;
        void SetName(const std::string& value);
        void SetVersion(const std::string& value);
        void SetDescription(const std::string& value);
        std::string GetMetaModelName() const;
        std::string GetMetaModelVersion() const;

        /// Header attributes (all scalar keys of `functionSpecification`) in file order.
        std::vector<std::pair<std::string, std::string>> GetAttributes() const;

        /// Entries of the signal, parameter or scheduling collection.
        std::vector<YamlNodePtr> GetCollection(const std::string& key) const;
        bool AddCollectionItem(const std::string& key, const YamlNodePtr item);
        bool RemoveCollectionItem(const std::string& key, const std::size_t index);

    private:
        const YamlNodePtr& Spec() const { return m_spec; }
        void SetEditableAttribute(const std::string& key, const std::string& value);

        YamlNodePtr m_root;
        YamlNodePtr m_spec;
        std::string m_sourcePath;
    };

} // namespace afd

#endif // AFD_FUNCTION_SPECIFICATION_H
