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

Testing
=======

Unit tests (GoogleTest)
-----------------------

The Vehicle Application Framework (VAF) assist with GoogleTest-based
unit testing of application modules by generating test skeletons and
mocks for all used interfaces. The idea is that the writer of a unit
test can mock any input/output of the interfaces to produce test cases
that verify the actual application code without having to connect to
real or simulated external test communication partners. The generated
test project builds together with generated test-specific base code,
which vastly simplifies the writing of the unit test.

Unit tests are activated by default but can be disabled with the
following flag that needs to be passed to the preset stage:

::

   vaf make preset -d -DVAF_BUILD_TESTS=OFF

| The test code, which can be edited and extended by the user is located in the file:
| ``<app-module project directory>/implementation/test/unittest/src/tests.cpp``

One example skeleton test case is generated for reference. It is up to
the user to adapt it and add further tests. Writing a test case consists
of 4 steps:

1. Test case creation.

::

   TEST_F(AppModule1UnitTest, Test_1) {

2. Create mock objects of interfaces as used by the application.

::

   auto HvacStatusConsumerMock = std::make_shared<demo::HvacStatusConsumerMock>();
   auto DataExchangeProviderMock = std::make_shared<demo::DataExchangeInterfaceProviderMock>();

3. Write expected outcome of the test, i.e., what you want to test.

::

   EXPECT_CALL(*DataExchangeProviderMock, RegisterOperationHandler_MyFunction(_)).Times(1);

4. Create an application module object using above created interface
   mocks.

::

   auto AppModule1 = std::make_shared<demo::AppModule1>( demo::AppModule1 ::ConstructorToken {
       HvacStatusConsumerMock,
       DataExchangeProviderMock});
   }

SIL tests (SIL Kit)
-------------------

Software In the Loop (SIL) testing is supported using `Vector SIL
Kit <https://github.com/vectorgrp/sil-kit>`__. Any interface modeled in
Configuration as Code (CaC) can be selected to be routed via SIL Kit.
SIL Kit uses a registry application, which acts as a message broker. In
other words, all messages must go through that router. The connection to
SIL Kit in the VAF is configured using CaC and is done similar to other
interface connections. Only difference, one has to connect to the SIL
Kit registry by specifying the registry URI and instance name of the
communication interface. Providers and consumers that want to connect
should configure the same instance name and URI.

CaC snippet connecting an interface to an application module using SIL
Kit:

.. code:: python

   demo_app.connect_consumed_interface_to_silkit(
       Appmodule1, # The application module
       Appmodule1.ConsumedInterfaces.VelocityServiceConsumer, # The connected interface
       "Silkit_VelocityService", # Instance name used to identify the interface in SIL Kit registry
       "silkit://192.168.128.129:8500" # URI of the running SIL Kit registry to connect to
   )
   demo_app.connect_provided_interface_to_silkit(
       Appmodule2, # The application module
       Appmodule2.ProvidedInterfaces.VelocityServiceProvider, # The connected interface
       "Silkit_VelocityService", # Instance name used to identify the interface in SIL Kit registry
       "silkit://192.168.128.129:8500" # URI of the running SIL Kit registry to connect to
   )

VAF utilizes `Protobuf <https://protobuf.dev/>`__ to serialize the data
such that communication partners on any type of platform easily can
serialize or deserialize the payload. Proto files are generated for all
defined datatypes and interfaces. These proto files are then compiled
using the protoc compiler. Further, transformers get generated for each
type to enable easy translation between protobuf types and VAF types.
