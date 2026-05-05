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
   #
   # Contributors:
   #   Thomas Pfleiderer - first documentation
   # *******************************************************************************
   
Requirements
============

Common Fields Requirements
--------------------------

.. stkh_req:: REQ-COMMON_FIELDS-001
   :id: stkh_req__14574370__001
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Each interface (data or parameter) shall define the following attributes:
   Type, Default value, Datatype, Min and Max range limits, Unit, Description,
   and ASIL level.

.. stkh_req:: REQ-COMMON_FIELDS-002
   :id: stkh_req__14574372__002
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Error interface extensions shall include fields for Datatype, Maturation
   time, Severity, Reset time, Reset condition, Description, and Dependencies
   (optional).

.. stkh_req:: REQ-COMMON_FIELDS-003
   :id: stkh_req__14574374__003
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Safety reaction extensions shall include fields for Datatype, Error list,
   and Description.

.. stkh_req:: REQ-COMMON_FIELDS-004
   :id: stkh_req__14574376__004
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Mode interface extensions shall include a Datatype definition.

.. stkh_req:: REQ-COMMON_FIELDS-005
   :id: stkh_req__14574378__005
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Scheduling interface extensions shall include detailed execution and
   supervision attributes.


Data Requirements
-----------------

.. stkh_req:: REQ-DATA-001
   :id: stkh_req__14574420__001
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Standardization of interfaces shall also include coordinate system
   (ISO, SAE), vehicle reference point (virtual sensor mounting point,
   e.g. for velocity and accelerations), kinematic/kinetic value
   (e.g. for accelerations), and signal properties such as sample rate,
   accuracy, precision, under-/over-estimated, and integrity.


Dependency Requirements
-----------------------

.. stkh_req:: REQ-DEPENDENCY-001
   :id: stkh_req__1457438__001
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   The system shall allow explicit declaration of interface dependencies
   (e.g. error dependencies and safety reaction dependencies).

.. stkh_req:: REQ-DEPENDENCY-002
   :id: stkh_req__14574382__002
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   If an interface depends on another, the dependency chain shall be
   documented within the VSS model.

.. stkh_req:: REQ-DEPENDENCY-003
   :id: stkh_req__14574384__003
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Circular dependencies between interfaces shall be automatically flagged
   during validation.


Error Handling Requirements
---------------------------

.. stkh_req:: REQ-ERROR_HANDLING-001
   :id: stkh_req__14574330__001
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   The system shall provide standardized error interfaces that enable
   applications to send error log requests and error reset requests to BSW
   failure/fault management components.

.. stkh_req:: REQ-ERROR_HANDLING-002
   :id: stkh_req__14574332__002
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   The naming convention for error interfaces shall follow the format:
   FunctionName_ErrorName_ErrorSts.

.. stkh_req:: REQ-ERROR_HANDLING-003
   :id: stkh_req__14574334__003
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Each error interface shall define the following attributes: Datatype,
   Maturation time, Severity, Reset time, Reset condition, Description, and
   Dependencies (optional).

.. stkh_req:: REQ-ERROR_HANDLING-004
   :id: stkh_req__14574336__004
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   The system shall support consistent error handling and logging practices
   across platforms to ensure interoperability.


Extensibility and Maintainability Requirements
----------------------------------------------

.. stkh_req:: REQ-EXTENSIBILITY_MAINTAINABILITY-001
   :id: stkh_req__14574404__001
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   The system shall support adding new error or safety reaction types without
   requiring redefinition of existing interfaces.

.. stkh_req:: REQ-EXTENSIBILITY_MAINTAINABILITY-002
   :id: stkh_req__14574406__002
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   The VSS extension schema shall support optional fields that can be populated
   by OEM-specific requirements.

.. stkh_req:: REQ-EXTENSIBILITY_MAINTAINABILITY-003
   :id: stkh_req__14574408__003
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Deprecated fields shall be marked clearly, and backward compatibility shall
   be maintained for at least two release cycles.


Generic Requirements
--------------------

