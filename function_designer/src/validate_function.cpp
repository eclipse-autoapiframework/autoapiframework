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
 *   Thomas Pfleiderer - support 4.1, Support YAML comments, Drag & Drop, Enums in afs files added
 ********************************************************************************/
 
#include "validate_function.h"

#include "meta_model.h"
#include "function_specification.h"

#include <cstdint>
#include <algorithm>
#include <regex>

#include "yaml_parser.h"

namespace afd 
{
    bool ValidateFunction::SyntaxCheckIsOK(const MetaModel& metaModel, const std::string& content, std::string& error)
    {
        YamlParser parser;
        const YamlNodePtr root = parser.ParseText(content, error);
        if (!root)
        {
            return false;
        }
        const YamlNodePtr functionSpecification = root->Find(FunctionSpecification::kRootKey);
        if (!functionSpecification || !functionSpecification->IsMap())
        {
            error = std::string("'") + std::string(FunctionSpecification::kRootKey) + "' root key not found.";
            return false;
        }

        if (!HeaderSyntaxCheck(functionSpecification, error))
        {
            return false;
        }

        if (!ValidateEnumDefinitions(functionSpecification, metaModel, error))
        {
            return false;
        }

        if (!InterfaceTypesSyntaxCheck(functionSpecification, metaModel, error))
        {
            return false;
        }

        return true;
    }

    bool ValidateFunction::HeaderSyntaxCheck(const YamlNodePtr& root, std::string& error)
    {
        auto name = root->ScalarOf(FunctionSpecification::kNameKey);
        auto version = root->ScalarOf(FunctionSpecification::kVersionKey);
     
        if (name.size() == 0 || version.size() == 0)
        {
            error = std::string(FunctionSpecification::kNameKey) + " or " + std::string(FunctionSpecification::kVersionKey) 
                    + std::string(" in the function specification not found.");  
            return false;
        }        
        return true;
    }

    bool ValidateFunction::ValidateEnumDefinitions(const YamlNodePtr& root, const MetaModel& metaModel, std::string& error)
    {
        const YamlNodePtr enums = root->Find(FunctionSpecification::kEnumsKey);
        if (!enums)
        {
            return true;
        }
        if (!enums->IsMap())
        {
            error = std::string(FunctionSpecification::kEnumsKey) + " must be a mapping.";
            return false;
        }
        for (const auto& entry : enums->GetMap())
        {
            YamlNodePtr valueList = entry.second;
            if (valueList && valueList->IsMap())
            {
                for (const auto& attribute : valueList->GetMap())
                {
                    if (attribute.first != kDescriptionKey && attribute.first != FunctionSpecification::kEnumValuesKey)
                    {
                        error = "Enum '" + entry.first + "': invalid property '" + attribute.first + "'.";
                        return false;
                    }
                }
                valueList = valueList->Find(FunctionSpecification::kEnumValuesKey);
            }
            if (!valueList || !valueList->IsSequence())
            {
                error = "Enum '" + entry.first + "' must be a list of values or a mapping with '" + FunctionSpecification::kEnumValuesKey + "'.";
                return false;
            }

            std::vector<std::string> values;
            for (const YamlNodePtr& value : valueList->GetSequence())
            {
                if (!value || !value->IsScalar() || value->GetScalar().empty())
                {
                    error = "Enum '" + entry.first + "': values must be non-empty scalars.";
                    return false;
                }
                if (std::find(values.begin(), values.end(), value->GetScalar()) != values.end())
                {
                    error = "Enum '" + entry.first + "': duplicate value '" + value->GetScalar() + "'.";
                    return false;
                }
                values.push_back(value->GetScalar());
            }

            // An enum that also exists in the meta model must be identical to it.
            if (const std::vector<std::string>* expected = metaModel.FindEnumValues(entry.first))
            {
                if (*expected != values)
                {
                    error = "Enum '" + entry.first + "' differs from the meta model definition.\n\nValid would be:\n";
                    for (const std::string& value : *expected)
                    {
                        error += "  - " + value + "\n";
                    }
                    return false;
                }
            }
        }
        return true;
    }

    bool ValidateFunction::InterfaceTypesSyntaxCheck(const YamlNodePtr& root, const MetaModel& metaModel, std::string& error)
    {
        const YamlNodePtr dataInterfaces = root->Find(FunctionSpecification::kDataInterfacesKey);
        const YamlNodePtr parameters = root->Find(FunctionSpecification::kParametersKey);
        const YamlNodePtr scheduling = root->Find(FunctionSpecification::kSchedulingKey);   
        const YamlNodePtr errors = root->Find(FunctionSpecification::kErrorsKey);
        if (!dataInterfaces || !parameters || !scheduling)
        {
            error = std::string(FunctionSpecification::kDataInterfacesKey) + " or " + std::string(FunctionSpecification::kParametersKey) 
                + " or " + std::string(FunctionSpecification::kSchedulingKey) + " not found.";
            return false;
        }

        // Signals
        if (!SyntaxCheckForSequence(dataInterfaces, FunctionSpecification::kNamePathKey, error, FunctionSpecification::kDataInterfacesKey)) 
        {
            return false;
        }
        if (!ValidateProperties(root, dataInterfaces, afd::kDataInterfaceTypeKey, metaModel, error))
        {
            return false;
        }       

        // Parameters
        if (!SyntaxCheckForSequence(parameters, FunctionSpecification::kNamePathKey, error, FunctionSpecification::kParametersKey)) 
        {
            return false;
        }
        if (!ValidateParameterNames(parameters, error))
        {
            return false;
        }
        if (!ValidateProperties(root, parameters, afd::kParameterInterfaceTypeKey, metaModel, error))
        {
            return false;
        }                          

        // Scheduling
        if (!SyntaxCheckForSequence(scheduling, FunctionSpecification::kFunctionNameKey, error, FunctionSpecification::kSchedulingKey)) 
        {
            return false;
        }
        if (!ValidateProperties(root, scheduling, afd::kSchedulingInterfaceTypeKey, metaModel, error))
        {
            return false;
        } 

        if (!ValidateSupervision(scheduling, error))        
        {
            return false;
        }                

        // Errors
        if (errors)
        {
            if (!SyntaxCheckForSequence(errors, FunctionSpecification::kNamePathKey, error, FunctionSpecification::kErrorsKey))
            {
                return false;
            }
            
            if (!ValidateProperties(root, errors, afd::kErrorInterfaceTypeKey, metaModel, error))
            {
                return false;
            }
        }

        return true;
    }

    bool ValidateFunction::ValidateProperties(const YamlNodePtr& functionSpecification, const YamlNodePtr& dataInterfaces, 
                            const std::string& interfaceTypeName, const MetaModel& metaModel, std::string& error)
    {
        if (!ValidatePropertyExists(dataInterfaces, interfaceTypeName, metaModel, error))
        {
            return false;
        }
        if (!ValidateBoolean(dataInterfaces, interfaceTypeName, metaModel, error))
        {
            return false;
        }           
        if (!ValidateEnums(dataInterfaces, interfaceTypeName, metaModel, error))
        {
            return false;
        }
        if (!ValidateMandatory(dataInterfaces, interfaceTypeName, metaModel, error))
        {
            return false;
        }  
        if (!ValidateType(functionSpecification, dataInterfaces, metaModel, error))
        {
            return false;
        }
        return true;
    }

