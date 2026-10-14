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
 
#ifndef AFD_VALIDATE_FUNCTION_H
#define AFD_VALIDATE_FUNCTION_H

#include <map>
#include <string>
#include <vector>

#include "meta_model.h"
#include "yaml_node.h"

namespace afd 
{
    class ValidateFunction
    {
    public:
        bool SyntaxCheckIsOK(const MetaModel& metaModel, const std::string& content, std::string& error);
    
    private:
        bool HeaderSyntaxCheck(const YamlNodePtr& root, std::string& error);
        bool InterfaceTypesSyntaxCheck(const YamlNodePtr& root, const MetaModel& metaModel, std::string& error);
        bool SyntaxCheckForSequence(const YamlNodePtr& node, const std::string& key, std::string& error, const std::string& displayName);
        bool ValidatePropertyExists(const YamlNodePtr& dataInterfaces, const std::string& interfaceTypeName, const MetaModel& metaModel, std::string& error);
        bool ValidateProperties(const YamlNodePtr& dataInterfaces, const std::string& interfaceTypeName, const MetaModel& metaModel, std::string& error);
        bool ValidateEnums(const YamlNodePtr& dataInterfaces, const std::string& interfaceTypeName, const MetaModel& metaModel, std::string& error);        
        bool ValidateBoolean(const YamlNodePtr& dataInterfaces, const std::string& interfaceTypeName, const MetaModel& metaModel, std::string& error);        
        bool ValidateMandatory(const YamlNodePtr& dataInterfaces, const std::string& interfaceTypeName, const MetaModel& metaModel, std::string& error);        
        bool ValidateType(const YamlNodePtr& dataInterfaces, std::string& error);    
        bool ValidateBooleanType(const std::string& itemName, const std::string& defaultValue, const std::string& min, const std::string& max, std::string& error );      
        bool IsBoolean(const std::string& str);         
        bool ValidateFloatType(const std::string& itemName, const std::string& defaultValue, const std::string& min, const std::string& max, std::string& error );
        bool IsFloat(const std::string& str);
        bool IsLargerThanFloat(const std::string& strMax, const std::string& strMin);        
        bool ValidateintegerType(const std::string& itemName, const std::string& defaultValue, const std::string& min, const std::string& max, std::string& error );        
        bool Isinteger(const std::string& str);       
        bool IsLargerThanInteger(const std::string& strMax, const std::string& strMin);             
        bool ValidateDoubleType(const std::string& itemName, const std::string& defaultValue, const std::string& min, const std::string& max, std::string& error );
        bool IsDouble(const std::string& str);
        bool IsLargerThanDouble(const std::string& strMax, const std::string& strMin); 
        bool SkipCertainPropertiesFromSyntaxCheck(const std::string& property, bool enumCheck, bool mandatoryCheck);     
        bool ValidateCertainProperties(const YamlNodePtr& node, const std::string& nodeName, std::string& error );
        bool SkipASILProperty(const YamlNodePtr& item, const std::string& interfaceTypeName);
        bool IsDigit(const std::string& str) const;          
    };
} // namespace afd

#endif // AFD_VALIDATE_FUNCTION_H
