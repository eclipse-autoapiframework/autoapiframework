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
 *   Thomas Pfleiderer - YAML comment tests
 *   Thomas Pfleiderer - support 4.1, Support YAML comments, Drag & Drop, Enums in afs files added
 ********************************************************************************/

#include "yaml_node.h"
#include "yaml_parser.h"
#include "yaml_writer.h"

#include <iostream>
#include <stdexcept>
#include <string>

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
    // YAML Comment Tests - Now with comment preservation support
    // -------------------------------------------------------------------------
    // These tests verify that the 4 comment scenarios from comments.md
    // are now PRESERVED during round-trip operations.
    // -------------------------------------------------------------------------

    // Example 1: Basic Comment
    // Comments begin with # and continue to the end of the line.
    // They are now preserved in round-trip operations.
    void TestBasicCommentPreserved()
    {
        afd::YamlParser parser;
        afd::YamlWriter writer;

        const afd::YamlNodePtr root = Parse(parser,
            "# This is a comment\n"
            "name: VehicleSpeed\n");

        // The data is parsed correctly
        Require(root->ScalarOf("name") == "VehicleSpeed", 
            "basic comment: data value should be parsed");

        // The comment is NOW preserved when writing back
        const std::string output = writer.WriteText(root);
        Require(output.find("# This is a comment") != std::string::npos,
            "basic comment: comments should be preserved in output");
    }

    // Example 2: Inline Comment
    // Comments can appear after a value on the same line.
    // They are now preserved.
    void TestInlineCommentPreserved()
    {
        afd::YamlParser parser;
        afd::YamlWriter writer;

        const afd::YamlNodePtr root = Parse(parser,
            "dataType: enum  # Signal uses an enumeration\n"
            "enumRef: GearPosition\n");

        // The data is parsed correctly
        Require(root->ScalarOf("dataType") == "enum", 
            "inline comment: dataType should be parsed");
        Require(root->ScalarOf("enumRef") == "GearPosition", 
            "inline comment: enumRef should be parsed");

        // The inline comment is NOW preserved when writing back
        const std::string output = writer.WriteText(root);
        Require(output.find("# Signal uses an enumeration") != std::string::npos,
            "inline comment: inline comments should be preserved");
    }

    // Example 3: Multiple Comment Lines
    // Since YAML does not support block comments, each line must begin with #.
    // Multiple comment lines are now preserved.
    void TestMultipleCommentLinesPreserved()
    {
        afd::YamlParser parser;
        afd::YamlWriter writer;

        const afd::YamlNodePtr root = Parse(parser,
            "# Define the signal type\n"
            "# and the associated enumeration.\n"
            "dataType: enum\n"
            "enumRef: GearPosition\n");

        // The data is parsed correctly
        Require(root->ScalarOf("dataType") == "enum", 
            "multiple comment lines: dataType should be parsed");
        Require(root->ScalarOf("enumRef") == "GearPosition", 
            "multiple comment lines: enumRef should be parsed");

        // The multi-line comments are NOW preserved when writing back
        const std::string output = writer.WriteText(root);
        Require(output.find("# Define the signal type") != std::string::npos,
            "multiple comment lines: first comment should be preserved");
        Require(output.find("# and the associated enumeration") != std::string::npos,
            "multiple comment lines: second comment should be preserved");
    }

    void TestDocumentLeadingCommentsStayAtStart()
    {
        afd::YamlParser parser;
        afd::YamlWriter writer;
        const afd::YamlNodePtr root = Parse(parser,
            "# Document comment one\n"
            "# Document comment two\n"
            "functionSpecification:\n"
            "  name: Example\n");

        const std::string output = writer.WriteText(root);
        const std::string expectedPrefix =
            "# Document comment one\n"
            "# Document comment two\n"
            "functionSpecification:\n";
        Require(output.compare(0, expectedPrefix.size(), expectedPrefix) == 0,
            "document comments should remain before the first node");

        const afd::YamlNodePtr reparsed = Parse(parser, output);
        Require(reparsed->GetLeadingComments().size() == 2,
            "document comments should remain attached to the document root");
        Require(reparsed->Find("functionSpecification")->Find("name")->GetLeadingComments().empty(),
            "document comments should not move to the first nested node");
    }

    // Example 4: Commenting Out Configuration
    // A common use of comments is to temporarily disable YAML entries.
    // Commented-out lines are now preserved.
    void TestCommentedOutConfigurationPreserved()
    {
        afd::YamlParser parser;
        afd::YamlWriter writer;

        const afd::YamlNodePtr root = Parse(parser,
            "# dataType: int32\n"
            "# default: 0\n"
            "dataType: enum\n"
            "default: GearPosition\n");

        // The active configuration is parsed correctly
        Require(root->ScalarOf("dataType") == "enum", 
            "commenting out: active dataType should be parsed");
        Require(root->ScalarOf("default") == "GearPosition", 
            "commenting out: active default should be parsed");

        // The commented-out lines are NOW preserved when writing back
        const std::string output = writer.WriteText(root);
        Require(output.find("# dataType: int32") != std::string::npos,
            "commenting out: commented-out dataType should be preserved");
        Require(output.find("# default: 0") != std::string::npos,
            "commenting out: commented-out default should be preserved");
    }

    // Test: # character in quoted strings is NOT treated as a comment
    // This is supported correctly and values are preserved.
    void TestHashInQuotedStringPreserved()
    {
        afd::YamlParser parser;
        afd::YamlWriter writer;

        const afd::YamlNodePtr root = Parse(parser,
            "message: \"# is not a comment here\"\n"
            "errorCode: 'Error #42'\n");

        Require(root->ScalarOf("message") == "# is not a comment here", 
            "# in quoted string: double-quoted hash should be preserved");
        Require(root->ScalarOf("errorCode") == "Error #42", 
            "# in quoted string: single-quoted hash should be preserved");

        // Values survive round-trip because they're quoted
        const std::string output = writer.WriteText(root);
        const afd::YamlNodePtr reparsed = Parse(parser, output);
        Require(reparsed->ScalarOf("message") == "# is not a comment here",
            "hash in quoted string: round-trip should preserve the value");
        Require(reparsed->ScalarOf("errorCode") == "Error #42",
            "hash in quoted string: round-trip should preserve the value");
    }

    // Test: # character in block scalars is literal text
    // This is supported correctly (the literal text is preserved).
    void TestHashInBlockScalarPreserved()
    {
        afd::YamlParser parser;
        afd::YamlWriter writer;

        const afd::YamlNodePtr root = Parse(parser,
            "description: >-\n"
            "  larger text\n"
            "  # Vehicle is parked\n");

        // The # becomes part of the literal string value
        const std::string descValue = root->ScalarOf("description");
        Require(descValue.find("# Vehicle is parked") != std::string::npos, 
            "hash in block scalar: # should be part of the literal value");

        // The literal string with # does survive round-trip
        const std::string output = writer.WriteText(root);
        const afd::YamlNodePtr reparsed = Parse(parser, output);
        Require(reparsed->ScalarOf("description").find("# Vehicle is parked") != std::string::npos,
            "hash in block scalar: round-trip should preserve literal # in content");
    }

    // Summary: Comprehensive round-trip test with all comment types
    void TestCommentRoundTripComprehensive()
    {
        afd::YamlParser parser;
        afd::YamlWriter writer;

        // This YAML demonstrates all the comment scenarios
        const std::string input =
            "# Comment lines are preserved\n"
            "dataType: enum      # inline comments preserved\n"
            "# commented: out    # this entire line is preserved\n"
            "value: \"# but this hash is part of value\"\n";

        const afd::YamlNodePtr root = Parse(parser, input);

        // After parsing and writing back, comments should be preserved
        const std::string output = writer.WriteText(root);
        Require(output.find("# Comment lines are preserved") != std::string::npos,
            "round-trip: full-line comment should be preserved");
        Require(output.find("# inline comments preserved") != std::string::npos,
            "round-trip: inline comment should be preserved");
        Require(output.find("# commented: out") != std::string::npos,
            "round-trip: commented-out line should be preserved");
        Require(output.find("# but this hash is part of value") != std::string::npos,
            "round-trip: hash inside quoted string should be preserved");

        // Verify the data is still correct
        const afd::YamlNodePtr reparsed = Parse(parser, output);
        Require(reparsed->ScalarOf("dataType") == "enum",
            "round-trip: active dataType should be preserved");
        Require(reparsed->ScalarOf("value") == "# but this hash is part of value",
            "round-trip: quoted value with hash should be preserved");
    }
}

int main()
{
    struct TestCase
    {
        const char* name;
        void (*run)();
    };

    const TestCase tests[] = {
        {"TestBasicCommentPreserved", TestBasicCommentPreserved},
        {"TestInlineCommentPreserved", TestInlineCommentPreserved},
        {"TestMultipleCommentLinesPreserved", TestMultipleCommentLinesPreserved},
        {"TestDocumentLeadingCommentsStayAtStart", TestDocumentLeadingCommentsStayAtStart},
        {"TestCommentedOutConfigurationPreserved", TestCommentedOutConfigurationPreserved},
        // Test that # in values is handled correctly        
        {"TestHashInQuotedStringPreserved", TestHashInQuotedStringPreserved},
        {"TestHashInBlockScalarPreserved", TestHashInBlockScalarPreserved},
        // Summary test with all comment types        
        {"TestCommentRoundTripComprehensive", TestCommentRoundTripComprehensive},
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
                  << " YAML comment tests passed.\n";
        return 0;
    }

    std::cerr << failedTests << " of " << (sizeof(tests) / sizeof(tests[0]))
              << " YAML comment tests failed.\n";
    return 1;
}
