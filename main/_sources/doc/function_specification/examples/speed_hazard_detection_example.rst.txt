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
   #   Thomas Pfleiderer - documentation
   #   Saran Gundlapalli - speed_hazard_detection_example.rst updated according to metamodel v0.4.0
   # *******************************************************************************

SpeedHazardDetection Example
============================

This example shows a single cyclic ``Step`` runnable. Canonical VSS semantics are reused where available, runtime quality is explicitly indicated through enum-based runtime companion interfaces, and ``FunctionResult`` is represented as the runtime execution-result contract.

Calibration parameters use VSS-style hierarchical paths under the relevant vehicle-domain branch. Where such paths are not part of the standard catalogue, they shall be treated as approved/illustrative extensions.


.. literalinclude:: speed_hazard_detection.afs.yaml
   :language: yaml
   :linenos:
