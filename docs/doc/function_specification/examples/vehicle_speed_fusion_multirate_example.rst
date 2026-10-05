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
   #   Saran Gundlapalli - vehicle_speed_fusion_multirate_example.rst updated according to metamodel v0.4.0
   # *******************************************************************************

VehicleSpeedFusion Multi-Rate Example
=====================================

This example shows a ``VehicleSpeedFusion.Step`` runnable that depends on a separately modeled preprocessing function, ``VehicleSpeedPreProcessor.Step``. The runnables are  introduced following the standard naming convention which defines ``Init``, ``Step`` and ``Terminate`` entry points per function.

The Data interfaces reuse standard wheel-speed VSS paths and ``Vehicle.Speed``. Runtime quality is explicitly indicated through ``runtimeCompanion.qualityCode`` as an enum-based qualifier interface, without duplicating the normal Data metadata.

.. literalinclude:: vehicle_speed_fusion_multirate.afs.yaml
   :language: yaml
   :linenos:
