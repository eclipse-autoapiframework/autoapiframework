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
#ifndef AFD_SUPERVISION_EDITOR_H
#define AFD_SUPERVISION_EDITOR_H

#include <wx/wx.h>

#include <string>
#include <utility>
#include <vector>

#include "yaml_node.h"

namespace afd 
{
    /// @brief Strings for the nodes and properties of the supervision editor.
    static constexpr const char* kAlive = "alive";
    static constexpr const char* kDeadline = "deadline";
    static constexpr const char* kLogical = "logical";

    static constexpr const char* kSupervisionRequired = "Supervision required";
    static constexpr const char* kType = "type";
    static constexpr const char* kMinIndications = "minIndications";
    static constexpr const char* kMaxIndications = "maxIndications";
    static constexpr const char* kReferenceCycleMs = "referenceCycleMs";
    static constexpr const char* kMaxExecutionTimeMs = "maxExecutionTimeMs";
    static constexpr const char* kPredecessorRefs = "predecessorRefs";
    static constexpr const char* kSuccessorRefs = "successorRefs";

    static constexpr const char* kEditSupervisionDlgTitle = "Edit Supervision";

    class SupervisionEditor
    {
    public:
        SupervisionEditor(wxDialog& dialog, wxSizer& parentSizer, const YamlNodePtr& existing);

        YamlNodePtr Build() const;

    private:
        struct DetailField
        {
            std::string name;
            bool isList;
            wxTextCtrl* control;
        };

        struct DetailGroup
        {
            std::string type;
            wxPanel* panel;
            std::vector<DetailField> fields;
        };

        unsigned int TypeIndex(const std::string& type) const;
        void AddGroup(wxSizer* detailsSizer, const YamlNodePtr& existing,
                      const std::string& type,
                      const std::vector<std::pair<std::string, bool>>& fieldDefinitions);
        void UpdateVisibility();
        bool IsInteger(const std::string& str) const;
        bool IsDigit(const std::string& str) const;        

        wxDialog& m_dialog;
        wxCheckBox* m_required = nullptr;
        wxPanel* m_details = nullptr;
        wxCheckListBox* m_types = nullptr;
        std::vector<DetailGroup> m_groups;
    };

} // namespace afd

#endif // AFD_SUPERVISION_EDITOR_H