.. stkh_req:: REQ-GENERIC-001
   :id: stkh_req__14574454__001
   :rationale: Information
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Possible example interfaces that can be standardized across the systems.

.. stkh_req:: REQ-GENERIC-002
   :id: stkh_req__14574456__002
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Thermal management interfaces shall be standardized for coolant, oil, and
   component temperatures under ``Vehicle.System.Thermal.*``.

.. stkh_req:: REQ-GENERIC-003
   :id: stkh_req__14574458__003
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Compute resource interfaces shall be standardized:
   ``Vehicle.System.CPU.Load``, ``Memory.Usage``, ``GPU.Load``, and
   ``Storage.Free``.

.. stkh_req:: REQ-GENERIC-004
   :id: stkh_req__14574460__004
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Network link interfaces shall be standardized for CAN, LIN, and Ethernet
   status, error counters, and link speed.

.. stkh_req:: REQ-GENERIC-005
   :id: stkh_req__14574462__005
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Localization fix quality shall be standardized:
   ``Vehicle.Body.GNSS.FixType``, HDOP/VDOP, and ``Position.Confidence``.

.. stkh_req:: REQ-GENERIC-005
   :id: stkh_req__14574464__006
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   OTA/software state shall be standardized:
   ``Vehicle.System.OTA.UpdateState`` and PendingReboot flag.

.. stkh_req:: REQ-GENERIC-007
   :id: stkh_req__14574466__007
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Variant and feature-coding information shall be standardized:
   ``Vehicle.System.Config.VIN``, VariantId, Market, and FeatureFlags.


Interoperability Requirements
-----------------------------

.. stkh_req:: REQ-INTEROPERABILITY-001
   :id: stkh_req__14574386__001
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Interfaces shall be designed to remain platform-independent, supporting
   reuse across multiple ECU architectures.

.. stkh_req:: REQ-INTEROPERABILITY-002
   :id: stkh_req__14574388__002
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   The VSS extension shall provide mapping guidelines to AUTOSAR ports/signals
   for seamless integration.


Mode Management Requirements
----------------------------

.. stkh_req:: REQ-MODE_MANAGEMENT-001
   :id: stkh_req__14574346__001
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   The system shall provide mode management interfaces to convey system or
   functional mode information.

.. stkh_req:: REQ-MODE_MANAGEMENT-002
   :id: stkh_req__14574348__002
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   The naming convention for mode management interfaces shall follow the
   format: ``<FunctionName>_Mode_In`` and ``<FunctionName>_Mode_Out``.

.. stkh_req:: REQ-MODE_MANAGEMENT-003
   :id: stkh_req__14574350__003
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Mode management interfaces shall support adaptation of applications to
   diverse operational states.

.. stkh_req:: REQ-MODE_MANAGEMENT-004
   :id: stkh_req__14574352__004
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Each mode management interface shall define a Datatype for the mode signal.


Parameters Requirements
-----------------------

.. stkh_req:: REQ-PARAMETERS-001
   :id: stkh_req__14574422__001
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Config parameters shall include validation constraints and
   effectiveRangeContext tied to variant/market.

.. stkh_req:: REQ-PARAMETERS-002
   :id: stkh_req__14574424__002
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Service-only parameters shall be protected by role-based access and audit
   logging.

.. stkh_req:: REQ-PARAMETERS-003
   :id: stkh_req__14574426__003
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Calibration artifacts shall declare calibrationId, calibrationDate, and
   expiry and be referenced by providers.


Safety Reaction Requirements
----------------------------

.. stkh_req:: REQ-SAFETY_REACTION-001
   :id: stkh_req__14574338__001
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   The system shall provide safety reaction interfaces to communicate critical
   error or safety events from BSW failure management to the application layer.

.. stkh_req:: REQ-SAFETY_REACTION-002
   :id: stkh_req__14574340__002
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   The naming convention for safety reaction interfaces shall follow the
   format: ``<SafetyCondition>_SftyCondSts``.

