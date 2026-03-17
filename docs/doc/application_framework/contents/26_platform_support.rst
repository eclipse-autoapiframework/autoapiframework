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

Platform support
================

Model import
------------

VSS import (vafvssimport)
~~~~~~~~~~~~~~~~~~~~~~~~~

The ``vafvssimport`` package allows to import a VSS-based data catalogue
into module interfaces and datatypes of the internal Vehicle Application
Framework (VAF) model. The importer requires the JSON data export from
VSS.

Each branch of the tree-like VSS model is converted to a nested module interface. The parent branches are used as namespace for the current interface. All child elements of a branch are represented as data elements of that interface. If these elements are branches by themselves, they are imported as structs, while leaves result in primitive types.

With this structure, the user is able to either access individual leafs of the VSS catalogue by using one of the nested interfaces, or complete branches by working with the structs of interfaces that sit higher in the hierarchy.

VSS supports restrictions on input values of parameters. For numeric
datatypes, a range can be specified, while string parameters support a
list of allowed values. The numeric ranges are imported into the VAF
model as is, while the allowed string values are converted to enums for
better usability. VSS allows a maximum size to be specified for array
types. Parameters with this attribute set are imported as fixed-size
arrays, while those without are imported as vectors.