    bool ValidateFunction::ValidateSupervision(const YamlNodePtr& dataInterfaces, std::string& error)
    {
        for (const YamlNodePtr& item : dataInterfaces->GetSequence())
        {
            if (!item || !item->IsMap())
            {
                error = "Each collection item must be a mapping.";
                return false;
            }

            // in scheduling it is functionName
            std::string itemFunctionName = item->Find(kFunctionNameKey) ? item->Find(kFunctionNameKey)->GetScalar() : "<unknown>";               
            auto itemSupervision = item->Find(kSupervisionKey);  
            if (itemSupervision)
            {
                std::string required = itemSupervision->Find(kSupervisionRequiredKey) ? itemSupervision->Find(kSupervisionRequiredKey)->GetScalar() : "<unknown>";
                if (required == kFalseKey)
                {
                    continue;
                }
                else if (required == kTrueKey)
                {
                    auto type = itemSupervision->Find(kSupervisionTypeKey);                    
                    auto alive = itemSupervision->Find(kSupervisionAliveKey);
                    auto deadline = itemSupervision->Find(kSupervisionDeadlineKey);
                    auto logical = itemSupervision->Find(kSupervisionLogicalKey);
                    if(!type || (!alive && !deadline && !logical))
                    {
                        error = "Missing supervision fields for item '" + itemFunctionName + "'";
                        return false;
                    }
                    for (const YamlNodePtr& typeItem : type->GetSequence())
                    {
                        if (!typeItem || !typeItem->IsScalar() || typeItem->GetScalar().empty())
                        {
                            error = "Each supervision type must be a non-empty scalar for item '" + itemFunctionName + "'";
                            return false;
                        }

                        auto requiredType = itemSupervision->Find(typeItem->GetScalar());     
                        if (!requiredType)
                        {
                            error = "Missing required supervision type " + typeItem->GetScalar() + " for item '" + itemFunctionName + "'";
                            return false;
                        }
                    }
                    if (alive)
                    {
                        auto minIndications = alive->Find(kMinIndicationsKey);
                        auto maxIndications = alive->Find(kMaxIndicationsKey);
                        auto referenceCycleMs = alive->Find(kReferenceCycleMsKey);
                        if (!minIndications && !maxIndications && !referenceCycleMs)
                        {
                            error = "Supervision of type '" + std::string(kSupervisionAliveKey) + "' has no content for item '" + itemFunctionName + "'";
                            return false;
                        }
                    }
                    if (deadline)
                    {
                        auto maxExecutionTimeMs = deadline->Find(kMaxExecutionTimeMsKey);
                        if (!maxExecutionTimeMs)
                        {
                            error = "Supervision of type '" + std::string(kSupervisionDeadlineKey) + "' has no content for item '" + itemFunctionName + "'";
                            return false;
                        }
                    }
                    if (logical)
                    {
                        auto predecessorRefs = logical->Find(kPredecessorRefsKey);
                        auto successorRefs = logical->Find(kSuccessorRefsKey);
                        if (!predecessorRefs && !successorRefs)
                        {
                            error = "Supervision of type '" + std::string(kSupervisionLogicalKey) + "' has no content for item '" + itemFunctionName + "'";
                            return false;
                        }
                    }
                    continue;
                }                
                else
                {
                    error = "Invalid supervision node: '" + itemFunctionName + "'\n expect node '" + std::string(kSupervisionRequiredKey)
                            + " = " + std::string(kTrueKey) + " or " + std::string(kFalseKey)+ "'";
                    return false;
                }             
            }
        }
        return true;
    }    

    bool ValidateFunction::SyntaxCheckForSequence(const YamlNodePtr& node, const std::string& key, std::string& error, const std::string& displayName)
    {
        for (const YamlNodePtr& dataInterface : node->GetSequence())
        {
            const YamlNodePtr namePath = dataInterface
                ? dataInterface->Find(key)
                : nullptr;
            if (!dataInterface || !dataInterface->IsMap() || !namePath || !namePath->IsScalar() || namePath->GetScalar().empty())
            {
                error = "Each " + displayName + " must be a mapping with a non-empty " + key + ".";
                return false;
            }
        }        
        return true;
    }

    bool ValidateFunction::ValidateParameterNames(const YamlNodePtr& parameters, std::string& error)
    {
        static const std::regex parameterNamePattern(R"(^Vehicle(?:\.[A-Za-z0-9_]+){2,}$)");
        for (const YamlNodePtr& parameter : parameters->GetSequence())
        {
            const std::string name = parameter->Find(FunctionSpecification::kNamePathKey)->GetScalar();
            if (!std::regex_match(name, parameterNamePattern))
            {
                error = "Parameter name '" + name + "' must follow the hierarchical naming convention "
                    "Vehicle.<Domain/Subdomains>.<Parameter>.";
                return false;
            }
        }
        return true;
    }

