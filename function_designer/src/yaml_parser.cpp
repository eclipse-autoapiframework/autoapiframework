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

#include "yaml_parser.h"

#include <fstream>
#include <sstream>

namespace afd 
{
    namespace 
    {
        constexpr int kNoIndent = -1;

        std::string Trim(const std::string& text) 
        {
            const std::string spaces = " \t\r\n";
            const std::size_t begin = text.find_first_not_of(spaces);
            if (begin == std::string::npos) 
            {
                return std::string();
            }
            const std::size_t end = text.find_last_not_of(spaces);
            return text.substr(begin, end - begin + 1);
        }

        int IndentOf(const std::string& line) 
        {
            int indent = 0;
            while (indent < static_cast<int>(line.size()) && line[static_cast<std::size_t>(indent)] == ' ') 
            {
                ++indent;
            }
            return indent;
        }

        /// Extracts a trailing `# comment` that is not part of a quoted scalar.
        /// Returns a pair of (content_without_comment, comment_text).
        /// If no comment is found, the comment text will be empty.
        std::pair<std::string, std::string> ExtractComment(const std::string& text) 
        {
            char quote = '\0';
            for (std::size_t i = 0; i < text.size(); ++i) 
            {
                const char c = text[i];
                if (quote != '\0') 
                {
                    if (c == quote) 
                    {
                        quote = '\0';
                    }
                } 
                else if (c == '"' || c == '\'') 
                {
                    quote = c;
                } 
                else if (c == '#' && (i == 0 || text[i - 1] == ' ' || text[i - 1] == '\t')) 
                {
                    // Found a comment. Trim trailing spaces from content.
                    std::string content = text.substr(0, i);
                    const std::string spaces = " \t";
                    const std::size_t end = content.find_last_not_of(spaces);
                    if (end != std::string::npos) 
                    {
                        content = content.substr(0, end + 1);
                    } else 
                    {
                        content = std::string();
                    }
                    return std::make_pair(content, text.substr(i));
                }
            }
            return std::make_pair(text, std::string());
        }

        /// Finds the `:` that separates a mapping key from its value, ignoring colons
        /// inside quotes and colons that are not followed by a space or end of line.
        std::size_t FindKeySeparator(const std::string& text) 
        {
            char quote = '\0';
            for (std::size_t i = 0; i < text.size(); ++i) 
            {
                const char c = text[i];
                if (quote != '\0') 
                {
                    if (c == quote) {
                        quote = '\0';
                    }
                } 
                else if (c == '"' || c == '\'') 
                {
                    quote = c;
                } 
                else if (c == '#' && i > 0 && text[i - 1] == ' ') 
                {
                    return std::string::npos;
                } 
                else if (c == ':' && (i + 1 == text.size() || text[i + 1] == ' ')) 
                {
                    return i;
                }
            }
            return std::string::npos;
        }

        std::string Unquote(const std::string& text, ScalarStyle& style) 
        {
            style = ScalarStyle::Plain;
            if (text.size() >= 2 && text.front() == '"' && text.back() == '"') 
            {
                style = ScalarStyle::DoubleQuote;
                std::string out;
                for (std::size_t i = 1; i + 1 < text.size(); ++i) 
                {
                    if (text[i] == '\\' && i + 2 < text.size()) 
                    {
                        const char next = text[i + 1];
                        switch (next) 
                        {
                            case 'n': out += '\n'; break;
                            case 't': out += '\t'; break;
                            case '"': out += '"'; break;
                            case '\\': out += '\\'; break;
                            default: out += next; break;
                        }
                        ++i;
                    } else 
                    {
                        out += text[i];
                    }
                }
                return out;
            }
            if (text.size() >= 2 && text.front() == '\'' && text.back() == '\'') 
            {
                style = ScalarStyle::SingleQuote;
                std::string out;
                for (std::size_t i = 1; i + 1 < text.size(); ++i) 
                {
                    out += text[i];
                    if (text[i] == '\'' && i + 2 < text.size() && text[i + 1] == '\'') 
                    {
                        ++i;
                    }
                }
                return out;
            }
            return text;
        }

