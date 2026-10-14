/*******************************************
*************************************
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
 
#include "yaml_writer.h"

#include <fstream>
#include <sstream>

namespace afd 
{
    namespace 
    {
        std::string Indent(int indent) { return std::string(static_cast<std::size_t>(indent), ' '); }

        bool NeedsQuotes(const std::string& value) 
        {
            if (value.empty()) 
            {
                return true;
            }
            static const std::string leading = "?:,[]{}#&*!|>'\"%@`";
            if (leading.find(value.front()) != std::string::npos) 
            {
                return true;
            }
            // A leading '-' only starts a sequence entry when followed by a space.
            if (value.front() == '-' && (value.size() == 1 || value[1] == ' ')) 
            {
                return true;
            }
            if (value.front() == ' ' || value.back() == ' ') 
            {
                return true;
            }
            if (value.find(": ") != std::string::npos || value.back() == ':') 
            {
                return true;
            }
            if (value.find(" #") != std::string::npos) 
            {
                return true;
            }
            return value.find('\n') != std::string::npos;
        }

        std::string DoubleQuoted(const std::string& value) 
        {
            std::string out = "\"";
            for (const char c : value) 
            {
                switch (c) 
                {
                    case '"': out += "\\\""; break;
                    case '\\': out += "\\\\"; break;
                    case '\n': out += "\\n"; break;
                    case '\t': out += "\\t"; break;
                    default: out += c; break;
                }
            }
            out += '"';
            return out;
        }

        std::string FormatScalar(const YamlNodePtr& node) 
        {
            const std::string& value = node->GetScalar();
            switch (node->GetStyle()) 
            {
                case ScalarStyle::DoubleQuote:
                    return DoubleQuoted(value);
                case ScalarStyle::SingleQuote: 
                {
                    std::string out = "'";
                    for (const char c : value) 
                    {
                        out += c;
                        if (c == '\'') 
                        {
                            out += '\'';
                        }
                    }
                    out += '\'';
                    return out;
                }
                default:
                    return NeedsQuotes(value) ? DoubleQuoted(value) : value;
            }
        }

        std::vector<std::string> SplitWords(const std::string& text) 
        {
            std::vector<std::string> words;
            std::istringstream stream(text);
            std::string word;
            while (stream >> word) 
            {
                words.push_back(word);
            }
            return words;
        }

    } // namespace

    void YamlWriter::EmitBlockScalar(const YamlNodePtr& node, int indent, std::string& out) const 
    {
        const std::string prefix = Indent(indent);
        if (node->GetStyle() == ScalarStyle::Literal) 
        {
            std::istringstream stream(node->GetScalar());
            std::string line;
            while (std::getline(stream, line)) 
            {
                out += prefix + line + "\n";
            }
            return;
        }

        // Folded: re-wrap the paragraph, keeping explicit line breaks.
        std::istringstream paragraphs(node->GetScalar());
        std::string paragraph;
        bool first = true;
        while (std::getline(paragraphs, paragraph)) 
        {
            if (!first) 
            {
                out += "\n";
            }
            first = false;
            std::string line;
            for (const std::string& word : SplitWords(paragraph)) 
            {
                if (line.empty()) 
                {
                    line = word;
                } 
                else if (static_cast<int>(prefix.size() + line.size() + 1 + word.size()) <= m_wrapColumn) 
                {
                    line += " " + word;
                } 
                else 
                {
                    out += prefix + line + "\n";
                    line = word;
                }
            }
            if (!line.empty()) 
            {
                out += prefix + line + "\n";
            }
        }
    }

    void YamlWriter::EmitMap(const YamlNodePtr& node, int indent, std::string& out, bool inSequenceItem) const 
    {
        bool firstKey = true;
        for (const auto& entry : node->GetMap()) 
        {
            const std::string key = NeedsQuotes(entry.first) ? DoubleQuoted(entry.first) : entry.first;
            const YamlNodePtr& value = entry.second;
            // Block scalars and collections are separated by a blank line, as in
            // the reference function specifications.
            const bool blockScalar = value && value->IsScalar() &&
                                    (value->GetStyle() == ScalarStyle::Folded ||
                                    value->GetStyle() == ScalarStyle::Literal);
            const bool sequence = value && value->IsSequence() && !value->GetSequence().empty();
            if (!firstKey && (sequence || (blockScalar && !inSequenceItem))) 
            {
                out += "\n";
            }
            firstKey = false;
            if (!value) 
            {
                out += Indent(indent) + key + ":\n";
                continue;
            }
            if (value->IsScalar()) 
            {
                const ScalarStyle style = value->GetStyle();
                if (style == ScalarStyle::Folded || style == ScalarStyle::Literal) 
                {
                    out += Indent(indent) + key + (style == ScalarStyle::Folded ? ": >-\n" : ": |-\n");
                    EmitBlockScalar(value, indent + 2, out);
                } 
                else if (value->GetScalar().empty()) 
                {
                    out += Indent(indent) + key + ":\n";
                } 
                else 
                {
                    out += Indent(indent) + key + ": " + FormatScalar(value) + "\n";
                }
                continue;
            }
            if ((value->IsMap() && value->GetMap().empty()) ||
                (value->IsSequence() && value->GetSequence().empty())) 
            {
                out += Indent(indent) + key + (value->IsMap() ? ": {}\n" : ": []\n");
                continue;
            }
            out += Indent(indent) + key + ":\n";
            EmitNode(value, indent + 2, out);
        }
    }

    void YamlWriter::EmitSequence(const YamlNodePtr& node, int indent, std::string& out) const 
    {
        bool first = true;
        for (const YamlNodePtr& item : node->GetSequence()) 
        {
            if (!item) 
            {
                continue;
            }
            if (item->IsScalar()) 
            {
                out += Indent(indent) + "- " + FormatScalar(item) + "\n";
                first = false;
                continue;
            }
            if (!first) 
            {
                out += "\n"; // blank line between structured items, as in the examples
            }
            first = false;
            std::string block;
            EmitNode(item, indent + 2, block, true);
            if (block.size() > static_cast<std::size_t>(indent)) 
            {
                block[static_cast<std::size_t>(indent)] = '-';
            }
            out += block;
        }
    }

    void YamlWriter::EmitNode(const YamlNodePtr& node, int indent, std::string& out, bool inSequenceItem) const 
    {
        if (!node) 
        {
            return;
        }
        if (node->IsMap()) 
        {
            EmitMap(node, indent, out, inSequenceItem);
        } 
        else if (node->IsSequence()) 
        {
            EmitSequence(node, indent, out);
        } 
        else 
        {
            out += Indent(indent) + FormatScalar(node) + "\n";
        }
    }

    std::string YamlWriter::WriteText(const YamlNodePtr& root) const 
    {
        std::string out;
        EmitNode(root, 0, out);
        return out;
    }

    bool YamlWriter::WriteFile(const std::string& path, const YamlNodePtr& root, std::string& error) const 
    {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (!file) 
        {
            error = "cannot write file: " + path;
            return false;
        }
        file << WriteText(root);
        if (!file) {
            error = "error while writing file: " + path;
            return false;
        }
        return true;
    }

} // namespace afd