    bool ValidateFunction::ValidatePropertyExists(const YamlNodePtr& dataInterfaces, const std::string& interfaceTypeName,
                                            const MetaModel& metaModel, std::string& error)
    {
        std::vector<std::string> validPropertyNamesFromMetaModel = metaModel.ColumnsFor(interfaceTypeName, {});
        YamlNodePtr companionDefinitions;
        if (const YamlNodePtr* type = metaModel.FindInterfaceType(interfaceTypeName))
        {
            companionDefinitions = interfaceTypeName == kDataInterfaceTypeKey
                ? (*type)->Find(kRuntimeCompanionKey) : nullptr;
        }
        validPropertyNamesFromMetaModel.push_back(kEnumRefKey); // exception: enumRef is allowed too

        bool syntaxError = false;
        std::string text = "";
        for (const YamlNodePtr& item : dataInterfaces->GetSequence())
        {
            if (!item || !item->IsMap())
            {
                error = "Each collection item must be a mapping.";
                return false;
            }
            for (const auto& property : item->GetMap())
            {
                if (property.first == kRuntimeCompanionKey && companionDefinitions && companionDefinitions->IsMap())
                {
                    if (!property.second || !property.second->IsMap())
                    {
                        error = std::string(kRuntimeCompanionKey) + " must be a mapping.";
                        return false;
                    }
                    for (const auto& companion : property.second->GetMap())
                    {
                        if (!companionDefinitions->Find(companion.first))
                        {
                            error = "Invalid " + std::string(kRuntimeCompanionKey) + ": " + companion.first;
                            return false;
                        }
                    }
                    continue;
                }
                auto it = std::find(validPropertyNamesFromMetaModel.begin(), 
                                    validPropertyNamesFromMetaModel.end(), property.first);
                if (it == validPropertyNamesFromMetaModel.end())
                {
                    syntaxError = true;
                    text += " " + property.first;
                }
            }
        }

        if (syntaxError)
        {
            error = "Invalid properties found: " + text;
            return false;
        }
        return true;
    }

    bool ValidateFunction::SkipASILProperty(const YamlNodePtr& item, const std::string& interfaceTypeName)
    {
        if (interfaceTypeName == kDataInterfaceTypeKey)
        {
            std::string itemFusa = item->Find(kFuSaKey) ? item->Find(kFuSaKey)->GetScalar() : "";  
            if (itemFusa == kFalseKey)
            {
                return true;
            }
        }

        return false;
    }

    bool ValidateFunction::ValidateEnums(const YamlNodePtr& dataInterfaces, const std::string& interfaceTypeName,
                                         const MetaModel& metaModel, std::string& error)
    {
        std::map<std::string, std::string> propertiesWithEnumRef;
        if (const YamlNodePtr* type = metaModel.FindInterfaceType(interfaceTypeName))
        {
            const YamlNodePtr properties = (*type)->Find(kPropertiesKey);
            if (properties && properties->IsMap())
            {
                for (const auto& property : properties->GetMap())                
                {
                    const YamlNodePtr enumRef = property.second->Find(kEnumRefKey);
                    if (enumRef && enumRef->IsScalar())
                    {
                        propertiesWithEnumRef.insert(std::make_pair(property.first, enumRef->GetScalar()));
                    }
                }
            }
        }

        for (const YamlNodePtr& item : dataInterfaces->GetSequence())
        {
            if (!item || !item->IsMap())
            {
                error = "Each collection item must be a mapping.";
                return false;
            }

            bool skipASILCheck = SkipASILProperty(item, interfaceTypeName);
            // in scheduling it is functionName
            std::string itemFunctionName = item->Find(kFunctionNameKey) ? item->Find(kFunctionNameKey)->GetScalar() : "<unknown>";               
            std::string itemName = item->Find(kNameKey) ? item->Find(kNameKey)->GetScalar() : itemFunctionName;                   
          
            for (const auto& property : item->GetMap())
            {
                if (SkipCertainPropertiesFromSyntaxCheck(property.first, true, false))
                {
                    continue;
                }
                if (skipASILCheck && (property.first == kASILKey))
                {
                    if (property.second->GetScalar() != "")
                    {
                        error =  itemName +":\n" + kFuSaKey + " is set to false, " + kASILKey +" should be empty, instead it is " + property.second->GetScalar() + ".";
                        return false;                        
                    }

                    continue;
                }                

                auto it = propertiesWithEnumRef.find(property.first.c_str());
                if (it != propertiesWithEnumRef.end())
                {
                    if (!metaModel.ValidateEnumValue(property.first, it->second, property.second->GetScalar(), error))
                    {
                        error +=  "\n(" + itemName + ")";
                        return false;
                    }  
                }
            }
        }        
        return true;
    }

    bool ValidateFunction::ValidateDefaultValueEnum(const std::string& itemName, const YamlNodePtr& functionSpecification, 
                            const std::string& defaultValue, const std::string& enumName, const MetaModel& metaModel, std::string& error)
    {    
        auto enums = GetEnums(functionSpecification);
        for (const auto& enumPair : enums)
        {
            if (enumPair.first == enumName)
            {
                for (const auto& enumValue : enumPair.second)
                {
                    if (enumValue == defaultValue)
                    {
                        return true;
                    }
                }
                error = "Default value syntax check: failed\n\nInvalid enum value '" + defaultValue +"' for property '" + enumName + "'. (" + itemName + ")";
                error += "\n\nValid would be:\n";
                for (const auto& value : enumPair.second)
                {
                    error += "  - " + value + "\n";
                }
                return false;
            }
        }

        if (!metaModel.ValidateEnumValue( "", enumName, defaultValue, error))
        {
            return false;
        }
        return true;
    }