        class Reader 
        {
        public:
            explicit Reader(const std::string& text) 
            {
                std::istringstream stream(text);
                std::string line;
                while (std::getline(stream, line)) 
                {
                    if (!line.empty() && line.back() == '\r') 
                    {
                        line.pop_back();
                    }
                    m_lines.push_back(line);
                }
            }

            YamlNodePtr Parse(std::string& error) 
            {
                m_error.clear();
                m_leadingComments.clear();
                SkipBlankLines();
                CollectLeadingComments();
                std::vector<std::string> documentComments;
                documentComments.swap(m_leadingComments);
                if (AtEnd()) 
                {
                    YamlNodePtr root = YamlNode::MakeMap();
                    for (const auto& comment : documentComments)
                    {
                        root->AddLeadingComment(comment);
                    }
                    return root;
                }
                YamlNodePtr root = ParseBlock(IndentOf(m_lines[m_index]));
                if (!m_error.empty()) 
                {
                    error = m_error;
                    return nullptr;
                }
                for (const auto& comment : documentComments)
                {
                    root->AddLeadingComment(comment);
                }
                return root;
            }

        private:
            std::vector<std::string> m_lines;
            std::size_t m_index = 0;
            std::string m_error;
            std::vector<std::string> m_leadingComments;  // Comments before current node

            bool AtEnd() const { return m_index >= m_lines.size(); }

            void SkipBlankLines() 
            {
                while (!AtEnd() && Trim(m_lines[m_index]).empty()) 
                {
                    ++m_index;
                }
            }

            void CollectLeadingComments() 
            {
                std::vector<std::string> newComments;
                while (!AtEnd()) 
                {
                    const std::string trimmed = Trim(m_lines[m_index]);
                    if (trimmed.empty()) 
                    {
                        ++m_index;
                    } 
                    else if (trimmed[0] == '#') 
                    {
                        newComments.push_back(trimmed);
                        ++m_index;
                    } 
                    else 
                    {
                        break;
                    }
                }
                // Append new comments to existing ones
                m_leadingComments.insert(m_leadingComments.end(), newComments.begin(), newComments.end());
            }

            void SkipIgnorable() 
            {
                SkipBlankLines();
                CollectLeadingComments();
            }

            /// Indent of the next content line, or kNoIndent at end of input.
            int PeekIndent() 
            {
                const std::size_t saved = m_index;
                SkipBlankLines();
                const int indent = AtEnd() ? kNoIndent : IndentOf(m_lines[m_index]);
                m_index = saved;
                return indent;
            }

            void Fail(const std::string& message) 
            {
                if (m_error.empty()) 
                {
                    m_error = "line " + std::to_string(m_index + 1) + ": " + message;
                }
                m_index = m_lines.size();
            }

            YamlNodePtr ParseBlock(int indent) 
            {
                SkipIgnorable();
                if (AtEnd()) 
                {
                    return YamlNode::MakeScalar(std::string());
                }
                const std::string trimmed = Trim(m_lines[m_index]);
                if (trimmed == "-" || trimmed.rfind("- ", 0) == 0) 
                {
                    return ParseSequence(indent);
                }
                return ParseMapping(indent);
            }

            YamlNodePtr ParseMapping(int indent) 
            {
                YamlNodePtr map = YamlNode::MakeMap();
                while (m_error.empty()) 
                {
                    SkipIgnorable();
                    if (AtEnd()) 
                    {
                        break;
                    }
                    const int lineIndent = IndentOf(m_lines[m_index]);
                    if (lineIndent < indent) 
                    {
                        break;
                    }
                    if (lineIndent > indent) 
                    {
                        Fail("unexpected indentation");
                        break;
                    }
                    const std::string content = Trim(m_lines[m_index]);
                    if (content == "-" || content.rfind("- ", 0) == 0) 
                    {
                        break;
                    }
                    if (content == "---" || content == "...") {
                        Fail("multiple YAML documents are not supported");
                        break;
                    }
                    if (content.front() == '{' || content.front() == '[') 
                    {
                        Fail("flow collections are not supported");
                        break;
                    }

                    const std::size_t separator = FindKeySeparator(content);
                    if (separator == std::string::npos) 
                    {
                        Fail("expected 'key: value'");
                        break;
                    }
                    ScalarStyle keyStyle = ScalarStyle::Plain;
                    const std::string key = Unquote(Trim(content.substr(0, separator)), keyStyle);
                    
                    // Extract inline comment from the value part
                    const auto& [valuePart, inlineComment] = ExtractComment(content.substr(separator + 1));
                    const std::string rest = Trim(valuePart);
                    ++m_index;

                    YamlNodePtr value = ParseValue(indent, rest);
                    
                    // Add leading comments to the value node
                    for (const auto& comment : m_leadingComments) 
                    {
                        value->AddLeadingComment(comment);
                    }
                    m_leadingComments.clear();
                    
                    // Add inline comment to the value node
                    if (!inlineComment.empty()) 
                    {
                        value->SetInlineComment(inlineComment);
                    }
                    
                    map->Set(key, value);
                }
                return map;
            }

            YamlNodePtr ParseSequence(int indent) 
            {
                YamlNodePtr sequence = YamlNode::MakeSequence();
                while (m_error.empty()) 
                {
                    SkipIgnorable();
                    if (AtEnd()) 
                    {
                        break;
                    }
                    const int lineIndent = IndentOf(m_lines[m_index]);
                    const std::string content = Trim(m_lines[m_index]);
                    if (lineIndent != indent || (content != "-" && content.rfind("- ", 0) != 0)) 
                    {
                        break;
                    }

                    const std::string item = Trim(content.substr(1));
                    if (item == "{}" || item == "[]")
                    {
                        ++m_index;
                        YamlNodePtr node = item == "{}" ? YamlNode::MakeMap() : YamlNode::MakeSequence();
                        for (const auto& comment : m_leadingComments) 
                        {
                            node->AddLeadingComment(comment);
                        }
                        m_leadingComments.clear();
                        sequence->Append(node);
                        continue;
                    }
                    if (!item.empty() && (item.front() == '{' || item.front() == '['))
                    {
                        Fail("flow collections are not supported");
                        break;
                    }
                    if (item.empty()) 
                    {
                        ++m_index;
                        const int childIndent = PeekIndent();
                        YamlNodePtr node;
                        if (childIndent == kNoIndent || childIndent <= indent) 
                        {
                            node = YamlNode::MakeScalar(std::string());
                        } 
                        else 
                        {
                            node = ParseBlock(childIndent);
                        }
                        for (const auto& comment : m_leadingComments) 
                        {
                            node->AddLeadingComment(comment);
                        }
                        m_leadingComments.clear();
                        sequence->Append(node);
                        continue;
                    }

                    if (FindKeySeparator(item) != std::string::npos) 
                    {
                        // Re-align `- key: value` to a plain mapping line so that the
                        // remaining keys of the item are parsed with the same indent.
                        const int itemIndent = indent + 2;
                        m_lines[m_index] = std::string(static_cast<std::size_t>(itemIndent), ' ') + item;
                        YamlNodePtr node = ParseMapping(itemIndent);
                        for (const auto& comment : m_leadingComments) 
                        {
                            node->AddLeadingComment(comment);
                        }
                        m_leadingComments.clear();
                        sequence->Append(node);
                        continue;
                    }

                    ++m_index;
                    // Extract inline comment from sequence item
                    const auto& [itemContent, inlineComment] = ExtractComment(item);
                    ScalarStyle style = ScalarStyle::Plain;
                    const std::string value = Unquote(Trim(itemContent), style);
                    YamlNodePtr node = YamlNode::MakeScalar(value, style);
                    
                    // Add leading comments
                    for (const auto& comment : m_leadingComments) 
                    {
                        node->AddLeadingComment(comment);
                    }
                    m_leadingComments.clear();
                    
                    // Add inline comment
                    if (!inlineComment.empty()) 
                    {
                        node->SetInlineComment(inlineComment);
                    }
                    
                    sequence->Append(node);
                }
                return sequence;
            }

            YamlNodePtr ParseValue(int keyIndent, const std::string& rest) 
            {
                if (rest == "{}")
                {
                    return YamlNode::MakeMap();
                }
                if (rest == "[]")
                {
                    return YamlNode::MakeSequence();
                }
                if (!rest.empty() && (rest[0] == '>' || rest[0] == '|')) 
                {
                    return ParseBlockScalar(keyIndent, rest);
                }
                if (!rest.empty()) 
                {
                    ScalarStyle style = ScalarStyle::Plain;
                    const std::string value = Unquote(rest, style);
                    return YamlNode::MakeScalar(value, style);
                }
                const int childIndent = PeekIndent();
                if (childIndent == kNoIndent || childIndent < keyIndent) 
                {
                    return YamlNode::MakeScalar(std::string());
                }
                if (childIndent == keyIndent) 
                {
                    SkipIgnorable();
                    const std::string next = Trim(m_lines[m_index]);
                    if (next == "-" || next.rfind("- ", 0) == 0) 
                    {
                        return ParseSequence(keyIndent); // sequence not indented below its key
                    }
                    return YamlNode::MakeScalar(std::string());
                }
                return ParseBlock(childIndent);
            }

            YamlNodePtr ParseBlockScalar(int keyIndent, const std::string& header) 
            {
                const ScalarStyle style = header[0] == '>' ? ScalarStyle::Folded : ScalarStyle::Literal;
                std::vector<std::string> lines;
                int blockIndent = kNoIndent;
                while (!AtEnd()) 
                {
                    const std::string& raw = m_lines[m_index];
                    if (Trim(raw).empty()) 
                    {
                        lines.push_back(std::string());
                        ++m_index;
                        continue;
                    }
                    const int lineIndent = IndentOf(raw);
                    if (lineIndent <= keyIndent) 
                    {
                        break;
                    }
                    if (blockIndent == kNoIndent) 
                    {
                        blockIndent = lineIndent;
                    }
                    lines.push_back(raw.substr(static_cast<std::size_t>(blockIndent > lineIndent ? lineIndent : blockIndent)));
                    ++m_index;
                }
                while (!lines.empty() && Trim(lines.back()).empty()) 
                {
                    lines.pop_back();
                    --m_index;
                }

                std::string value;
                for (std::size_t i = 0; i < lines.size(); ++i) 
                {
                    if (i > 0) 
                    {
                        if (style == ScalarStyle::Literal) 
                        {
                            value += '\n';
                        } 
                        else 
                        {
                            value += lines[i].empty() ? "\n" : " ";
                        }
                    }
                    value += Trim(lines[i]);
                }
                return YamlNode::MakeScalar(value, style);
            }
        };

    } // namespace

    YamlNodePtr YamlParser::ParseText(const std::string& text, std::string& error) 
    {
        error.clear();
        Reader reader(text);
        return reader.Parse(error);
    }

    YamlNodePtr YamlParser::ParseFile(const std::string& path, std::string& error) 
    {
        std::ifstream file(path);
        if (!file) 
        {
            error = "cannot open file: " + path;
            return nullptr;
        }
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return ParseText(buffer.str(), error);
    }

    std::string YamlParser::GetFileContent(const std::string& path) 
    {
        std::ifstream file(path);
        if (!file) 
        {
            return "";
        }
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }    

} // namespace afd
