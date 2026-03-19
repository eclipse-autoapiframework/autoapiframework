..
   # *******************************************************************************
   # Copyright (c) 2024 Contributors to the Eclipse Foundation
   #
   # See the NOTICE file(s) distributed with this work for additional
   # information regarding copyright ownership.
   #
   # This program and the accompanying materials are made available under the
   # terms of the Apache License Version 2.0 which is available at
   # https://www.apache.org/licenses/LICENSE-2.0
   #
   # SPDX-License-Identifier: Apache-2.0
   # *******************************************************************************

Building blocks
===============


.. toctree::
   :maxdepth: 1

   1. SW architecture <21_sw_architecture>
   2. Workflow <22_workflow>
   3. Model <23_model>
   4. API <24_api>
   5. Configuration <25_configuration>
   6. Platform support <26_platform_support>
   7. Code generation <27_code_generation>
   8. SW library <28_sw_library>
   9. Testing <29_testing>
   10. Glossary <30_glossary>

The *Application Framework* is a compile-time solution that puts the
focus on the executable. The essential building blocks of the framework
are illustrated below. The workflow covers all steps, from project
creation over modeling and configuration, import of design artifacts,
generation, build, and test. The software architecture on
executable-level is completely modular as illustrated on the right-hand
side. Generated and static code artifacts that are provided by the
framework go hand in hand with code that is added or integrated by the
developer in the scope of so-called application modules.

.. image:: figures/af-concept.svg

.. raw:: html
   
    <br><br>

**The definition of Application in this project is clear and unambiguous:**
An *Application* is of distributed kind and considered a thematic bracket. On technical level it can consist of 1..N executables. Each executable in turn can consist of 1..M application modules.


 