    bool ValidateFunction::ValidateBoolean(const YamlNodePtr& dataInterfaces, const std::string& interfaceTypeName,
                                         const MetaModel& metaModel, std::string& error)
    {
        std::map<std::string, std::string> propertiesWithBooleanType;
        if (const YamlNodePtr* type = metaModel.FindInterfaceType(interfaceTypeName))
        {
            const YamlNodePtr properties = (*type)->Find(kPropertiesKey);
            if (properties && properties->IsMap())
            {
                for (const auto& property : properties->GetMap())                
                {
                    const YamlNodePtr propertyTypeNode = property.second->Find(kDatatypeKey );
                    if (propertyTypeNode && propertyTypeNode->IsScalar() && propertyTypeNode->GetScalar() == kBooleanType)
                    {
                        propertiesWithBooleanType.insert(std::make_pair(property.first, propertyTypeNode->GetScalar()));
                    }
                }
            }
        }

        for (const YamlNodePtr& item : dataInterfaces->GetSequence())
        {
            if (!item || !item->IsMap())
            {
                error = "Each collection item must be a mapping.";
                return false;
            }
            // in scheduling it is functionName
            std::string itemFunctionName = item->Find(kFunctionNameKey) ? item->Find(kFunctionNameKey)->GetScalar() : "<unknown>";               
            std::string itemName = item->Find(kNameKey) ? item->Find(kNameKey)->GetScalar() : itemFunctionName;            
            for (const auto& property : item->GetMap())
            {
                auto it = propertiesWithBooleanType.find(property.first.c_str());
                if (it != propertiesWithBooleanType.end())
                {
                    if (property.second->GetScalar() != kTrueKey && property.second->GetScalar() != kFalseKey)
                    {
                        error = property.first + " = " + property.second->GetScalar() +
                                " but must be either '" + std::string(kTrueKey) + "' or '" + std::string(kFalseKey) + "'. (" + itemName + ")";
                        return false;
                    }
                }
            }
        }        
        return true;
    }
    
    bool ValidateFunction::ValidateMandatory(const YamlNodePtr& dataInterfaces, const std::string& interfaceTypeName,
                                         const MetaModel& metaModel, std::string& error)
    {
        std::map<std::string, std::string> propertiesWithMandatory;
        if (const YamlNodePtr* type = metaModel.FindInterfaceType(interfaceTypeName))
        {
            const YamlNodePtr properties = (*type)->Find(kPropertiesKey);
            if (properties && properties->IsMap())
            {
                for (const auto& property : properties->GetMap())                
                {
                    const YamlNodePtr mandatoryNode = property.second->Find(kMandatoryKey);
                    if (mandatoryNode && mandatoryNode->IsScalar() && mandatoryNode->GetScalar() == kTrueKey)
                    {
                        propertiesWithMandatory.insert(std::make_pair(property.first, mandatoryNode->GetScalar()));
                    }
                }
            }
        }

        for (const YamlNodePtr& item : dataInterfaces->GetSequence())
        {
            if (!item || !item->IsMap())
            {
                error = "Each collection item must be a mapping.";
                return false;
            }

            // all mandatory properties must be present in the item
            for (const auto& property : propertiesWithMandatory)
            {
                if (SkipCertainPropertiesFromSyntaxCheck(property.first, false, true))
                {
                    continue;
                }                
                if (!item->Find(property.first))
                {
                    error = property.first + " is missing but property is mandatory. (" + (item->Find(kNameKey) ? item->Find(kNameKey)->GetScalar() : "<unknown>") + ")";
                    return false;
                }
            }

            // in scheduling it is functionName
            std::string itemFunctionName = item->Find(kFunctionNameKey) ? item->Find(kFunctionNameKey)->GetScalar() : "<unknown>";            
            std::string itemName = item->Find(kNameKey) ? item->Find(kNameKey)->GetScalar() : itemFunctionName;
            for (const auto& property : item->GetMap())
            {
                if (SkipCertainPropertiesFromSyntaxCheck(property.first, false, true))
                {
                    continue;
                }

                auto it = propertiesWithMandatory.find(property.first.c_str());
                if (it != propertiesWithMandatory.end())
                {
                    if (property.second->GetScalar() == "" || property.second->GetScalar() == " ")
                    {
                        error = property.first + " is empty but property is mandatory. (" + itemName + ")";
                        return false;
                    }
                }
            }
        }        
        return true;
    }