.. stkh_req:: REQ-SAFETY_REACTION-003
   :id: stkh_req__14574342__003
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Each safety reaction interface shall define the following attributes:
   Datatype, Error list (causing the condition), and Description.

.. stkh_req:: REQ-SAFETY_REACTION-004
   :id: stkh_req__14574344__004
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Safety reaction interfaces shall ensure that applications can respond
   appropriately to safety conditions according to standardized protocols.


Scheduling Requirements
-----------------------

.. stkh_req:: REQ-SCHEDULING-001
   :id: stkh_req__14574354__001
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   The system shall provide scheduling interfaces as function calls instead of
   signal-based communication.

.. stkh_req:: REQ-SCHEDULING-002
   :id: stkh_req__14574356__002
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Scheduling interfaces shall be defined using the following mandatory
   functions: ``void <FunctionName>_Init()``, ``void <FunctionName>_Step()``,
   and ``void <FunctionName>_Terminate()``.

.. stkh_req:: REQ-SCHEDULING-003
   :id: stkh_req__14574358__003
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Each scheduling interface shall include the following attributes, where
   applicable: Run type, Cycle time, Description, Initial execution offset
   (optional), Priority (optional), Function scheduling mechanism, Debounce
   time (optional), Implemented ASIL level, Previous runnable (optional),
   Supervision type, Alive limits (optional), Deadline limits (optional),
   Logical check (optional), and Stack size (optional).

.. stkh_req:: REQ-SCHEDULING-004
   :id: stkh_req__14574360__004
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Scheduling interfaces shall support watchdog supervision in the BSW, where
   required by safety-critical functions.

.. stkh_req:: REQ-SCHEDULING-005
   :id: stkh_req__14574362__005
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Scheduling interfaces shall allow functions to declare memory requirements
   for integration purposes.

.. stkh_req:: REQ-SCHEDULING-006
   :id: stkh_req__14574364__006
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Components shall implement ``init()``, ``step()``, and ``terminate()`` with
   explicit timing contracts and declarations.

.. stkh_req:: REQ-SCHEDULING-007
   :id: stkh_req__14574366__007
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   ``Step()`` cycle time, priority, and debounce shall be configurable and
   supervised via alive/deadline monitors.

.. stkh_req:: REQ-SCHEDULING-008
   :id: stkh_req__14574368__008
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   ``Init()`` shall read persisted state and configuration; ``terminate()``
   shall flush buffers and persist configured nodes.


Security and Safety Requirements
--------------------------------

.. stkh_req:: REQ-SECURITY_SAFETY-001
   :id: stkh_req__14574390__001
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Each interface shall include a mandatory ASIL classification.

.. stkh_req:: REQ-SECURITY_SAFETY-002
   :id: stkh_req__14574392__002
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Safety-related interfaces shall define fault containment regions (FCRs) they
   belong to.

.. stkh_req:: REQ-SECURITY_SAFETY-003
   :id: stkh_req__14574394__003
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Each interface extension shall include cybersecurity attributes, such as
   authentication requirements or data integrity checks, if applicable.

.. stkh_req:: REQ-SECURITY_SAFETY-004
   :id: stkh_req__14574396__004
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Error and safety reaction interfaces shall define fallback behavior when the
   receiving application is unresponsive.


Timing and Performance Requirements
-----------------------------------

.. stkh_req:: REQ-TIMING_PERFORMANCE-001
   :id: stkh_req__14574398__001
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Each interface extension shall define latency requirements (maximum
   acceptable delay in ms).

.. stkh_req:: REQ-TIMING_PERFORMANCE-002
   :id: stkh_req__14574400__002
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Cyclic scheduling interfaces shall define worst-case execution time (WCET)
   per cycle.

.. stkh_req:: REQ-TIMING_PERFORMANCE-003
   :id: stkh_req__14574402__003
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Mode and error interfaces shall support time-stamping to ensure
   chronological traceability.


Tools and Validation Requirements
---------------------------------

