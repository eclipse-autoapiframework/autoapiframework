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

#include "json_signal_parser.h"

#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>

namespace afd 
{
    namespace 
    {
        wxString ToWx(const std::string& text) { return wxString::FromUTF8(text.c_str()); }
    } // namespace

    const JsonValue* JsonValue::Find(const std::string& key) const
    {
        if (type != Type::Object)
        {
            return nullptr;
        }
        for (const auto& entry : object)
        {
            if (entry.first == key)
            {
                return &entry.second;
            }
        }
        return nullptr;
    }

    bool JsonParser::Parse(const std::string& text, JsonValue& out, std::string& error)
    {
        m_text = &text;
        m_pos = 0;
        if (!ParseValue(out))
        {
            error = m_error.empty() ? "Invalid JSON" : m_error;
            return false;
        }
        return true;
    }

    char JsonParser::Peek() const { return m_pos < m_text->size() ? (*m_text)[m_pos] : '\0'; }
    char JsonParser::Get() { return m_pos < m_text->size() ? (*m_text)[m_pos++] : '\0'; }

    void JsonParser::SkipWhitespace()
    {
        while (m_pos < m_text->size() && std::isspace(static_cast<unsigned char>((*m_text)[m_pos])))
        {
            ++m_pos;
        }
    }

    bool JsonParser::ParseValue(JsonValue& out)
    {
        SkipWhitespace();
        const char c = Peek();
        if (c == '{') return ParseObject(out);
        if (c == '[') return ParseArray(out);
        if (c == '"') return ParseString(out);
        if (c == 't' || c == 'f') return ParseBool(out);
        if (c == 'n') return ParseNull(out);
        return ParseNumber(out);
    }

    bool JsonParser::ParseObject(JsonValue& out)
    {
        out.type = JsonValue::Type::Object;
        ++m_pos; // '{'
        SkipWhitespace();
        if (Peek() == '}')
        {
            ++m_pos;
            return true;
        }
        while (true)
        {
            SkipWhitespace();
            JsonValue key;
            if (Peek() != '"' || !ParseString(key))
            {
                m_error = "Expected string key";
                return false;
            }
            SkipWhitespace();
            if (Get() != ':')
            {
                m_error = "Expected ':'";
                return false;
            }
            JsonValue value;
            if (!ParseValue(value))
            {
                return false;
            }
            out.object.emplace_back(key.stringValue, std::move(value));
            SkipWhitespace();
            const char next = Get();
            if (next == ',') continue;
            if (next == '}') break;
            m_error = "Expected ',' or '}'";
            return false;
        }
        return true;
    }

    bool JsonParser::ParseArray(JsonValue& out)
    {
        out.type = JsonValue::Type::Array;
        ++m_pos; // '['
        SkipWhitespace();
        if (Peek() == ']')
        {
            ++m_pos;
            return true;
        }
        while (true)
        {
            JsonValue value;
            if (!ParseValue(value))
            {
                return false;
            }
            out.array.push_back(std::move(value));
            SkipWhitespace();
            const char next = Get();
            if (next == ',') continue;
            if (next == ']') break;
            m_error = "Expected ',' or ']'";
            return false;
        }
        return true;
    }

    bool JsonParser::ParseString(JsonValue& out)
    {
        if (Get() != '"')
        {
            m_error = "Expected '\"'";
            return false;
        }
        out.type = JsonValue::Type::String;
        std::string value;
        while (true)
        {
            if (m_pos >= m_text->size())
            {
                m_error = "Unterminated string";
                return false;
            }
            const char c = Get();
            if (c == '"') break;
            if (c == '\\')
            {
                const char escaped = Get();
                switch (escaped)
                {
                case '"': value += '"'; break;
                case '\\': value += '\\'; break;
                case '/': value += '/'; break;
                case 'n': value += '\n'; break;
                case 't': value += '\t'; break;
                case 'r': value += '\r'; break;
                case 'b': value += '\b'; break;
                case 'f': value += '\f'; break;
                case 'u': m_pos += 4; break; // unicode escapes are not converted
                default: value += escaped; break;
                }
            }
            else
            {
                value += c;
            }
        }
        out.stringValue = std::move(value);
        return true;
    }

    bool JsonParser::ParseNumber(JsonValue& out)
    {
        const std::size_t start = m_pos;
        if (Peek() == '-') ++m_pos;
        while (std::isdigit(static_cast<unsigned char>(Peek()))) ++m_pos;
        if (Peek() == '.')
        {
            ++m_pos;
            while (std::isdigit(static_cast<unsigned char>(Peek()))) ++m_pos;
        }
        if (Peek() == 'e' || Peek() == 'E')
        {
            ++m_pos;
            if (Peek() == '+' || Peek() == '-') ++m_pos;
            while (std::isdigit(static_cast<unsigned char>(Peek()))) ++m_pos;
        }
        if (m_pos == start)
        {
            m_error = "Invalid number";
            return false;
        }
        out.type = JsonValue::Type::Number;
        out.numberValue = std::atof(m_text->substr(start, m_pos - start).c_str());
        return true;
    }