    bool ValidateFunction::ValidateType(const YamlNodePtr& functionSpecification, const YamlNodePtr& dataInterfaces, 
                                        const MetaModel& metaModel, std::string& error)
    {
        for (const YamlNodePtr& item : dataInterfaces->GetSequence())
        {
            if (!item || !item->IsMap())
            {
                error = "Each collection item must be a mapping.";
                return false;
            }
            // in scheduling it is functionName
            std::string itemFunctionName = item->Find(kFunctionNameKey) ? item->Find(kFunctionNameKey)->GetScalar() : "<unknown>";               
            std::string itemName = item->Find(kNameKey) ? item->Find(kNameKey)->GetScalar() : itemFunctionName;  
            std::string itemMin = item->Find(FunctionSpecification::kMinKey) ? item->Find(FunctionSpecification::kMinKey)->GetScalar() : "";
            std::string itemMax = item->Find(FunctionSpecification::kMaxKey) ? item->Find(FunctionSpecification::kMaxKey)->GetScalar() : "";
            std::string itemDefault = item->Find(FunctionSpecification::kDefaultValueKey) ? item->Find(FunctionSpecification::kDefaultValueKey)->GetScalar() : "";                        
            std::string itemDataType = item->Find(FunctionSpecification::kDataTypeKey) ? item->Find(FunctionSpecification::kDataTypeKey)->GetScalar() : "";
            if (itemDataType == FunctionSpecification::kFloatDataType)
            {                
                if (!ValidateFloatType(itemName, itemDefault, itemMin, itemMax, error))
                {
                    return false;
                }   
            }
            if ((itemDataType == FunctionSpecification::kInt8DataType ) || (itemDataType == FunctionSpecification::kUInt8DataType ) 
               || (itemDataType == FunctionSpecification::kInt16DataType ) || (itemDataType == FunctionSpecification::kUInt16DataType ) 
               || (itemDataType == FunctionSpecification::kInt32DataType ) || (itemDataType == FunctionSpecification::kUInt32DataType )
               || (itemDataType == FunctionSpecification::kInt64DataType ) || (itemDataType == FunctionSpecification::kUInt64DataType ))               
            {                
                if (!ValidateintegerType(itemName, itemDefault, itemMin, itemMax, error))
                {
                    return false;
                }   
            }
            if (itemDataType == FunctionSpecification::kDoubleDataType) 
            {                
                if (!ValidateDoubleType(itemName, itemDefault, itemMin, itemMax, error))
                {
                    return false;
                }   
            }
            if ((itemDataType == FunctionSpecification::kBoolDataType) || (itemDataType == kBooleanType))
            {                
                if (!ValidateBooleanType(itemName, itemDefault, itemMin, itemMax, error))
                {
                    return false;
                }   
            }
            if (itemDataType == kEnumKey)                  
            {
                std::string enumRef = item->Find(kEnumRefKey) ? item->Find(kEnumRefKey)->GetScalar() : "";
                if (enumRef.empty())
                {
                    error = itemName + ":\ntype = enum, enumRef expected but not found.";
                    return false;
                }
                if (!ValidateDefaultValueEnum(itemName, functionSpecification, itemDefault, enumRef, metaModel, error))
                {
                    return false;
                }
            }

            if (!ValidateCertainProperties(item, itemName, error))
            {     
                return false;
            }            
        }  

        return true;
    }

    bool ValidateFunction::ValidateCertainProperties(const YamlNodePtr& node, const std::string& nodeName, std::string& error )
    {
        // signal related properties
        std::string itemPrecision = node->Find(FunctionSpecification::kPrecisionKey) ? node->Find(FunctionSpecification::kPrecisionKey)->GetScalar() : "";
        if (!Isinteger(itemPrecision) && !IsFloat(itemPrecision) && !IsDouble(itemPrecision))
        {
            error = nodeName + ":\n\n" + FunctionSpecification::kPrecisionKey + " must be integer/float/double: "  + itemPrecision;        
            return false;
        }
        std::string itemMinUpdatePeriodMs = node->Find(FunctionSpecification::kMinUpdatePeriodMsKey) ? node->Find(FunctionSpecification::kMinUpdatePeriodMsKey)->GetScalar() : "";            
        if (!Isinteger(itemMinUpdatePeriodMs))
        {
            error = nodeName + ":\n\n" + FunctionSpecification::kMinUpdatePeriodMsKey + " must be integer: "  + itemMinUpdatePeriodMs;        
            return false;
        }
        // scheduling related properties
        std::string itemStackSizeBytes = node->Find(FunctionSpecification::kStackSizeBytesKey) ? node->Find(FunctionSpecification::kStackSizeBytesKey)->GetScalar() : "";              
        if (!Isinteger(itemStackSizeBytes))
        {
            error = nodeName + ":\n\n" + FunctionSpecification::kStackSizeBytesKey + " must be integer: "  + itemStackSizeBytes;        
            return false;
        }          
        std::string itemCycleTimeMs = node->Find(FunctionSpecification::kCycleTimeMsKey) ? node->Find(FunctionSpecification::kCycleTimeMsKey)->GetScalar() : "";              
        if (!Isinteger(itemCycleTimeMs))
        {
            error = nodeName + ":\n\n" + FunctionSpecification::kCycleTimeMsKey + " must be integer: "  + itemCycleTimeMs;        
            return false;
        }  
        // Error interface related properties
        std::string itemMaturationTimeMs = node->Find(FunctionSpecification::kMaturationTimeMsKey) ? node->Find(FunctionSpecification::kMaturationTimeMsKey)->GetScalar() : "";              
        if (!Isinteger(itemMaturationTimeMs))
        {
            error = nodeName + ":\n\n" + FunctionSpecification::kMaturationTimeMsKey + " must be integer: "  + itemMaturationTimeMs;        
            return false;
        }  
        std::string itemresetTimeMs = node->Find(FunctionSpecification::kResetTimeMsKey) ? node->Find(FunctionSpecification::kResetTimeMsKey)->GetScalar() : "";              
        if (!Isinteger(itemresetTimeMs))
        {
            error = nodeName + ":\n\n" + FunctionSpecification::kResetTimeMsKey + " must be integer: "  + itemresetTimeMs;        
            return false;
        } 

        return true;
    }

    bool ValidateFunction::ValidateBooleanType(const std::string& itemName, const std::string& defaultValue,
                         const std::string& min, const std::string& max, std::string& error )
    {
        if (IsBoolean(defaultValue) && IsBoolean(min) && IsBoolean(max))
        {
            return true;
        }

        error = itemName + ":\nall values must be boolean\n\n" + min + " (min), " + max + " (max), " + defaultValue + " (default)"; 
        error +=  + "\n\naccepted boolean values are true, false, 1, 0, yes, no";          
        return false;
    }    

    bool ValidateFunction::IsBoolean(const std::string& str)
    {
        if (str.empty() || str == "null")
        {
            return true;
        }

        return str == "true" || str == "false" || str == "1" || str == "0" || str == "yes" || str == "no";
    }

    bool ValidateFunction::ValidateFloatType(const std::string& itemName, const std::string& defaultValue,
                         const std::string& min, const std::string& max, std::string& error )
    {
        if (IsFloat(defaultValue) && IsFloat(min) && IsFloat(max))
        {
            if (!IsLargerThanFloat(max, min))
            {
                error = itemName + ":\nmax should be greater than min\n\n" + max + " (max), " + min + " (min)";  
                return false;
            }
            return true;
        }

        error = itemName + ":\nall values must be empty or float\n\n" + min + " (min), " + max + " (max), " + defaultValue + " (default)";  
        return false;
    }

    bool ValidateFunction::IsFloat(const std::string& str)
    {
        if (str.empty() || str == "null")
        {
            return true;
        }

        try
        {
            size_t pos;
            std::stof(str, &pos);
            
            return pos == str.length(); // Ensure the entire string was parsed
        }
        catch (...)
        {
            return false;
        }
    }
    
    bool ValidateFunction::IsLargerThanFloat(const std::string& strMax, const std::string& strMin)
    {   
        if (strMax.empty() || strMax == "null" || strMin.empty() || strMin == "null")
        {
            return true;
        }
        try
        {
            return std::stof(strMax) > std::stof(strMin);
        }
        catch (...)
        {
            return false;
        }
    }

