
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
   #   Thomas Pfleiderer - Function specification added
   #   Saran Gundlapalli - wheel_speed_plausibility_check_example.rst updated according to metamodel v0.4.1
   # *******************************************************************************

WheelSpeedPlausibilityCheck Error and Supervision Example
===========================================================

This example demonstrates standard VSS wheel-speed inputs, runtime-quality handling, deadline/logical supervision and the **candidate/provisional** lightweight ``errorInterfaces`` concept aligned to metamodel v0.4.1.

Runtime quality is explicitly indicated for each Data interface through ``runtimeCompanion.qualityCode`` as an enum-based qualifier interface, without duplicating normal Data metadata. ``FunctionResult`` is shown as a runtime result contract rather than a hard-coded ``success`` value.

The preprocessing dependency is modeled as a separate runnable ``WheelSpeedPreProcessor.Step``. This keeps the current metamodel naming convention ``<FunctionName>.Init`` / ``.Step`` / ``.Terminate`` intact. The complete specification of ``WheelSpeedPreProcessor`` belongs to its own Function Specification File.

The Error interfaces carry application-level error/status information without redefining DSM/DTC or event-memory handling. Diagnostic metadata in this example is illustrative and must be replaced by requirement-based values where such metadata is required for downstream diagnostic configuration during integration.

.. literalinclude:: wheel_speed_plausibility_check.afs.yaml
   :language: yaml
   :linenos:
