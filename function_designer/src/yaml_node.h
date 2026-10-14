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
 
#ifndef AFD_YAML_NODE_H
#define AFD_YAML_NODE_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace afd 
{
    /// Scalar presentation style, kept so that a parsed file can be written back
    /// in the very same layout it was read in.
    enum class ScalarStyle 
    {
        Plain,       ///< value
        SingleQuote, ///< 'value'
        DoubleQuote, ///< "value"
        Folded,      ///< >-
        Literal      ///< |-
    };

    class YamlNode;
    using YamlNodePtr = std::shared_ptr<YamlNode>;

    /// Minimal, order preserving YAML document node (scalar, mapping or sequence).
    class YamlNode 
    {
    public:
        enum class Type { Scalar, Map, Sequence };

        using MapEntry = std::pair<std::string, YamlNodePtr>;

        static YamlNodePtr MakeScalar(std::string value, ScalarStyle style = ScalarStyle::Plain);
        static YamlNodePtr MakeMap();
        static YamlNodePtr MakeSequence();

        Type GetType() const { return m_type; }
        bool IsScalar() const { return m_type == Type::Scalar; }
        bool IsMap() const { return m_type == Type::Map; }
        bool IsSequence() const { return m_type == Type::Sequence; }

        const std::string& GetScalar() const { return m_scalar; }
        void SetScalar(std::string value) { m_scalar = std::move(value); }
        ScalarStyle GetStyle() const { return m_style; }
        void SetStyle(ScalarStyle style) { m_style = style; }

        const std::vector<MapEntry>& GetMap() const { return m_map; }
        const std::vector<YamlNodePtr>& GetSequence() const { return m_sequence; }

        /// Appends a key to a mapping. Duplicate keys are overwritten in place.
        void Set(const std::string& key, YamlNodePtr value);
        void Append(YamlNodePtr value);
        bool RemoveAt(std::vector<YamlNodePtr>::size_type index);

        /// Returns the child for @p key or nullptr if absent / not a mapping.
        YamlNodePtr Find(const std::string& key) const;
        /// Returns the scalar text of @p key or an empty string if absent.
        std::string ScalarOf(const std::string& key) const;

    private:
        Type m_type = Type::Scalar;
        std::string m_scalar;
        ScalarStyle m_style = ScalarStyle::Plain;
        std::vector<MapEntry> m_map;
        std::vector<YamlNodePtr> m_sequence;
    };

} // namespace afd

#endif // AFD_YAML_NODE_H
