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
 
#include "supervision_editor.h"

#include "meta_model.h"

namespace afd 
{
    namespace
    {
        wxString ToWx(const std::string& text)
        {
            return wxString::FromUTF8(text.c_str());
        }

        std::string ToStd(const wxString& text)
        {
            return std::string(text.utf8_str());
        }
    }

    SupervisionEditor::SupervisionEditor(wxDialog& dialog, wxSizer& parentSizer,
                                         const YamlNodePtr& existing)
        : m_dialog(dialog)
    {
        const bool required = existing &&
            existing->ScalarOf(kSupervisionRequiredKey) == "true";
        m_required = new wxCheckBox(&dialog, wxID_ANY, "Supervision required");
        m_required->SetValue(required);
        parentSizer.Add(m_required, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);

        m_details = new wxPanel(&dialog, wxID_ANY);
        wxBoxSizer* detailsSizer = new wxBoxSizer(wxVERTICAL);
        detailsSizer->Add(new wxStaticText(m_details, wxID_ANY, "Types"), 0, wxBOTTOM, 4);
        m_types = new wxCheckListBox(m_details, wxID_ANY);
        m_types->Append("alive");
        m_types->Append("deadline");
        m_types->Append("logical");
        detailsSizer->Add(m_types, 0, wxEXPAND | wxBOTTOM, 8);

        const YamlNodePtr existingTypes = existing ? existing->Find("type") : nullptr;
        for (unsigned int index = 0; index < m_types->GetCount(); ++index)
        {
            if (existingTypes && existingTypes->IsSequence())
            {
                for (const YamlNodePtr& type : existingTypes->GetSequence())
                {
                    if (type && type->IsScalar() &&
                        type->GetScalar() == ToStd(m_types->GetString(index)))
                    {
                        m_types->Check(index);
                    }
                }
            }
        }

        AddGroup(detailsSizer, existing, "alive",
                 {{"minIndications", false}, {"maxIndications", false},
                  {"referenceCycleMs", false}});
        AddGroup(detailsSizer, existing, "deadline",
             {{"maxExecutionTimeMs", false}});
        AddGroup(detailsSizer, existing, "logical",
                 {{"predecessorRefs", true}, {"successorRefs", true}});

        m_details->SetSizer(detailsSizer);
        parentSizer.Add(m_details, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);
        m_required->Bind(wxEVT_CHECKBOX,
            [this](wxCommandEvent&) { UpdateVisibility(); });
        m_types->Bind(wxEVT_CHECKLISTBOX,
            [this](wxCommandEvent&) { UpdateVisibility(); });
        m_dialog.Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            if (m_required->GetValue())
            {
                bool hasType = false;
                for (unsigned int index = 0; index < m_types->GetCount(); ++index)
                {
                    if (m_types->IsChecked(index))
                    {
                        hasType = true;
                        break;
                    }
                }
                if (!hasType)
                {
                    wxMessageBox("Select at least one supervision type.", "Edit Supervision",
                                 wxOK | wxICON_WARNING, &m_dialog);
                    m_types->SetFocus();
                    return;
                }
                for (const DetailGroup& group : m_groups)
                {
                    if (group.type == "deadline" && m_types->IsChecked(TypeIndex("deadline")))
                    {
                        for (const DetailField& field : group.fields)
                        {
                            if (field.name == "maxExecutionTimeMs" &&
                                field.control->GetValue().Strip(wxString::both).empty())
                            {
                                wxMessageBox("'maxExecutionTimeMs' is mandatory.\nEnter maxExecutionTimeMs for deadline supervision.",
                                             "Edit Supervision", wxOK | wxICON_WARNING, &m_dialog);
                                field.control->SetFocus();
                                return;
                            }
                            if (field.name == "maxExecutionTimeMs" &&
                                !field.control->GetValue().Strip(wxString::both).empty() &&
                                !IsDigit(ToStd(field.control->GetValue().Strip(wxString::both))))
                            {
                                wxMessageBox("'maxExecutionTimeMs' must be a number.",
                                                "Edit Supervision", wxOK | wxICON_WARNING, &m_dialog);
                                field.control->SetFocus();
                                return;
                            } 
                        }
                    }
                    if (group.type == "alive" && m_types->IsChecked(TypeIndex("alive")))
                    {
                        for (const DetailField& field : group.fields)
                        {
                            if (field.name == "minIndications" &&
                                !field.control->GetValue().Strip(wxString::both).empty() &&
                                !IsInteger(ToStd(field.control->GetValue().Strip(wxString::both))))
                            {
                                wxMessageBox("'minIndications' must be of type uint32.",
                                                "Edit Supervision", wxOK | wxICON_WARNING, &m_dialog);
                                field.control->SetFocus();
                                return;
                            }
                            if (field.name == "maxIndications" &&
                                !field.control->GetValue().Strip(wxString::both).empty() &&
                                !IsInteger(ToStd(field.control->GetValue().Strip(wxString::both))))
                            {
                                wxMessageBox("'maxIndications' must be of type uint32.",
                                                "Edit Supervision", wxOK | wxICON_WARNING, &m_dialog);
                                field.control->SetFocus();
                                return;
                            }
                            if (field.name == "referenceCycleMs" &&
                                !field.control->GetValue().Strip(wxString::both).empty() &&
                                !IsDigit(ToStd(field.control->GetValue().Strip(wxString::both))))
                            {
                                wxMessageBox("'referenceCycleMs' must be a number.",
                                                "Edit Supervision", wxOK | wxICON_WARNING, &m_dialog);
                                field.control->SetFocus();
                                return;
                            }                            
                        }                        
                    }
                }
            }
            event.Skip();
        }, wxID_OK);
        UpdateVisibility();
    }

    YamlNodePtr SupervisionEditor::Build() const
    {
        YamlNodePtr supervision = YamlNode::MakeMap();
        const bool required = m_required->GetValue();
        supervision->Set(kSupervisionRequiredKey,
                         YamlNode::MakeScalar(required ? "true" : "false"));
        if (!required)
        {
            return supervision;
        }

        YamlNodePtr types = YamlNode::MakeSequence();
        for (unsigned int index = 0; index < m_types->GetCount(); ++index)
        {
            if (m_types->IsChecked(index))
            {
                types->Append(YamlNode::MakeScalar(ToStd(m_types->GetString(index))));
            }
        }
        supervision->Set("type", types);

        for (const DetailGroup& group : m_groups)
        {
            const unsigned int index = TypeIndex(group.type);
            if (!m_types->IsChecked(index))
            {
                continue;
            }

            YamlNodePtr values = YamlNode::MakeMap();
            for (const DetailField& field : group.fields)
            {
                const wxString text = field.control->GetValue().Strip(wxString::both);
                if (field.isList)
                {
                    YamlNodePtr list = YamlNode::MakeSequence();
                    for (const wxString& item : wxSplit(text, ','))
                    {
                        const wxString trimmed = item.Strip(wxString::both);
                        if (!trimmed.empty())
                        {
                            list->Append(YamlNode::MakeScalar(ToStd(trimmed)));
                        }
                    }
                    if (!list->GetSequence().empty())
                    {
                        values->Set(field.name, list);
                    }
                }
                else if (!text.empty() || field.name == "maxExecutionTimeMs")
                {
                    values->Set(field.name, YamlNode::MakeScalar(ToStd(text)));
                }
            }
            supervision->Set(group.type, values);
        }
        return supervision;
    }

    unsigned int SupervisionEditor::TypeIndex(const std::string& type) const
    {
        if (type == "deadline")
        {
            return 1;
        }
        return type == "logical" ? 2 : 0;
    }

    void SupervisionEditor::AddGroup(
        wxSizer* detailsSizer, const YamlNodePtr& existing, const std::string& type,
        const std::vector<std::pair<std::string, bool>>& fieldDefinitions)
    {
        wxPanel* panel = new wxPanel(m_details, wxID_ANY);
        wxStaticBoxSizer* groupSizer = new wxStaticBoxSizer(wxVERTICAL, panel, ToWx(type));
        wxFlexGridSizer* grid = new wxFlexGridSizer(2, 4, 8);
        grid->AddGrowableCol(1, 1);
        const YamlNodePtr existingGroup = existing ? existing->Find(type) : nullptr;
        DetailGroup group{type, panel, {}};
        for (const auto& definition : fieldDefinitions)
        {
            const std::string& name = definition.first;
            const YamlNodePtr value = existingGroup ? existingGroup->Find(name) : nullptr;
            wxString initial;
            if (value && value->IsScalar())
            {
                initial = ToWx(value->GetScalar());
            }
            else if (value && value->IsSequence())
            {
                for (const YamlNodePtr& item : value->GetSequence())
                {
                    if (item && item->IsScalar())
                    {
                        if (!initial.empty())
                        {
                            initial += ", ";
                        }
                        initial += ToWx(item->GetScalar());
                    }
                }
            }
            grid->Add(new wxStaticText(panel, wxID_ANY, ToWx(name)), 0,
                      wxALIGN_CENTER_VERTICAL);
            wxTextCtrl* control = new wxTextCtrl(panel, wxID_ANY, initial);
            grid->Add(control, 1, wxEXPAND);
            group.fields.push_back({name, definition.second, control});
        }
        groupSizer->Add(grid, 1, wxEXPAND | wxALL, 6);
        panel->SetSizer(groupSizer);
        detailsSizer->Add(panel, 0, wxEXPAND | wxBOTTOM, 6);
        m_groups.push_back(std::move(group));
    }

    void SupervisionEditor::UpdateVisibility()
    {
        const bool required = m_required->GetValue();
        m_details->Show(required);
        for (const DetailGroup& group : m_groups)
        {
            group.panel->Show(required && m_types->IsChecked(TypeIndex(group.type)));
        }
        m_dialog.Layout();
        m_dialog.Fit();
    }

    bool SupervisionEditor::IsInteger(const std::string& str) const
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
    
    bool SupervisionEditor::IsDigit(const std::string& str) const
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
} // namespace afd