    bool ValidateFunction::ValidateintegerType(const std::string& itemName, const std::string& defaultValue,
                         const std::string& min, const std::string& max, std::string& error )
    {
        if (Isinteger(defaultValue) && Isinteger(min) && Isinteger(max))
        {
            if (!IsLargerThanInteger(max, min))
            {
                error = itemName + ":\nmax should be greater than min\n\n" + max + " (max), " + min + " (min)";  
                return false;
            }            
            return true;
        }

        error = itemName + ":\nall values must be empty or integer\n\n" + min + " (min), " + max + " (max), " + defaultValue + " (default)";          
        return false;
    }    
    
    bool ValidateFunction::Isinteger(const std::string& str)
    {
        if (str.empty() || str == "null")
        {
            return true;
        }

        try
        {
            size_t pos;
            std::stoi(str, &pos);
            
            return pos == str.length(); // Ensure the entire string was parsed
        }
        catch (...)
        {
            return false;
        }
    }   

    bool ValidateFunction::IsLargerThanInteger(const std::string& strMax, const std::string& strMin)
    {   
        if (strMax.empty() || strMax == "null" || strMin.empty() || strMin == "null")
        {
            return true;
        }
        try
        {
            return std::stoi(strMax) > std::stoi(strMin);
        }
        catch (...)
        {
            return false;
        }
    }

    bool ValidateFunction::ValidateDoubleType(const std::string& itemName, const std::string& defaultValue,
                         const std::string& min, const std::string& max, std::string& error )
    {
        if (IsDouble(defaultValue) && IsDouble(min) && IsDouble(max))
        {
            if (!IsLargerThanDouble(max, min))
            {
                error = itemName + ":\nmax should be greater than min\n\n" + max + " (max), " + min + " (min)";  
                return false;
            }
            return true;
        }

        error = itemName + ":\nall values must be empty or double\n\n" + min + " (min), " + max + " (max), " + defaultValue + " (default)";        
        return false;
    }    
    
    bool ValidateFunction::IsDouble(const std::string& str)
    {
        if (str.empty() || str == "null")
        {
            return true;
        }

        try
        {
            size_t pos;
            std::stod(str, &pos);
            
            return pos == str.length(); // Ensure the entire string was parsed
        }
        catch (...)
        {
            return false;
        }
    }     
    
    bool ValidateFunction::IsLargerThanDouble(const std::string& strMax, const std::string& strMin)
    {   
        if (strMax.empty() || strMax == "null" || strMin.empty() || strMin == "null")
        {
            return true;
        }        
        try
        {
            return std::stod(strMax) > std::stod(strMin);
        }
        catch (...)
        {
            return false;
        }
    }
    
    bool ValidateFunction::SkipCertainPropertiesFromSyntaxCheck(const std::string& property, bool enumCheck, bool mandatoryCheck)
    {
        if (enumCheck || mandatoryCheck)
        {
            if ((property == kExecutionResultKey) || (property == kSupervisionKey))
            {
                return true;
            }
        }            

        return false;
    }

    bool ValidateFunction::IsDigit(const std::string& str) const
    {
        if (str.empty() || str == "null")
        {
            return true;
        }

        for (char c : str)
        {
            if (!std::isdigit(static_cast<unsigned char>(c)))
            {
                return false;
            }
        }
        return true;
    }

    std::vector<std::pair<std::string, std::vector<std::string>>> ValidateFunction::GetEnums(const YamlNodePtr functionSpecification) const
    {
        std::vector<std::pair<std::string, std::vector<std::string>>> result;
        const YamlNodePtr enums = functionSpecification ? functionSpecification->Find(kEnumsKey) : nullptr;
        if (!enums || !enums->IsMap())
        {
            return result;
        }
        for (const auto& entry : enums->GetMap())
        {
            YamlNodePtr valueList = entry.second;
            if (valueList && valueList->IsMap())
            {
                valueList = valueList->Find(kEnumValuesKey);
            }
            std::vector<std::string> values;
            if (valueList && valueList->IsSequence())
            {
                for (const YamlNodePtr& value : valueList->GetSequence())
                {
                    if (value && value->IsScalar())
                    {
                        values.push_back(value->GetScalar());
                    }
                }
            }
            result.emplace_back(entry.first, std::move(values));
        }
        return result;
    }
} // namespace afd