.. stkh_req:: REQ-TOOLS_AND_VALIDATION-001
   :id: stkh_req__14574410__001
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   A validation tool shall check whether all VSS extensions conform to naming
   conventions and mandatory fields.

.. stkh_req:: REQ-TOOLS_AND_VALIDATION-002
   :id: stkh_req__14574412__002
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Interface definitions shall be exportable to commonly used formats
   (e.g. JSON, YAML, ARXML).

.. stkh_req:: REQ-TOOLS_AND_VALIDATION-003
   :id: stkh_req__14574414__003
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Simulation/test stubs shall be automatically generatable for each
   scheduling function and interface type.

.. stkh_req:: REQ-TOOLS_AND_VALIDATION-004
   :id: stkh_req__14574416__004
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   A static validator shall check naming conventions, mandatory fields, and
   mapping coverage of all VSS nodes.

.. stkh_req:: REQ-TOOLS_AND_VALIDATION-005
   :id: stkh_req__14574418__005
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Endianness and serialization format shall be declared per topic when binary
   transports are used.


Architecture Requirements
-------------------------

.. stkh_req:: REQ-ARCHITECTURE-0001
   :id: stkh_req__14574428__001
   :rationale: Information
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO
   
   Concept overview of VAPI and VSS coexistence and use cases explaining the
   architecture layering of the concept.

.. stkh_req:: REQ-ARCHITECTURE-002
   :id: stkh_req__14574430__002
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   The architecture shall separate concerns into Application, VAPI Abstraction
   Devices, Basic Services, and Platform layers with defined interfaces and no
   cross-layer shortcut dependencies.

.. stkh_req:: REQ-ARCHITECTURE-003
   :id: stkh_req__14574432__003
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   All northbound interfaces to the Application shall be expressed as VSS nodes
   or VSS-backed APIs to ensure a uniform data model.

.. stkh_req:: REQ-ARCHITECTURE-004
   :id: stkh_req__14574434__004
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Southbound provider integrations shall be implemented as plug-ins loaded by
   the Abstraction Device layer via a stable provider API.

.. stkh_req:: REQ-ARCHITECTURE-005
   :id: stkh_req__14574436__005
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Cross-cutting concerns (time, logging, security, configuration) shall be provided only through Basic Services and not duplicated in providers.

.. stkh_req:: REQ-ARCHITECTURE-006
   :id: stkh_req__14574438__006
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   The system shall support late binding: providers may be selected or replaced
   at deployment without changing application code.

.. stkh_req:: REQ-ARCHITECTURE-007
   :id: stkh_req__14574440__007
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   The architecture shall support multiple providers for the same VSS node with
   a deterministic arbitration policy.

.. stkh_req:: REQ-ARCHITECTURE-008
   :id: stkh_req__14574442__008
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   A declarative configuration file shall describe which providers populate
   which VSS nodes as per vehicle/ECU variant.

.. stkh_req:: REQ-ARCHITECTURE-009
   :id: stkh_req__14574444__009
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Abstraction Devices shall expose capability discovery including sensor type,
   supported sampling modes, ranges, units, and data quality attributes.

.. stkh_req:: REQ-ARCHITECTURE-010
   :id: stkh_req__14574446__010
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Abstraction Devices shall publish data via a common producer API with QoS
   hints (rate, latency budget, burst limit).

.. stkh_req:: REQ-ARCHITECTURE-011
   :id: stkh_req__14574448__011
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Providers shall report per-sample metadata: timestampSource, freshnessMs,
   qualityCode, and sourceId.

.. stkh_req:: REQ-ARCHITECTURE-012
   :id: stkh_req__14574450__012
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Providers shall surface health state and degradationMode and map them to
   error/safety reaction interfaces.

.. stkh_req:: REQ-ARCHITECTURE-013
   :id: stkh_req__14574452__013
   :rationale: Non Functional Requirement (NFR)
   :status: valid
   :reqtype: Non-Functional
   :safety: ASIL_B
   :security: NO

   Providers shall support configuration via typed parameters with validation
   against schema and range constraints.
