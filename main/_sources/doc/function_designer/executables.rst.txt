..
   # *******************************************************************************
   # Copyright (c) 2026 ZF Friedrichshafen AG
   #
   # See the NOTICE file(s) distributed with this work for additional
   # information regarding copyright ownership.
   #
   # This program and the accompanying materials are made available under the
   # terms of the Apache License Version 2.0 which is available at
   # https://www.apache.org/licenses/LICENSE-2.0
   #
   # SPDX-License-Identifier: Apache-2.0
   #
   # Contributors:
   #   Thomas Pfleiderer - afs file parser added
   # *******************************************************************************

Executables
===========

.. list-table:: Executable descriptions
   :header-rows: 1
   :widths: 30 70

   * - executable name
     - Description
   * - ``AutoAPIFunctionDesigner.exe``
     - The wxWidgets GUI for creating, editing, validating and saving .afs function specifications against the meta model.
   * - ``AutoAPIFunctionDesigner_SyntaxCheck.exe``
     - A command-line tool that validates an .afs file against the Autoapiframework meta model without requiring the GUI. 
  
       For comprehensive validation, it should be used together with AfsFileParser.exe. 
       The syntax check verifies model conformance, while the parser confirms that the expected elements 
       (such as signals, parameters, and enums) are actually detected and counted. 
       
       Together, the tools provide greater confidence that an AFS file is both syntactically valid and contains the expected content.
   * - ``AfsFileParser.exe``
     - A command-line tool that parses an .afs file and prints how many enums, signals, parameters, schedules and errors it contains.
  
       This program is a very lightweight parser intended primarily for structural analysis of AFS files. 
       It does not perform syntax or semantic validation. For example, it can detect that a file contains three signal definitions, 
       but it does not verify whether these signals are defined according to the AFS specification or whether 
       their attributes and references are valid. 
       
       Consequently, the reported counts should be considered informational only and 
       not as proof that the input file is syntactically correct or specification-compliant.  
   * - ``function_designer_tests.exe``
     - The test program built from yaml_tests.cpp and the other sources listed in CMakeLists.txt, mainly covering the YAML parser and writer.
   * - ``syntax_check_tests.exe``
     - The test program for the syntax check; the test source is syntax_check_tests.cpp.
   * - ``afs_file_parser_tests.exe``
     - The test program for the AfsFileParser; the test source is afs_file_parser_tests.cpp. 


The project contains the project executables listed above, which include the GUI application, command-line tools, 
and test programs for various components of the Function Designer. 
The commandline tools allow for automated processing and validation of .afs files without requiring the GUI.

Syntax check
------------
The ``AutoAPIFunctionDesigner_SyntaxCheck.exe`` is a command-line tool that validates an .afs file against the meta model without 
requiring the GUI. It helps ensure that the file adheres to the expected structure and rules defined in the meta model.

The tool loads the meta model file 'autoapiframework_metadata_V04.yaml' from the example folder or 'autoapiframework_meta_model.yaml' from the executable folder. 

AFS syntax check usage:

.. code-block:: powershell

   AutoAPIFunctionDesigner_SyntaxCheck.exe [file.afs]

AFS file parser
---------------

The AFS file parser is a command-line tool that parses an .afs file and provides a summary of its contents, 
including the number of enums, signals, parameters, schedules, and any errors found. 
This allows for quick inspection and validation of .afs files without using the GUI.

AFS file parser usage:

.. code-block:: powershell

   AfsFileParser.exe [file.afs]

or

.. code-block:: powershell

   AfsFileParser.exe [file.afs] [expected.txt]

An optional second argument is a text file with the expected content. 
Mismatches are reported and the exit code is 3. 
Supervision lines specify the expected monitoring for specific elements. 
Supported lines (# starts a comment):

.. code-block:: powershell

   enums = 2
   signals = 3
   parameters = 2
   schedules = 3
   errors = 0
   supervision SpeedHazardDetection.Step = alive,deadline   # none | alive | deadline | logical


