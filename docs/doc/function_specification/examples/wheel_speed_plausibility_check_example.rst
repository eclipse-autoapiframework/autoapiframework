
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
   #   Saran Gundlapalli - wheel_speed_plausibility_check_example.rst updated according to metamodel v0.4.0
   # *******************************************************************************

WheelSpeedPlausibilityCheck Error and Supervision Example
===========================================================

This example demonstrates standard VSS wheel-speed inputs, runtime-quality handling, deadline/logical supervision and the **candidate/provisional** ``errorInterfaces`` concept.

Runtime quality is explicitly indicated for each Data interface through ``runtimeCompanion.qualityCode`` as an enum-based qualifier interface, without duplicating normal Data metadata. ``FunctionResult`` is shown as a runtime result contract rather than a hard-coded ``success`` value.

The preprocessing dependency is modeled as a separate runnable ``WheelSpeedPreProcessor.Step``. This keeps the current metamodel naming convention ``<FunctionName>.Init`` / ``.Step`` / ``.Terminate`` intact. The complete specification of ``WheelSpeedPreProcessor`` belongs to its own Function Specification File.

The diagnostic values in this example are illustrative and must be replaced by requirement-based values before standardization.

.. literalinclude:: wheel_speed_plausibility_check.afs.yaml
   :language: yaml
   :linenos:
