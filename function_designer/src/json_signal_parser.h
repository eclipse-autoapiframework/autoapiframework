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

#ifndef AFD_JSON_SIGNAL_PARSER_H
#define AFD_JSON_SIGNAL_PARSER_H

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include <wx/dialog.h>

class wxComboBox;
class wxListCtrl;
class wxTextCtrl;
class wxWindow;

namespace afd 
{
    /// @brief Expected strings within the VSS JSON file and the corresponding keys in
    /// the autoapiframework function specification file if they do not match.        
    static constexpr const char* kCovesaPath = "path";
    static constexpr const char* kCovesaType = "type";
    static constexpr const char* kMetaModelDataType = "dataType";
    static constexpr const char* kCovesaDatatype = "datatype";
    static constexpr const char* kCovesaDescription = "description";
    static constexpr const char* kCovesaUnit = "unit";
    static constexpr const char* kCovesaComment = "comment";
    static constexpr const char* kCovesaMin = "min";
    static constexpr const char* kCovesaMax = "max";
    static constexpr const char* kCovesaAllowed = "allowed";
    static constexpr const char* kMetaModelDefaultValue = "defaultValue";
    static constexpr const char* kCovesaDefault = "default";    
    static constexpr const char* kCovesaUuid = "uuid";
    static constexpr const char* kCovesaArraySize = "arraysize";

    /// Minimal JSON value as produced by the `vspec export json` output.
    struct JsonValue
    {
        enum class Type { Null, Bool, Number, String, Object, Array };

        Type type = Type::Null;
        bool boolValue = false;
        double numberValue = 0.0;
        std::string stringValue;
        std::vector<std::pair<std::string, JsonValue>> object;
        std::vector<JsonValue> array;

        const JsonValue* Find(const std::string& key) const;
    };

    /// Recursive descent parser for the JSON subset needed to read `vspec export json` output.
    class JsonParser
    {
    public:
        bool Parse(const std::string& text, JsonValue& out, std::string& error);

    private:
        char Peek() const;
        char Get();
        void SkipWhitespace();
        bool ParseValue(JsonValue& out);
        bool ParseObject(JsonValue& out);
        bool ParseArray(JsonValue& out);
        bool ParseString(JsonValue& out);
        bool ParseNumber(JsonValue& out);
        bool ParseBool(JsonValue& out);
        bool ParseNull(JsonValue& out);

        const std::string* m_text = nullptr;
        std::size_t m_pos = 0;
        std::string m_error;
    };

    /// Flattened leaf entry of a `vspec export json` signal tree.
    struct VssSignal
    {
        std::string path;
        std::string type;
        std::string dataType;
        std::string description;
        std::string unit;
        std::string comment;
        std::string min;
        std::string max;
        std::string allowed;
        std::string defaultValue;
        std::string uuid;
        std::string arraySize;
    };

    /// Recursively collects leaf signals (non-branch nodes) with their dotted path.
    void FlattenVssTree(const JsonValue& node, const std::string& prefix, std::vector<VssSignal>& out);

    /// Lets the user filter a flat VSS signal list and move entries into a selected box.
    class SelectSignalsDialog : public wxDialog
    {
    public:
        SelectSignalsDialog(wxWindow* parent, const std::vector<VssSignal>& signals);

        std::vector<VssSignal> GetSelectedSignals() const { return m_selected; }

    private:
        /// Row height driven size that fits roughly kVisibleSignalRows entries without scrolling.
        static constexpr int kVisibleSignalRows = 22;
        static constexpr int kSignalListHeight = kVisibleSignalRows * 18 + 24;

        wxListCtrl* CreateSignalList();
        static void FillRow(wxListCtrl* list, long row, const VssSignal& signal);
        void RefreshAvailableList();
        void RefreshSelectedList();
        void RefreshBothLists();
        static std::vector<long> GetSelectedRows(wxListCtrl* list);
        void MoveSelected(wxListCtrl* list, std::vector<VssSignal>& source, std::vector<VssSignal>& destination);
        void MoveAllVisible();

        wxTextCtrl* m_filterText = nullptr;
        wxComboBox* m_typeFilter = nullptr;
        wxComboBox* m_datatypeFilter = nullptr;
        wxListCtrl* m_availableList = nullptr;
        wxListCtrl* m_selectedList = nullptr;
        std::vector<VssSignal> m_available;
        std::vector<VssSignal> m_selected;
        std::vector<std::size_t> m_availableDisplayIndex;
    };

} // namespace afd

#endif // AFD_JSON_SIGNAL_PARSER_H