    bool JsonParser::ParseBool(JsonValue& out)
    {
        if (m_text->compare(m_pos, 4, "true") == 0)
        {
            out.type = JsonValue::Type::Bool;
            out.boolValue = true;
            m_pos += 4;
            return true;
        }
        if (m_text->compare(m_pos, 5, "false") == 0)
        {
            out.type = JsonValue::Type::Bool;
            out.boolValue = false;
            m_pos += 5;
            return true;
        }
        m_error = "Invalid literal";
        return false;
    }

    bool JsonParser::ParseNull(JsonValue& out)
    {
        if (m_text->compare(m_pos, 4, "null") == 0)
        {
            out.type = JsonValue::Type::Null;
            m_pos += 4;
            return true;
        }
        m_error = "Invalid literal";
        return false;
    }

    void FlattenVssTree(const JsonValue& node, const std::string& prefix, std::vector<VssSignal>& out)
    {
        if (node.type != JsonValue::Type::Object)
        {
            return;
        }
        for (const auto& entry : node.object)
        {
            const JsonValue& value = entry.second;
            if (value.type != JsonValue::Type::Object)
            {
                continue;
            }
            const std::string path = prefix.empty() ? entry.first : prefix + "." + entry.first;
            const JsonValue* typeNode = value.Find("type");
            const std::string typeStr = typeNode && typeNode->type == JsonValue::Type::String
                ? typeNode->stringValue : std::string();
            const JsonValue* children = value.Find("children");

            if (typeStr == "branch" || (children && children->type == JsonValue::Type::Object))
            {
                if (children)
                {
                    FlattenVssTree(*children, path, out);
                }
                continue;
            }

            VssSignal signal;
            signal.path = path;
            signal.type = typeStr;
            if (const JsonValue* type = value.Find(kCovesaType); type && type->type == JsonValue::Type::String)
            {
                signal.type = type->stringValue;
            }            
            if (const JsonValue* dataType = value.Find(kCovesaDatatype); dataType && dataType->type == JsonValue::Type::String)
            {
                signal.dataType = dataType->stringValue;
            }
            if (const JsonValue* description = value.Find(kCovesaDescription); description && description->type == JsonValue::Type::String)
            {
                signal.description = description->stringValue;
            }
            if (const JsonValue* unit = value.Find(kCovesaUnit); unit && unit->type == JsonValue::Type::String)
            {
                signal.unit = unit->stringValue;
            }
            if (const JsonValue* comment = value.Find(kCovesaComment); comment && comment->type == JsonValue::Type::String)
            {
                signal.comment = comment->stringValue;
            }
            if (const JsonValue* min = value.Find(kCovesaMin); min && min->type == JsonValue::Type::String)
            {
                signal.min = min->stringValue;
            }
            if (const JsonValue* max = value.Find(kCovesaMax); max && max->type == JsonValue::Type::String)
            {
                signal.max = max->stringValue;
            }
            if (const JsonValue* allowed = value.Find(kCovesaAllowed); allowed && allowed->type == JsonValue::Type::String)
            {
                signal.allowed = allowed->stringValue;
            }
            if (const JsonValue* defaultValue = value.Find(kCovesaDefault); defaultValue && defaultValue->type == JsonValue::Type::String)
            {
                signal.defaultValue = defaultValue->stringValue;
            }
            if (const JsonValue* uuid = value.Find(kCovesaUuid); uuid && uuid->type == JsonValue::Type::String)
            {
                signal.uuid = uuid->stringValue;
            }
            if (const JsonValue* arraySize = value.Find(kCovesaArraySize); arraySize && arraySize->type == JsonValue::Type::String)
            {
                signal.arraySize = arraySize->stringValue;
            }            out.push_back(std::move(signal));
        }
    }

    SelectSignalsDialog::SelectSignalsDialog(wxWindow* parent, const std::vector<VssSignal>& signals)
        : wxDialog(parent, wxID_ANY, "Select signals", wxDefaultPosition, wxSize(1020, 980),
                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
        , m_available(signals)
    {
        wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);

        wxBoxSizer* filterSizer = new wxBoxSizer(wxHORIZONTAL);
        filterSizer->Add(new wxStaticText(this, wxID_ANY, "Filter:"), 0,
                          wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
        m_filterText = new wxTextCtrl(this, wxID_ANY);
        filterSizer->Add(m_filterText, 1);

        filterSizer->Add(new wxStaticText(this, wxID_ANY, "Type:"), 0,
                          wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 6);
        const wxArrayString typeChoices = {"all", "actuator", "sensor", "attribute", "property", "branch"};
        m_typeFilter = new wxComboBox(this, wxID_ANY, typeChoices[0], wxDefaultPosition, wxDefaultSize,
                                      typeChoices, wxCB_READONLY);
        filterSizer->Add(m_typeFilter, 0);

        filterSizer->Add(new wxStaticText(this, wxID_ANY, "Datatype:"), 0,
                          wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 6);
        const wxArrayString datatypeChoices = {"all", "boolean", "float", "string", "int8", "int16",
                                               "int32","int64", "uint8", "uint16", "uint32", "uint64"};
        m_datatypeFilter = new wxComboBox(this, wxID_ANY, datatypeChoices[0], wxDefaultPosition,
                                          wxDefaultSize, datatypeChoices, wxCB_READONLY);
        filterSizer->Add(m_datatypeFilter, 0);
        topSizer->Add(filterSizer, 0, wxEXPAND | wxALL, 8);

        topSizer->Add(new wxStaticText(this, wxID_ANY, "Available signals"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
        m_availableList = CreateSignalList();
        m_availableList->SetMinSize(wxSize(-1, kSignalListHeight));
        topSizer->Add(m_availableList, 1, wxEXPAND | wxALL, 8);

        wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
        wxButton* addButton = new wxButton(this, wxID_ANY, "Add ->");
        wxButton* addAllButton = new wxButton(this, wxID_ANY, "Add all ->");
        wxButton* removeButton = new wxButton(this, wxID_ANY, "<- Remove");
        wxButton* removeAllButton = new wxButton(this, wxID_ANY, "<- Remove all");
        buttonSizer->AddStretchSpacer();
        buttonSizer->Add(addButton, 0, wxRIGHT, 4);
        buttonSizer->Add(addAllButton, 0, wxRIGHT, 16);
        buttonSizer->Add(removeButton, 0, wxRIGHT, 4);
        buttonSizer->Add(removeAllButton, 0);
        buttonSizer->AddStretchSpacer();
        topSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 4);

        topSizer->Add(new wxStaticText(this, wxID_ANY, "Selected signals"), 0, wxLEFT | wxRIGHT, 8);
        m_selectedList = CreateSignalList();
        m_selectedList->SetMinSize(wxSize(-1, kSignalListHeight));
        topSizer->Add(m_selectedList, 1, wxEXPAND | wxALL, 8);

        topSizer->Add(CreateSeparatedButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 8);
        SetSizer(topSizer);
        CentreOnParent();

        m_filterText->Bind(wxEVT_TEXT, [this](wxCommandEvent&) { RefreshAvailableList(); });
        m_typeFilter->Bind(wxEVT_COMBOBOX, [this](wxCommandEvent&) { RefreshAvailableList(); });
        m_datatypeFilter->Bind(wxEVT_COMBOBOX, [this](wxCommandEvent&) { RefreshAvailableList(); });
        addButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&)
        {
            MoveSelected(m_availableList, m_available, m_selected);
            RefreshBothLists();
        });
        removeButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&)
        {
            MoveSelected(m_selectedList, m_selected, m_available);
            RefreshBothLists();
        });
        addAllButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&)
        {
            MoveAllVisible();
            RefreshBothLists();
        });
        removeAllButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&)
        {
            for (VssSignal& signal : m_selected)
            {
                m_available.push_back(std::move(signal));
            }
            m_selected.clear();
            RefreshBothLists();
        });
        m_availableList->Bind(wxEVT_LIST_ITEM_ACTIVATED, [this](wxListEvent&)
        {
            MoveSelected(m_availableList, m_available, m_selected);
            RefreshBothLists();
        });
        m_selectedList->Bind(wxEVT_LIST_ITEM_ACTIVATED, [this](wxListEvent&)
        {
            MoveSelected(m_selectedList, m_selected, m_available);
            RefreshBothLists();
        });

        RefreshBothLists();
    }

    wxListCtrl* SelectSignalsDialog::CreateSignalList()
    {
        wxListCtrl* list = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                          wxLC_REPORT | wxLC_HRULES | wxLC_VRULES);
        list->AppendColumn("Signal", wxLIST_FORMAT_LEFT, 420);
        list->AppendColumn("Type", wxLIST_FORMAT_LEFT, 80);
        list->AppendColumn("Datatype", wxLIST_FORMAT_LEFT, 80);
        list->AppendColumn("Unit", wxLIST_FORMAT_LEFT, 80);
        list->AppendColumn("Description", wxLIST_FORMAT_LEFT, 300);
        list->AppendColumn("Comment", wxLIST_FORMAT_LEFT, 200);
        list->AppendColumn("Min", wxLIST_FORMAT_LEFT, 80);
        list->AppendColumn("Max", wxLIST_FORMAT_LEFT, 80);
        list->AppendColumn("Allowed", wxLIST_FORMAT_LEFT, 80);
        list->AppendColumn("Default", wxLIST_FORMAT_LEFT, 80);
        list->AppendColumn("UUID", wxLIST_FORMAT_LEFT, 200);
        list->AppendColumn("Array Size", wxLIST_FORMAT_LEFT, 80);
        return list;
    }

    void SelectSignalsDialog::FillRow(wxListCtrl* list, long row, const VssSignal& signal)
    {
        list->InsertItem(row, ToWx(signal.path));
        list->SetItem(row, 1, ToWx(signal.type));
        list->SetItem(row, 2, ToWx(signal.dataType));
        list->SetItem(row, 3, ToWx(signal.unit));
        list->SetItem(row, 4, ToWx(signal.description));
        list->SetItem(row, 5, ToWx(signal.comment));
        list->SetItem(row, 6, ToWx(signal.min));
        list->SetItem(row, 7, ToWx(signal.max));
        list->SetItem(row, 8, ToWx(signal.allowed));
        list->SetItem(row, 9, ToWx(signal.defaultValue));
        list->SetItem(row, 10, ToWx(signal.uuid));
        list->SetItem(row, 11, ToWx(signal.arraySize));
    }

    void SelectSignalsDialog::RefreshAvailableList()
    {
        const wxString filter = m_filterText->GetValue().Lower();
        const wxString typeFilter = m_typeFilter->GetValue();
        const wxString datatypeFilter = m_datatypeFilter->GetValue();
        m_availableDisplayIndex.clear();
        m_availableList->DeleteAllItems();
        long row = 0;
        for (std::size_t i = 0; i < m_available.size(); ++i)
        {
            const VssSignal& signal = m_available[i];
            if (!filter.empty() && ToWx(signal.path).Lower().Find(filter) == wxNOT_FOUND)
            {
                continue;
            }
            if (typeFilter != "all" && !typeFilter.IsSameAs(ToWx(signal.type), false))
            {
                continue;
            }
            if (datatypeFilter != "all" && !datatypeFilter.IsSameAs(ToWx(signal.dataType), false))
            {
                continue;
            }
            FillRow(m_availableList, row, signal);
            m_availableDisplayIndex.push_back(i);
            ++row;
        }
    }

    void SelectSignalsDialog::RefreshSelectedList()
    {
        m_selectedList->DeleteAllItems();
        long row = 0;
        for (const VssSignal& signal : m_selected)
        {
            FillRow(m_selectedList, row, signal);
            ++row;
        }
    }

    void SelectSignalsDialog::RefreshBothLists()
    {
        RefreshAvailableList();
        RefreshSelectedList();
    }

    std::vector<long> SelectSignalsDialog::GetSelectedRows(wxListCtrl* list)
    {
        std::vector<long> rows;
        for (long row = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED); row != -1;
             row = list->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED))
        {
            rows.push_back(row);
        }
        return rows;
    }

    void SelectSignalsDialog::MoveSelected(wxListCtrl* list, std::vector<VssSignal>& source,
                                           std::vector<VssSignal>& destination)
    {
        const std::vector<long> rows = GetSelectedRows(list);
        if (rows.empty())
        {
            return;
        }

        std::vector<std::size_t> sourceIndices;
        sourceIndices.reserve(rows.size());
        for (long row : rows)
        {
            const std::size_t sourceIndex = (list == m_availableList)
                ? m_availableDisplayIndex[static_cast<std::size_t>(row)]
                : static_cast<std::size_t>(row);
            sourceIndices.push_back(sourceIndex);
        }
        std::sort(sourceIndices.begin(), sourceIndices.end());

        for (auto it = sourceIndices.rbegin(); it != sourceIndices.rend(); ++it)
        {
            destination.push_back(source[*it]);
            source.erase(source.begin() + *it);
        }
    }

    void SelectSignalsDialog::MoveAllVisible()
    {
        std::vector<std::size_t> indices = m_availableDisplayIndex;
        std::sort(indices.begin(), indices.end());
        for (auto it = indices.rbegin(); it != indices.rend(); ++it)
        {
            m_selected.push_back(m_available[*it]);
            m_available.erase(m_available.begin() + *it);
        }
    }

} // namespace afd
