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
 *   Thomas Pfleiderer - CTest added
 *   Thomas Pfleiderer - support 4.1, Support YAML comments, Drag & Drop, Enums in afs files added
 ********************************************************************************/

#include "function_specification.h"
#include "yaml_node.h"
#include "yaml_parser.h"
#include "yaml_writer.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    void Require(bool condition, const std::string& message)
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    afd::YamlNodePtr Parse(afd::YamlParser& parser, const std::string& text)
    {
        std::string error;
        afd::YamlNodePtr root = parser.ParseText(text, error);
        Require(root != nullptr, "parse failed: " + error);
        Require(error.empty(), "successful parse returned an error: " + error);
        return root;
    }

    // -------------------------------------------------------------------------
    // Existing tests
    // -------------------------------------------------------------------------

    void TestOrderedNodes()
    {
        afd::YamlNodePtr map = afd::YamlNode::MakeMap();
        map->Set("first", afd::YamlNode::MakeScalar("one"));
        map->Set("second", afd::YamlNode::MakeScalar("two"));
        map->Set("first", afd::YamlNode::MakeScalar("updated"));

        Require(map->GetMap().size() == 2, "replacing a map key added a duplicate");
        Require(map->GetMap()[0].first == "first", "replacing a key changed map order");
        Require(map->ScalarOf("first") == "updated", "replacing a key did not update its value");

        afd::YamlNodePtr sequence = afd::YamlNode::MakeSequence();
        sequence->Append(afd::YamlNode::MakeScalar("one"));
        Require(!sequence->RemoveAt(1), "removing an out-of-range sequence item succeeded");
        Require(sequence->RemoveAt(0), "removing a valid sequence item failed");
        Require(sequence->GetSequence().empty(), "removed sequence item is still present");
    }

    void TestScalarsAndInvalidInput()
    {
        afd::YamlParser parser;
        const afd::YamlNodePtr root = Parse(parser,
            "name: \"value # kept\" # comment\n"
            "quoted: 'it''s quoted'\n"
            "emptyMap: {}\n"
            "emptySequence: []\n"
            "description: >-\n"
            "  first line\n"
            "  second line\n"
            "literal: |-\n"
            "  first line\n"
            "  second line\n");

        Require(root->ScalarOf("name") == "value # kept", "quoted comment marker was stripped");
        Require(root->ScalarOf("quoted") == "it's quoted", "single-quoted scalar was not unescaped");
        Require(root->Find("emptyMap")->IsMap() && root->Find("emptyMap")->GetMap().empty(),
            "empty map was not parsed as a map");
        Require(root->Find("emptySequence")->IsSequence() && root->Find("emptySequence")->GetSequence().empty(),
            "empty sequence was not parsed as a sequence");
        Require(root->Find("description")->GetScalar() == "first line second line",
                "folded block scalar was not folded");
        Require(root->Find("literal")->GetScalar() == "first line\nsecond line",
                "literal block scalar lost its line break");

        std::string error;
        Require(parser.ParseText("name: first\n---\nname: second\n", error) == nullptr,
            "multiple YAML documents were accepted");
        Require(!error.empty(), "invalid input did not report an error");
    }

    void TestExampleRoundTrips()
    {
        const std::vector<std::string> examples = {
            "speed_hazard_detection.afs",
            "vehicle_speed_fusion_multirate.afs",
            "wheel_speed_plausibility_check.afs"
        };
        afd::YamlParser parser;
        afd::YamlWriter writer;

        for (const std::string& example : examples)
        {
            const std::string path = std::string(AFD_EXAMPLES_DIR) + "/" + example;
            std::string error;
            const afd::YamlNodePtr root = parser.ParseFile(path, error);
            Require(root != nullptr, example + " did not parse: " + error);

            const afd::YamlNodePtr specification = root->Find("functionSpecification");
            Require(specification && specification->IsMap(), example + " has no functionSpecification map");
            Require(!specification->ScalarOf("name").empty(), example + " has no function name");

            const afd::YamlNodePtr interfaces = specification->Find("dataInterfaces");
            Require(interfaces && interfaces->IsSequence() && !interfaces->GetSequence().empty(),
                    example + " has no dataInterfaces entries");
            Require(interfaces->GetSequence().front()->Find("name") != nullptr,
                    example + " first data interface has no name");

            const std::string serialized = writer.WriteText(root);
            const afd::YamlNodePtr reparsed = Parse(parser, serialized);
            const afd::YamlNodePtr reparsedSpecification = reparsed->Find("functionSpecification");
            Require(reparsedSpecification &&
                    reparsedSpecification->ScalarOf("name") == specification->ScalarOf("name"),
                    example + " changed its function name after writing and reparsing");
            const afd::YamlNodePtr reparsedInterfaces = reparsedSpecification->Find("dataInterfaces");
            Require(reparsedInterfaces && reparsedInterfaces->IsSequence() &&
                    reparsedInterfaces->GetSequence().size() == interfaces->GetSequence().size(),
                    example + " changed its data interface count after writing and reparsing");
        }

    }

    void TestAddMissingCollections()
    {
        afd::FunctionSpecification specification;
        std::string error;
        const std::string path = std::string(AFD_TESTS_DIR) + "/missing_collections.afs";
        Require(specification.Load("0.4.1", path, error), "load failed: " + error);

        const std::vector<std::string> keys = {
            afd::FunctionSpecification::kErrorsKey,
            afd::FunctionSpecification::kParametersKey,
            afd::FunctionSpecification::kSchedulingKey
        };
        afd::YamlNodePtr item = afd::YamlNode::MakeMap();
        item->Set("name", afd::YamlNode::MakeScalar("Added"));
        for (const std::string& key : keys)
        {
            Require(specification.GetCollection(key).empty(), key + " should initially be absent");
            Require(specification.AddCollectionItem(key, item), "could not add to missing " + key);
            Require(specification.AddCollectionItem(key, item), "could not append to " + key);
            Require(specification.GetCollection(key).size() == 2, key + " entries were not retained");
        }
        Require(!specification.AddCollectionItem("invalidCollection", item),
                "non-sequence collection should not be overwritten");

        afd::YamlParser parser;
        const afd::YamlNodePtr root = Parse(parser, specification.ToText());
        const afd::YamlNodePtr spec = root->Find("functionSpecification");
        Require(spec->ScalarOf("invalidCollection") == "not-a-sequence",
                "invalid collection was overwritten");
        Require(spec->Find("dataInterfaces")->GetSequence().size() == 1,
                "existing data interface was lost");
        for (const std::string& key : keys)
        {
            const afd::YamlNodePtr collection = spec->Find(key);
            Require(collection && collection->IsSequence() && collection->GetSequence().size() == 2,
                    key + " was not serialized as a sequence");
        }
    }

    void TestSequenceMapLeadingCommentRoundTrip()
    {
        afd::YamlParser parser;
        afd::YamlWriter writer;
        const afd::YamlNodePtr root = Parse(parser,
            "items:\n"
            "  - name: first\n"
            "    description: example\n");
        const afd::YamlNodePtr items = root->Find("items");
        items->GetSequence().front()->Find("name")->AddLeadingComment("# first item name");

        const std::string serialized = writer.WriteText(root);
        Require(serialized.find("# first item name") != std::string::npos,
                "sequence map key comment was lost");
        const afd::YamlNodePtr reparsed = Parse(parser, serialized);
        const afd::YamlNodePtr reparsedItems = reparsed->Find("items");
        Require(reparsedItems && reparsedItems->IsSequence() &&
                reparsedItems->GetSequence().size() == 1,
                "sequence map item was not preserved");
        const afd::YamlNodePtr item = reparsedItems->GetSequence().front();
        Require(item->ScalarOf("name") == "first" && item->ScalarOf("description") == "example",
                "sequence map values were not preserved");
        Require(!item->Find("name")->GetLeadingComments().empty() &&
                item->Find("name")->GetLeadingComments().front() == "# first item name",
                "sequence map key comment was not preserved");
    }

    // -------------------------------------------------------------------------
    // Node API tests
    // -------------------------------------------------------------------------

    // Find() must return nullptr for absent keys and when called on non-map nodes.
    void TestFindReturnsNullForMissingKey()
    {
        afd::YamlNodePtr map = afd::YamlNode::MakeMap();
        map->Set("present", afd::YamlNode::MakeScalar("value"));

        Require(map->Find("present") != nullptr,
                "existing key should be found");
        Require(map->Find("absent") == nullptr,
                "absent key should return nullptr");

        // Find on a sequence node: the internal map is empty, so it always
        // returns nullptr without crashing.
        const afd::YamlNodePtr seq = afd::YamlNode::MakeSequence();
        seq->Append(afd::YamlNode::MakeScalar("item"));
        Require(seq->Find("any") == nullptr,
                "Find on a sequence node should return nullptr");

        // Same guarantee for a scalar node.
        const afd::YamlNodePtr scalar = afd::YamlNode::MakeScalar("text");
        Require(scalar->Find("any") == nullptr,
                "Find on a scalar node should return nullptr");
    }

    // IsScalar / IsMap / IsSequence must be mutually exclusive and accurate.
    void TestNodeTypePredicates()
    {
        const afd::YamlNodePtr scalarNode = afd::YamlNode::MakeScalar("hello");
        Require( scalarNode->IsScalar(),    "scalar node: IsScalar should be true");
        Require(!scalarNode->IsMap(),       "scalar node: IsMap should be false");
        Require(!scalarNode->IsSequence(),  "scalar node: IsSequence should be false");

        const afd::YamlNodePtr mapNode = afd::YamlNode::MakeMap();
        Require(!mapNode->IsScalar(),       "map node: IsScalar should be false");
        Require( mapNode->IsMap(),          "map node: IsMap should be true");
        Require(!mapNode->IsSequence(),     "map node: IsSequence should be false");

        const afd::YamlNodePtr seqNode = afd::YamlNode::MakeSequence();
        Require(!seqNode->IsScalar(),       "sequence node: IsScalar should be false");
        Require(!seqNode->IsMap(),          "sequence node: IsMap should be false");
        Require( seqNode->IsSequence(),     "sequence node: IsSequence should be true");
    }

    // -------------------------------------------------------------------------
    // Parser tests
    // -------------------------------------------------------------------------

    // Multi-level nesting must be fully navigable via chained Find() calls.
    void TestDeepNesting()
    {
        afd::YamlParser parser;
        const afd::YamlNodePtr root = Parse(parser,
            "level1:\n"
            "  level2:\n"
            "    level3: deep_value\n"
            "    items:\n"
            "      - alpha\n"
            "      - beta\n"
            "  sibling: adjacent\n");

        const afd::YamlNodePtr lvl1 = root->Find("level1");
        Require(lvl1 && lvl1->IsMap(), "level1 should be a map");

        const afd::YamlNodePtr lvl2 = lvl1->Find("level2");
        Require(lvl2 && lvl2->IsMap(), "level2 should be a map");
        Require(lvl2->ScalarOf("level3") == "deep_value",
                "scalar value three levels deep is wrong");

        const afd::YamlNodePtr items = lvl2->Find("items");
        Require(items && items->IsSequence() && items->GetSequence().size() == 2,
                "nested sequence should have 2 items");
        Require(items->GetSequence()[0]->GetScalar() == "alpha",
                "first nested sequence item wrong");
        Require(items->GetSequence()[1]->GetScalar() == "beta",
                "second nested sequence item wrong");

        // Sibling key lives at level1, not inside level2.
        Require(lvl1->ScalarOf("sibling") == "adjacent",
                "sibling key at level1 should be reachable");
        Require(lvl2->Find("sibling") == nullptr,
                "sibling should not be visible from inside level2");
    }

    // A sequence of maps is the primary structure used by every section of an
    // .afs file (dataInterfaces, scheduling, etc.).
    void TestSequenceOfMaps()
    {
        afd::YamlParser parser;
        const afd::YamlNodePtr root = Parse(parser,
            "items:\n"
            "  - name: first\n"
            "    value: 1\n"
            "  - name: second\n"
            "    value: 2\n"
            "  - name: third\n"
            "    value: 3\n");

        const afd::YamlNodePtr items = root->Find("items");
        Require(items && items->IsSequence(),
                "items should be a sequence");
        Require(items->GetSequence().size() == 3,
                "items should have 3 entries");

        // Verify each entry is a map and has the expected key/value pairs.
        const std::vector<std::string> expectedNames  = { "first", "second", "third" };
        const std::vector<std::string> expectedValues = { "1",     "2",      "3"     };
        for (std::size_t i = 0; i < 3; ++i)
        {
            const afd::YamlNodePtr entry = items->GetSequence()[i];
            Require(entry && entry->IsMap(),
                    "sequence entry " + std::to_string(i) + " should be a map");
            Require(entry->ScalarOf("name")  == expectedNames[i],
                    "entry " + std::to_string(i) + " name wrong");
            Require(entry->ScalarOf("value") == expectedValues[i],
                    "entry " + std::to_string(i) + " value wrong");
        }
    }

    // -------------------------------------------------------------------------
    // Writer tests
    // -------------------------------------------------------------------------

    // Scalar values that contain characters requiring YAML quoting must survive
    // a write-then-parse round-trip with their content intact.
    void TestWriterSpecialCharactersRoundTrip()
    {
        afd::YamlParser parser;
        afd::YamlWriter writer;

        afd::YamlNodePtr map = afd::YamlNode::MakeMap();
        // Contains ": " — triggers quoting because it looks like a mapping key.
        map->Set("withColon",    afd::YamlNode::MakeScalar("key: value"));
        // Contains " #" — looks like an inline comment.
        map->Set("withHash",     afd::YamlNode::MakeScalar("text # inline"));
        // Starts with "#" — would be treated as a full-line comment.
        map->Set("leadingHash",  afd::YamlNode::MakeScalar("# starts with hash"));
        // Starts with "'" — in the set of leading chars that require quoting.
        map->Set("leadingQuote", afd::YamlNode::MakeScalar("'quoted-looking'"));
        // Ordinary value — no quoting needed.
        map->Set("plain",        afd::YamlNode::MakeScalar("ordinary"));

        const std::string yaml = writer.WriteText(map);

        std::string error;
        const afd::YamlNodePtr reparsed = parser.ParseText(yaml, error);
        Require(reparsed != nullptr,
                "special-character yaml failed to re-parse: " + error);

        Require(reparsed->ScalarOf("withColon")    == "key: value",
                "colon value changed after round-trip");
        Require(reparsed->ScalarOf("withHash")     == "text # inline",
                "hash value changed after round-trip");
        Require(reparsed->ScalarOf("leadingHash")  == "# starts with hash",
                "leading-hash value changed after round-trip");
        Require(reparsed->ScalarOf("leadingQuote") == "'quoted-looking'",
                "leading-quote value changed after round-trip");
        Require(reparsed->ScalarOf("plain")        == "ordinary",
                "plain value changed after round-trip");
    }

    // Programmatically constructed empty maps and sequences must be emitted
    // as flow-style {} / [] and parse back as empty collections.
    void TestWriterEmptyCollectionsRoundTrip()
    {
        afd::YamlParser parser;
        afd::YamlWriter writer;

        afd::YamlNodePtr root = afd::YamlNode::MakeMap();
        root->Set("emptyMap", afd::YamlNode::MakeMap());
        root->Set("emptySeq", afd::YamlNode::MakeSequence());
        root->Set("marker",   afd::YamlNode::MakeScalar("end"));

        const std::string yaml = writer.WriteText(root);

        // Quick content check — the writer should emit flow-style placeholders.
        Require(yaml.find("emptyMap: {}") != std::string::npos,
                "empty map was not written as '{}'");
        Require(yaml.find("emptySeq: []") != std::string::npos,
                "empty sequence was not written as '[]'");

        // Parse back and verify the types survive.
        std::string error;
        const afd::YamlNodePtr reparsed = parser.ParseText(yaml, error);
        Require(reparsed != nullptr,
                "empty-collections yaml failed to re-parse: " + error);

        const afd::YamlNodePtr em = reparsed->Find("emptyMap");
        Require(em && em->IsMap() && em->GetMap().empty(),
                "emptyMap should parse back as an empty map");

        const afd::YamlNodePtr es = reparsed->Find("emptySeq");
        Require(es && es->IsSequence() && es->GetSequence().empty(),
                "emptySeq should parse back as an empty sequence");

        Require(reparsed->ScalarOf("marker") == "end",
                "marker value wrong after empty-collections round-trip");
    }

    // Block scalars (folded >- and literal |-) built programmatically must
    // survive a write-then-parse round-trip with their content intact.
    void TestWriterBlockScalarsRoundTrip()
    {
        afd::YamlParser parser;
        afd::YamlWriter writer;

        afd::YamlNodePtr root = afd::YamlNode::MakeMap();
        // Folded: multiple space-separated words stored as one line; the writer
        // re-wraps them and the parser joins them back with a space.
        root->Set("folded",
                  afd::YamlNode::MakeScalar("first line second line", afd::ScalarStyle::Folded));
        // Literal: embedded newline must be preserved exactly as-is.
        root->Set("literal",
                  afd::YamlNode::MakeScalar("first line\nsecond line", afd::ScalarStyle::Literal));

        const std::string yaml = writer.WriteText(root);

        std::string error;
        const afd::YamlNodePtr reparsed = parser.ParseText(yaml, error);
        Require(reparsed != nullptr,
                "block-scalar yaml failed to re-parse: " + error);

        Require(reparsed->Find("folded")->GetScalar() == "first line second line",
                "folded scalar content changed after round-trip");
        Require(reparsed->Find("literal")->GetScalar() == "first line\nsecond line",
                "literal scalar lost its embedded newline after round-trip");
    }

} // namespace

int main()
{
    struct TestCase
    {
        const char* name;
        void (*run)();
    };

    const TestCase tests[] = {
        // Original tests        
        {"TestOrderedNodes", TestOrderedNodes},
        {"TestScalarsAndInvalidInput", TestScalarsAndInvalidInput},
        {"TestExampleRoundTrips", TestExampleRoundTrips},     
        {"TestAddMissingCollections", TestAddMissingCollections},
        {"TestSequenceMapLeadingCommentRoundTrip", TestSequenceMapLeadingCommentRoundTrip},
        // Node API tests           
        {"TestFindReturnsNullForMissingKey", TestFindReturnsNullForMissingKey},
        {"TestNodeTypePredicates", TestNodeTypePredicates},
        // Parser tests        
        {"TestDeepNesting", TestDeepNesting},
        {"TestSequenceOfMaps", TestSequenceOfMaps},
        // Writer tests        
        {"TestWriterSpecialCharactersRoundTrip", TestWriterSpecialCharactersRoundTrip},
        {"TestWriterEmptyCollectionsRoundTrip", TestWriterEmptyCollectionsRoundTrip},
        {"TestWriterBlockScalarsRoundTrip", TestWriterBlockScalarsRoundTrip},
    };

    int failedTests = 0;
    for (const TestCase& test : tests)
    {
        try
        {
            test.run();
            std::cout << "[PASS] " << test.name << '\n';
        }
        catch (const std::exception& error)
        {
            std::cerr << "[FAIL] " << test.name << ": " << error.what() << '\n';
            ++failedTests;
        }
    }

    if (failedTests == 0)
    {
        std::cout << "All " << (sizeof(tests) / sizeof(tests[0]))
                  << " function designer tests passed.\n";
        return 0;
    }

    std::cerr << failedTests << " of " << (sizeof(tests) / sizeof(tests[0]))
              << " function designer tests failed.\n";
    return 1;
}
